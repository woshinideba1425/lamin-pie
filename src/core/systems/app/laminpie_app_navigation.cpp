#include "laminpie_app_navigation.hpp"
#include "src/core/systems/laminpie_system_internal.h"

namespace laminpie::system::app {

Laminpie_App_Navigation::Laminpie_App_Navigation(AppNode root)
: _root_app(root), _current_app(root), _current_page(nullptr), _app_node_list()
    {
    if(_root_app) {
            _app_node_list.push_back(root);
        SYSTEM_APP_LOG_INFO("Navigation system initialized with root app: %s", std::string(_root_app->appId).c_str());
    } else {
        SYSTEM_APP_LOG_WARN("Navigation system initialized without a root app");
    }
}

bool Laminpie_App_Navigation::NavigateToApp(const std::string& appId)
    {
    SYSTEM_APP_LOG_INFO("Attempting to navigate to app: %s", appId.c_str());
    
    AppNode target = FindApp(appId);
    if (!target) {
        SYSTEM_APP_LOG_ERROR("Cannot navigate to app: %s - not found", appId.c_str());
        return false;
    }
    
    if (_current_app == target) {
        SYSTEM_APP_LOG_INFO("Already at requested app: %s", appId.c_str());
        return true;
    }
    
    // Save previous app before switching
    AppNode previous = _current_app;
    _current_app = target;
    
    // Reset current page when changing apps
    _current_page = nullptr;
    
    // If app has pages, set the first one as current
    if (!target->appPages.empty() && !target->appPages.begin()->second.empty()) {
        _current_page = target->appPages.begin()->second[0];
        SYSTEM_APP_LOG_INFO("Set initial page to: %s", _current_page->pageId.c_str());
    }
    
    SYSTEM_APP_LOG_INFO("Successfully navigated from app: %s to app: %s", 
                       previous ? std::string(previous->appId).c_str() : "null", 
                       std::string(_current_app->appId).c_str());
    return true;
}

bool Laminpie_App_Navigation::NavigateBackToParentApp()
    {
    if (!_current_app || _current_app == _root_app) {
        SYSTEM_APP_LOG_WARN("Cannot navigate back: already at root or invalid state");
        return false;
    }
    
    if (!_current_app->parent) {
        SYSTEM_APP_LOG_WARN("Current app has no parent to navigate back to");
        return false;
    }
    
    AppNode parent = _current_app->parent;
    SYSTEM_APP_LOG_INFO("Navigating back from %s to parent app %s", 
                       std::string(_current_app->appId).c_str(), 
                       std::string(parent->appId).c_str());
    
    _current_app = parent;
    
    // Reset current page when changing apps
    _current_page = nullptr;
    
    // If parent app has pages, set the first one as current
    if (!parent->appPages.empty() && !parent->appPages.begin()->second.empty()) {
        _current_page = parent->appPages.begin()->second[0];
        SYSTEM_APP_LOG_INFO("Set parent app page to: %s", _current_page->pageId.c_str());
    }
    
    return true;
}

bool Laminpie_App_Navigation::NavigateToNextApp()
{
    if (!_current_app) {
        SYSTEM_APP_LOG_ERROR("Cannot navigate to next app: current app is null");
        return false;
    }
    
    if (!_current_app->next) {
        SYSTEM_APP_LOG_WARN("Current app has no next app to navigate to");
        return false;
    }
    
    AppNode next = _current_app->next;
    SYSTEM_APP_LOG_INFO("Navigating from %s to next app %s", 
                       std::string(_current_app->appId).c_str(), 
                       std::string(next->appId).c_str());
    
    _current_app = next;
    
    // Reset current page when changing apps
    _current_page = nullptr;
    
    // If next app has pages, set the first one as current
    if (!next->appPages.empty() && !next->appPages.begin()->second.empty()) {
        _current_page = next->appPages.begin()->second[0];
        SYSTEM_APP_LOG_INFO("Set next app page to: %s", _current_page->pageId.c_str());
    }
    
    return true;
}

bool Laminpie_App_Navigation::NavigateToPreviousApp()
{
    if (!_current_app) {
        SYSTEM_APP_LOG_ERROR("Cannot navigate to previous app: current app is null");
        return false;
    }
    
    if (!_current_app->Previous) {
        SYSTEM_APP_LOG_WARN("Current app has no previous app to navigate to");
        return false;
    }
    
    AppNode previous = _current_app->Previous;
    SYSTEM_APP_LOG_INFO("Navigating from %s to previous app %s", 
                       std::string(_current_app->appId).c_str(), 
                       std::string(previous->appId).c_str());
    
    _current_app = previous;
    
    // Reset current page when changing apps
    _current_page = nullptr;
    
    // If previous app has pages, set the first one as current
    if (!previous->appPages.empty() && !previous->appPages.begin()->second.empty()) {
        _current_page = previous->appPages.begin()->second[0];
        SYSTEM_APP_LOG_INFO("Set previous app page to: %s", _current_page->pageId.c_str());
    }
    
    return true;
}

bool Laminpie_App_Navigation::NavigateToPage(const std::string& pageId)
{
    if (!_current_app) {
        SYSTEM_APP_LOG_ERROR("Cannot navigate to page: no current app");
        return false;
    }
    
    SYSTEM_APP_LOG_INFO("Attempting to navigate to page: %s in current app", pageId.c_str());
    
    PageNode targetPage = FindPage(pageId);
    if (!targetPage) {
        SYSTEM_APP_LOG_ERROR("Page %s not found in current app", pageId.c_str());
        return false;
    }
    
    if (_current_page == targetPage) {
        SYSTEM_APP_LOG_INFO("Already at requested page: %s", pageId.c_str());
        return true;
    }
    
    PageNode previousPage = _current_page;
    _current_page = targetPage;
    
    SYSTEM_APP_LOG_INFO("Successfully navigated from page: %s to page: %s", 
                       previousPage ? previousPage->pageId.c_str() : "null", 
                       _current_page->pageId.c_str());
    return true;
}

bool Laminpie_App_Navigation::NavigateToAppPage(const std::string& appId, const std::string& pageId)
{
    SYSTEM_APP_LOG_INFO("Attempting to navigate to app: %s, page: %s", appId.c_str(), pageId.c_str());
    
    // First navigate to the app
    if (!NavigateToApp(appId)) {
        return false;
    }
    
    // Then navigate to the specific page
    return NavigateToPage(pageId);
}

bool Laminpie_App_Navigation::NavigateBackPage()
{
    if (!_current_page) {
        SYSTEM_APP_LOG_ERROR("Cannot navigate back: no current page");
        return false;
    }
    
    if (!_current_page->parent) {
        SYSTEM_APP_LOG_WARN("Current page has no parent to navigate back to");
        return false;
    }
    
    PageNode parentPage = _current_page->parent;
    SYSTEM_APP_LOG_INFO("Navigating back from page %s to parent page %s", 
                       _current_page->pageId.c_str(), 
                       parentPage->pageId.c_str());
    
    _current_page = parentPage;
    return true;
}

bool Laminpie_App_Navigation::NavigateToNextPage()
{
    if (!_current_page) {
        SYSTEM_APP_LOG_ERROR("Cannot navigate to next page: no current page");
        return false;
    }
    
    if (!_current_page->next) {
        SYSTEM_APP_LOG_WARN("Current page has no next page to navigate to");
        return false;
    }
    
    PageNode nextPage = _current_page->next;
    SYSTEM_APP_LOG_INFO("Navigating from page %s to next page %s", 
                       _current_page->pageId.c_str(), 
                       nextPage->pageId.c_str());
    
    _current_page = nextPage;
    return true;
}

bool Laminpie_App_Navigation::NavigateToPreviousPage()
{
    if (!_current_page) {
        SYSTEM_APP_LOG_ERROR("Cannot navigate to previous page: no current page");
        return false;
    }
    
    if (!_current_page->previous) {
        SYSTEM_APP_LOG_WARN("Current page has no previous page to navigate to");
        return false;
    }
    
    PageNode previousPage = _current_page->previous;
    SYSTEM_APP_LOG_INFO("Navigating from page %s to previous page %s", 
                       _current_page->pageId.c_str(), 
                       previousPage->pageId.c_str());
    
    _current_page = previousPage;
    return true;
}

void Laminpie_App_Navigation::RegisterApp(AppNode app)
{
    if (!app) {
        SYSTEM_APP_LOG_ERROR("Cannot register null app");
        return;
    }
    
    // Check if app already exists
    for (const auto& existingApp : _app_node_list) {
        if (existingApp->appId == app->appId) {
            SYSTEM_APP_LOG_WARN("App with ID %s already registered, updating", std::string(app->appId).c_str());
            
            // Update the existing app with new data (but keep relationships)
            existingApp->appName = app->appName;
            existingApp->appPages = app->appPages;
            return;
        }
    }
    
    // Add new app to list
    _app_node_list.push_back(app);
    SYSTEM_APP_LOG_INFO("Registered new app: %s", std::string(app->appId).c_str());
    
    // If this is the first app, make it root and current
    if (_app_node_list.size() == 1) {
        _root_app = app;
        _current_app = app;
        SYSTEM_APP_LOG_INFO("Set as root and current app");
            }
        }

void Laminpie_App_Navigation::SetAppRelationship(const std::string& parentId, const std::string& childId)
{
    AppNode parentApp = FindApp(parentId);
    AppNode childApp = FindApp(childId);
    
    if (!parentApp) {
        SYSTEM_APP_LOG_ERROR("Cannot set relationship: parent app %s not found", parentId.c_str());
        return;
    }
    
    if (!childApp) {
        SYSTEM_APP_LOG_ERROR("Cannot set relationship: child app %s not found", childId.c_str());
        return;
    }
    
    // Set parent-child relationship
    parentApp->firstChild = childApp;
    childApp->parent = parentApp;
    
    SYSTEM_APP_LOG_INFO("Set parent-child relationship between %s and %s", 
                       std::string(parentApp->appId).c_str(), 
                       std::string(childApp->appId).c_str());
}

AppNode Laminpie_App_Navigation::FindApp(const std::string& appId) const
{
    for (const auto& app : _app_node_list) {
        if (app->appId == appId) {
            return app;
        }
    }
    
        SYSTEM_APP_LOG_ERROR("App navigation error: appId %s not found", appId.c_str());
    return nullptr;
}

PageNode Laminpie_App_Navigation::FindPage(const std::string& pageId) const
{
    if (!_current_app) {
        SYSTEM_APP_LOG_ERROR("Cannot find page: no current app");
        return nullptr;
    }

    // Search in all page collections of the current app
    for (const auto& pageCollection : _current_app->appPages) {
        for (const auto& page : pageCollection.second) {
            if (page->pageId == pageId) {
                return page;
            }
        }
    }
    
    SYSTEM_APP_LOG_ERROR("Page navigation error: pageId %s not found in current app", pageId.c_str());
    return nullptr;
}

} // namespace laminpie::system::app