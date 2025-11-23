#pragma once

#include <core/platform/platform.h>

#include <deps/GLFW/glfw3.h>

/* TYPES */

typedef struct EngPlatformInterface_PLATFORM_BACKEND_GLFW {
    GLFWwindow* window;
} EngPlatformInterface_PLATFORM_BACKEND_GLFW;

/* FUNCS */

EngPlatformInterface* eng_PLATFORM_BACKEND_GLFW_make_interface(void);
