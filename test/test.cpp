#include <iostream>
#include <fcntl.h>
#include <unistd.h>

#define FRAME_BUF_LEN (10)

using namespace std;

uint8_t frame_buf[FRAME_BUF_LEN];

int main(int argc, char *argv[]) {
    string frame_file;
    int pipe_fd, frame_fd;
    ssize_t frame_len;

    if (argc != 2) {
        cerr << "usage: ./test.out pipe" << endl;
        exit(1);
    }

    if ((pipe_fd = open(argv[1], O_WRONLY | O_NOCTTY)) < 0) {
        cerr << "can not open " << argv[1] << ", rc = " << pipe_fd << endl;
        exit(1);
    }
    cout << argv[1] << " opened, fd = " << pipe_fd << endl;

    while (true) {
        cout << "> ";
        cin >> frame_file;
        if ((frame_fd = open(frame_file.c_str(), O_RDONLY)) < 0) {
            cerr << "failed to open " << frame_file << endl;
            continue;
        }
        while ((frame_len = read(frame_fd, frame_buf, sizeof(frame_buf))) > 0) {
            write(pipe_fd, frame_buf, frame_len);
        }
        close(frame_fd);
    }

    close(pipe_fd);

    return 0;
}
