#pragma once
#include "lvgl.h"
#include <vector>
#include <functional>

struct Config {
    uint16_t       spacing     = 8;          // item 间距
    uint16_t       padding_h   = 4;          // 左右内边距
    uint16_t       padding_v   = 4;          // 上下内边距
    uint32_t       anim_time   = 300;        // 默认动画时长 ms
    lv_anim_path_cb_t anim_path  = lv_anim_path_ease_out;
};

class Panel {
public:
    enum class LifeCycle {
        ON_INIT     = 0,
        ON_CREATE   = 1 << 0,
        ON_DESTROY  = 1 << 1,
    };

    explicit Panel(lv_obj_t *parent):
        _panel(lv_obj_create(parent))
    {
        lv_obj_clear_flag(_panel, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(_panel, 0, 0);
        lv_obj_set_style_radius(_panel, 0, 0);
        lv_obj_set_style_clip_corner(_panel, false, 0);
        lv_obj_set_style_border_width(_panel, 0, 0);
        lv_obj_set_style_pad_left(_panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(_panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    Panel(const Panel&) = delete;
    Panel& operator=(const Panel&) = delete;

    Panel(Panel&&) = delete;
    Panel& operator=(Panel&&) noexcept = delete;

    virtual ~Panel() noexcept;

    virtual void onCreate() {}
    virtual void onDestroy() {}

    inline lv_obj_t *panel() {
        return _panel;
    }

    inline LifeCycle lifeCycle() {
        return _lifeCycle;
    }

private:
    lv_obj_t *_panel;
    LifeCycle _lifeCycle = LifeCycle::ON_INIT;
};


class SlideListContainer {
public:
    enum class Direction { InFromRight, OutToLeft };


    // —— 构造 & 析构 ——
    explicit SlideListContainer(lv_obj_t* parent,
                                const Config& cfg = Config{});
    ~SlideListContainer();                       // 负责销毁内部容器，但不销毁外部传入的 item

    // —— Item 管理 ——
    void addItem(Panel& item);                // 末尾追加
    void insertItem(std::size_t idx, Panel& item); // 指定位置插入
    Panel& getItemAt(std::size_t idx) { return *items_[idx]; }
    void removeItem(Panel& item);             // 不 delete，只摘链
    std::size_t itemCount() const { return items_.size(); }

    // —— 动画控制 ——
    void slide(Direction dir);                   // 自动播放
    void slideOut(lv_obj_t* item, int index);
    void cancelAnim();                           // 立即停在当前帧
    bool animating() const { return anim_running_; }

    // —— 手势开关 ——
    void enableSwipe(bool enable = true);

    // —— 配置存取 ——
    void setAnimTime(uint32_t ms)        { cfg_.anim_time = ms; }
    void setAnimPath(lv_anim_path_cb_t cb) { cfg_.anim_path = cb; }

    // —— 事件回调 ——
    std::function<void()> onSlideInFinished;
    std::function<void()> onSlideOutFinished;
    std::function<void()> onItemChanged;

    lv_obj_t* root() const { return container_; } // 若需直接挂子控件可获取根节点

private:
    struct ItemUserData {
        SlideListContainer* container;  // 容器实例指针
        int index;                     // item在容器中的索引
    };

    void layout();                // 重新排版
    void playAnim(Direction dir, int index); // 包装 lv_anim_t
    static void animCb(void* obj, int32_t x);  // lv_anim 执行器
    static void animReadyCb(lv_anim_t* a);     // lv_anim 结束回调
    static void swipeEventCb(lv_event_t* e);   // LV_EVENT_GESTURE 处理
    static void inputEventCb(lv_event_t* e);   // 自定义输入事件处理
    lv_obj_t*              container_{nullptr};
    std::vector<Panel*> items_;
    Config                 cfg_;
    bool                   anim_running_{false};

    lv_obj_t* item_pending_removal_ = nullptr;
    int item_pending_removal_idx_ = -1; // 可选，如果仅靠 item 指针不够
    bool is_single_item_slide_out_ = false;
    int active_animations_count_ = 0;
    lv_coord_t             container_width{0};
};
