#include "laminpie_boot_manager.hpp"
#include "laminpie_system_event_type.hpp"
#include "laminpie_system_internal.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <string>

namespace laminpie::system::framework {

Laminpie_Boot_Manager::Laminpie_Boot_Manager(Laminpie_Boot_ManagerData_t &data)
    : _boot_manager_data(data) {
    // Generate unique request ID for this boot session
    _current_request_id = std::to_string(std::chrono::system_clock::now().time_since_epoch().count() * std::random_device{}());
    
    SYSTEM_CORE_LOG_INFO("BootManager: Initialized with request_id: %s", _current_request_id.c_str());
}

Laminpie_Boot_Manager::~Laminpie_Boot_Manager(void) {
    SYSTEM_CORE_LOG_INFO("BootManager: Destroyed");
}

bool Laminpie_Boot_Manager::ConsignToBoot(Laminpie_Core_Framework &core_framework) {
    SYSTEM_CORE_LOG_INFO("BootManager: Starting boot sequence with request_id: %s", _current_request_id.c_str());
    
    // Boot phases configuration
    struct BootPhase {
        std::function<bool()> fn;
        Laminpie_Boot_Event_Type type;
        const char* name;
    };
    
    std::vector<BootPhase> phases = {
        {[this]{ return InitializeHardware(); }, Laminpie_Boot_Event_Type::kBoot_Stage_HardWareInit, "Hardware"},
        {[this]{ return InitializeBsp(); }, Laminpie_Boot_Event_Type::kBoot_Stage_BSPInit, "BSP"},
        {[this]{ return InitializeDrivers(); }, Laminpie_Boot_Event_Type::kBoot_Stage_DriverInit, "Drivers"},
        {[this]{ return InitializeMiddleware(); }, Laminpie_Boot_Event_Type::kBoot_Stage_MiddlewareInit, "Middleware"},
        {[this]{ return InitializeSystemServices(); }, Laminpie_Boot_Event_Type::kBoot_Stage_SytemServiceInit, "SystemServices"},
        {[this]{ return LoadResources(); }, Laminpie_Boot_Event_Type::kBoot_Stage_Resourceload, "Resources"},
        {[this]{ return InitializeApplication(); }, Laminpie_Boot_Event_Type::kBoot_Stage_AppInit, "Application"}
    };

    // Execute boot phases
    for (size_t i = 0; i < phases.size(); ++i) {
        const auto& phase = phases[i];
        
        // Notify phase start
        NotifyPhase(phase.type);
        
        // Execute phase with timeout
        if (!RunPhaseWithTimeout(phase.fn, phase.type)) {
            SYSTEM_CORE_LOG_ERROR("BootManager: Failed at phase: %s", phase.name);
            NotifyError(phase.type, -1, std::string("Failed to initialize ") + phase.name);
            _boot_status = Laminpie_Boot_Event_Type::kBoot_Stage_Failed;
            return false;
        }
        
        // Notify phase completion
        NotifyComplete(phase.type);
        SYSTEM_CORE_LOG_INFO("BootManager: Completed phase: %s", phase.name);
        
        _previous_phase = phase.type;
    }

    // Boot completed successfully
    _boot_status = Laminpie_Boot_Event_Type::kBoot_Stage_Complete;
    NotifyPhase(Laminpie_Boot_Event_Type::kBoot_Stage_Complete);
    
    SYSTEM_CORE_LOG_INFO("BootManager: Boot sequence completed successfully");
    return true;
}

void Laminpie_Boot_Manager::NotifyPhase(Laminpie_Boot_Event_Type phase) {
    uint8_t progress = CalculateProgress(phase);
    uint16_t step_index = GetPhaseIndex(phase);
    
    Boot_EventData_t event_data(phase, _previous_phase, progress);
    event_data.step_index = step_index;
    event_data.request_id = _current_request_id;
    event_data.source_module = "BOOT_MGR";
    
    _boot_manager_data.core_event.dispatchEvent(event_data);
    
    SYSTEM_CORE_LOG_DEBUG("BootManager: Phase notification sent - phase: %d, progress: %d%%", 
                          static_cast<int>(phase), progress);
}

void Laminpie_Boot_Manager::NotifyError(Laminpie_Boot_Event_Type phase, int err, const std::string &msg) {
    Boot_EventData_t event_data(Laminpie_Boot_Event_Type::kBoot_Stage_Failed, err, msg, phase);
    event_data.request_id = _current_request_id;
    event_data.source_module = "BOOT_MGR";
    event_data.previous_phase = _previous_phase;
    event_data.progress_percent = CalculateProgress(phase);
    
    _boot_manager_data.core_event.dispatchEvent(event_data);
    
    SYSTEM_CORE_LOG_ERROR("BootManager: Error notification sent - phase: %d, error: %d, message: %s", 
                          static_cast<int>(phase), err, msg.c_str());
}

void Laminpie_Boot_Manager::NotifyComplete(Laminpie_Boot_Event_Type phase) {
    uint8_t progress = CalculateProgress(phase);
    uint16_t step_index = GetPhaseIndex(phase);
    
    Boot_EventData_t event_data(phase, _previous_phase, progress);
    event_data.step_index = step_index;
    event_data.request_id = _current_request_id;
    event_data.source_module = "BOOT_MGR";
    
    _boot_manager_data.core_event.dispatchEvent(event_data);
    
    SYSTEM_CORE_LOG_DEBUG("BootManager: Completion notification sent - phase: %d, progress: %d%%", 
                          static_cast<int>(phase), progress);
}

uint8_t Laminpie_Boot_Manager::CalculateProgress(Laminpie_Boot_Event_Type phase) const {
    uint16_t phase_index = GetPhaseIndex(phase);
    if (phase == Laminpie_Boot_Event_Type::kBoot_Stage_Complete) {
        return 100;
    }
    if (phase == Laminpie_Boot_Event_Type::kBoot_Stage_Failed) {
        return CalculateProgress(_previous_phase); // Return progress at failure point
    }
    
    // Calculate percentage based on phase index (0-6 for 7 phases)
    return static_cast<uint8_t>((phase_index * 100) / 7);
}

uint16_t Laminpie_Boot_Manager::GetPhaseIndex(Laminpie_Boot_Event_Type phase) const {
    switch (phase) {
        case Laminpie_Boot_Event_Type::kBoot_Stage_HardWareInit: return 0;
        case Laminpie_Boot_Event_Type::kBoot_Stage_BSPInit: return 1;
        case Laminpie_Boot_Event_Type::kBoot_Stage_DriverInit: return 2;
        case Laminpie_Boot_Event_Type::kBoot_Stage_MiddlewareInit: return 3;
        case Laminpie_Boot_Event_Type::kBoot_Stage_SytemServiceInit: return 4;
        case Laminpie_Boot_Event_Type::kBoot_Stage_Resourceload: return 5;
        case Laminpie_Boot_Event_Type::kBoot_Stage_AppInit: return 6;
        case Laminpie_Boot_Event_Type::kBoot_Stage_Complete: return 7;
        default: return 0;
    }
}

const char* Laminpie_Boot_Manager::GetPhaseName(Laminpie_Boot_Event_Type phase) const {
    switch (phase) {
        case Laminpie_Boot_Event_Type::kBoot_Stage_HardWareInit: return "Hardware";
        case Laminpie_Boot_Event_Type::kBoot_Stage_BSPInit: return "BSP";
        case Laminpie_Boot_Event_Type::kBoot_Stage_DriverInit: return "Drivers";
        case Laminpie_Boot_Event_Type::kBoot_Stage_MiddlewareInit: return "Middleware";
        case Laminpie_Boot_Event_Type::kBoot_Stage_SytemServiceInit: return "SystemServices";
        case Laminpie_Boot_Event_Type::kBoot_Stage_Resourceload: return "Resources";
        case Laminpie_Boot_Event_Type::kBoot_Stage_AppInit: return "Application";
        case Laminpie_Boot_Event_Type::kBoot_Stage_Complete: return "Complete";
        case Laminpie_Boot_Event_Type::kBoot_Stage_Failed: return "Failed";
        default: return "Unknown";
    }
}

}