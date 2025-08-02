#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "../laminpie_system_internal.h"

namespace laminpie::system::app {

struct Laminpie_Page_Node{
    using PagePtr = std::shared_ptr<Laminpie_Page_Node>;
        std::string pageId;
        std::string pageName;

        PagePtr parent;
        PagePtr next;
        PagePtr previous;
        PagePtr firstChild;

        void* pageData;

        Laminpie_Page_Node(const std::string &pageId, const std::string &pageName, void* pageData)
        : pageId(pageId), pageName(pageName), pageData(pageData){}

        ~Laminpie_Page_Node() = default;
};

struct Laminpie_App_Node{
    using Ptr = std::shared_ptr<Laminpie_App_Node>;

    std::string_view appId;  // has to be passed in by the caller
    std::string_view appName;
    std::unordered_map<std::string, std::vector<Laminpie_Page_Node::PagePtr>> appPages;

    Ptr parent;
    Ptr next;
    Ptr Previous;
    Ptr firstChild;

    Laminpie_App_Node(const std::string_view &appId, const std::string_view &appName)
    : appId(appId), appName(appName),
    parent(nullptr), next(nullptr), Previous(nullptr), firstChild(nullptr){}

    ~Laminpie_App_Node() = default;
};

using AppNode = Laminpie_App_Node::Ptr;
using PageNode = Laminpie_Page_Node::PagePtr;

class Laminpie_App_Navigation{
public:

    Laminpie_App_Navigation(AppNode root);

    // 应用导航方法
    bool NavigateToApp(const std::string& appId);
    bool NavigateBackToParentApp();
    bool NavigateToNextApp();
    bool NavigateToPreviousApp();

    // 页面导航方法
    bool NavigateToPage(const std::string& pageId);
    bool NavigateToAppPage(const std::string& appId, const std::string& pageId);
    bool NavigateBackPage();
    bool NavigateToNextPage();
    bool NavigateToPreviousPage();

    // 获取当前状态
    AppNode GetCurrentApp() const { return _current_app; }
    PageNode GetCurrentPage() const { return _current_page; }
    
    // 注册应用节点
    void RegisterApp(AppNode app);
    
    void SetAppRelationship(const std::string& parentId, const std::string& childId);
private:
    AppNode _root_app;
    AppNode _current_app;
    PageNode _current_page;
    std::vector<AppNode> _app_node_list;

    AppNode FindApp(const std::string& appId) const;
    PageNode FindPage(const std::string& pageId) const;
};
}