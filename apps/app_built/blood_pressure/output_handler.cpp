#include "output_handler.h"

#include <cmath>  // 对于 std::round
#include "Lcd/hal_lcd.h"

int HPData;
int lowHPData;
char ui_lowHPData[5];
char ui_HPData[5];
// char udp_highbP[20];
// char udp_lowhbP[20];
// extern myWiFi wifi;

int HandleFix(float input){
    int output = static_cast<int>(std::round(input));
  if (output > 135) {
    output += 5;
  }else if(output > 115 && output<130){
    output -= 6; 
  }
  return output;
}

void IndicateOutput(lv_obj_t *target, int input, int low, int mid, int hig){
  char ui_buf[50];

  // 将数值转换为字符串
  snprintf(ui_buf, sizeof(ui_buf), "%d", input);
  lv_label_set_text(target, ui_buf);

  // 设置颜色
  if(input >= low && input < mid){
      lv_obj_set_style_text_color(target, lv_color_hex(0x7EDE18), LV_PART_MAIN | LV_STATE_DEFAULT ); // 绿色
  }
  else if(input >= mid && input < hig){
      lv_obj_set_style_text_color(target, lv_color_hex(0xFFE135), LV_PART_MAIN | LV_STATE_DEFAULT ); // 黄色
  }
  else if(input >= hig){
      lv_obj_set_style_text_color(target, lv_color_hex(0xDE2418), LV_PART_MAIN | LV_STATE_DEFAULT ); // 红色
  }
  else {
      // 你可以在这里设置默认的颜色或处理不在任何范围内的情况
      lv_obj_set_style_text_color(target, lv_color_hex(0x7EDE18), LV_PART_MAIN | LV_STATE_DEFAULT ); // 黑色
  }
}

void HandleOutput(tflite::ErrorReporter* error_reporter, float highPressure, float lowPressure) {
  // 对高压值进行四舍五入和修正

    int roundedHighPressure = HandleFix(highPressure);
    TF_LITE_REPORT_ERROR(error_reporter, "BP result: %d", roundedHighPressure);

    IndicateOutput(ui_Label15,roundedHighPressure,90,120,140);

    // 对低压值进行四舍五入
    int roundedLowPressure = static_cast<int>(std::round(lowPressure));
    TF_LITE_REPORT_ERROR(error_reporter, "low BP result: %d", roundedLowPressure);
    IndicateOutput(ui_diya1,roundedLowPressure,0,80,90);
    lvgl_unlock();
  // 发送修正后的高压值和低压值
  // snprintf(udp_highbP, sizeof(udp_highbP), "high bpdata : %d", HPData);
  // snprintf(udp_lowhbP, sizeof(udp_lowhbP), "low bpdata : %d", lowHPData);
  // wifi.sendUDPdata(udp_highbP);
  // wifi.sendUDPdata(udp_lowhbP);
}

