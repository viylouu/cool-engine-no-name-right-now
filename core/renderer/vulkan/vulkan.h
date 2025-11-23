#pragma once

#include <core/renderer/renderer.h>

#include <deps/vulkan/vulkan.h>

/* TYPES */

typedef struct EngRendererInterface_RENDERER_BACKEND_VULKAN {
    VkInstance instance;

    VkPhysicalDevice physical_device;
    VkDevice device;

    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSurfaceKHR surface;
} EngRendererInterface_RENDERER_BACKEND_VULKAN;

/* FUNCS */

EngRendererInterface* eng_RENDERER_BACKEND_VULKAN_make_interface(void);
