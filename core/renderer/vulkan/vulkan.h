#pragma once

#include <core/renderer/renderer.h>

#include <deps/vulkan/vulkan.h>

/* VARS */

#define MAX_FRAMES_IN_FLIGHT 2

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
    VkFramebuffer* swapchain_framebuffers;
    uint32_t swapchain_framebuffer_count;

    VkRenderPass render_pass;

    //VkPipelineLayout pipeline_layout;
    //VkPipeline graphics_pipeline;

    VkCommandPool command_pool;

    // pointer spam time
    VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT];

    VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];

    uint32_t current_frame;
    uint32_t image_index;

    uint8_t framebuffer_resized;
} EngRendererInterface_RENDERER_BACKEND_VULKAN;

/* FUNCS */

EngRendererInterface* eng_RENDERER_BACKEND_VULKAN_make_interface(void);
