// legacy
#include <cerrno>
#include <cstdio>
#include <cstring>
// common
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
// Unix
#include <fcntl.h>
#include <unistd.h>
// utilities
#include <hidapi/hidapi.h>
#include "horizon_link.h"
#include "horizon_joysticks.h"
#include "shared_buffer.h"

using namespace std;

#define CMD_BUF_LEN (81)
#define FILE_NAME_LEN (65)
#define HID_RAW_BUF_LEN (20)

// global variables
int g_pipe_fd;
mutex g_pipe_mtx;
condition_variable_any g_pipe_cv;

hid_device *g_hid_handle;
mutex g_hid_mtx;
condition_variable_any g_hid_cv;

atomic<int> g_pipe_status(0);
atomic<int> g_hid_status(0);

// buffers
char cli_buf[CMD_BUF_LEN];
char pipe_name[FILE_NAME_LEN];
XboxOneJS::raw_type xbox_raw_buf;
SharedBuffer sbus_shared_buf(sizeof(hlink_sbus_t));

void thr_hid(void) {
    hid_device *hid_handle;
    int bytes_read;
    XboxOneJS xbox_one;

    while (true) {
        g_hid_mtx.lock();
        g_hid_cv.wait(g_hid_mtx);
        hid_handle = g_hid_handle;
        g_hid_mtx.unlock();

        while (true) {
            try {
                bytes_read = hid_read(hid_handle, xbox_raw_buf, sizeof(XboxOneJS::raw_type));
                if (bytes_read < 0) {
                    throw "usb-hid plugged out";
                } else if (bytes_read != sizeof(XboxOneJS::raw_type)) {
                    throw "something wrong while reading usb-hid";
                }
                xbox_one << xbox_raw_buf;
                //xbox_one.HorizonJS::debug();
                sbus_shared_buf.push([xbox_one](uint8_t *sbus_buf)
                        { xbox_one >> (hlink_sbus_t*) sbus_buf; });
            } catch (const char *errstr) {
                printf("\n%s", errstr);
                fflush(stdout);
                xbox_one.reset();
                sbus_shared_buf.push([xbox_one](uint8_t *sbus_buf)
                        { xbox_one >> (hlink_sbus_t*) (sbus_buf); });
                hid_close(hid_handle);
                g_hid_status.store(-1, memory_order_relaxed);
                break;
            }
        }
    }
}

void thr_hlink(void) {
    int pipe_fd;

    while (true) {
        g_pipe_mtx.lock();
        g_pipe_cv.wait(g_pipe_mtx);
        pipe_fd = g_pipe_fd;
        g_pipe_mtx.unlock();

        while (true) {
            sbus_shared_buf.fetch([](uint8_t *buf) {
                    hlink_sbus_t *sbus = (hlink_sbus_t*) buf;
                    for (int i = 0; i < 16; i++) {
                        printf("%4hd,", (int16_t) sbus->channel[i]);
                    }
                    printf("%02hhX", sbus->flags);
                    printf("        \r");
                    fflush(stdout);
                });
            this_thread::sleep_for(chrono::milliseconds(20));
        }

        printf("\npipe has closed");
        fflush(stdout);
        close(pipe_fd);
        g_pipe_status.store(-1, memory_order_relaxed);
    }
}

void cli_usbhid(int hid_status) {
    uint16_t hid_vid, hid_pid;
    hid_device *hid_handle;

    // sanity check
    if (hid_status > 0) {
        printf("you can't reopen/change usb-hid at this point\n");
        return;
    }

    // parsing cmds...
    sscanf(cli_buf + 1, "%hx %hx", &hid_vid, &hid_pid);
    try {
        hid_handle = hid_open(hid_vid, hid_pid, NULL);
        if (!hid_handle) {
            snprintf(cli_buf, sizeof(cli_buf), "can't open usb-hid (vid=0x%04hx,pid=0x%04hx)",
                    hid_vid, hid_pid);
            throw cli_buf;
        }
        printf("usb-hid (vid=0x%04hx,pid=0x%04hx) opened, notify hid thread\n",
                hid_vid, hid_pid);
        g_hid_status.store(1, memory_order_relaxed);
        g_hid_mtx.lock();
        g_hid_handle = hid_handle;
        g_hid_cv.notify_all();
        g_hid_mtx.unlock();
    } catch (const char *errstr) {
        printf("%s\n", errstr);
        if (hid_handle) {
            hid_close(hid_handle);
        }
        g_hid_status.store(0, memory_order_relaxed);
    }

    return;
}

void cli_pipe(int pipe_status) {
    int pipe_fd;

    // sanity check
    if (pipe_status > 0) {
        printf("you can't reopen/change pipe at this point\n");
        return;
    }

    sscanf(cli_buf + 1, "%s", pipe_name);
    try {
        pipe_fd = open(pipe_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (pipe_fd < 0) {
            snprintf(cli_buf, sizeof(cli_buf), "can't open \"%s\": %s", pipe_name, strerror(errno));
            throw cli_buf;
        }
        if (fcntl(pipe_fd, F_SETFL, 0) != 0) {
            snprintf(cli_buf, sizeof(cli_buf), "can't reset to block mode: %s", strerror(errno));
            throw cli_buf;
        }
        printf("\"%s\" opened, fd = %d, notify hlink thread\n",
                pipe_name, pipe_fd);
        g_pipe_status.store(1, memory_order_relaxed);
        g_pipe_mtx.lock();
        g_pipe_fd = pipe_fd;
        g_pipe_cv.notify_all();
        g_pipe_mtx.unlock();
    } catch (const char *errstr) {
        printf("%s\n", errstr);
        if (pipe_fd >= 0) {
            close(pipe_fd);
        }
        g_pipe_status.store(0, memory_order_relaxed);
    }

    return;
}

char make_prompt(int status) {
    return (status == 0) ? '*'
        : (status > 0) ? '>'
        : '!';
}

int main() {
    int pipe_status, hid_status;

    if (hid_init() < 0) {
        fputs("can't init hidapi\n", stderr);
        exit(1);
    }

    thread thid(thr_hid);
    thread thlink(thr_hlink);

    while (true) {
        // print CLI prompt
        hid_status = g_hid_status.load(memory_order_relaxed);
        pipe_status = g_pipe_status.load(memory_order_relaxed);
        printf("%c%c >", make_prompt(hid_status), make_prompt(pipe_status));

        // read command
        if (!fgets(cli_buf, sizeof(cli_buf), stdin)) {
            perror("fget error");
            exit(1);
        }

        // process command
        switch (cli_buf[0]) {
        case 'u':
            cli_usbhid(hid_status);
            break;
        case 'p':
            cli_pipe(pipe_status);
            break;
        case 'q':
            printf("bye-bye\n");
            exit(0);
        default:
            if (cli_buf[0] != '\n') {
                printf("unknown command\n");
            }
            break;
        }
    }

    // should not get here
    fputs("unreachable code!\n", stderr);
    exit(1);
}
