#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "apue.h"
#include "horizon_link.h"

#define RX_BUF_LEN (1)

using namespace std;

uint8_t rx_buf[RX_BUF_LEN];
pthread_t isr_tid, comm_tid;

void* thr_comm(void *arg) {
    return NULL;
}

void* thr_isr(void *arg) {
    int rx_fd = *((int *) arg);
    ssize_t rx_bytes = -1;

    while ((rx_bytes = read(rx_fd, rx_buf, sizeof(rx_buf))) > 0) {
        cout << rx_bytes << " bytes received" << endl;
        for (int i = 0; i < rx_bytes; i++) {
            if (hlink_receive_data(rx_buf[i])) {
                // do something...
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int rx_fd = -1;
    int err;

    if (argc != 2) {
        apue_err_quit("usage: ./horizon_gs.out dev");
    }

    if ((rx_fd = open(argv[1], O_RDONLY | O_NOCTTY | O_NONBLOCK)) < 0) {
        apue_err_sys("failed to open %s", argv[1]);
    }
    cout << argv[1] << " opened, fd = " << rx_fd << endl;

    if (fcntl(rx_fd, F_SETFL, 0) < 0) {
        apue_err_sys("failed to reset file status flags");
    }
    cout << "reset to block mode" << endl;

    err = pthread_create(&isr_tid, NULL, thr_isr, &rx_fd);
    if (err != 0) {
        apue_err_exit(err, "failed to create isr thread");
    }

    err = pthread_join(isr_tid, NULL);
    if (err != 0) {
        apue_err_exit(err, "can't join with isr thread");
    }

    cout << "bye-bye" << endl;

    return 0;
}
