#ifndef _HORIZON_JOYSTICKS_H
#define _HORIZON_JOYSTICKS_H

#include <array>
#include "horizon_link.h"

class HorizonJS {
public:
    HorizonJS() { channel.fill(0.5f); }
    virtual ~HorizonJS() = default;

    void operator>>(hlink_sbus_t*) const;
    virtual void operator<<(const uint8_t*) = 0;
    virtual void reset() {
        channel.fill(0.5f);
        trim = 0;
        lost_cntl = true;
        ap_activated = override_enabled = false;
    }
    virtual void debug();
protected:
    std::array<float, 13> channel; // rudder, elevator, throttle, aileron ...
            // the last three channels are used for trim
    uint8_t trim = 0; // bit[7:6] r, bit[5:4] e, bit[3:2] x, bit[1:0] a
    bool lost_cntl = true;
    // sticky states
    bool override_enabled = false;
    bool ap_activated = false;
};

class XboxOneJS: public HorizonJS {
public:
    typedef uint8_t raw_type[14];
    typedef uint16_t Button;

    void operator<<(const raw_type) override;
    void reset() override {
        curr_buttons = prev_buttons = 0;
        trigger_rt = trigger_lt = 0;
        stick_ry = stick_rx = stick_ly = stick_lx = 0;
        HorizonJS::reset();
    }
    void debug() override;
    bool is_pressed(Button b) const {
        return b & curr_buttons;
    }
    bool is_toggled(Button b) const {
        // currently not pressed but previously pressed
        return !(curr_buttons & b) && (prev_buttons & b);
    }

    static constexpr Button button_up     = 0x0001;
    static constexpr Button button_down   = 0x0002;
    static constexpr Button button_left   = 0x0004;
    static constexpr Button button_right  = 0x0008;
    static constexpr Button button_menu   = 0x0010;
    static constexpr Button button_window = 0x0020;
    static constexpr Button button_ls     = 0x0040;
    static constexpr Button button_rs     = 0x0080;
    static constexpr Button button_lb     = 0x0100;
    static constexpr Button button_rb     = 0x0200;
    static constexpr Button button_xbox   = 0x0400;
    // unknown button
    static constexpr Button button_a      = 0x1000;
    static constexpr Button button_b      = 0x2000;
    static constexpr Button button_x      = 0x4000;
    static constexpr Button button_y      = 0x8000;
protected:
    virtual void convert2base();
    // buttons
    Button prev_buttons = 0;
    Button curr_buttons = 0;
    // triggers
    uint8_t trigger_lt = 0;
    uint8_t trigger_rt = 0;
    // sticks
    int16_t stick_lx = 0;
    int16_t stick_ly = 0;
    int16_t stick_rx = 0;
    int16_t stick_ry = 0;
};

class XboxOneExtra: public XboxOneJS {
public:
    void operator>>(hlink_tlv_set_t*) const;
private:
    void convert2base() override;
};

#endif // _HORIZON_JOYSTICKS_H
