#include "usb.hpp"
#include "bsp/board.h"
#include "tusb.h"

// hid
static int hid_report_id = -1;
bool hid_keyboard_detected = false;
bool hid_mouse_detected = false;
bool joystick_detected[2] = {false, false};
uint8_t hid_keys[6]{};

const char *nibble_to_bitstring[16] = {
  "0000", "0001", "0010", "0011",
  "0100", "0101", "0110", "0111",
  "1000", "1001", "1010", "1011",
  "1100", "1101", "1110", "1111",
};

extern void mouse_callback(int8_t x, int8_t y, uint8_t buttons, int8_t wheel);
extern void keyboard_callback(uint8_t *keys, uint8_t modifiers);
extern void gamepad_callback(int8_t x, int8_t y, uint16_t buttons);

static inline bool is_picade_joystick(uint8_t dev_addr) {
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  return vid == 0xcafe && pid == 0x400d;
}

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
  uint16_t vid = 0, pid = 0;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  printf("Mount %i %i, %04x:%04x\n", dev_addr, instance, vid, pid);

  auto protocol = tuh_hid_interface_protocol(dev_addr, instance);

  if(protocol == HID_ITF_PROTOCOL_KEYBOARD && !hid_keyboard_detected) {
    printf("Got HID keyboard...%d\n", instance);
  } else if (protocol == HID_ITF_PROTOCOL_MOUSE && !hid_mouse_detected) {
    printf("Got HID mouse...%d \n", instance);
  } else {
    printf("Got protocol %i \n", protocol);
    if (is_picade_joystick(dev_addr)) {
      printf("Got Picade Max Joystick... %d\n", instance);
      joystick_detected[instance] = true;
    }
  }
  
  hid_keyboard_detected = protocol == HID_ITF_PROTOCOL_KEYBOARD;
  hid_mouse_detected = protocol == HID_ITF_PROTOCOL_MOUSE;

  tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  hid_keyboard_detected = false;
  hid_mouse_detected = false;
  joystick_detected[0] = false;
  joystick_detected[1] = false;
}

typedef struct TU_ATTR_PACKED
{
  int8_t  x;         ///< Delta x  movement of left analog-stick
  int8_t  y;         ///< Delta y  movement of left analog-stick
  uint16_t buttons;  ///< Buttons mask for currently pressed buttons
} hid_picade_gamepad_report_t;

bool operator==(const hid_picade_gamepad_report_t& lhs, const hid_picade_gamepad_report_t& rhs)
{
    return lhs.x == rhs.x
        && lhs.y == rhs.y
        && lhs.buttons == rhs.buttons;
}

bool operator!=(const hid_picade_gamepad_report_t& lhs, const hid_picade_gamepad_report_t& rhs)
{
    return !(lhs == rhs);
}

hid_picade_gamepad_report_t last_input[2];

void int_to_bin(int16_t in, char *binstr) {
  for(auto i = 0u; i < 16; i++) {
    binstr[i] = char(48 + ((in >> i) & 0b1));
  }

  binstr[16] = '\0';
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {

  auto report_data = hid_report_id == -1 ? report : report + 1;

  //printf("Report %i %i, %i\n", dev_addr, instance, hid_report_id);

  auto protocol = tuh_hid_interface_protocol(dev_addr, instance);


  if(protocol == HID_ITF_PROTOCOL_KEYBOARD) {
    hid_keyboard_detected = true;
    auto keyboard_report = (hid_keyboard_report_t const*) report;
    memcpy(hid_keys, keyboard_report->keycode, 6);
    /*printf("Keyboard %i %i %i %i %i %i 0b%s%s\n",
            hid_keys[0],
            hid_keys[1],
            hid_keys[2],
            hid_keys[3],
            hid_keys[4],
            hid_keys[5],
            nibble_to_bitstring[keyboard_report->modifier >> 4],
            nibble_to_bitstring[keyboard_report->modifier & 0x0F]);*/
    keyboard_callback(hid_keys, keyboard_report->modifier);
  } else if(protocol == HID_ITF_PROTOCOL_MOUSE) {
    hid_mouse_detected = true;
    auto mouse_report = (hid_mouse_report_t const*) report;
    printf("Mouse %i %i %i 0b%s%s\n", mouse_report->x, mouse_report->y, mouse_report->wheel,
            nibble_to_bitstring[mouse_report->buttons >> 4],
            nibble_to_bitstring[mouse_report->buttons & 0x0F]);
    //mouse_callback(mouse_report->x, mouse_report->y, mouse_report->buttons, mouse_report->wheel);
  } else if (protocol == HID_ITF_PROTOCOL_NONE && (instance == 0 || instance == 1)) {
    auto gamepad_report = (hid_picade_gamepad_report_t const*) report;
    char buttons[17];

    if(last_input[instance] != *gamepad_report) {
      int_to_bin(gamepad_report->buttons, buttons);
      //printf("Picade %i %i %s\n", gamepad_report->x, gamepad_report->y, buttons);

      last_input[instance] = *gamepad_report;

      gamepad_callback(gamepad_report->x, gamepad_report->y, gamepad_report->buttons);
    }
  }

  tuh_hid_receive_report(dev_addr, instance);
}

void init_usb() {
  board_init();
  tusb_init();
  tuh_init(BOARD_TUH_RHPORT);
}

void update_usb() {
  tuh_task();
}

void usb_debug(const char *message) {

}