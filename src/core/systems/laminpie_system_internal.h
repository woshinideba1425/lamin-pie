#pragma once 

#include "laminpie_internal.h"

#if !LAMINPIE_ENABLE_SYSTEMS
#   error "LAMINPIE_ENABLE_SYSTEMS is not enabled, enable it in the menuconfig or lamin_conf.h"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////// Core //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(LAMINPIE_CORE_ENABLE_DEBUG_LOG)
#   if defined(CONFIG_LAMINPIE_CORE_ENABLE_DEBUG_LOG)
#       define LAMINPIE_CORE_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_CORE_ENABLE_DEBUG_LOG
#   else
#       define LAMINPIE_CORE_ENABLE_DEBUG_LOG  (0)
#   endif
#endif

#if LAMINPIE_CORE_ENABLE_DEBUG_LOG
#   if !defined(LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG)
#       if defined(CONFIG_LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG)
#           define LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG
#       else
#           define LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG  (0)
#       endif
#   endif
#   if !defined(LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG)
#       if defined(CONFIG_LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG)
#           define LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG
#       else
#           define LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG  (0)
#       endif
#   endif
#   if !defined(LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG)
#       if defined(CONFIG_LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG)
#           define LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG
#       else
#           define LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG  (0)
#       endif
#   endif
#   if !defined(ESP_BROOKESIA_CORE_MANAGER_ENABLE_DEBUG_LOG)
#       if defined(CONFIG_ESP_BROOKESIA_CORE_MANAGER_ENABLE_DEBUG_LOG)
#           define ESP_BROOKESIA_CORE_MANAGER_ENABLE_DEBUG_LOG  CONFIG_ESP_BROOKESIA_CORE_MANAGER_ENABLE_DEBUG_LOG
#       else
#           define ESP_BROOKESIA_CORE_MANAGER_ENABLE_DEBUG_LOG  (0)
#       endif
#   endif
#   if !defined(ESP_BROOKESIA_CORE_CORE_ENABLE_DEBUG_LOG)
#       if defined(CONFIG_ESP_BROOKESIA_CORE_CORE_ENABLE_DEBUG_LOG)
#           define ESP_BROOKESIA_CORE_CORE_ENABLE_DEBUG_LOG  CONFIG_ESP_BROOKESIA_CORE_CORE_ENABLE_DEBUG_LOG
#       else
#           define ESP_BROOKESIA_CORE_CORE_ENABLE_DEBUG_LOG  (0)
#       endif
#   endif
#endif