#include "SlideListContainer.h"
#include <stdio.h>

Panel::~Panel() noexcept {
    if(_panel) {
        lv_obj_del(_panel);
        _panel = nullptr;
    }
}

SlideListContainer::SlideListContainer(lv_obj_t* parent, const Config& cfg)
    : cfg_(cfg), item_pending_removal_(nullptr), item_pending_removal_idx_(-1), is_single_item_slide_out_(false), active_animations_count_(0) {
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_pos(container_, lv_pct(0), lv_pct(0));

    lv_obj_set_style_pad_row(container_, cfg_.spacing, 0);
    lv_obj_set_style_pad_left(container_, cfg_.padding_h, 0);
    lv_obj_set_style_pad_right(container_, cfg_.padding_h, 0);
    lv_obj_set_style_pad_top(container_, cfg_.padding_v, 0);
    lv_obj_set_style_pad_bottom(container_, cfg_.padding_v, 0);
    lv_obj_set_style_border_width(container_, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container_, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_scroll_dir(container_, LV_DIR_VER);
    lv_obj_set_user_data(container_, this);


    // 强制更新布局，确保容器尺寸正确计算
    lv_obj_update_layout(container_);

}

SlideListContainer::~SlideListContainer() {
    // 释放所有item的用户数据
    for (auto* item : items_) {
        ItemUserData* userData = (ItemUserData*)lv_obj_get_user_data(item->panel());
        delete userData;
    }

    // 销毁容器对象，LVGL会自动销毁其子对象(items)
    lv_obj_del(container_);
}

void SlideListContainer::addItem(Panel& item) {
    // 如果item已有父对象，先从原父对象中移除
    lv_obj_t* parent = lv_obj_get_parent(item.panel());
    if (parent != NULL && parent != container_) {
        lv_obj_set_parent(item.panel(), container_);
    }

    printf("addItem: item.panel()=%p, parent=%p\n", item.panel(), lv_obj_get_parent(item.panel()));

    // 强制更新布局，确保获取正确的容器宽度
    lv_obj_update_layout(container_);
    lv_coord_t current_container_width = lv_obj_get_width(container_);

    lv_obj_set_x(item.panel(), 0); // 设置 base_x 为 0
    lv_obj_set_style_translate_x(item.panel(), current_container_width, 0); // 通过 translate_x 将其初始放置在屏幕右侧外
    printf("addItem: item %p, base_x=0, initial translate_x=%d\n", (void*)&item, current_container_width);

    // 创建并设置用户数据
    ItemUserData* userData = new ItemUserData{this, static_cast<int>(items_.size())};
    printf("userData.index: %d\n", userData->index);
    lv_obj_set_user_data(item.panel(), userData);

    item.onCreate();
    items_.push_back(&item);
    lv_obj_add_event_cb(item.panel(), swipeEventCb, LV_EVENT_GESTURE, userData); // swipeEventCb for gesture
    // Consider if enableSwipe and inputEventCb are also active, ensure one primary gesture handling
    layout(); // 添加 item 后重新布局
}

void SlideListContainer::insertItem(std::size_t idx, Panel& item) {
    if(idx > items_.size()) {
        addItem(item);
        return;
    }

    // 如果item已有父对象，先从原父对象中移除
    lv_obj_t* parent = lv_obj_get_parent(item.panel());
    if (parent != NULL && parent != container_) {
        lv_obj_set_parent(item.panel(), container_);
    }

    // 创建并设置用户数据
    ItemUserData* userData = new ItemUserData{this, static_cast<int>(idx)};
    lv_obj_set_user_data(item.panel(), userData);

    items_.insert(items_.begin() + idx, &item);
    lv_obj_add_event_cb(item.panel(), swipeEventCb, LV_EVENT_GESTURE, userData);

    // 更新后面所有item的索引
    for (size_t i = idx + 1; i < items_.size(); ++i) {
        ItemUserData* data = (ItemUserData*)lv_obj_get_user_data(items_[i]->panel());
        if (data) {
            data->index = static_cast<int>(i);
        }   
    }

    layout(); // 插入 item 后重新布局
}

void SlideListContainer::removeItem(Panel& item) {
    auto it = std::find(items_.begin(), items_.end(), &item);
    if (it != items_.end()) {
        size_t idx = it - items_.begin();

        // 释放用户数据
        ItemUserData* userData = (ItemUserData*)lv_obj_get_user_data(item.panel());
        delete userData;

        item.onDestroy();
        items_.erase(it);

        // 更新后面所有item的索引
        for (size_t i = idx; i < items_.size(); ++i) {
            ItemUserData* data = (ItemUserData*)lv_obj_get_user_data(items_[i]->panel());
            if (data) {
                data->index = static_cast<int>(i);
            }
        }

        layout(); // 移除 item 后重新布局
    }
}

void SlideListContainer::slide(Direction dir) {
    if (anim_running_) {
        cancelAnim(); // 如果动画正在进行，先取消
    }
    anim_running_ = true;
    playAnim(dir, -1);
}

void SlideListContainer::slideOut(lv_obj_t* item, int index) {
    if (anim_running_) {
        // 如果已有动画，可以选择取消或者排队，这里简单处理为取消旧的
        // 或者直接返回，不允许在动画期间启动新的单个滑出
        // cancelAnim();
        printf("Animation already running, cannot slideOut item %d\n", index);
        return;
    }

    // 检查 item 和 index 是否有效
    if (!item || index < 0 || static_cast<size_t>(index) >= items_.size() || items_[static_cast<size_t>(index)]->panel() != item) {
        printf("slideOut: Invalid item or index. Item: %p, Index: %d\n", (void*)item, index);
        return;
    }

    anim_running_ = true;
    item_pending_removal_ = item; // 标记这个 item 在动画结束后需要被移除
    item_pending_removal_idx_ = index; // (可选)
    is_single_item_slide_out_ = true; // 标记这是一个单个 item 的滑出
    playAnim(Direction::OutToLeft, index);
}

void SlideListContainer::cancelAnim() {
    lv_anim_del(container_, NULL);
    for (Panel* item : items_) {
        lv_anim_del(item->panel(), NULL);
    }
    anim_running_ = false;
}

// 添加静态成员函数来处理输入事件
void SlideListContainer::inputEventCb(lv_event_t* e) {
    // 获取事件代码
    lv_event_code_t code = lv_event_get_code(e);
    // lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e); // obj 是触发事件的 item
    lv_obj_t* target_item_obj = (lv_obj_t*)lv_event_get_target(e); // 获取触发事件的 item 对象本身
    ItemUserData* event_udata = (ItemUserData*)lv_obj_get_user_data(target_item_obj); // 获取该 item 的用户数据

    static lv_point_t start_point;
    static bool tracking = false;
    if (!event_udata) { // 确保用户数据有效
        return;
    }
    SlideListContainer* container_instance = event_udata->container;
    int item_idx = event_udata->index;


    if (code == LV_EVENT_PRESSED) {
        // 保存按下位置
        lv_indev_t* indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &start_point);
            printf("click event: x=%d, y=%d, index=%d\n", start_point.x, start_point.y, item_idx);
            tracking = true;
        }
    }
    else if (code == LV_EVENT_RELEASED && tracking) {
        // 计算释放位置与按下位置的差距
        lv_point_t end_point;
        lv_indev_t* indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &end_point);
            int dx = end_point.x - start_point.x;
            int dy = end_point.y - start_point.y;

            // 检测是否为水平滑动手势
            if (abs(dx) > 50 && abs(dx) > abs(dy) * 2) {
                if (container_instance) {
                    if (dx < 0) { // 左滑
                        printf("detect right swipe: dx=%d, dy=%d, index=%d\n", dx, dy, item_idx);
                        // lv_obj_t* item_to_slide = container_instance->items_[item_idx]; // 在 slideOut 中会校验
                        if (item_idx >= 0 && static_cast<size_t>(item_idx) < container_instance->items_.size()) {
                             lv_obj_t* actual_item_obj = container_instance->items_[static_cast<size_t>(item_idx)]->panel();
                             container_instance->slideOut(actual_item_obj, item_idx); // 触发滑动
                        } else {
                            printf("inputEventCb: Invalid item_idx %d for right swipe.\n", item_idx);
                        }
                    }
                }
            }
            tracking = false;
        }
    }
}

void SlideListContainer::enableSwipe(bool enable) {
    // 打印当前状态
    printf("Enabling swipe gesture: %s\n", enable ? "true" : "false");

    if (enable) {
        // 容器本身通常不需要直接处理这些手势，由 item 处理
        // lv_obj_add_flag(container_, LV_OBJ_FLAG_CLICKABLE);

        // 为每个item也添加相同的处理
        for (size_t i = 0; i < items_.size(); i++) {
            lv_obj_t* item = items_[i]->panel();
            // 设置项目可以接收点击
            lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);

            // 获取 item 自身存储的 ItemUserData* (在 addItem/insertItem 中设置的)
            ItemUserData* item_specific_user_data = (ItemUserData*)lv_obj_get_user_data(item);
            lv_obj_add_event_cb(item, inputEventCb, LV_EVENT_ALL, item_specific_user_data);
        }
    } else {
        for (Panel* item : items_) {
            lv_obj_clear_flag(item->panel(), LV_OBJ_FLAG_CLICKABLE);
            // 获取 item 自身存储的 ItemUserData* 以便精确移除回调
            ItemUserData* item_specific_user_data = (ItemUserData*)lv_obj_get_user_data(item->panel());
            lv_obj_remove_event_cb_with_user_data(item->panel(), inputEventCb, item_specific_user_data);
        }
    }
}

void SlideListContainer::layout() {
    if (items_.empty()) {
        return;
    }

    lv_coord_t y_pos = cfg_.padding_v;

    for (Panel* item : items_) {
        // 强制将 item 的 base_x 设置为 0，y 坐标按顺序排列
        // 水平滑动效果完全由 style_translate_x 控制
        lv_obj_set_pos(item->panel(), 0, y_pos);
        lv_coord_t item_height = lv_obj_get_height(item->panel());

        // 打印调试信息
        printf("Layout: Item %p at base_x=0, y=%d, height=%d. Current translate_x: %d\n",
               (void*)item->panel(), y_pos, item_height, lv_obj_get_style_translate_x(item->panel(), LV_PART_MAIN));

        // 更新下一个 item 的 y 坐标
        y_pos += item_height + cfg_.spacing;
    }

    // 强制刷新绘制
    lv_obj_invalidate(container_);

    if (onItemChanged) {
        onItemChanged();
    }
}

// lv_anim 执行器回调 - 修改为操作 translate_x
void SlideListContainer::animCb(void* obj, int32_t value) {
    lv_obj_set_style_translate_x((lv_obj_t*)obj, value, 0);
}

void SlideListContainer::playAnim(Direction dir, int index) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_time(&a, cfg_.anim_time);
    lv_anim_set_path_cb(&a, cfg_.anim_path);
    lv_anim_set_ready_cb(&a, animReadyCb);
    lv_anim_set_user_data(&a, this);
    lv_anim_set_exec_cb(&a, animCb); // 使用 animCb 来设置 translate_x

    lv_obj_update_layout(container_);
    lv_coord_t container_width = lv_obj_get_width(container_);
    bool animation_started_at_all = false;
    active_animations_count_ = 0;

    if (dir == Direction::InFromRight) { // 所有item从右侧滑过到左侧并消失 (translate_x 动画)
        is_single_item_slide_out_ = false;
        item_pending_removal_ = nullptr;
        uint32_t delay_per_item = 100;

        if (!items_.empty()) {
            animation_started_at_all = true;
        }
        for (size_t i = 0; i < items_.size(); i++) {
            lv_obj_t* item = items_[i]->panel();
            // 起始 translate_x：容器右侧外
            lv_coord_t start_translate_x = container_width;
            // 结束 translate_x：容器左侧外 (确保完全滑出)
            lv_coord_t end_translate_x = 0; // lv_obj_get_width(item) 也可以考虑，但-CW通常足够

            uint32_t delay = i * delay_per_item;
            lv_anim_set_delay(&a, delay);

            printf("item %zu (InFromRight, translate_x anim): start_val: %d, end_val: %d, delay: %u ms\n",
                   i, start_translate_x, end_translate_x, delay);

            lv_anim_set_var(&a, item);
            lv_anim_set_values(&a, start_translate_x, end_translate_x);
            if (lv_anim_start(&a)) {
                active_animations_count_++;
            }
        }
    } else if (dir == Direction::OutToLeft) { // item 滑动 (translate_x 动画)
        if (index == -1) { // 所有 items 从左侧滑入到 x=0 (视觉上)
            is_single_item_slide_out_ = false;
            item_pending_removal_ = nullptr;
            uint32_t delay_per_item = 100;
            if (!items_.empty()) {
                animation_started_at_all = true;
            }
            for (size_t i = 0; i < items_.size(); i++) {
                lv_obj_t* current_item = items_[i]->panel();
                lv_coord_t start_translate_x = container_width;
                lv_coord_t end_translate_x = 0;

                uint32_t current_delay = i * delay_per_item;
                lv_anim_set_delay(&a, current_delay);

                printf("item %zu (OutToRight, all, translate_x anim): start_val: %d, end_val: %d, delay: %u ms\n",
                       i, start_translate_x, end_translate_x, current_delay);

                lv_anim_set_var(&a, current_item);
                lv_anim_set_values(&a, start_translate_x, end_translate_x);
                if (lv_anim_start(&a)) {
                    active_animations_count_++;
                }
            }
        } else if (index >= 0 && static_cast<size_t>(index) < items_.size()) { // 单个 item 从 x=0 (视觉上) 滑出到左侧
            lv_obj_t* item_to_slide = items_[static_cast<size_t>(index)]->panel();
            if (item_to_slide != item_pending_removal_ && is_single_item_slide_out_) {
                 printf("Warning: Mismatch between item_to_slide and item_pending_removal_ during single slide out.\n");
            }
            lv_coord_t start_translate_x = lv_obj_get_style_translate_x(item_to_slide, LV_PART_MAIN);
            lv_coord_t end_translate_x = -lv_obj_get_width(item_to_slide) - 50; // 或 -container_width

            uint32_t single_item_delay = 0;
            lv_anim_set_delay(&a, single_item_delay);

            printf("item %d (OutToRight, single, translate_x anim): start_val: %d, end_val: %d, delay: %u ms\n",
                   index, start_translate_x, end_translate_x, single_item_delay);

            lv_anim_set_var(&a, item_to_slide);
            lv_anim_set_values(&a, start_translate_x, end_translate_x);
            if (lv_anim_start(&a)) {
                active_animations_count_++;
                animation_started_at_all = true;
            }
        } else {
            printf("Warning: playAnim OutToRight called with invalid index: %d. No animation started.\n", index);
            is_single_item_slide_out_ = false;
            item_pending_removal_ = nullptr;
        }
    }

    if (!animation_started_at_all || active_animations_count_ == 0) {
        anim_running_ = false;
        is_single_item_slide_out_ = false;
        item_pending_removal_ = nullptr;
        printf("No animations were started or all failed to start. anim_running_ set to false.\n");
    } else {
        printf("Total animations started for translate_x: %d\n", active_animations_count_);
    }
}

// lv_anim 结束回调
void SlideListContainer::animReadyCb(lv_anim_t* a) {
    SlideListContainer* instance = (SlideListContainer*)lv_anim_get_user_data(a);
    if (instance) {
        instance->active_animations_count_--; // 一个动画已完成
        printf("Animation ready. Target: %p. Remaining animations: %d\n", a->var, instance->active_animations_count_);

        bool was_single_item_slide_out = instance->is_single_item_slide_out_; // 缓存状态
        lv_obj_t* item_that_finished_anim = (lv_obj_t*)a->var;

        if (was_single_item_slide_out && instance->item_pending_removal_ == item_that_finished_anim) {
            lv_obj_t* item_to_delete = instance->item_pending_removal_;
            printf("Single item slide-out animation finished for item: %p. Removing item.\n", (void*)item_to_delete);

            instance->item_pending_removal_ = nullptr;
            instance->is_single_item_slide_out_ = false;
            // removeItem 会处理后续的 layout
            // 需要找到对应的Panel对象
            for (size_t i = 0; i < instance->items_.size(); i++) {
                if (instance->items_[i]->panel() == item_to_delete) {
                    instance->removeItem(*(instance->items_[i]));
                    break;
                }
            }

            if (instance->onSlideOutFinished) {
                instance->onSlideOutFinished();
            }
        }

        // 当所有相关的动画都完成后，才重置 anim_running_
        if (instance->active_animations_count_ <= 0) {
            printf("All animations finished. Setting anim_running_ to false.\n");
            instance->anim_running_ = false;
            instance->active_animations_count_ = 0; // 确保重置为0，防止负数

            // 清理可能未处理的 pending_removal 状态，以防万一 (例如，如果动画中途被取消)
            if (instance->item_pending_removal_) {
                 printf("Warning: item_pending_removal_ was still set when all animations finished. Clearing.\n");
                 instance->item_pending_removal_ = nullptr;
            }
            if (instance->is_single_item_slide_out_) { // 如果这个标志没被清除，也清除它
                 printf("Warning: is_single_item_slide_out_ was still true. Clearing.\n");
                 instance->is_single_item_slide_out_ = false;
            }


            if (instance->onSlideInFinished && !was_single_item_slide_out) {
                instance->onSlideInFinished();
            }
        }
    }
}

void SlideListContainer::swipeEventCb(lv_event_t* e) {
    // 获取事件类型、触发事件的对象和用户数据
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
    ItemUserData* userData = (ItemUserData*)lv_event_get_user_data(e);

    if (userData && code == LV_EVENT_GESTURE) {
        SlideListContainer* container = userData->container;
        int itemIndex = userData->index;

        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());

        if (dir == LV_DIR_LEFT) {
            // 创建动画处理所有项目的滑出，使用错位效果
            container->slide(Direction::OutToLeft);
        }
    }
}
