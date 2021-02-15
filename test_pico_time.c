/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "lcd_1602_i2c_lib.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"

#define CONV_FACTOR (3.25f / (1 << 12))

int count = 0;
uint16_t temp_dat;
const uint LED_PIN = 25;
bool state = true;
unsigned char chr[1];

bool disp_counter( struct repeating_timer *t ) {

    temp_dat = adc_read();
    gpio_put(LED_PIN, state);
    state = !state;
    count += (state == true);
    chr[0] += (state == true);

    return true;
}

int main() {

    int x, y, c;
    int cnt = 0;
    char mes_lcd[MAX_CHARS+1];
    float temp, voltage;
    struct repeating_timer timer; 

    // This example will use I2C0 on GPIO4 (SDA) and GPIO5 (SCL)
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);
    // Make the I2C pins available to picotool
    bi_decl( bi_2pins_with_func(4, 5, GPIO_FUNC_I2C));
    // init LCD
    lcd_init();
    // init stdio
    stdio_init_all();
    x = y = 0;
    // init ADC
    adc_init();
    // enable temperature sensor
    adc_set_temp_sensor_enabled(true);
    // select ADC input
    adc_select_input(4);    // ADC selected

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // set 
    add_repeating_timer_ms( 500, disp_counter, NULL, &timer );

    while (1) {

        voltage = temp_dat * CONV_FACTOR;
        temp = 27 - (voltage - 0.706) / 0.001721;
        //printf("voltage:%2.2f, tempraure:%2.2f\n", voltage, temp);
        sprintf(mes_lcd, "v=%2.3f, T=%2.1f", voltage, temp);
        lcd_set_cursor(0, 0);
        lcd_string(mes_lcd);
        //sleep_ms(1000);
        sprintf(mes_lcd, "count:%d", count);
        lcd_set_cursor(1, 0);
        lcd_string(mes_lcd);
        lcd_set_cursor(1, 15);
        lcd_string(chr);
    }

    return 0;
}
