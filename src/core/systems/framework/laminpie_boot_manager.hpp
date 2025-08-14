#pragma once
#include "laminpie_core_framework.hpp"
#include "laminpie_system_event_type.hpp"

namespace laminpie::system::framework {
using namespace laminpie::system::event;

typedef struct {
	Laminpie_CoreHome &core_display;
	app::Laminpie_App_Manager &core_manager;
	event::LaminPie_EventDispatcher &core_event;
	device::DeviceManager &device_manager;
	lv_display_t *display_device;

	gui::LockCallback lv_lock_cb = nullptr;
	gui::UnlockCallback lv_unlock_cb = nullptr;
    Boot_EventData_t boot_event_data;
}Laminpie_Boot_ManagerData_t;

class Laminpie_Boot_Manager{
public:
    Laminpie_Boot_Manager(Laminpie_Boot_ManagerData_t &data);
    ~Laminpie_Boot_Manager(void);

    bool ConsignToBoot(Laminpie_Core_Framework &core_framework);

protected:
	virtual bool InitializeHardware();
	virtual bool InitializeBsp();
	virtual bool InitializeDrivers();
	virtual bool InitializeMiddleware();
	virtual bool InitializeSystemServices();
	virtual bool LoadResources();
	virtual bool InitializeApplication();
	virtual bool PostSelfTest();

private:
    Laminpie_Boot_Manager(const Laminpie_Boot_Manager &) = delete;
    Laminpie_Boot_Manager &operator=(const Laminpie_Boot_Manager &) = delete;

	void NotifyPhase(Laminpie_Boot_Event_Type phase);
	void NotifyError(Laminpie_Boot_Event_Type phase, int err, const std::string &msg);
	void NotifyComplete(Laminpie_Boot_Event_Type phase);

    bool RunPhaseWithTimeout(std::function<bool()> phase_fn, Laminpie_Boot_Event_Type phase);
    bool RunPhase(std::function<bool()> phase_fn, Laminpie_Boot_Event_Type phase);

    // helper methods
    uint8_t CalculateProgress(Laminpie_Boot_Event_Type phase) const;
    uint16_t GetPhaseIndex(Laminpie_Boot_Event_Type phase) const;
    const char* GetPhaseName(Laminpie_Boot_Event_Type phase) const;

    Laminpie_Boot_ManagerData_t _boot_manager_data;
    Laminpie_Boot_Event_Type _boot_status;
    Laminpie_Boot_Event_Type _previous_phase {Laminpie_Boot_Event_Type::kBoot_Event_Type_Max};
    std::string _current_request_id;
};
}