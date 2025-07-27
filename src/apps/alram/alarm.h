#pragma once
#include "../../app/app.hpp"
#include "../../framework/framwork.h"
#include "ArduinoJson.hpp"
#include "SlideListContainer.h"
#include <string>
#include <sys/_stdint.h>
#include <vector>
namespace LAMINATEPIE
{
    
    namespace BUILTIN_APP
    {
        enum Todo_type_t
        {
            TODO_EVEVRYDAY,
            TODO_MONDAY_TO_FRIDAY,
            TODO_ONCE,
        };

        struct Todo_t
        {
            // 构造函数，计算remain_time
            Todo_t(const DataTime_t& current_dt, int aim_hour, int aim_min, Todo_type_t type, std::string aim_time_str)
                : type_(type), aim_hour_(aim_hour), aim_min_(aim_min), aim_time_str_(aim_time_str), is_active_(true)
            {
                update_remain_time(current_dt);
            }

            // 更新剩余时间，使用DataTime_t
            void update_remain_time(const DataTime_t& current_dt)
            {
                int64_t current_time = current_dt.TotalSeconds();
                
                // 目标时间的DataTime_t
                DataTime_t target_dt = current_dt; // 复制当前时间
                target_dt.hour = aim_hour_;
                target_dt.min = aim_min_;
                target_dt.sec = 0;
                
                int64_t target_time = target_dt.TotalSeconds();
                
                if (type_ == TODO_ONCE) {
                    wday_str_ = "Once";
                    if (target_time <= current_time) {
                        remain_time = 0;
                        is_active_ = false;
                    } else {
                        remain_time = target_time - current_time;
                    }
                }
                else if (type_ == TODO_EVEVRYDAY) {
                    if (target_time <= current_time) {
                        // 跳到明天
                        target_dt.mday += 1;
                        target_time = target_dt.TotalSeconds();
                    }
                    remain_time = target_time - current_time;
                    wday_str_ = "Every Day";
                }
                else if (type_ == TODO_MONDAY_TO_FRIDAY) {
                    // 周一到周五逻辑，使用DataTime_t
                    int add_days = 0;
                    int wday = current_dt.wday; // 0-6, 0是周日
                    
                    while (true) {
                        int next_wday = (wday + add_days) % 7;
                        if (next_wday >= 1 && next_wday <= 5) { // 1-5是周一到周五
                            DataTime_t next_dt = current_dt;
                            next_dt.mday += add_days;
                            next_dt.hour = aim_hour_;
                            next_dt.min = aim_min_;
                            next_dt.sec = 0;
                            
                            int64_t next_time = next_dt.TotalSeconds();
                            if (next_time > current_time) {
                                remain_time = next_time - current_time;
                                break;
                            }
                        }
                        add_days++;
                        if (add_days > 7) {
                            remain_time = 0;
                            break;
                        }
                    }
                    wday_str_ = "Monday to Friday";
                }
                
                ESP_LOGI("Todo_t", "更新后remain_time: %lld", remain_time);
            }
            
            int id;
            int64_t remain_time;
            Todo_type_t type_;
            int aim_hour_;  // 目标小时
            int aim_min_;   // 目标分钟
            std::string aim_time_str_; // 目标时间字符串
            std::string wday_str_; // 星期几
            bool is_active_ = false;

            bool operator > (const Todo_t& other) const {
                return remain_time > other.remain_time;
            }

            bool operator < (const Todo_t& other) const {
                return remain_time < other.remain_time;
            }
        };

        // 前向声明，解决循环引用
        class AlarmApp;

        class AlarmPanel : public Panel 
        {
        public:
            explicit AlarmPanel(lv_obj_t *parent, std::shared_ptr<Todo_t> todo, void* app_ptr);
            virtual void onCreate() override;
            virtual void onDestroy() override;
            std::shared_ptr<Todo_t> getTodo(){
                return _todo;
            }
            void updateSwitchState(bool active);
        private:
            // UI组件
            lv_obj_t *ui_clock__switch;
            lv_obj_t *ui_tm_hour_min;
            lv_obj_t *ui_tm_wday;

            // 时间数据
            std::string _hour;    // 时
            std::string _minute;  // 分
            std::string _wday;    // 星期几

            std::shared_ptr<Todo_t> _todo;
            AlarmApp* _app_ptr;   // 指向AlarmApp的指针

            bool _alarm_switch = false;
        };

        class AlarmApp : public FreeRTOSAppBase
        {
        private:
            Framework *_framework = Framework::getInstance();
            FreeRTOSAppBase *_alarm_app;
            const char *TAG = "AlarmApp";
            ArduinoJson::JsonDocument config;
            HAL *update;
            ST::SystemTask *system_task;
            PCF8563 *rtc;
            SlideListContainer* _alarm_list = nullptr;
            std::vector<std::shared_ptr<Todo_t>> _todo_list;
        
        private:
            void load_config_from_nvs();
            void save_config_to_nvs();

            static void on_set_alert(lv_event_t* e);
            static void on_add_alarm(lv_event_t* e);
            
        public:
            AlarmApp() : _framework(Framework::getInstance()), _alarm_app(nullptr), update(nullptr){
                Config cfg;
                cfg.spacing = 10;
                cfg.padding_h = 15;
                cfg.padding_v = 10;
                cfg.anim_time = 500; // 500毫秒，让动画慢一点便于观察

                _alarm_list = new SlideListContainer(ui_clock_container, cfg);
                _alarm_list->enableSwipe(true);
            }
            ~AlarmApp() = default;

            // 根据ID移除Todo项
            void removeTodoById(int id) {
                _todo_list.erase(
                    std::remove_if(_todo_list.begin(), _todo_list.end(),
                        [id](const std::shared_ptr<Todo_t>& t){ return t->id == id; }),
                    _todo_list.end()
                );
                // 更新配置
                save_config_to_nvs();
            }

            void updatePanelUI(Todo_t &todo);

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