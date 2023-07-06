#include <Arduino.h>
#include <BleGamepad.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "ui/ui.h"
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <BleGamepad.h>

BleGamepad bleGamepad("BLE Flight Controller", "lemmingDev", 100);
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

TaskHandle_t task;

DynamicJsonDocument doc(1024);

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

void readSerialPortData(void *pd)
{
  while (1)
  {
    if (Serial.available() > 0)
    {
      String str = Serial.readString();
      Serial.flush();

      deserializeJson(doc, str);
      lv_label_set_text(ui_Com1Res, doc["com1_active"]);
      lv_label_set_text(ui_Com1Stdby, doc["com1_standby"]);
    }

    vTaskDelay(10);
  }
}

void setupDisplay(void *pt)
{
  lv_init();

  tft.begin();        /* TFT init */
  tft.setRotation(3); /* Landscape orientation, flipped */

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  lv_indev_drv_register(&indev_drv);

  ui_init();

  for (;;)
  {
    // lv_timer_handler();
    lv_tick_inc(1);
    lv_task_handler();
    vTaskDelay(5);
  }
}

void requestData(TimerHandle_t xTimer)
{
  Serial.write("request");
}

void setup()
{
  Serial.begin(115200);
  bleGamepad.begin();
  xTaskCreatePinnedToCore(setupDisplay, "setupDisplay", 1024 * 10, NULL, 1, NULL, 1);
  // xTaskCreatePinnedToCore(readSerialPortData, "readSerialPortData", 1024, NULL, 1, NULL, 1);
  TimerHandle_t t = xTimerCreate("timer", pdMS_TO_TICKS(1000 * 3), true, NULL, requestData);
  xTimerStart(t, 10 * 1000);
}

void loop()
{
  // lv_timer_handler();
  // if (Serial.available() > 0)
  // {
  //   vTaskDelay(100);

  //   // int size = Serial.available();
  //   // char buffer[size];
  //   // Serial.readBytes(buffer, size);
  //   String str = Serial.readString();
  //   // 如何再loop中调用ui_label_set_text
  //   _ui_label_set_property(ui_Label2, _UI_LABEL_PROPERTY_TEXT, "aasssddd");

  //   Serial.write(str.c_str());
  // }

  // vTaskDelay(10);
}

void serialEvent()
{
  String str = Serial.readString();
  Serial.flush();

  deserializeJson(doc, str);
  lv_label_set_text(ui_Com1Res, doc["com1_active"]);
  lv_label_set_text(ui_Com1Stdby, doc["com1_standby"]);
}