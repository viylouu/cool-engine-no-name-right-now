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

    VkSwapchainKHR swapchain;
    VkImage* swapchain_images;
    uint32_t swapchain_image_count;
    VkFormat swapchain_image_format;
    VkExtent2D swapchain_extent;
    VkImageView* swapchain_image_views;
    uint32_t swapchain_image_view_count;

    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
} EngRendererInterface_RENDERER_BACKEND_VULKAN;

/* FUNCS */

EngRendererInterface* eng_RENDERER_BACKEND_VULKAN_make_interface(void);
