#include "laminpie_core_display.hpp"
#include "laminpie_core_framework.hpp"
#include "laminpie_log.hpp"
#include "laminpie_system_internal.h"
#include "style/laminpie_gui_style.hpp"

using namespace laminpie::gui;
using namespace laminpie::system::framework;

Laminpie_CoreDisplay::Laminpie_CoreDisplay(Laminpie_Core_Framework &core, const Laminpie_CoreDisplayData &data):
    _core(core),
    _core_data(data),
    _main_screen(nullptr),
    _system_screen(nullptr),
    _main_screen_obj(nullptr),
    _system_screen_obj(nullptr),
    _container_style_index(0)
{
}

Laminpie_CoreDisplay::~Laminpie_CoreDisplay()
{
    SYSTEM_CORE_LOG_DEBUG("Destroy(@0x%p)", this);
    if(!DelCore()){
        SYSTEM_CORE_LOG_ERROR("Delete failed");
    }
}

bool Laminpie_CoreDisplay::ShowContainerBorder(void)
{
    SYSTEM_CORE_LOG_DEBUG("Show container border");
    utils::CheckFalseReturn(CheckCoreInitialized(), false, "Not initialized");

    for (size_t i = 0; i < _container_styles.size(); i++) {
        lv_style_set_outline_width(&_container_styles[i], _core_data.container.styles[i].outline_width);
    }

    return true;
}

bool Laminpie_CoreDisplay::HideContainerBorder(void)
{
    SYSTEM_CORE_LOG_DEBUG("Hide container border");
    utils::CheckFalseReturn(CheckCoreInitialized(), false, "Not initialized");

    for (auto &style : _container_styles) {
        lv_style_set_outline_width(&style, 0);
    }

    return true;
}

lv_style_t *Laminpie_CoreDisplay::GetCoreContainerStyle(void)
{
    int index = _container_style_index++;
    if (_container_style_index > (_container_styles.size() - 1)) {
        _container_style_index = 0;
    }

    return &_container_styles[index];
}

bool Laminpie_CoreDisplay::CalibrateCoreObjectSize(const StyleSize &parent, StyleSize &target) const
{
    utils::CheckFalseReturn(target.calibrate(parent), false, "Calibrate failed");
    utils::CheckFalseReturn(CalibrateStyleSizeInternal(target), false, "Calibrate internal failed");

    return true;
}

bool Laminpie_CoreDisplay::CalibrateCoreObjectSize(
    const StyleSize &parent, StyleSize &target, bool check_width, bool check_height
) const
{
    utils::CheckFalseReturn(target.calibrate(parent, check_width, check_height), false, "Calibrate failed");
    utils::CheckFalseReturn(CalibrateStyleSizeInternal(target), false, "Calibrate internal failed");

    return true;
}

bool Laminpie_CoreDisplay::CalibrateCoreObjectSize(
    const StyleSize &parent, StyleSize &target, bool allow_zero
) const
{
    utils::CheckFalseReturn(target.calibrate(parent, allow_zero), false, "Calibrate failed");
    utils::CheckFalseReturn(CalibrateStyleSizeInternal(target), false, "Calibrate internal failed");

    return true;
}

bool Laminpie_CoreDisplay::CalibrateCoreFont(const StyleSize *parent, StyleFont &target) const
{
    utils::CheckFalseReturn(target.calibrate(
                                     parent,
    [&](int size_px) {
        return (const void *)GetFontBySize(size_px);
    },
    [&](int height, int *size_px) {
        return (const void *)GetFontByHeight(height, size_px);
    },
    [&](const void *font) {
        return ((const lv_font_t *)font)->line_height;
    }
                                 ), false, "Calibrate failed");

    return true;
}

bool Laminpie_CoreDisplay::CalibrateCoreIconImage(const StyleImage &target) const
{
    utils::CheckFalseReturn(target.calibrate(), false, "Calibrate failed");

    return true;
}

bool Laminpie_CoreDisplay::ProcessMainScreenLoad(void)
{
    utils::CheckFalseReturn(CheckCoreInitialized(), false, "Not initialized");
    utils::CheckFalseReturn(_main_screen->isValid(), false, "Invalid main screen");

    lv_scr_load(_main_screen->getNativeHandle());

    return true;
}

bool Laminpie_CoreDisplay::BeginCore(void)
{
    lv_display_t *display = _core.GetDisplayDevice();

    SYSTEM_CORE_LOG_DEBUG("Begin(0x%p)", this);
    utils::CheckFalseReturn(!CheckCoreInitialized(), false, "Already initialized");
    utils::CheckFalseReturn(display == nullptr, false, "Invalid display device");

    SaveLvScreens();

    /* Create objects */
    // Main screen
    _main_screen = std::make_unique<LvScreen>();
    utils::CheckFalseReturn(_main_screen->isValid(), false, "Invalid lvgl current screen");
    _main_screen_obj = std::make_unique<LvContainer>(_main_screen.get());
    utils::CheckFalseReturn(_main_screen_obj->isValid(), false, "Create main screen failed");
    // System screen
    _system_screen = std::make_unique<LvScreen>();
    utils::CheckFalseReturn(_system_screen->isValid(), false, "Invalid lvgl top screen");
    _system_screen_obj = std::make_unique<LvContainer>(_system_screen.get());
    utils::CheckFalseReturn(_system_screen_obj->isValid(), false, "Create system screen failed");

    /* Setup objects */
    // Container styles
    for (auto &style : _container_styles) {
        lv_style_init(&style);
        lv_style_set_size(&style, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_style_set_radius(&style, 0);
        lv_style_set_border_width(&style, 0);
        lv_style_set_pad_all(&style, 0);
        lv_style_set_pad_gap(&style, 0);
        lv_style_set_bg_opa(&style, LV_OPA_TRANSP);
        lv_style_set_outline_width(&style, 0);
    }
    // Main screen
    lv_obj_align(_main_screen_obj->getNativeHandle(), LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(_main_screen_obj->getNativeHandle(), LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(_main_screen_obj->getNativeHandle(), GetCoreContainerStyle(), 0);
    // System screen
    lv_obj_align(_system_screen_obj->getNativeHandle(), LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(_system_screen_obj->getNativeHandle(), LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(_system_screen_obj->getNativeHandle(), GetCoreContainerStyle(), 0);

    // Update object style
    utils::CheckFalseReturn(UpdateByNewData(), false, "Update object style failed");
    utils::CheckFalseReturn(HideContainerBorder(), false, "Hide container border failed");

    display->sys_layer = _system_screen->getNativeHandle();
    lv_screen_load(_main_screen->getNativeHandle());

    return true;
}

bool Laminpie_CoreDisplay::DelCore(void)
{
    SYSTEM_CORE_LOG_DEBUG("Delete(0x%p)", this);

    if (!CheckCoreInitialized()) {
        return true;
    }

    LoadLvScreens();

    for (auto &style : _container_styles) {
        lv_style_reset(&style);
    }
    _main_screen_obj = nullptr;
    _system_screen_obj = nullptr;
    _main_screen = nullptr;
    _system_screen = nullptr;
    _container_style_index = 0;
    _default_size_font_map.clear();
    _default_height_font_map.clear();
    _update_size_font_map.clear();
    _update_height_font_map.clear();

    return true;
}

bool Laminpie_CoreDisplay::UpdateByNewData(void)
{
    const StyleSize &screen_size = _core.GetCoreData().screen_size;

    SYSTEM_CORE_LOG_DEBUG("Update core home by new data");

    utils::CheckFalseReturn(CheckCoreInitialized(), false, "Not initialized");

    utils::CheckFalseReturn(
        _main_screen_obj->setStyleAttribute(screen_size), false, "Set main screen size failed"
    );
    utils::CheckFalseReturn(
        _main_screen_obj->setStyleAttribute(STYLE_FLAG_CLIP_CORNER, true), false, "Set system screen clip corner failed"
    );
    utils::CheckFalseReturn(
        _main_screen_obj->setStyleAttribute(STYLE_COLOR_ITEM_BACKGROUND, _core_data.background.color),
        false, "Set main screen background color failed"
    );
    if (_core_data.background.wallpaper_image_resource.resource != nullptr) {
        utils::CheckFalseReturn(_main_screen_obj->setStyleAttribute(
                                         _core_data.background.wallpaper_image_resource), false, "Set main screen wallpaper image failed"
                                    );
    }

    utils::CheckFalseReturn(
        _system_screen_obj->setStyleAttribute(screen_size), false, "Set system screen size failed"
    );
    utils::CheckFalseReturn(
        _system_screen_obj->setStyleAttribute(STYLE_FLAG_CLIP_CORNER, true), false, "Set system screen clip corner failed"
    );

    // Text
    _default_size_font_map = _update_size_font_map;

    // Container styles
    for (size_t i = 0; i < _container_styles.size(); i++) {
        lv_style_set_outline_width(&_container_styles[i], _core_data.container.styles[i].outline_width);
        lv_style_set_outline_color(&_container_styles[i],
                                   lv_color_hex(_core_data.container.styles[i].outline_color.color));
        lv_style_set_outline_opa(&_container_styles[i], _core_data.container.styles[i].outline_color.opacity);
    }

    return true;
}

bool Laminpie_CoreDisplay::CalibrateCoreData(Laminpie_CoreDisplayData &data)
{
    const lv_font_t *font_resource = nullptr;

    // Text
    _update_size_font_map.clear();
    _update_height_font_map.clear();
    for (int i = 0; i < data.text.default_fonts_num; i++) {
        utils::CheckValueAndReturn(data.text.default_fonts[i].size_px, StyleFont::FONT_SIZE_MIN,
                                     StyleFont::FONT_SIZE_MAX, false, "Invalid default font(%d) size", i);
        utils::CheckNullAndReturn(data.text.default_fonts[i].font_resource, false, "Invalid default font(%d) dsc", i);
        font_resource = (lv_font_t *)data.text.default_fonts[i].font_resource;
        // Save font for function ``
        _update_size_font_map[data.text.default_fonts[i].size_px] = font_resource;
        _update_height_font_map[font_resource->line_height] = font_resource;
    }
    // Check if all default fonts are set, if not, use internal fonts
    for (int i = StyleFont::FONT_SIZE_MIN; i <= StyleFont::FONT_SIZE_MAX; i += 2) {
        if (_update_size_font_map.find(i) == _update_size_font_map.end()) {
            SYSTEM_APP_LOG_WARN("Default font size(%d) is not found, try to use internal font instead", i);
            if (!esp_brookesia_core_utils_get_internal_font_by_size(i, &font_resource)) {
                continue;
            }
            _update_size_font_map[i] = font_resource;
            if (_update_height_font_map.find(font_resource->line_height) == _update_height_font_map.end()) {
                _update_height_font_map[font_resource->line_height] = font_resource;
            }
        }
    }

    return true;
}

void Laminpie_CoreDisplay::SaveLvScreens(void)
{
    auto display = _core.GetDisplayDevice();
    _lv_main_screen = lv_display_get_screen_active(display);
    _lv_system_screen = lv_display_get_layer_sys(display);
}

void Laminpie_CoreDisplay::LoadLvScreens(void)
{
    auto display = _core.GetDisplayDevice();
    display->sys_layer = _lv_system_screen;
    lv_scr_load(_lv_main_screen);
}

bool Laminpie_CoreDisplay::CalibrateStyleSizeInternal(StyleSize &target) const
{
    if (target.width == StyleSize::LENGTH_AUTO) {
        target.width = LV_SIZE_CONTENT;
    }
    if (target.height == StyleSize::LENGTH_AUTO) {
        target.height = LV_SIZE_CONTENT;
    }
    if (target.radius == StyleSize::RADIUS_CIRCLE) {
        target.radius = LV_RADIUS_CIRCLE;
    }

    return true;
}

const lv_font_t *Laminpie_CoreDisplay::GetFontBySize(int size_px) const
{
    utils::CheckValueAndReturn(size_px, StyleFont::FONT_SIZE_MIN, StyleFont::FONT_SIZE_MAX, nullptr, "Invalid size");

    auto it = _update_size_font_map.find(size_px);
    utils::CheckFalseReturn(it != _update_size_font_map.end(), nullptr, "Font size(%d) is not found", size_px);

    return it->second;
}

const lv_font_t *Laminpie_CoreDisplay::GetFontByHeight(int height, int *size_px) const
{
    int ret_size = 0;

    auto lower = _update_height_font_map.lower_bound(height);
    if ((lower->first != height) && (lower != _update_height_font_map.begin())) {
        lower--;
    }
    utils::CheckFalseReturn(lower != _update_height_font_map.end(), nullptr, "Font height(%d) is not found", height);

    if (size_px != nullptr) {
        for (auto &it : _update_size_font_map) {
            if (it.second == lower->second) {
                ret_size = it.first;
                break;
            }
        }
        utils::CheckFalseReturn(ret_size != 0, nullptr, "Font size is not found");
        *size_px = ret_size;
    }

    return lower->second;
}
