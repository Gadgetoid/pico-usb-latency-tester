#include "pico/stdlib.h"
#include "pico/rand.h"
#include <cstdio>
#include <iterator>
#include <sstream>
#include "usb.hpp"
#include "scancodes.h"

enum state {
    IDLE,
    WAITING
};

bool triggered = false;
absolute_time_t time_triggered;

void gpio_setup_output(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, true);
    gpio_put(pin, 0);
}

void gpio_setup_input_pu(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, false);
    gpio_set_pulls(pin, true, false);
}

void keyboard_callback(uint8_t *keys, uint8_t modifiers) {
    if(keys[0] == 41) {
        absolute_time_t time_detected = get_absolute_time();
        int64_t us = absolute_time_diff_us(time_triggered, time_detected) - 2440;
        printf("Got ESC in %lld microseconds\n", us);
        gpio_put(2, 0);
        triggered = false;
    }
}

void gamepad_callback(int8_t x, int8_t y, uint16_t buttons) {
    //if(x == -127) {
    if (buttons & 0b0000000000000010) {
        absolute_time_t time_detected = get_absolute_time();
        int64_t us = absolute_time_diff_us(time_triggered, time_detected) - 2440;
        //printf("Got button set in %lld microseconds\n", us);
        printf("%lld\n", us);
        gpio_put(2, 0);
        time_triggered = get_absolute_time();
    //} else if (x == 0) {
    } else {
        absolute_time_t time_detected = get_absolute_time();
        int64_t us = absolute_time_diff_us(time_triggered, time_detected);
        //printf("Got button reset in %lld microseconds\n", us);
        triggered = false;
    }
}

void gpio_callback(uint gpio, uint32_t events) {
    if (!triggered) return;
    absolute_time_t time_detected = get_absolute_time();
    int64_t us = absolute_time_diff_us(time_triggered, time_detected);
    printf("Got GPIO in %lld microseconds\n", us);
    gpio_put(3, 0);
    triggered = false;
}

int main() {
    stdio_init_all();
    //initialise_rand();
    init_usb();

    gpio_setup_output(2);
    gpio_setup_output(3);
    gpio_setup_output(4);
    gpio_setup_output(5);

    //gpio_setup_input_pu(14);
    //gpio_set_irq_enabled_with_callback(14, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    printf("Running...\n");

    /*while (true) {
        if(!triggered) {
            uint32_t r = 1000 + random() & 0xfff;
            sleep_ms(r);
            gpio_put(3, 1);
            time_triggered = get_absolute_time();
            triggered = true;
        }
    }*/

    uint8_t value = 0;
    while(true) {
        if(!triggered) {
            //uint32_t r = 500 + (random() & 0x3e8);
            //uint32_t r = 1000 + (get_rand_32() & 0x1ff);
            uint32_t r = 500;
            sleep_ms(r);
            gpio_put(2, 1);
            time_triggered = get_absolute_time();
            triggered = true;
        }
        update_usb();
    }
}