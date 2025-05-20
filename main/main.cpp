/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <string>
#include <sstream>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"

#include "led_strip.h"

#include <M5Unified.h>

#define LED_STRIP_BLINK_GPIO  21
#define LED_STRIP_LED_NUMBERS 3
#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)

static const char *TAG = "LED_STRIP";

led_strip_handle_t led_strip;

static uint8_t rgb_state[3][3] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};

void update_display() {
    M5.Display.clear();
    M5.Display.setCursor(10, 10);
    M5.Display.print("Unit RGB TEST");
    M5.Display.setCursor(10, 50);
    M5.Display.print("L: ");
    M5.Display.print(rgb_state[0][0]);
    M5.Display.print(", ");
    M5.Display.print(rgb_state[0][1]);
    M5.Display.print(", ");
    M5.Display.print(rgb_state[0][2]);

    M5.Display.setCursor(10, 100);
    M5.Display.print("M: ");
    M5.Display.print(rgb_state[1][0]);
    M5.Display.print(", ");
    M5.Display.print(rgb_state[1][1]);
    M5.Display.print(", ");
    M5.Display.print(rgb_state[1][2]);

    M5.Display.setCursor(10, 150);
    M5.Display.print("R: ");
    M5.Display.print(rgb_state[2][0]);
    M5.Display.print(", ");
    M5.Display.print(rgb_state[2][1]);
    M5.Display.print(", ");
    M5.Display.print(rgb_state[2][2]);
}

void move_rgb(int index) 
{
    if (rgb_state[index][0] == 255) 
    {
        rgb_state[index][0] = 0;
        rgb_state[index][1] = 255;
        rgb_state[index][2] = 0;
    } 
    else 
    if (rgb_state[index][1] == 255)
    {
        rgb_state[index][0] = 0;
        rgb_state[index][1] = 0;
        rgb_state[index][2] = 255;
    }
    else 
    if (rgb_state[index][2] == 255)
    {
        rgb_state[index][0] = 255;
        rgb_state[index][1] = 0;
        rgb_state[index][2] = 0;
    }
}

extern "C" void app_main(void)
{
    M5.begin();
    M5.Ex_I2C.release();
    M5.Speaker.tone(2000, 100, 0, false);
    M5.Speaker.tone(1000, 100, 0, false);
    M5.Display.fillScreen(0x222222);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);
    update_display();

    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_BLINK_GPIO,   // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_LED_NUMBERS,        // The number of LEDs in the strip,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
        .led_model = LED_MODEL_SK6812,            // LED strip model
        .flags = {
            .invert_out = false                   // whether to invert the output signal
        },
    };

    led_strip_rmt_config_t rmt_config = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
        .rmt_channel = 0,
#else
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_RMT_RES_HZ,
        .flags = {
            .with_dma = false
        },
#endif
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");
    led_strip_set_pixel(led_strip, 0, rgb_state[0][0], rgb_state[0][1], rgb_state[0][2]);
    led_strip_set_pixel(led_strip, 1, rgb_state[1][0], rgb_state[1][1], rgb_state[1][2]);
    led_strip_set_pixel(led_strip, 2, rgb_state[2][0], rgb_state[2][1], rgb_state[2][2]);
    led_strip_refresh(led_strip);
    
    while (1) 
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        M5.update();
        if (M5.BtnA.wasClicked()) {
            M5.Speaker.tone(2000, 200);
            M5_LOGI("Btn Left Clicked");
            move_rgb(0);
            update_display();
        }
        if (M5.BtnB.wasClicked()) {
            M5.Speaker.tone(2000, 200);
            M5_LOGI("Btn Middle Clicked");
            move_rgb(1);
            update_display();
        }
        if (M5.BtnC.wasClicked()) {
            M5.Speaker.tone(2000, 200);
            M5_LOGI("Btn Right Clicked");
            move_rgb(2);
            update_display();
        }
        led_strip_set_pixel(led_strip, 0, rgb_state[0][0], rgb_state[0][1], rgb_state[0][2]);
        led_strip_set_pixel(led_strip, 1, rgb_state[1][0], rgb_state[1][1], rgb_state[1][2]);
        led_strip_set_pixel(led_strip, 2, rgb_state[2][0], rgb_state[2][1], rgb_state[2][2]);
        led_strip_refresh(led_strip);
    }
}
