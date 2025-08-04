/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "laminpie_gui_internal.h"
#if !LAMINPIE_LVGL_CONTAINER_ENABLE_DEBUG_LOG
#   define LAMINPIE_LVGL_UTILS_DISABLE_DEBUG_LOG
#endif
#include "private/laminpie_lv_utils.hpp"
#include "laminpie_lv_container.hpp"

namespace laminpie::gui {

LvContainer::LvContainer(const LvObject *parent):
    LvObject((parent != nullptr) ? lv_obj_create(parent->getNativeHandle()) : nullptr)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: parent(0x%p)", parent);

    CheckFalseExit(isValid(), "Failed to create container");
    CheckFalseExit(
        setStyleAttribute(
            StyleSize::RECT(StyleSize::LENGTH_AUTO, StyleSize::LENGTH_AUTO)
        ), "Set style attribute failed"
    );
    CheckFalseExit(
        setStyleAttribute(
            StyleColorItem::STYLE_COLOR_ITEM_BACKGROUND, StyleColor::COLOR_WITH_OPACITY(0, 0)
        ), "Set style attribute failed"
    );
    CheckFalseExit(
        setStyleAttribute(
            StyleWidthItem::STYLE_WIDTH_ITEM_BORDER, 0
        ), "Set style attribute failed"
    );
    CheckFalseExit(
        setStyleAttribute(
            StyleWidthItem::STYLE_WIDTH_ITEM_OUTLINE, 0
        ), "Set style attribute failed"
    );
    CheckFalseExit(
        setStyleAttribute(StyleGap{0, 0, 0, 0, 0, 0}), "Set style attribute failed"
    );

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
}

} // namespace laminpie::gui
