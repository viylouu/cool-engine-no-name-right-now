#pragma once

#include <core/platform/platform.h>

/* TYPES */

typedef struct EngPlatformInterface_BACKEND_GLFW EngPlatformInterface_BACKEND_GLFW;

/* FUNCS */

EngPlatformInterface* eng_PLATFORM_BACKEND_GLFW_make_interface(void);
