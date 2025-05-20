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
#define LED_STRIP_LED_NUMBERS 1
#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)

static const char *TAG = "LED_STRIP";

led_strip_handle_t led_strip;

static uint8_t rgb_state[3] = {255, 255, 255};

void update_display() {
    M5.Display.clear();
    M5.Display.setCursor(10, 10);
    M5.Display.print("Unit RGB TEST");
    M5.Display.setCursor(10, 50);
    M5.Display.print("R:");
    M5.Display.print(rgb_state[0] == 255 ? "ON" : "OFF");
    M5.Display.setCursor(10, 100);
    M5.Display.print("G:");
    M5.Display.print(rgb_state[1] == 255 ? "ON" : "OFF");
    M5.Display.setCursor(10, 150);
    M5.Display.print("B:");
    M5.Display.print(rgb_state[2] == 255 ? "ON" : "OFF");
}

extern "C" void app_main(void)
{
    M5.begin();
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
    led_strip_set_pixel(led_strip, 0, 255, 255, 255);
    led_strip_refresh(led_strip);
    
    while (1) 
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        M5.update();
        if (M5.BtnA.wasClicked()) {
            M5.Speaker.tone(2000, 200);
            M5_LOGI("Btn Left Clicked");
            rgb_state[0] = (rgb_state[0] == 255) ? 0 : 255;
            update_display();
        }
        if (M5.BtnB.wasClicked()) {
            M5.Speaker.tone(2000, 200);
            M5_LOGI("Btn Middle Clicked");
            rgb_state[1] = (rgb_state[1] == 255) ? 0 : 255;
            update_display();
        }
        if (M5.BtnC.wasClicked()) {
            M5.Speaker.tone(2000, 200);
            M5_LOGI("Btn Right Clicked");
            rgb_state[2] = (rgb_state[2] == 255) ? 0 : 255;
            update_display();
        }
        
        static uint8_t breath_val = 0;
        static bool breath_increasing = true;
        
        // 计算呼吸效果的亮度值
        if (breath_increasing) {
            breath_val = (breath_val >= 255) ? 255 : breath_val + 5;
            if (breath_val >= 255) breath_increasing = false;
        } else {
            breath_val = (breath_val <= 0) ? 0 : breath_val - 5;
            if (breath_val <= 0) breath_increasing = true;
        }
        
        // 应用呼吸效果，如果RGB值为0则不应用呼吸效果
        uint32_t r = rgb_state[0] > 0 ? (rgb_state[0] * breath_val) / 255 : 0;
        uint32_t g = rgb_state[1] > 0 ? (rgb_state[1] * breath_val) / 255 : 0;
        uint32_t b = rgb_state[2] > 0 ? (rgb_state[2] * breath_val) / 255 : 0;
        
        led_strip_set_pixel(led_strip, 0, r, g, b);
        led_strip_refresh(led_strip);

    }
}
