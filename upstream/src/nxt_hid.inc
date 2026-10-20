/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.2.0, 2026. Concept: Nguyễn Xuân Thông. */
static int find_audio_hid_device(char* event_path, size_t path_size, bool find_bluetooth) {
    FILE* f = fopen("/proc/bus/input/devices", "r");
    if (!f) return -1;

    char line[512];
    char name[256] = {0};
    char handlers[256] = {0};
    bool is_usb = false;
    bool is_bluetooth_avrcp = false;
    bool has_kbd = false;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "N: Name=", 8) == 0) {
            // New device, reset state
            strncpy(name, line + 8, sizeof(name) - 1);
            handlers[0] = 0;
            is_usb = false;
            is_bluetooth_avrcp = false;
            has_kbd = false;
            // Check if Bluetooth AVRCP device
            if (strstr(name, "AVRCP")) {
                is_bluetooth_avrcp = true;
            }
        }
        else if (strncmp(line, "P: Phys=", 8) == 0) {
            // Check if USB device
            if (strstr(line, "usb-")) {
                is_usb = true;
            }
        }
        else if (strncmp(line, "H: Handlers=", 12) == 0) {
            strncpy(handlers, line + 12, sizeof(handlers) - 1);
            // Check if it has kbd handler (keyboard/keypad events)
            if (strstr(handlers, "kbd")) {
                has_kbd = true;
            }
        }
        else if (line[0] == '\n') {
            // End of device entry
            bool match = false;
            if (find_bluetooth && is_bluetooth_avrcp && has_kbd) {
                match = true;
            } else if (!find_bluetooth && is_usb && has_kbd) {
                match = true;
            }

            if (match && handlers[0]) {
                // Find event device number
                char* event_ptr = strstr(handlers, "event");
                if (event_ptr) {
                    int event_num = -1;
                    sscanf(event_ptr, "event%d", &event_num);
                    if (event_num >= 0) {
                        snprintf(event_path, path_size, "/dev/input/event%d", event_num);
                        fclose(f);
                        return 0;
                    }
                }
            }
        }
    }

    fclose(f);
    return -1;
}

void Player_initUSBHID(void) {
    if (usb_hid_fd >= 0) {
        close(usb_hid_fd);
        usb_hid_fd = -1;
    }

    char event_path[64];

    // Try USB DAC HID first
    if (usbdac_audio_active) {
        if (find_audio_hid_device(event_path, sizeof(event_path), false) == 0) {
            usb_hid_fd = open(event_path, O_RDONLY | O_NONBLOCK);
            if (usb_hid_fd >= 0) {
                return;
            }
        }
    }

    // Try Bluetooth AVRCP
    if (bluetooth_audio_active) {
        if (find_audio_hid_device(event_path, sizeof(event_path), true) == 0) {
            usb_hid_fd = open(event_path, O_RDONLY | O_NONBLOCK);
            if (usb_hid_fd >= 0) {
                LOG_info("HID: Bluetooth remote at %s\n", event_path);
                hid_missing_logged = false;
                return;
            }
        }
        if (!hid_missing_logged) {
            LOG_info("HID: no Bluetooth remote yet, looking again each %d ms\n", HID_RETRY_MS);
            hid_missing_logged = true;
        }
    }
}

USBHIDEvent Player_pollUSBHID(void) {
    if (usb_hid_fd < 0) {
        uint32_t now = SDL_GetTicks();
        if ((bluetooth_audio_active || usbdac_audio_active) && (int32_t)(now - hid_retry_at) >= 0) {
            hid_retry_at = now + HID_RETRY_MS;
            Player_initUSBHID();
        }
        if (usb_hid_fd < 0) return USB_HID_EVENT_NONE;
    }

    struct input_event_raw ev;
    ssize_t got;
    while ((got = read(usb_hid_fd, &ev, sizeof(ev))) == sizeof(ev)) {
        // Only handle key press events (value=1), ignore release (0) and repeat (2)
        if (ev.type == EV_KEY && ev.value == 1) {
            LOG_debug("HID: key %d\n", ev.code);
            switch (ev.code) {
                case KEY_VOLUMEUP:
                    return USB_HID_EVENT_VOLUME_UP;
                case KEY_VOLUMEDOWN:
                    return USB_HID_EVENT_VOLUME_DOWN;
                case KEY_NEXTSONG:
                    return USB_HID_EVENT_NEXT_TRACK;
                case KEY_PLAYPAUSE:
                case KEY_PLAYCD:
                case KEY_PAUSECD:
                    return USB_HID_EVENT_PLAY_PAUSE;
                case KEY_PREVIOUSSONG:
                    return USB_HID_EVENT_PREV_TRACK;
            }
        }
    }

    // The remote went away (a disconnect): close it, and the retry finds the next one
    if (got < 0 && errno == ENODEV) {
        LOG_info("HID: remote went away\n");
        Player_quitUSBHID();
    }

    return USB_HID_EVENT_NONE;
}

void Player_quitUSBHID(void) {
    if (usb_hid_fd >= 0) {
        close(usb_hid_fd);
        usb_hid_fd = -1;
    }
}
