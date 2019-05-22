#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include "apue.h"
#include "horizon_link.h"

#define RX_BUF_LEN (3)

using namespace std;

uint8_t rx_buf[RX_BUF_LEN];

int main(int argc, char *argv[]) {
    int rx_fd = -1;
    ssize_t rx_bytes = -1;

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

    while ((rx_bytes = read(rx_fd, rx_buf, sizeof(rx_buf))) > 0) {
        cout << rx_bytes << " bytes received" << endl;
        for (int i = 0; i < rx_bytes; i++) {
            if (hlink_receive_data(rx_buf[i])) {
                hlink_process_frame();
            }
        }
    }

    cout << "bye-bye" << endl;

    return 0;
}
