#include <cstdio> // for debugging
#include "horizon_joysticks.h"

#define MIN_THROTTLE    (1040)
#define CENTER_THROTTLE (1500)
#define MAX_THROTTLE    (1960)

////////////////////////////////////////
// class HorizonJS
void HorizonJS::operator>>(hlink_sbus_t *sbus) const {
    if (lost_cntl) {
        sbus->flags = 1 << 2;
    } else {
        for (decltype(channel.size()) i = 0; i < channel.size(); i++) {
            sbus->channel[i] = (uint16_t)
                (channel[i] * (MAX_THROTTLE - MIN_THROTTLE) + MIN_THROTTLE);
        }
        // TODO: trim
        sbus->channel[13] = (((trim >> 6) & 0x03) == 0x02) ? MIN_THROTTLE
            : (((trim >> 6) & 0x03) == 0x01) ? MAX_THROTTLE
            : CENTER_THROTTLE;
        sbus->channel[14] = (((trim >> 4) & 0x03) == 0x02) ? MIN_THROTTLE
            : (((trim >> 4) & 0x03) == 0x01) ? MAX_THROTTLE
            : CENTER_THROTTLE;
        sbus->channel[15] = ((trim & 0x03) == 0x02) ? MIN_THROTTLE
            : ((trim & 0x03) == 0x01) ? MAX_THROTTLE
            : CENTER_THROTTLE;
        sbus->flags = 0;
        if (override_enabled) { sbus->flags |= 1 << 1; }
        if (ap_activated) { sbus->flags |= 1 << 0; }
    }
}

void HorizonJS::debug() {
    for (decltype(channel.size()) i = 0; i < channel.size(); i++) {
        printf("%3.0f,", channel[i] * 100.f);
    }
    printf("%02hhx", trim);
    printf("%s,", lost_cntl ? "lost" : "....");
    printf("%s,", override_enabled ? "force" : ".....");
    printf("%s", ap_activated ? "AP" : "..");
    printf("    \r");
    fflush(stdout);
}

////////////////////////////////////////
// class XboxOneJS
void XboxOneJS::operator<<(const raw_type raw_data) {
    // buttons
    prev_buttons = curr_buttons;
    curr_buttons = raw_data[2] | (raw_data[3] << 8);
    // triggers
    trigger_lt = raw_data[4];
    trigger_rt = raw_data[5];
    // joy sticks
    stick_lx = raw_data[6] | (raw_data[7] << 8);
    stick_ly = raw_data[8] | (raw_data[9] << 8);
    stick_rx = raw_data[10] | (raw_data[11] << 8);
    stick_ry = raw_data[12] | (raw_data[13] << 8);
    // convert to base format
    convert2base();
}

void XboxOneJS::debug() {
    printf("%04hX,", curr_buttons);
    printf("%3hhu,", trigger_lt);
    printf("%3hhu,", trigger_rt);
    printf("%6hd,", stick_lx);
    printf("%6hd,", stick_ly);
    printf("%6hd,", stick_rx);
    printf("%6hd", stick_ry);
    printf("    \r");
    fflush(stdout);
}

void XboxOneJS::convert2base() {
    // basic controll
    channel[0] = (float(trigger_rt) - float(trigger_lt) + float(255)) / 510.f; // rudder
    channel[1] = (stick_ry + 32768) / 65535.f; // elevator
    channel[2] = (stick_ly + 32768) / 65535.f; // throttle
    channel[3] = (stick_rx + 32768) / 65535.f; // aileron
    // custom
    channel[4] = (stick_lx + 32768) / 65535.f;
    // trim
    // high bit represents negative trim, low bit opposite direction.
    // if both directions' trim key are pressed, it takes the same effect as no key is pressed.
    trim = 0;
    if (is_pressed(button_lb)) { trim |= 0x80; }
    if (is_pressed(button_rb)) { trim |= 0x40; }
    if (is_pressed(button_up)) { trim |= 0x20; }
    if (is_pressed(button_down)) { trim |= 0x10; }
    if (is_pressed(button_left)) { trim |= 0x02; }
    if (is_pressed(button_right)) { trim |= 0x01; }
    // take over controll
    lost_cntl = false;
    // TODO: override?
    // auto-pilot
    if (is_toggled(button_xbox)) {
        ap_activated = !ap_activated;
    }
}
