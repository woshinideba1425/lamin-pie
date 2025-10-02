#include "heart_rate.h"
#include "Heartrate.h"

#define RED_AMPLITUDE 0.95
#define HEART_RATE_OFFSET 6
namespace LAMINATEPIE
{
    namespace BUILTIN_APP
    {
        void HeartRateApp::onSetup()
        {
            setTaskName("heartRate");
            setScreen(ui_ecgjiemian);
            setAllowBgRunning(false);
            setAppIcon((void *)&ICON_HEARTRATE);
        }

        void HeartRateApp::onCreate()
        {
            printf("[%s] onCreate\n", getAppName().c_str());
            update = &_framework->getHAL();
            update->max30105.init();
            update->max30105.setPulseAmplitudeRed(0xFF);
            //update->max30105.setPulseAmplitudeIR(0xFF);
            update->init_red_led(RED_AMPLITUDE);
            
        }

        void HeartRateApp::onResume()
        {
            printf("[%s] onResume\n", getAppName().c_str());
            update->max30105.setPulseAmplitudeRed(0xFF);
            update->init_red_led(RED_AMPLITUDE);
            heartRateData.initializationComplete = false;
            heartRateData.initDeltas.clear();
        }

        void HeartRateApp::taskLoop()
        {
            lv_disp_trig_activity(NULL); // keep screen activity
            processingData();
            update_ui();
            
        }

        void HeartRateApp::onRunningBG()
        {

        }
        
        void HeartRateApp::onPause()
        {
            printf("[%s] onPause\n", getAppName().c_str());
            update->max30105.setPulseAmplitudeRed(0);
            update->deinit_ir_led();
        }

        void HeartRateApp::onDestroy()
        {
            printf("[%s] onDestroy\n", getAppName().c_str());
            update->max30105.setPulseAmplitudeRed(0);
        }

        void HeartRateApp::processingData()
        {
            uint8_t HR;
            uint32_t IR = update->max30105.getRed();
            //printf("%d, ",static_cast<int>(IR));
            HR = checkHeartbeat(IR);
            lv_label_set_text_fmt(ui_heart2, "%d", HR);

            std::optional<double> rmssd = calculateRMSSD();
            if (rmssd.has_value()) {
                int rmssd_int = static_cast<int>(rmssd.value());  
                lv_label_set_text_fmt(ui_hrv_main, "%d", rmssd_int);  
            } else {
                lv_label_set_text_fmt(ui_hrv_main, "N/A");  
            }

        }

        uint8_t HeartRateApp::checkHeartbeat(uint32_t sample) 
        {
            // Step 1: 检测是否有新的心跳信号
            if (checkForBeat(sample)) {
                long currentTime = esp_timer_get_time() / 1000;
                long delta = currentTime - heartRateData.lastBeat;
                heartRateData.lastBeat = currentTime;

                // Step 2: 如果初始化未完成，进行初始化逻辑
                if (!heartRateData.initializationComplete) {
                    if (delta < 1000) {
                        heartRateData.initDeltas.push_back(delta);
                        // LOGI("HeartRateApp", "Storing delta in initialization buffer: %ld ms", delta);

                        // 如果缓冲区还未满，跳过进一步检查
                        if (heartRateData.initDeltas.size() < heartRateData.INIT_BUFFER_SIZE) {
                            return heartRateData.heartRate;  // 等待更多心跳信号
                        }

                        // 缓冲区已满，计算平均 delta
                        long avgDelta = std::accumulate(heartRateData.initDeltas.begin(), heartRateData.initDeltas.end(), 0L) / heartRateData.INIT_BUFFER_SIZE;
                        heartRateData.lastDelta = avgDelta;  // 将平均值作为初始的 lastDelta
                        heartRateData.initDeltas.clear();  // 清空缓冲区
                        heartRateData.initializationComplete = true;  // 标记初始化完成
                        // LOGI("HeartRateApp", "Initialization complete. Using average delta: %ld ms", avgDelta);
                        heartRateData.beatsPerMinute = 60.0 / (heartRateData.lastDelta / 1000.0) + HEART_RATE_OFFSET;
                        heartRateData.heartRate = static_cast<int>(heartRateData.beatsPerMinute);
                        // LOGI("HeartRateApp", "Initial heart rate calculated: %d BPM", heartRateData.heartRate);
                        // 将初始心率存入 rates 缓冲区
                        heartRateData.rates.push_back(static_cast<uint8_t>(heartRateData.beatsPerMinute));
                        // LOGI("HeartRateApp", "Storing initial BPM: %.2f, Size: %d", heartRateData.beatsPerMinute, heartRateData.rates.size());

                        // 如果 rates 大小超过缓冲区最大值，移除最早的值
                        if (heartRateData.rates.size() > HeartRateData::RATE_SIZE) {
                            heartRateData.rates.erase(heartRateData.rates.begin());
                        }
                        
                        return heartRateData.heartRate;  // 返回并等待下一次心跳
                    } else {
                        LOGW("HeartRateApp", "Skipped delta %ld ms as it is greater than 1200 ms", delta);
                        return heartRateData.heartRate;  // delta 超过 1200 毫秒，跳过这次存储
                    }
                }

                // Step 3: 计算每分钟心跳数
                heartRateData.beatsPerMinute = 60.0 / (delta / 1000.0) + HEART_RATE_OFFSET;
                // LOGI("HeartRateApp", "New heart beat detected! BPM: %.2f, Delta: %ld ms", heartRateData.beatsPerMinute, delta);
                // LOGI("HeartRateApp", "New heart beat detected! LastDelta: %ld ms, Delta: %ld ms", heartRateData.lastDelta, delta);

                // Step 4: 仅存储有效的 beatsPerMinute 值
                if (heartRateData.beatsPerMinute > 60 && heartRateData.beatsPerMinute < 255 && isValidInterval(delta, heartRateData.lastDelta)) {

                    // 检查通过，保存有效的 delta
                    heartRateData.lastDelta = delta;
                    heartRateData.deltas.push_back(delta);

                    heartRateData.rates.push_back(static_cast<uint8_t>(heartRateData.beatsPerMinute));

                    // 如果 rates 大小超过缓冲区最大值，移除最早的值
                    if (heartRateData.rates.size() > HeartRateData::RATE_SIZE) {
                        heartRateData.rates.erase(heartRateData.rates.begin());
                    }

                    // LOGI("HeartRateApp", "Storing valid BPM: %.2f, Size: %d", heartRateData.beatsPerMinute, heartRateData.rates.size());

                    // Step 5: 当有至少 RATE_SIZE 个有效读数时计算平均值
                    if (heartRateData.rates.size() >= heartRateData.RATE_SIZE) {
                        // 使用浮点数类型来保存中间结果以避免整数除法丢失精度
                        float totalBPM = std::accumulate(heartRateData.rates.begin(), heartRateData.rates.end(), 0.0f);
                        heartRateData.beatAvg = totalBPM / heartRateData.rates.size();

                        // LOGI("HeartRateApp", "Calculated average BPM: %.2f", heartRateData.beatAvg);

                        // 更新心跳平均值并返回
                        heartRateData.heartRate = static_cast<int>(heartRateData.beatAvg);
                        LOGI("HeartRateApp", "BPM: %.2f", heartRateData.beatAvg);
                        return heartRateData.heartRate;  // 返回计算的平均心跳值
                    }

                    // 增加 validRateCount，直到达到缓冲区的最大大小（RATE_SIZE）
                    heartRateData.validRateCount = heartRateData.rates.size();
                    return heartRateData.heartRate;  // 返回上一个有效的心跳值
                } else {
                    // LOGW("HeartRateApp", "Invalid BPM detected: %.2f", heartRateData.beatsPerMinute);
                    // 不更新 lastDelta，因为当前 delta 无效
                }
            }

            // 如果没有检测到新的心跳，则返回上一个有效心跳值
            return heartRateData.heartRate;
        }


        void HeartRateApp::update_ui()
        {
            lv_coord_t data = static_cast<lv_coord_t>(getProcessedData());
            if(lvgl_lock(20)){
               updateChartSeries(data);
               lvgl_unlock(); 
            }
        }

        // 变异性过滤器，用于判断当前心跳间隔是否异常
        bool HeartRateApp::isValidInterval(long currentDelta, long lastDelta) {
            const double ratioThreshold = 1.3;  // 比例阈值 (20% 的变化)
            
            double ratio = static_cast<double>(currentDelta) / static_cast<double>(lastDelta);
            
            // 检查比例变化是否在合理范围内
            if (ratio > ratioThreshold || ratio < 1.0 / ratioThreshold) {
                return false;  // 如果变化超出阈值，认为是无效数据
            }
            
            return true;
        }

        std::optional<double> HeartRateApp::calculateRMSSD() {
            // 检查是否有足够的有效心跳间隔来进行 RMSSD 计算
            if (heartRateData.deltas.size() < 8) {
                //LOGW("HeartRateApp", "Not enough data to calculate RMSSD. Size: %d", heartRateData.deltas.size());
                return std::nullopt;  // 返回空值，表示没有足够的数据进行计算
            }

            // 计算 RR 间期的差值平方和
            double sumSquaredDiffs = 0.0;
            for (size_t i = 1; i < heartRateData.deltas.size(); ++i) {
                long deltaRR = heartRateData.deltas[i] - heartRateData.deltas[i - 1];  // RR_{i+1} - RR_i
                sumSquaredDiffs += deltaRR * deltaRR;  // 累积差值的平方
            }

            // 计算 RMSSD
            double rmssd = sqrt(sumSquaredDiffs / (heartRateData.deltas.size() - 1)) - 10;
            //// LOGI("HeartRateApp", "Calculated RMSSD: %.2f", rmssd);

            return rmssd;  // 返回有效的 RMSSD 数值
        }

    }
}