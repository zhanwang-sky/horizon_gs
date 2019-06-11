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

void XboxOneJS::debug() {
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

void XboxOneJS::convert2base() {
    // basic controll
    channel[0] = (float(rt) - float(lt) + float(255)) / 510.f; // rudder
    channel[1] = (right_y + 32768) / 65535.f; // elevator
    channel[2] = (left_y + 32768) / 65535.f; // throttle
    channel[3] = (right_x + 32768) / 65535.f; // aileron
    // custom
    channel[4] = (left_x + 32768) / 65535.f;
    // trim
    // high bit represents negative trim, low bit opposite direction.
    // if both directions' trim key are pressed, it takes the same effect as no key is pressed.
    trim = 0;
    trim |= (curr_buttons2 & 0x01) << 7; // LB controlls left rudder trim
    trim |= (curr_buttons2 & 0x02) << 5; // RB controlls right rudder trim
    trim |= (curr_buttons1 & 0x01) << 5; // direction UP (elevator push down)
    trim |= (curr_buttons1 & 0x02) << 3; // direction DOWN (elevator pull up)
    // don't need to trim throttle
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
