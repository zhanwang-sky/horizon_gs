#ifndef _HORIZON_JOYSTICKS_H
#define _HORIZON_JOYSTICKS_H

#include <array>
#include "horizon_link.h"

class HorizonJS {
public:
    HorizonJS() = default;
    virtual ~HorizonJS() = default;

    void operator>>(hlink_sbus_t*) const;
    virtual void operator<<(const uint8_t*) = 0;
    virtual void reset() {
        lost_cntl = true;
        ap_activated = override_enabled = false;
    }
    virtual void debug();
protected:
    std::array<float, 13> channel; // rudder, elevator, throttle, aileron ...
            // the last three channels are used for trim
    uint8_t trim; // bit[7:6] r, bit[5:4] e, bit[3:2] x, bit[1:0] a
    bool lost_cntl = true;
    // sticky states
    bool override_enabled = false;
    bool ap_activated = false;
};

class XboxOneJS: public HorizonJS {
public:
    typedef uint8_t raw_type[14];

    XboxOneJS() { channel.fill(0.5f); }

    void operator<<(const raw_type) override;
    void reset() override {
        curr_buttons1 = 0;
        curr_buttons2 = 0;
        HorizonJS::reset();
    }
    void debug() override;
private:
    void convert2base();
    // buttons
    uint8_t prev_buttons1;
    uint8_t curr_buttons1 = 0;
    uint8_t prev_buttons2;
    uint8_t curr_buttons2 = 0;
    // triggers
    uint8_t lt;
    uint8_t rt;
    // sticks
    int16_t left_x;
    int16_t left_y;
    int16_t right_x;
    int16_t right_y;
};

#endif // _HORIZON_JOYSTICKS_H
