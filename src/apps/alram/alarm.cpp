#include "alarm.h"
#include "Lcd/hal_lcd.h"
#include "esp_log.h" // For LOGI/LOGE if not already included
#include "ui.h"
#include <sys/_stdint.h>

namespace LAMINATEPIE
{
    namespace BUILTIN_APP
    {
        AlarmPanel::AlarmPanel(lv_obj_t *parent, std::shared_ptr<Todo_t> todo, void* app_ptr):
                Panel(parent), _todo(todo), _app_ptr(static_cast<AlarmApp*>(app_ptr))
            {
                printf("AlarmPanel: parent=%p, _panel=%p\n", parent, panel());
            }   

            void AlarmPanel::onCreate() {

                LOGI("AlarmPanel", "onCreate: panel()=%p", panel());
                // 转换时间戳为各种格式
                lv_obj_t* panel_obj = panel();
                lv_obj_set_width(panel_obj, lv_pct(100));
                lv_obj_set_height(panel_obj, lv_pct(30));
                lv_obj_set_style_radius(panel_obj, 8, 0);
                lv_obj_set_style_bg_color(panel_obj, lv_color_hex(0xffffff), 0);
                lv_obj_set_style_shadow_width(panel_obj, 5, 0);

                ui_clock__switch = lv_switch_create(panel());
                lv_obj_set_width( ui_clock__switch, 55);
                lv_obj_set_height( ui_clock__switch, 35);
                lv_obj_set_align( ui_clock__switch, LV_ALIGN_RIGHT_MID );
                
                // 设置开关初始状态与_todo->is_active_一致
                if (_todo->is_active_) {
                    lv_obj_add_state(ui_clock__switch, LV_STATE_CHECKED);
                } else {
                    lv_obj_clear_state(ui_clock__switch, LV_STATE_CHECKED);
                }
                
                // 添加事件回调
                lv_obj_add_event_cb(ui_clock__switch, [](lv_event_t *e) {
                    AlarmPanel* panel = static_cast<AlarmPanel*>(lv_event_get_user_data(e));
                    lv_obj_t* switch_obj = lv_event_get_target(e);
                    bool is_checked = lv_obj_has_state(switch_obj, LV_STATE_CHECKED);
                    
                    // 更新Todo的激活状态
                    if (panel && panel->_todo) {
                        panel->_todo->is_active_ = is_checked;
                        LOGI("AlarmPanel", "闹钟状态已更改: %s", is_checked ? "激活" : "关闭");
                        
                    }
                }, LV_EVENT_VALUE_CHANGED, this);

                ui_tm_hour_min = lv_label_create(panel());
                lv_obj_set_width( ui_tm_hour_min, LV_SIZE_CONTENT);  /// 1
                lv_obj_set_height( ui_tm_hour_min, LV_SIZE_CONTENT);   /// 1
                lv_obj_set_align( ui_tm_hour_min, LV_ALIGN_LEFT_MID );

                // 显示时间字符串 (时:分)
                lv_label_set_text(ui_tm_hour_min, _todo->aim_time_str_.c_str());
                lv_obj_set_style_text_color(ui_tm_hour_min, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
                lv_obj_set_style_text_opa(ui_tm_hour_min, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
                lv_obj_set_style_text_font(ui_tm_hour_min, &lv_font_montserrat_26, LV_PART_MAIN| LV_STATE_DEFAULT);

                ui_tm_wday = lv_label_create(panel());
                lv_obj_set_width( ui_tm_wday, LV_SIZE_CONTENT);  /// 1
                lv_obj_set_height( ui_tm_wday, LV_SIZE_CONTENT);   /// 1
                lv_obj_set_align( ui_tm_wday, LV_ALIGN_OUT_LEFT_BOTTOM );
                // 显示星期几
                lv_label_set_text(ui_tm_wday, _todo->wday_str_.c_str());
                lv_obj_set_style_text_color(ui_tm_wday, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT );
                lv_obj_set_style_text_font(ui_tm_wday, &lv_font_montserrat_20, LV_PART_MAIN| LV_STATE_DEFAULT);
            }

            void AlarmPanel::onDestroy() {
                LOGI("AlarmPanel", "onDestroy: panel()=%p", panel());
                if (_app_ptr && _todo) {
                    _app_ptr->removeTodoById(_todo->id);
                }
                _todo.reset();
            }

        void AlarmApp::on_add_alarm(lv_event_t* e)
        {
            lv_obj_clear_flag(ui_clock_set_setting, LV_OBJ_FLAG_HIDDEN);
        }

        void AlarmPanel::updateSwitchState(bool active){
            if (active) {
                lv_obj_add_state(ui_clock__switch, LV_STATE_CHECKED);
            } else {
                lv_obj_clear_state(ui_clock__switch, LV_STATE_CHECKED);
            }
        }

        void AlarmApp::on_set_alert(lv_event_t* e)
        {
            AlarmApp* app = (AlarmApp*)lv_event_get_user_data(e);
            char hour[3];
            lv_roller_get_selected_str(ui_minute_roller, hour, 3);
            std::string hour_str = std::string(hour);
            char minute[3];
            lv_roller_get_selected_str(ui_hour_roller, minute, 3);
            std::string minute_str = std::string(minute);
            char am_pm[3];
            lv_roller_get_selected_str(ui_am_pm_roller, am_pm, 3);
            std::string am_pm_str = std::string(am_pm);

            std::string alert_time_str = hour_str + ":" + minute_str + am_pm_str;
            
            // 解析小时和分钟
            int aim_hour = std::stoi(hour_str);
            int aim_min = std::stoi(minute_str);
            
            // 处理AM/PM，转换为24小时制
            if (am_pm_str == "pm" && aim_hour < 12) {
                aim_hour += 12;
            } else if (am_pm_str == "am" && aim_hour == 12) {
                aim_hour = 0;
            }

            // 获取当前时间戳
            LAMINATEPIE::DataTime_t tm_cur;
            app->rtc->getTime(tm_cur);
            int64_t current_time = tm_cur.TotalSeconds();
            LOGI("lvgl_callback","current time year: %d, month: %d, day: %d, hour: %d, min: %d, wday: %d", 
                        tm_cur.year, tm_cur.mon, tm_cur.mday, tm_cur.hour, tm_cur.min, tm_cur.wday);
            LOGI("lvgl_callback", "aim_hour: %d, aim_min: %d, aim_time_str: %s", aim_hour, aim_min, alert_time_str.c_str());
            char option[20];
            lv_dropdown_get_selected_str(ui_repeat_drowdown, option, 20);
            
            // 生成唯一ID (简单使用当前时间戳作为ID)
            int todo_id = static_cast<int>(current_time);
            
            if(strcmp(option, "only one") == 0){
                // 创建单次闹钟
                std::shared_ptr<Todo_t> todo = std::make_shared<Todo_t>(tm_cur, aim_hour, 
                    aim_min, TODO_ONCE, alert_time_str);
                todo->id = todo_id;
                LOGI("lvgl_callback", "todo->remain_time: %lld", todo->remain_time);
                app->_todo_list.push_back(todo);
                if(app->getScreen() != NULL){
                    AlarmPanel* panel = new AlarmPanel(app->getScreen(), todo, app);
                    app->_alarm_list->addItem(*panel);
                    
                }
                
            }else if(strcmp(option, "every day") == 0){
                // 创建每日闹钟
                std::shared_ptr<Todo_t> todo = std::make_shared<Todo_t>(tm_cur, aim_hour, 
                    aim_min, TODO_EVEVRYDAY, alert_time_str);
                todo->id = todo_id;
                app->_todo_list.push_back(todo);
                if(app->getScreen() != NULL){
                    AlarmPanel* panel = new AlarmPanel(app->getScreen(), todo, app);
                    app->_alarm_list->addItem(*panel);
                }
                
            }else if(strcmp(option, "monday to friday") == 0){
                // 创建工作日闹钟
                std::shared_ptr<Todo_t> todo = std::make_shared<Todo_t>(tm_cur, aim_hour, 
                    aim_min, TODO_MONDAY_TO_FRIDAY, alert_time_str);
                todo->id = todo_id;
                app->_todo_list.push_back(todo);
                if(app->getScreen() != NULL){
                    AlarmPanel* panel = new AlarmPanel(app->getScreen(), todo, app);
                    app->_alarm_list->addItem(*panel);
                }
            }
            
            // 按剩余时间排序闹钟列表
            std::sort(app->_todo_list.begin(), app->_todo_list.end(), 
                [](const std::shared_ptr<Todo_t>& a, const std::shared_ptr<Todo_t>& b) {
                    return a->remain_time < b->remain_time;
                });
            
            // 保存配置
            app->save_config_to_nvs();
            app->_alarm_list->slide(SlideListContainer::Direction::InFromRight);
            app->_alarm_list->enableSwipe(true);
            lv_obj_add_flag(ui_clock_set_setting, LV_OBJ_FLAG_HIDDEN);
            LOGI("lvgl_callback", "on_set_alert: ui_clock_set_setting hidden");
        }

        void AlarmApp::save_config_to_nvs()
        {
            // 保存配置

        }

        void AlarmApp::load_config_from_nvs()
        {
            // 加载配置

        }
        
        void AlarmApp::onSetup()
        {
            setTaskName("AlarmApp");
            setScreen(ui_alarm_screen);
            setAllowBgRunning(false);
            setAppIcon((void *)&ICON_DEFAULT);
        }

        void AlarmApp::onCreate()
        {
            _framework->enableLowPowerMode();
            update = &_framework->getHAL();
            rtc = &update->bm8563;
            system_task = &_framework->getSystemTask();
            _todo_list.reserve(10);

            load_config_from_nvs();
            lv_obj_add_event_cb(ui_add_clock, on_add_alarm, LV_EVENT_CLICKED, this);
            lv_obj_add_event_cb(ui_But_confirm, on_set_alert, LV_EVENT_CLICKED, this);
            printf("[%s] onCreate\n", getAppName().c_str());
        }
        void AlarmApp::onResume()
        {
            _framework->enableLowPowerMode();
            printf("[%s] onResume\n", getAppName().c_str());
        }

        void AlarmApp::taskLoop()
        {
            LAMINATEPIE::DataTime_t current_time;
            rtc->getTime(current_time);

            int64_t min_remain_time = INT64_MAX;
            
            for (auto& todo : _todo_list) {
                bool was_active = todo->is_active_;
                LOGI("AlarmApp", "current_time: %lld", current_time.TotalSeconds());
                LOGI("AlarmPanel", "todo->is_active_: %d, id: %d", todo->is_active_, todo->id);
                if(todo->is_active_){
                    todo->update_remain_time(current_time);
                    LOGI("AlarmApp", "todo->remain_time: %lld", todo->remain_time);
                
                    if (todo->remain_time > 0 && todo->remain_time < min_remain_time) {
                        min_remain_time = todo->remain_time;
                    }
                }
                if (was_active && !todo->is_active_) {
                    updatePanelUI(*todo);
                }
            }


            
            // 只有存在激活的闹钟(min_remain_time不等于初始值)才设置唤醒闹钟
            if (min_remain_time != INT64_MAX) {
                _framework->getSystemTask().set_custom_wakeup_alarm(min_remain_time);
                LOGI("AlarmApp", "下一个闹钟剩余时间: %lld 秒", min_remain_time);
            }

            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        void AlarmApp::onRunningBG()
        {


        }
        void AlarmApp::onPause()
        {
            printf("[%s] onPause\n", getAppName().c_str());
            update->unselect(ENV_ID);
        }

        void AlarmApp::onDestroy()
        {
            printf("[%s] onDestroy\n", getAppName().c_str());

            update->unselect(ENV_ID);
        }

        void AlarmApp::updatePanelUI(Todo_t &todo){
            for (size_t i = 0; i < _alarm_list->itemCount(); i++) {
                Panel& panel = _alarm_list->getItemAt(i);
                AlarmPanel* alarm_panel = static_cast<AlarmPanel*>(&panel);
                if (alarm_panel) {
                    // 如果Todo不活跃，更新Switch状态
                    alarm_panel->updateSwitchState(todo.is_active_);
                }
            }           
        }
    }
}