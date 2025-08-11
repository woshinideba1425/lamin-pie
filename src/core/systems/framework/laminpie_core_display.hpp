#pragma once

#include <map>
#include <array>
#include "lvgl.h"
#include "lvgl/laminpie_lv.hpp"
#include "laminpie_app_base.hpp"
#include "laminpie_app_manager.hpp"

#define LAMINPIE_CORE_DISPLAY_DEFAULT_FONTS_NUM_MAX  \
    ((LAMINPIE_STYLE_FONT_SIZE_MAX - LAMINPIE_STYLE_FONT_SIZE_MIN) / 2 + 1)
#define LAMINPIE_CORE_DISPLAY_DEBUG_STYLES_NUM   6

struct Laminpie_CoreDisplayFonts {
    uint8_t fonts_num = 0;
    laminpie::gui::StyleFont fonts[laminpie::gui::StyleFont::FONT_SIZE_NUM]{};
};

struct Laminpie_CoreDisplayDebugStyles {
    uint8_t outline_width{0};
    laminpie::gui::StyleColor outline_color{};
};

struct Laminpie_CoreDisplayData {
    struct {
        laminpie::gui::StyleColor color;
        laminpie::gui::StyleImage wallpaper_image_resource;
    } background;
    struct {
        uint8_t default_fonts_num;
        laminpie::gui::StyleFont default_fonts[laminpie::gui::StyleFont::FONT_SIZE_NUM];
    } text;
    struct {
        struct {
            uint8_t outline_width;
            laminpie::gui::StyleColor outline_color;
        } styles[LAMINPIE_CORE_DISPLAY_DEBUG_STYLES_NUM];
    } container;
    // std::array<Laminpie_CoreDisplayFonts, laminpie::gui::STYLE_FONT_TYPE_MAX> fonts{};
    // std::array<Laminpie_CoreDisplayDebugStyles, LAMINPIE_CORE_DISPLAY_DEBUG_STYLES_NUM> debug_styles{};
};

namespace laminpie::system::framework {
class Laminpie_Framework;
class Laminpie_CoreDisplay {
    friend class Laminpie_App_Manager;
    friend class Laminpie_Framework;

    Laminpie_CoreDisplay(Laminpie_Framework &core, const Laminpie_CoreDisplayData &data);
    ~Laminpie_CoreDisplay();

    bool ShowContainerBorder(void);
    bool HideContainerBorder(void);

    bool CheckCoreInitialized(void) const       { return (_main_screen != nullptr); }
    lv_obj_t *GetMainScreen(void) const         { return _main_screen->getNativeHandle(); }
    lv_obj_t *GetSystemScreen(void) const       { return _system_screen->getNativeHandle(); }
    lv_obj_t *GetMainScreenObject(void) const   { return _main_screen_obj->getNativeHandle(); }
    lv_obj_t *GetSystemScreenObject(void) const { return _system_screen_obj->getNativeHandle(); }
    const gui::LvObject *GetMainScreenPtr(void) const       { return _main_screen.get(); }
    const gui::LvObject *GetSystemScreenPtr(void) const     { return _system_screen.get(); }
    const gui::LvObject *GetMainScreenObjectPtr(void) const       { return _main_screen_obj.get(); }
    const gui::LvObject *GetSystemScreenObjectPtr(void) const     { return _system_screen_obj.get(); }
    lv_style_t *GetCoreContainerStyle(void);

    bool CalibrateCoreObjectSize(
        const gui::StyleSize &parent, gui::StyleSize &target
    ) const;
    bool CalibrateCoreObjectSize(
        const gui::StyleSize &parent, gui::StyleSize &target,
        bool check_width, bool check_height
    ) const;
    bool CalibrateCoreObjectSize(
        const gui::StyleSize &parent, gui::StyleSize &target, bool allow_zero
    ) const;
    bool CalibrateCoreFont(const gui::StyleSize *parent, gui::StyleFont &target) const;
    bool CalibrateCoreIconImage(const gui::StyleImage &target) const;

private:
    virtual bool ProcessAppInstall(app::Laminpie_App_Base *app) = 0;
    virtual bool ProcessAppUninstall(app::Laminpie_App_Base *app) = 0;
    virtual bool ProcessAppCreate(app::Laminpie_App_Base *app) = 0;
    virtual bool ProcessAppResume(app::Laminpie_App_Base *app) { return true; }
    virtual bool ProcessAppPause(app::Laminpie_App_Base *app)  { return true; }
    virtual bool ProcessAppDestroy(app::Laminpie_App_Base *app){ return true; }
    virtual bool ProcessAppUpdate(app::Laminpie_App_Base *app) { return true; }
    virtual bool ProcessMainScreenLoad(void);
    virtual bool GetAppVisualArea(app::Laminpie_App_Base *app, lv_area_t &app_visual_area) const { return true; }

    bool BeginCore(void);
    bool DelCore(void);
    bool UpdateByNewData(void);
    bool CalibrateCoreData(Laminpie_CoreDisplayData &data);
    void SaveLvScreens(void);
    void LoadLvScreens(void);

    bool CalibrateStyleSizeInternal(gui::StyleSize &target) const;
    const lv_font_t *GetFontBySize(int size) const;
    const lv_font_t *GetFontByHeight(int height, int *size_px) const;

    lv_obj_t *_lv_main_screen = nullptr;
    lv_obj_t *_lv_system_screen = nullptr;

    gui::LvScreenUniquePtr _main_screen;
    gui::LvScreenUniquePtr _system_screen;
    gui::LvContainerUniquePtr _main_screen_obj;
    gui::LvContainerUniquePtr _system_screen_obj;

    uint8_t _container_style_index;
    std::array<lv_style_t, LAMINPIE_CORE_DISPLAY_DEBUG_STYLES_NUM> _container_styles;
    std::map<uint8_t, const lv_font_t *> _default_size_font_map;
    std::map<uint8_t, const lv_font_t *> _default_height_font_map;
    std::map<uint8_t, const lv_font_t *> _update_size_font_map;
    std::map<uint8_t, const lv_font_t *> _update_height_font_map;
};

}

#define LAMINPIE_CORE_HOME_DATA_DEFAULT_FONTS_NUM_MAX  LAMINPIE_CORE_DISPLAY_DEFAULT_FONTS_NUM_MAX
#define LAMINPIE_CORE_HOME_DATA_CONTAINER_STYLES_NUM   LAMINPIE_CORE_DISPLAY_DEBUG_STYLES_NUM
typedef laminpie::system::framework::Laminpie_CoreDisplay Laminpie_CoreHome;
typedef Laminpie_CoreDisplayData Laminpie_CoreHomeData_t;