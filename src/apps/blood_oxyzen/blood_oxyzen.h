#pragma once
#include "../../app/app.hpp"
#include "../../framework/framwork.h"
#include <numeric>  // std::accumulate
#include <vector>
#include <deque>
#define BUFFER_SIZE 300

namespace LAMINATEPIE
{
    namespace BUILTIN_APP
    {
        struct SpO2Data{
            int SpO2 = 0;
            std::deque<int> spo2History;  // 保存最近的4个SpO2数值
            const size_t SPO2_HISTORY_SIZE = 4;  // 记录的历史数值数量
            const int SPO2_STABILITY_THRESHOLD = 3;  // SpO2相差不超过这个阈值，认为是稳定的

        };

        enum class ProgressStage {
            COLLECTION,  // 数据收集阶段
            PROCESSING   // 数据处理阶段
        };

        class BloodOxyzenApp : public FreeRTOSAppBase
        {
        private:
            Framework *_framework = Framework::getInstance();
            FreeRTOSAppBase *_bloodOxyzen_app;
            const char *TAG = "BloodOxyzenApp";
            HAL *update;
            SpO2Data spo2Data;
            std::deque<uint32_t> red_buffer;
            std::deque<uint32_t> ir_buffer;
            int buffer_size = BUFFER_SIZE;  // 可以根据需求调整
            ProgressStage current_stage = ProgressStage::COLLECTION;  // 当前阶段，初始为数据收集
            int progress_counter = 0;  // 进度计数器
            const int process_stage_percentage = 90; // 数据收集阶段占90%
            const int total_percentage = 100;  // 总的百分比为100

        public:
            BloodOxyzenApp() : _framework(Framework::getInstance()), _bloodOxyzen_app(nullptr), update(nullptr) {}
            ~BloodOxyzenApp() = default;

            /*App task, called when app running*/
            void processingData();
            uint32_t calculateAC(const std::deque<uint32_t> &buffer, uint32_t dc_value);
            uint32_t calculateDC(const std::deque<uint32_t> &buffer);
            void collectData(uint32_t &red_data, uint32_t &ir_data);
            void showProgress();
            template <typename T>
            T calculateSpO2(T acRed, T dcRed, T acIR, T dcIR);

            /**
             * @brief Lifecycle callbacks for derived to override
             *
             */
            /* Setup App configs, called when App "install()" */
            void onSetup();
            // void wattingCallback(lv_event_t *e);

            /* Life cycle */
            void onCreate();
            void onResume();
            void taskLoop();
            void onRunningBG();
            void onPause();
            void onDestroy();
            
        };
    }
}