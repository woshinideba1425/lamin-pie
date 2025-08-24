#pragma once

#include "laminpie_app_base.hpp"
#include "laminpie_app_navigation.hpp"
#include "vector"
#include <unordered_map>  // 使用哈希表来存储应用名称与句柄的映射
#include "laminpie_event_dispatcher.hpp"

namespace laminpie::system::app {
    class Laminpie_App_Register {
        private:
            typedef struct {
                lv_draw_buf_t *image_resource;
            } Laminpie_AppSnapshot_t;

            mutable uint32_t _app_free_id{Laminpie_App_ID_Min};
            Laminpie_App_Base *_active_app{nullptr};
            std::unordered_map <int, Laminpie_App_Base *> _id_installed_app_map;
            std::unordered_map <int, Laminpie_App_Base *> _id_running_app_map;
            
        protected:
            std::unordered_map <int, std::shared_ptr<Laminpie_AppSnapshot_t>> _id_app_snapshot_map;

            framework::Laminpie_Core_Framework *_framework;
            Laminpie_App_Navigation* _navigation;

        public:
            friend class Laminpie_Core_Framework;

            Laminpie_App_Register(framework::Laminpie_Core_Framework *framework);
            ~Laminpie_App_Register() = default;

            int Install(Laminpie_App_Base* app, void* userData = nullptr);
            int Install(Laminpie_App_Base &app);
            bool InstallSystemApp(Laminpie_App_Base* app);

            bool Uninstall(Laminpie_App_Base* app);
            bool Uninstall(Laminpie_App_Base& app);
            bool Uninstall(int id);            

            uint16_t GetInstalledAppCount();
            int GetAppFreeId();
            Laminpie_App_Base *GetInstalledApp(int id);

            bool IsSystemApp(Laminpie_App_Base* app) const;
    };

    

}