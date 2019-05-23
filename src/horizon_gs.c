#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include "apue.h"
#include "horizon_link.h"

#define RX_BUF_LEN (1518)
#define SIG_COMM (SIGTSTP)

uint8_t rx_buf[RX_BUF_LEN];
pthread_t isr_tid, comm_tid;

void* thr_comm(void *arg) {
    sigset_t mask = *((sigset_t *) arg);
    int signo;
    int nr_tlv;
    hlink_sbus_t sbus;
    hlink_fport_ctrl_t fport_ctrl;
    hlink_tlv_set_t tlv_set;

    fport_ctrl.sbus = &sbus;

    memset(&tlv_set, 0, sizeof(tlv_set));
    tlv_set.fport_ctrl = &fport_ctrl;
    while (true) {
        sigwait(&mask, &signo);
        printf("\nframe received\n");
        nr_tlv = hlink_process_frame(&tlv_set);
        printf("%d TLV parsed\n", nr_tlv);
        if (tlv_set.fport_ctrl != NULL) {
            for (int i = 0; i < 16; i++) {
                printf("sbus.channel[%d] = %hu\n", i, sbus.channel[i]);
            }
            printf("sbus.flags = 0x%02X\n", sbus.flags);
            printf("rssi = 0x%02X\n", fport_ctrl.rssi);
        }
        tlv_set.fport_ctrl = &fport_ctrl;
    }

    // should not get here
    return NULL;
}

void* thr_isr(void *arg) {
    int rx_fd = *((int *) arg);
    ssize_t rx_bytes = -1;

    while ((rx_bytes = read(rx_fd, rx_buf, sizeof(rx_buf))) > 0) {
        for (int i = 0; i < rx_bytes; i++) {
            if (hlink_receive_data(rx_buf[i])) {
                pthread_kill(comm_tid, SIG_COMM);
                sleep(1);
            }
        }
    }

    // device colsed or read error
    return NULL;
}

int main(int argc, char *argv[]) {
    int rx_fd = -1;
    sigset_t mask;
    int err;

    if (argc != 2) {
        apue_err_quit("usage: ./horizon_gs.out dev");
    }

    if ((rx_fd = open(argv[1], O_RDONLY | O_NOCTTY | O_NONBLOCK)) < 0) {
        apue_err_sys("failed to open %s", argv[1]);
    }
    printf("file %s opened, fd = %d\n", argv[1], rx_fd);

    if (fcntl(rx_fd, F_SETFL, 0) < 0) {
        apue_err_sys("failed to reset file status flags");
    }
    printf("reset to block mode\n");

    sigemptyset(&mask);
    sigaddset(&mask, SIG_COMM);
    if ((err = sigprocmask(SIG_BLOCK, &mask, NULL)) != 0) {
        apue_err_exit(err, "SIG_BLOCK error");
    }

    err = pthread_create(&comm_tid, NULL, thr_comm, &mask);
    if (err != 0) {
        apue_err_exit(err, "failed to create comm thread");
    }
    err = pthread_create(&isr_tid, NULL, thr_isr, &rx_fd);
    if (err != 0) {
        apue_err_exit(err, "failed to create isr thread");
    }

    err = pthread_join(isr_tid, NULL);
    if (err != 0) {
        apue_err_exit(err, "can't join with isr thread");
    }

    printf("bye-bye\n");

    return 0;
}
