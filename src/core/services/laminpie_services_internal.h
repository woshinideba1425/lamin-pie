#pragma once
#include "../../laminpie_internal.h"

#if !LAMINPIE_ENABLE_SERVICES
#   error "Services is not enabled, please enable it in the menuconfig"
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////     Storage  ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(LAMINPIE_SERVICES_ENABLE_STORAGE)
#   if defined(CONFIG_LAMINPIE_SERVICES_ENABLE_STORAGE)
#       define LAMINPIE_SERVICES_ENABLE_STORAGE  CONFIG_LAMINPIE_SERVICES_ENABLE_STORAGE
#   else
#       define LAMINPIE_SERVICES_ENABLE_STORAGE  (0)
#   endif
#endif

#if LAMINPIE_SERVICES_ENABLE_STORAGE
#   if !defined(LAMINPIE_SERVICES_STORAGE_ENABLE_DEBUG_LOG)
#       if defined(CONFIG_LAMINPIE_STORAGE_ENABLE_DEBUG_LOG)
#           define LAMINPIE_SERVICES_STORAGE_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_STORAGE_ENABLE_DEBUG_LOG
#       else
#           define LAMINPIE_SERVICES_STORAGE_ENABLE_DEBUG_LOG  (0)
#       endif
#   endif
#endif
