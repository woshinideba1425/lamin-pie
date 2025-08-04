/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string>
#include <cmath>
#include "laminpie_gui_internal.h"
#include "laminpie_log.hpp"
#if !LAMINPIE_LVGL_OBJECT_ENABLE_DEBUG_LOG
#   define LAMINPIE_LVGL_UTILS_DISABLE_DEBUG_LOG
#endif
#include "private/laminpie_lv_utils.hpp"
#include "laminpie_lv_object.hpp"

namespace laminpie::gui {

LvObject::LvObject(lv_obj_t *p, bool is_auto_delete):
    _is_auto_delete(is_auto_delete),
    _native_handle(p)
{
    LOG_TRACE_GUARD_WITH_THIS();

    CheckFalseExit((p != nullptr) && checkLvObjIsValid(p), "Invalid object pointer(@0x%p)", p);

    lv_obj_add_event_cb(p, [](lv_event_t *e) {
        // ESP_UTILS_LOG_TRACE_ENTER();

        auto *obj = static_cast<LvObject *>(lv_event_get_user_data(e));
        CheckNullExit(obj, "Invalid user data");

        obj->_native_handle = nullptr;

        // ESP_UTILS_LOG_TRACE_EXIT();
    }, LV_EVENT_DELETE, this);

}

LvObject::~LvObject()
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();

    if (_is_auto_delete && isValid()) {
        lv_obj_delete(_native_handle);
    }

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
}

bool LvObject::setStyle(lv_style_t *style)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: style(0x%p)", style);

    CheckFalseReturn(isValid(), false, "Invalid object");
    CheckNullAndReturn(style, false, "Invalid style");

    lv_obj_add_style(_native_handle, style, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::removeStyle(lv_style_t *style)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: style(0x%p)", style);

    CheckFalseReturn(isValid(), false, "Invalid object");

    if (style == nullptr) {
        lv_obj_remove_style_all(_native_handle);
    } else {
        lv_obj_remove_style(_native_handle, style, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    }

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setStyleAttribute(StyleWidthItem width_type, int width)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: width_type(%d), width(%d)", width_type, width);

    CheckFalseReturn(isValid(), false, "Invalid object");

    switch (width_type) {
    case STYLE_WIDTH_ITEM_BORDER:
        lv_obj_set_style_border_width(_native_handle, width, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
        break;
    case STYLE_WIDTH_ITEM_OUTLINE:
        lv_obj_set_style_outline_width(_native_handle, width, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
        break;
    default:
        break;
    }

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setStyleAttribute(const StyleSize &size)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: size(width=%d, height=%d, radius=%d)", size.width, size.height, size.radius);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_set_style_width(_native_handle, size.width, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    lv_obj_set_style_height(_native_handle, size.height, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    lv_obj_set_style_radius(_native_handle, size.radius, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setStyleAttribute(const StyleFont &font)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: font(font_resource=0x%p)", font.font_resource);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_set_style_text_font(_native_handle, (lv_font_t *)font.font_resource, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setStyleAttribute(const StyleAlign &align)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: align(type=%d, offset_x=%d, offset_y=%d)", align.type, align.offset_x, align.offset_y);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_align(_native_handle, toLvAlign(align.type), align.offset_x, align.offset_y);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setStyleAttribute(const StyleLayoutFlex &layout)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: layout(flow=%d, main_place=%d, cross_place=%d, track_place=%d)",
                   layout.flow, layout.main_place, layout.cross_place, layout.track_place);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_set_style_layout(_native_handle, LV_LAYOUT_FLEX, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    lv_obj_set_style_flex_flow(_native_handle, toLvFlexFlow(layout.flow), (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    lv_obj_set_style_flex_main_place(
        _native_handle, toLvFlexAlign(layout.main_place), (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT
    );
    lv_obj_set_style_flex_cross_place(
        _native_handle, toLvFlexAlign(layout.cross_place), (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT
    );
    lv_obj_set_style_flex_track_place(
        _native_handle, toLvFlexAlign(layout.track_place), (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT
    );

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setStyleAttribute(const StyleGap &gap)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: gap(left=%d, right=%d, top=%d, bottom=%d, row=%d, column=%d)",
                   gap.left, gap.right, gap.top, gap.bottom, gap.row, gap.column);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_set_style_pad_left(_native_handle, gap.left, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(_native_handle, gap.right, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(_native_handle, gap.top, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(_native_handle, gap.bottom, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(_native_handle, gap.row, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(_native_handle, gap.column, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setStyleAttribute(
    StyleColorItem item, const StyleColor &color
)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: item(%d), color(color=0x%x, opacity=%d)", item, color.color, color.opacity);

    CheckFalseReturn(isValid(), false, "Invalid object");

    switch (item) {
    case STYLE_COLOR_ITEM_BACKGROUND:
        lv_obj_set_style_bg_color(_native_handle, toLvColor(color.color), (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(_native_handle, color.opacity, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
        break;
    case STYLE_COLOR_ITEM_TEXT:
        lv_obj_set_style_text_color(_native_handle, toLvColor(color.color), (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(_native_handle, color.opacity, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
        break;
    case STYLE_COLOR_ITEM_BORDER:
        lv_obj_set_style_border_color(_native_handle, toLvColor(color.color), (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(_native_handle, color.opacity, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
        break;
    default:
        break;
    }

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setStyleAttribute(const StyleImage &image)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: image(resource=0x%p, recolor.color=0x%x, recolor.opacity=%d)",
                   image.resource, image.recolor.color, image.recolor.opacity);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_set_style_bg_img_src(_native_handle, image.resource, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor(_native_handle, lv_color_hex(image.recolor.color), (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(_native_handle, image.recolor.opacity, (int)LV_PART_MAIN | (int)LV_STATE_DEFAULT);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setStyleAttribute(LvObject &target, const StyleAlign &align)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG(
        "Param: target(0x%p), align(type=%d, offset_x=%d, offset_y=%d)",
        &target, align.type, align.offset_x, align.offset_y
    );

    CheckFalseReturn(isValid(), false, "Invalid object");
    CheckFalseReturn(target.isValid(), false, "Invalid target");

    lv_obj_align_to(
        _native_handle, target._native_handle, toLvAlign(align.type), align.offset_x, align.offset_y
    );
    lv_obj_update_layout(_native_handle);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setStyleAttribute(StyleFlag flags, bool enable)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: flags(%d), enable(%d)", flags, enable);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_flag_t lv_flag = toLvFlags(flags);
    if (lv_flag) {
        if (enable) {
            lv_obj_add_flag(_native_handle, lv_flag);
        } else {
            lv_obj_remove_flag(_native_handle, lv_flag);
        }
    }

    if (flags | STYLE_FLAG_CLIP_CORNER) {
        lv_obj_set_style_clip_corner(
            _native_handle, enable, static_cast<int>(LV_PART_MAIN) | static_cast<int>(LV_STATE_DEFAULT)
        );
        if (enable) {
            lv_obj_remove_flag(_native_handle, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        } else {
            lv_obj_add_flag(_native_handle, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        }
    }

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setX(int x)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: x(%d)", x);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_set_x(_native_handle, x);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::setY(int y)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: y(%d)", y);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_set_y(_native_handle, y);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::scrollY_To(int y, bool is_animated)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: y(%d), is_animated(%d)", y, is_animated);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_scroll_to_y(_native_handle, y, is_animated ? LV_ANIM_ON : LV_ANIM_OFF);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::moveForeground(void)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_move_foreground(_native_handle);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::moveBackground(void)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_move_background(_native_handle);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::addEventCallback(lv_event_cb_t cb, lv_event_code_t code, void *user_data)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: cb(0x%p), code(%d), user_data(0x%p)", cb, code, user_data);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_add_event_cb(_native_handle, cb, code, user_data);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::delEventCallback(lv_event_cb_t cb, lv_event_code_t code, void *user_data)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: cb(0x%p), code(%d), user_data(0x%p)", cb, code, user_data);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_remove_event_cb_with_user_data(_native_handle, cb, user_data);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::delEventCallback(lv_event_cb_t cb)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: cb(0x%p)", cb);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_remove_event_cb(_native_handle, cb);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::hasState(lv_state_t state) const
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: state(%d)", state);

    CheckFalseReturn(isValid(), false, "Invalid object");

    bool result = lv_obj_has_state(_native_handle, state);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return result;
}

bool LvObject::hasFlags(StyleFlag flags) const
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: flags(%d)", flags);

    CheckFalseReturn(isValid(), false, "Invalid object");

    bool result = true;
    lv_obj_flag_t lv_flag = toLvFlags(flags);
    if (lv_flag) {
        result = lv_obj_has_flag(_native_handle, lv_flag);
    }

    if (flags | STYLE_FLAG_CLIP_CORNER) {
        result &= lv_obj_get_style_clip_corner(_native_handle, static_cast<int>(LV_PART_MAIN) | static_cast<int>(LV_STATE_DEFAULT));
    }

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return result;
}

bool LvObject::getX(int &x) const
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: x(0x%p)", &x);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_update_layout(_native_handle);
    x = lv_obj_get_x(_native_handle);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::getY(int &y) const
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: y(0x%p)", &y);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_update_layout(_native_handle);
    y = lv_obj_get_y(_native_handle);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool LvObject::getArea(lv_area_t &area) const
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: area(0x%p)", &area);

    CheckFalseReturn(isValid(), false, "Invalid object");

    lv_obj_update_layout(_native_handle);
    lv_obj_get_coords(_native_handle, &area);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

} // namespace laminpie::gui
