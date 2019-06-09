#ifndef _SHARED_BUFFER_H
#define _SHARED_BUFFER_H

// lock-free shared buffer

#include <atomic>
#include <functional>

class SharedBuffer {
public:
    SharedBuffer(size_t s):
        sz(s), buf(new uint8_t*[3]()),
        w_buf_id(0), r_buf_id(0) {
        buf[0] = new uint8_t[sz * 3];
        buf[1] = buf[0] + sz;
        buf[2] = buf[1] + sz;
    }
    ~SharedBuffer() {
        delete []buf[0];
        delete []buf;
    }
    SharedBuffer(const SharedBuffer&) = delete;
    SharedBuffer(SharedBuffer&&) = delete;
    SharedBuffer& operator=(const SharedBuffer&) = delete;
    SharedBuffer& operator=(SharedBuffer&&) = delete;

    void push(const std::function<void(uint8_t*, size_t)> &f) {
        size_t new_w_id = cal_new_w_id(w_buf_id.load(std::memory_order_relaxed),
                r_buf_id.load(std::memory_order_relaxed));
        f(buf[new_w_id], sz);
        w_buf_id.store(new_w_id, std::memory_order_release);
    }
    void fetch(const std::function<void(uint8_t*, size_t)> &f) {
        f(buf[r_buf_id.load(std::memory_order_relaxed)], sz);
        r_buf_id.store(w_buf_id.load(std::memory_order_relaxed), std::memory_order_release);
    }
private:
    size_t cal_new_w_id(size_t w_id, size_t r_id) {
        return (w_id == r_id) ? (w_id + 1) % 3 : 3 - (w_id + r_id);
    }
    const size_t sz;
    uint8_t **const buf;
    std::atomic<size_t> w_buf_id;
    std::atomic<size_t> r_buf_id;
};

#endif // _SHARED_BUFFER_H
