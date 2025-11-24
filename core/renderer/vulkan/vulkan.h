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
    VkFramebuffer* swapchain_framebuffers;
    uint32_t swapchain_framebuffer_count;

    VkRenderPass render_pass;

    //VkPipelineLayout pipeline_layout;
    //VkPipeline graphics_pipeline;

    VkCommandPool command_pool;

    // pointer spam time
    VkCommandBuffer* command_buffers;
    uint32_t command_buffer_count;

    VkSemaphore* image_available_semaphores;
    uint32_t image_available_semaphore_count;
    VkSemaphore* render_finished_semaphores;
    uint32_t render_finished_semaphore_count;
    VkFence* in_flight_fences;
    uint32_t in_flight_fence_count;

    uint32_t current_frame;
    uint32_t image_index;

    uint8_t framebuffer_resized;
} EngRendererInterface_RENDERER_BACKEND_VULKAN;

/* FUNCS */

EngRendererInterface* eng_RENDERER_BACKEND_VULKAN_make_interface(void);
