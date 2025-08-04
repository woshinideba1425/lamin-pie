/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "laminpie_gui_internal.h"
#if !LAMINPIE_LVGL_DISPLAY_ENABLE_DEBUG_LOG
#   define LAMINPIE_LVGL_UTILS_DISABLE_DEBUG_LOG
#endif
#include "private/laminpie_lv_utils.hpp"
#include "laminpie_lv_display.hpp"

namespace laminpie::gui {

LvDisplay::LvDisplay(lv_disp_t *display, const LvDisplayData *data):
    _data(*data),
    _display(display)
{
}

LvDisplay::~LvDisplay()
{
}

bool LvDisplay::setTouchDevice(lv_indev_t *touch)
{
    CheckNullAndReturn(touch, false, "Invalid touch device");

    _touch = touch;

    return true;
}

bool LvDisplay::fontCalibrateMethod(StyleFont &target, const StyleSize *parent) const
{
    uint8_t size_px = 0;
    const lv_font_t *font_resource = (const lv_font_t *)target.font_resource;

    if (target.flags.enable_height) {
        goto process_height;
    }

    // Size
    CheckValueAndReturn(
        target.size_px, StyleFont::FONT_SIZE_MIN, StyleFont::FONT_SIZE_MAX, false, "Invalid size"
    );
    // Font description
    if (font_resource == nullptr) {
        font_resource = getFontBySize(target.size_px);
        CheckNullAndReturn(font_resource, false, "Get default font failed");
        target.font_resource = font_resource;
        target.height = font_resource->line_height;
    }
    goto end;

process_height:
    // Height
    if (target.flags.enable_height_percent) {
        CheckNullAndReturn(parent, false, "Invalid parent size");
        CheckValueAndReturn(target.height_percent, 1, 100, false, "Invalid height percent");
        target.height = (parent->height * target.height_percent) / 100;
    } else if (parent != nullptr) {
        CheckValueAndReturn(target.height, 1, parent->height, false, "Invalid height");
    }

    // Font description & size
    font_resource = getFontByHeight(target.height, &size_px);
    CheckNullAndReturn(font_resource, false, "Get default font failed");
    target.font_resource = font_resource;
    target.size_px = size_px;

end:
    return true;
}

bool LvDisplay::updateByNewData(void)
{
    LAMINPIE_LVGL_LOG_DEBUG("Update lvgl display by new data");

    // Debug styles
    for (size_t i = 0; i < _debug_styles.size(); i++) {
        lv_style_set_outline_width(&_debug_styles[i], _data.debug_styles[i].outline_width);
        lv_style_set_outline_color(
            &_debug_styles[i], lv_color_hex(_data.debug_styles[i].outline_color.color)
        );
        lv_style_set_outline_opa(&_debug_styles[i], _data.debug_styles[i].outline_color.opacity);
    }

    return true;
}

const lv_font_t *LvDisplay::getFontBySize(uint8_t size_px) const
{
    CheckValueAndReturn(
        static_cast<uint8_t>(size_px), StyleFont::FONT_SIZE_MIN, StyleFont::FONT_SIZE_MAX, nullptr, "Invalid size"
    );

    auto it = _size_font_map.find(size_px);
    CheckFalseReturn(it != _size_font_map.end(), nullptr, "Font size(%d) is not found", size_px);

    return it->second;
}

const lv_font_t *LvDisplay::getFontByHeight(uint8_t height, uint8_t *size_px) const
{
    uint8_t ret_size = 0;

    auto lower = _height_font_map.lower_bound(height);
    if ((lower->first != height) && (lower != _height_font_map.begin())) {
        lower--;
    }
    CheckFalseReturn(lower != _height_font_map.end(), nullptr, "Font height(%d) is not found", height);

    if (size_px != nullptr) {
        for (auto &it : _size_font_map) {
            if (it.second == lower->second) {
                ret_size = it.first;
                break;
            }
        }
        CheckFalseReturn(ret_size != 0, nullptr, "Font size is not found");
        *size_px = ret_size;
    }

    return lower->second;
}

} // namespace laminpie::gui
