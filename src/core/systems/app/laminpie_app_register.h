#include "laminpie_app_base.hpp"
#include "vector"
#include <unordered_map>  // 使用哈希表来存储应用名称与句柄的映射

namespace laminpie::system::app {


    struct APPList_t {
        Laminpie_App_Base* app = nullptr;
        int id = -1;
        bool isSystemApp = false;  // 添加系统应用标识
    };


    class APP_Register {
        private:
            int _id;
            
        protected:
            std::vector<APPList_t> _app_list;

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
            int install(Laminpie_App_Base* app, void* userData = nullptr);
            
            /**
             * @brief Register app as system app
             * 
             * @param app App pointer to register as system app
             * @return bool Success status
             */
            bool registerSystemApp(Laminpie_App_Base* app);
            
            bool uninstall(Laminpie_App_Base* app);
            inline bool uninstall(int id) { return uninstall(getApp(id)); }
            inline bool uninstall(const char* name) { return uninstall(getApp(name)); }


            /* Basic API */
            inline std::vector<APPList_t> getAppList() {
                SYSTEM_APP_LOG_INFO("APP_Register", "Total installed apps: %d", _app_list.size());
    
                for (const auto& appEntry : _app_list) {
                    ESP_LOGI("APP_Register", "App ID: %d, App Name: %s, System App: %s", 
                             appEntry.id, 
                             appEntry.app->getAppName().c_str(), 
                             appEntry.isSystemApp ? "Yes" : "No");
                }
                 return _app_list; 
                 }
            inline uint16_t getAppNum() { return (uint16_t)_app_list.size(); }
            int getAppID(Laminpie_App_Base* app);
            int getAppID(const char* name);
            Laminpie_App_Base* getApp(int id);
            Laminpie_App_Base* getApp(const char* name);
            bool isSystemApp(Laminpie_App_Base* app) const;
    };

    

}