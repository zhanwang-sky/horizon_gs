#ifndef _HORIZON_JOYSTICK_H
#define _HORIZON_JOYSTICK_H

#include <array>
#include "horizon_link.h"

class HorizonJoystick {
public:
    HorizonJoystick():
        lost_cntl(true),
        override_pressed(false),
        ap_pressed(false),
        ap_activated(false) { }
    virtual ~HorizonJoystick() = default;

    void convert2sbus(hlink_sbus_t *sbus) const {
        for (size_t i = 0; i < reta.size(); i++) {
            sbus->channel[i] = reta[i];
        }
        // TODO: trim
        sbus->flags = 0;
        if (ap_activated) { sbus->flags |= 1 << 0; }
        if (override_pressed) { sbus->flags |= 1 << 1; }
        if (lost_cntl) { sbus->flags |= 1 << 2; }
    }
protected:
    virtual void store(uint8_t *raw_buf, int len);
    std::array<int16_t, 4> reta; // dudder, elevator, thrust, aileron
    uint8_t trim; // bit[0]: r, bit[1]: e, bit[2]: t, bit[3] a
    bool lost_cntl;
    bool override_pressed;
    bool ap_pressed;
    bool ap_activated;
};

class XboxOneJoystick: public HorizonJoystick {
public:
    XboxOneJoystick():
        HorizonJoystick() { }
    void store(uint8_t *raw_buf, int len) override {
    }
private:
    bool lb = false; // ap
    bool rb = false; // override
};

#endif // _HORIZON_JOYSTICK_H
