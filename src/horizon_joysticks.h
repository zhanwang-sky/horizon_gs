#ifndef _HORIZON_JOYSTICKS_H
#define _HORIZON_JOYSTICKS_H

#include <cstdio> // for debugging
#include <array>
#include "horizon_link.h"

class HorizonJS {
public:
    HorizonJS():
        channel({ 0 }),
        lost_cntl(true),
        override_enabled(false),
        ap_activated(false) { }
    virtual ~HorizonJS() = default;

    virtual void reset() {
        lost_cntl = true;
        ap_activated = override_enabled = false;
    }
    virtual void operator<<(const uint8_t *raw_buf) = 0;
    void operator>>(hlink_sbus_t *sbus) const {
        if (lost_cntl) {
            sbus->flags = 1 << 2;
        } else {
            // TODO: channels
            // TODO: trim
            sbus->flags = 0;
            if (override_enabled) { sbus->flags |= 1 << 1; }
            if (ap_activated) { sbus->flags |= 1 << 0; }
        }
    }
    virtual void debug() {
        for (decltype(channel.size()) i = 0; i < channel.size(); i++) {
            printf("%3.0f,", channel[i] * 100.f);
        }
        printf("%s,", lost_cntl ? "lost" : "    ");
        printf("%s,", override_enabled ? "force" : "     ");
        printf("%s", ap_activated ? "AP" : "  ");
        printf("    \r");
        fflush(stdout);
    }
protected:
    std::array<float, 13> channel; // rudder, elevator, thrust, aileron ...
        // the last three channels are used for trim
    uint8_t trim; // bit[7:6] r, bit[5:4] e, bit[3:2] x, bit[1:0] a
    bool lost_cntl;
    // sticky states
    bool override_enabled;
    bool ap_activated;
};

class XboxOneJS: public HorizonJS {
public:
    typedef uint8_t raw_type[14];
    void reset() override {
        curr_buttons1 = 0;
        curr_buttons2 = 0;
        HorizonJS::reset();
    }
    void operator<<(const raw_type raw_data) override {
        // buttons
        prev_buttons1 = curr_buttons1;
        curr_buttons1 = raw_data[2];
        prev_buttons2 = curr_buttons2;
        curr_buttons2 = raw_data[3];
        // triggers
        lt = raw_data[4];
        rt = raw_data[5];
        // joy sticks
        left_x = raw_data[6] | (raw_data[7] << 8);
        left_y = raw_data[8] | (raw_data[9] << 8);
        right_x = raw_data[10] | (raw_data[11] << 8);
        right_y = raw_data[12] | (raw_data[13] << 8);
        // convert to base format
        convert2base();
    }
    virtual void debug() override {
        printf("%6hd,", left_x);
        printf("%6hd,", left_y);
        printf("%6hd,", right_x);
        printf("%6hd,", right_y);
        printf("%3hhu,", lt);
        printf("%3hhu,", rt);
        printf("%02X,", curr_buttons1);
        printf("%02X", curr_buttons2);
        printf("    \r");
        fflush(stdout);
    }
private:
    void convert2base() {
        // basic controll
        channel[0] = (left_x + 32768) / 65536.f; // rudder
        channel[1] = (right_y + 32768) / 65536.f; // elevator
        channel[2] = (left_y + 32768) / 65536.f; // thrust
        channel[3] = (right_x + 32768) / 65536.f; // aileron
        // custom
        channel[4] = lt / 256.f;
        channel[5] = rt / 256.f;
        // trim
        trim = 0;
        // high bit represents negative trim, low bit opposite direction.
        // if both directions' trim key are pressed, it takes the same effect as no key is pressed.
        trim |= (curr_buttons2 & 0x01) << 7; // LB controlls left rudder trim
        trim |= (curr_buttons2 & 0x02) << 5; // RB controlls right rudder trim
        trim |= (curr_buttons1 & 0x01) << 5; // direction UP (elevator push down)
        trim |= (curr_buttons1 & 0x02) << 3; // direction DOWN (elevator pull up)
        // don't need to trim thrust
        trim |= (curr_buttons1 & 0x04) >> 1; // direction LEFT (aileron left roll)
        trim |= (curr_buttons1 & 0x08) >> 3; // direction RIGHT (aileron right roll)
        // take over controll
        lost_cntl = false;
        // TODO: override?
        // auto-pilot
        if (!(curr_buttons2 & 0x04) && (prev_buttons2 & 0x04)) {
            // currently not pressed and previously pressed
            ap_activated = !ap_activated; // toggle
        }
    }
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
