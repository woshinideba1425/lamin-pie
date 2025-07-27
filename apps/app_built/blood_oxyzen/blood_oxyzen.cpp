#include "blood_oxyzen.h"
#include "Heartrate.h"
#include "driver/ledc.h"
#include <algorithm>

namespace LAMINATEPIE
{
    //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    //LED Pulse Amplitude Configuration
    //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //Default is 0x1F which gets us 6.4mA
    //powerLevel = 0x02, 0.4mA - Presence detection of ~4 inch
    //powerLevel = 0x1F, 6.4mA - Presence detection of ~8 inch
    //powerLevel = 0x7F, 25.4mA - Presence detection of ~8 inch
    //powerLevel = 0xFF, 50.0mA - Presence detection of ~12 inch
    #define REDLEVEL 0x02
    #define IRLEVEL 0x7F

    namespace BUILTIN_APP
    {
        void BloodOxyzenApp::onSetup()
        {
            setTaskName("bloodOxyzen");
            setScreen(ui_SpO2);
            setAllowBgRunning(false);
            setAppIcon((void *)&ICON_BLOODOXYGEN);
        }

        void BloodOxyzenApp::onCreate()
        {
            printf("[%s] onCreate\n", getAppName().c_str());
            
            update = &_framework->getHAL();
            update->max30105.setPulseAmplitudeRed(REDLEVEL);
            update->max30105.setPulseAmplitudeIR(IRLEVEL);
            update->init_ir_led(0.9);
            update->init_red_led(0.9);

            progress_counter = 0;  // 初始化进度计数器
            current_stage = ProgressStage::COLLECTION;  // 设置为数据收集阶段
        }

        void BloodOxyzenApp::onResume()
        {
            printf("[%s] onResume\n", getAppName().c_str());
            update->init_ir_led(0.2);
            update->init_red_led(0.2);
        }

        void BloodOxyzenApp::taskLoop()
        {
            lv_disp_trig_activity(NULL); // keep screen activity

            uint32_t red_data, ir_data;
            collectData(red_data, ir_data);  // 调用通用数据采集函数

            red_buffer.push_back(red_data);  // 将红光数据存入缓冲区

            // 如果红光缓冲区满了，则处理SpO2
            if (red_buffer.size() >= buffer_size) {
                ir_buffer.push_back(ir_data);  // 将红外数据存入缓冲区

                current_stage = ProgressStage::PROCESSING;
                progress_counter = 0; 
                
                if (ir_buffer.size() >= buffer_size) {

                    progress_counter = 1;  // 数据处理开始进度
                    processingData();  // 处理数据
                    progress_counter = 10;  // 数据处理完成，进度设为 100%
                    showProgress();  // 显示处理完成进度

                    if (spo2Data.SpO2 == -1) {
                        lv_label_set_text_fmt(ui_SPO22, "Invalid");
                    } else {
                        lv_label_set_text_fmt(ui_SPO22, "%d%%", spo2Data.SpO2);
                    }

                    red_buffer.clear();
                    ir_buffer.clear();

                    current_stage = ProgressStage::COLLECTION;
                    progress_counter = 0;
                }
            }
        }

        void BloodOxyzenApp::onRunningBG()
        {


        }

        void BloodOxyzenApp::onPause()
        {
            printf("[%s] onPause\n", getAppName().c_str());

            red_buffer.clear();
            ir_buffer.clear();

            update->max30105.setPulseAmplitudeRed(0);
            update->max30105.setPulseAmplitudeIR(0);
            update->deinit_ir_led();

            progress_counter = 0; 
        }

        void BloodOxyzenApp::onDestroy()
        {
            printf("[%s] onDestroy\n", getAppName().c_str());

            red_buffer.clear();
            ir_buffer.clear();

            update->max30105.setPulseAmplitudeRed(0);
            update->max30105.setPulseAmplitudeIR(0);
            update->deinit_ir_led();

            progress_counter = 0;
        }

        template <typename T>
        T BloodOxyzenApp::calculateSpO2(T acRed, T dcRed, T acIR, T dcIR) {
            ESP_LOGI("BloodOxyzenApp", 
                    "Before calculation - AC_Red: %d, DC_Red: %d, AC_IR: %d, DC_IR: %d", 
                    acRed, dcRed, acIR, dcIR);

            // 检查 DC 分量是否为零以避免除以零错误
            if (dcRed == 0 || dcIR == 0) {
                ESP_LOGE("BloodOxyzenApp", "DC values cannot be zero!");
                return static_cast<T>(0);
            }

            // 归一化 AC 分量，使 AC 信号与其 DC 分量成比例
            double normalized_acRed = static_cast<double>(acRed) / static_cast<double>(dcRed);
            double normalized_acIR = static_cast<double>(acIR) / static_cast<double>(dcIR);

            // 计算红光和红外光的比率
            double red_ratio = std::abs(normalized_acRed);
            double ir_ratio = std::abs(normalized_acIR);

            ESP_LOGI("BloodOxyzenApp", "Red ratio: %.4f, IR ratio: %.4f", red_ratio, ir_ratio);

            // 计算 R 值
            double R = red_ratio / ir_ratio;
            ESP_LOGI("BloodOxyzenApp", "R value: %.4f", R);

            // 使用 R 值计算 SpO2
            double SpO2_float = 100.63-2.543*R;

            if (SpO2_float > 100.0 || SpO2_float < 80.0 || acIR < 5000) {
                return static_cast<T>(-1); // 超出范围，返回 -1 表示无效
            }

            ESP_LOGI("BloodOxyzenApp", "Calculated SpO2 (float): %.2f", SpO2_float);

            // 根据模板参数类型返回结果
            if constexpr (std::is_floating_point<T>::value) {
                return static_cast<T>(SpO2_float);
            } else {
                return static_cast<T>(std::round(SpO2_float));
            }
        }


        void BloodOxyzenApp::processingData(){
            // 计算红光和红外光的AC/DC比值，SpO2的计算和之前的一样

            // for (auto value : red_buffer) {
            //     printf("R[%d] ", value);
            // }
            // printf("\n");

            int32_t dc_red = calculateDC(red_buffer);
            int32_t dc_ir = calculateDC(ir_buffer);
            int32_t ac_red = calculateAC(red_buffer, dc_red);
            int32_t ac_ir = calculateAC(ir_buffer, dc_ir);
            
            spo2Data.SpO2 = calculateSpO2(ac_red, dc_red, ac_ir, dc_ir);
            // 显示或存储计算的SpO2
            printf("SpO2: %d\n", spo2Data.SpO2);
        }

            // 计算AC分量（去掉DC分量）
            uint32_t BloodOxyzenApp::calculateAC(const std::deque<uint32_t>& buffer, uint32_t dc_value) {
                std::deque<uint32_t> sorted_buffer = buffer;
                std::sort(sorted_buffer.begin(), sorted_buffer.end());
                uint32_t median_value = sorted_buffer[sorted_buffer.size() / 2];  // 计算中位数
                return median_value - dc_value;  // 使用中位数减去DC值
            }

        uint32_t BloodOxyzenApp::calculateDC(const std::deque<uint32_t>& buffer) {
            if (buffer.empty()) return 0;

            const size_t window_size = std::min(buffer.size(), static_cast<size_t>(40)); // 设置窗口大小为40
            int32_t dc_value = 0;  // 用于存储当前的DC估计值
            int32_t sum = 0;

            for (size_t i = buffer.size() - window_size; i < buffer.size(); ++i) {
                sum += buffer[i];
            }

            // 使用滑动平均计算DC
            dc_value = sum / window_size;

            return dc_value;  // 返回最后的DC估计值
        }

        void BloodOxyzenApp::collectData(uint32_t &red_data, uint32_t &ir_data)
        {
            // 打开红光LED, 关闭红外LED，采集红光数据
            update->max30105.setPulseAmplitudeRed(REDLEVEL);  // 设置红光LED强度
            update->max30105.setPulseAmplitudeIR(0);      // 关闭红外LED
            update->deinit_ir_led();
            update->resum_red_led(0.9);
            red_data = update->max30105.getRed();         // 获取红光数据

            // 打开红外LED, 关闭红光LED，采集红外数据
            update->max30105.setPulseAmplitudeRed(0);     // 关闭红光LED
            update->max30105.setPulseAmplitudeIR(IRLEVEL);
            update->deinit_red_led();
            update->resum_ir_led(0.9);
            ir_data = update->max30105.getIR();           // 获取红外数据

            progress_counter++;  // 每次采集数据后增加进度计数器
            showProgress();  // 更新并显示进度
        }

        // 显示进度
        void BloodOxyzenApp::showProgress() {
            int progress = 0;
            const char* stage_text = "";

            if (current_stage == ProgressStage::COLLECTION) {
                // 数据收集阶段，百分比在 0~90 之间
                progress = (progress_counter * process_stage_percentage) / buffer_size;
                stage_text = "数据收集中";
            } else if (current_stage == ProgressStage::PROCESSING) {
                // 数据处理阶段，百分比在 90~100 之间
                progress = process_stage_percentage + 
                        (progress_counter * (total_percentage - process_stage_percentage)) / 10;
                stage_text = "数据处理中";
            }

            // 打印到控制台（调试用）
            //printf("%s | Progress: %d%%\n", stage_text, progress);

            // 更新 LVGL 进度条的值
            lv_bar_set_value(ui_progress_Bar1, progress, LV_ANIM_ON);

            // 更新标签的文本，显示状态和进度
            char label_text[64];
            snprintf(label_text, sizeof(label_text), "%s ", stage_text);
            lv_label_set_text(ui_SPO2l, label_text);
        }

    }
}