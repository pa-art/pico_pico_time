/**
 * Test program for add_repeating_timer_ms() function;
 * Feb.16, 2021  Pa@ART
 */

#include <stdio.h>
#include <string.h>
#include "lcd_1602_i2c_lib.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"

#define CONV_FACTOR (3.25f / (1 << 12)) // ADC data -> voltage
#define ADC_TEMP    4 // temperature sensor input 
#define LED_PIN 25  // LED pin#
#define I2C_SDA 4   // SDA pin#
#define I2C_SCL 5   // SCL pin#
#define INTERVAL    500 // [ms]

int count = 0;
uint16_t temp_dat;
bool state = true;
unsigned char chr[1];

// timer handler
bool disp_counter( struct repeating_timer *t ) {
    // read ADC(temperature) data
    temp_dat = adc_read();
    // flip LED state
    state = !state;
    // count up
    count += (state == true);
    chr[0] += (state == true);

    return true;
}

int main() {

    char mes_lcd[MAX_CHARS+1];
    float temp, voltage, temp_p, voltage_p;
    struct repeating_timer timer; 

    // This example will use I2C0 on GPIO4 (SDA) and GPIO5 (SCL)
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // Make the I2C pins available to picotool
    bi_decl( bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
    // init LCD
    lcd_init();
    // initialize LED pin
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    // init stdio
    stdio_init_all();
    // init ADC
    adc_init();
    // enable temperature sensor
    adc_set_temp_sensor_enabled(true);
    // select ADC input
    adc_select_input(ADC_TEMP);    // ADC selected
    // set timer
    add_repeating_timer_ms( INTERVAL, disp_counter, NULL, &timer );
    temp = voltage = temp_p = voltage_p = 0;

    while (1) {

        voltage = temp_dat * CONV_FACTOR;
        temp = 27 - (voltage - 0.706) / 0.001721;
        // make massage from voltage and temp
        sprintf(mes_lcd, "v=%2.3f, T=%2.1f", voltage, temp);
        // if temp or voltage varies
        if ((temp_p != temp) || (voltage_p != voltage)) {
            // clear row 0
            lcd_set_cursor(0, 0);
            lcd_string("                ");
        }
        // copy temp and voltage
        temp_p = temp; voltage_p = voltage;
        // display message
        lcd_set_cursor(0, 0);
        lcd_string(mes_lcd);
        // make message from count
        sprintf(mes_lcd, "count:%d", count);
        // display message
        lcd_set_cursor(1, 0);
        lcd_string(mes_lcd);
        // display countup character 
        lcd_set_cursor(1, 15);
        lcd_string(chr);
        // output state to LED
        gpio_put(LED_PIN, state);
    }

    return 0;
}
