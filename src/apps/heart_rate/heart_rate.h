#pragma once

#include "../../app/app.hpp"
#include "../../framework/framwork.h"
#include <numeric>  // std::accumulate
#include <vector>
#include <optional>


namespace LAMINATEPIE
{

    // 自定义 PSRAM 分配器
    template <typename T>
    struct PsramAllocator {
        typedef T value_type;

        PsramAllocator() noexcept {}

        template <typename U>
        PsramAllocator(const PsramAllocator<U>&) noexcept {}

        T* allocate(std::size_t n) {
            if (n == 0) {
                return nullptr;
            }
            if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
                abort();
            }

            T* ptr = static_cast<T*>(heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM));
            if (!ptr) {
                abort();
            }
            return ptr;
        }

        void deallocate(T* p, std::size_t) noexcept {
            heap_caps_free(p);
        }
    };

    template <typename T, typename U>
    bool operator==(const PsramAllocator<T>&, const PsramAllocator<U>&) { return true; }

    template <typename T, typename U>
    bool operator!=(const PsramAllocator<T>&, const PsramAllocator<U>&) { return false; }

    namespace BUILTIN_APP
    {

    struct HeartRateData
    {
        static const int INIT_BUFFER_SIZE = 3;  // 初始化缓冲区大小
        std::vector<long> initDeltas;  // 用于存储初始化阶段的心跳间隔
        bool initializationComplete = false;  // 初始化是否完成的标志位

        long lastBeat = 0;                  // Time of the last beat
        long lastDelta = 0;                 // Time of the last Delta
        float beatsPerMinute = 0.0f;        // Current heart rate in BPM
        static const uint8_t RATE_SIZE = 5; // Number of values to average

        std::vector<uint8_t> rates;         // Dynamic heart rate array for averaging
        std::vector<long> deltas;           // Dynamic heart rate delta array for RMSSD
        uint8_t rateSpot = 0;               // Current index for storing heart rate
        uint8_t validRateCount = 0;         // Counter for valid heart rate readings
        float beatAvg = 0.0f;               // Average heart rate

        char beatsPerMinuteStr[16] = {0};  // String for displaying current heart rate
        char beatAvgStr[16] = {0};         // String for displaying average heart rate
        bool checkstate = false;           // State for determining if heart rate calculation is stable
        int heartRate = 0;                 // Processed fluid information
    };

        class HeartRateApp : public FreeRTOSAppBase
        {
        private:
            Framework *_framework = Framework::getInstance();
            FreeRTOSAppBase *_heartRate_app;
            const char *TAG = "HeartRateApp";
            HAL *update;
            HeartRateData heartRateData;
            std::vector<float, PsramAllocator<float>> input_buffer;
            int32_t ir_avg_reg = 0;

        public:
            HeartRateApp() : _framework(Framework::getInstance()), _heartRate_app(nullptr), update(nullptr) {}
            ~HeartRateApp() = default;
            
            /*App task, called when app running*/
            uint8_t checkHeartbeat(uint32_t sample);
            void update_ui();
            void processingData();
            std::vector<float, PsramAllocator<float>> _filter_heart_rate(const std::vector<float, PsramAllocator<float>>& input_signal);
            bool isValidInterval(long currentDelta, long lastDelta);
            std::optional<double> calculateRMSSD();

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