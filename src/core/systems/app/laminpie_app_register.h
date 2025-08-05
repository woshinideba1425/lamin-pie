#include "laminpie_app_base.hpp"
#include "vector"
#include <unordered_map>  // 使用哈希表来存储应用名称与句柄的映射
#include "laminpie_event_dispatcher.hpp"

namespace laminpie::system::app {


    struct APPList_t {
        Laminpie_App_Base* app = nullptr;
        int id = -1;
        bool isSystemApp = false;  // 添加系统应用标识
    };

    class APP_Register {
        private:
            int _id;
            typedef struct {
                lv_draw_buf_t *image_resource;
            } Laminpie_AppSnapshot_t;

            mutable uint32_t _app_free_id{Laminpie_App_ID_Min};
            Laminpie_App_Base *_active_app{nullptr};
            std::unordered_map <int, Laminpie_App_Base *> _id_installed_app_map;
            std::unordered_map <int, Laminpie_App_Base *> _id_running_app_map;
            
        protected:
            std::vector<APPList_t> _app_list;
            std::unordered_map <int, std::shared_ptr<Laminpie_AppSnapshot_t>> _id_app_snapshot_map;

        public:
            APP_Register() : _id(0) {}
            ~APP_Register() = default;


            /**
             * @brief Install App into register 
             * 
             * @param app App pointer 
             * @param database System database
             * @param userData Custom user data
             * @return int App ID
             */
            int Install(Laminpie_App_Base* app, void* userData = nullptr);
            
            /**
             * @brief Register app as system app
             * 
             * @param app App pointer to register as system app
             * @return bool Success status
             */
            bool RegisterSystemApp(Laminpie_App_Base* app);
            
            bool Uninstall(Laminpie_App_Base* app);
            inline bool Uninstall(int id) { return Uninstall(GetApp(id)); }
            inline bool Uninstall(const char* name) { return Uninstall(GetApp(name)); }


            /* Basic API */
            inline std::vector<APPList_t> GetAppList() {
                SYSTEM_APP_LOG_INFO("Total installed apps: %d", _app_list.size());

                for (const auto& appEntry : _app_list) {
                    ESP_LOGI("APP_Register", "App ID: %d, App Name: %s, System App: %s", 
                                appEntry.id, 
                                appEntry.app->getName().c_str(), 
                                appEntry.isSystemApp ? "Yes" : "No");
                }
                    return _app_list; 
            }
            inline uint16_t GetRegistedAppCount() { return (uint16_t)_app_list.size(); }
            uint8_t GetRunningAppCount(void) const { return _id_running_app_map.size(); }
            int GetAppFreeId(void) const { return _app_free_id++; }
            int GetRunningAppIndexById(int id);
            int GetRunningAppIndexByApp(Laminpie_App_Base* app);
            int GetAppId(Laminpie_App_Base* app);
            int GetAppId(const char* name);

            Laminpie_App_Base *GetActiveApp(void) const { return _active_app; }
            void ResetActiveApp(void);
            Laminpie_App_Base* GetApp(int id);
            Laminpie_App_Base* GetApp(const char* name);
            bool IsSystemApp(Laminpie_App_Base* app) const;
    };

    

}