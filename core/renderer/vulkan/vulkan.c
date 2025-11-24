#include "vulkan.h"

// only for glfw helpers and shit
#include <deps/GLFW/glfw3.h>
#include <core/platform/glfw/glfw.h>

#include <core/data.h>

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_WAYLAND_KHR
// im not sure if i should have xcb or xlib, so ill just do both
#define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <deps/vulkan/vulkan.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* VARS */

const char* requiredextensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const int MAX_FRAMES_IN_FLIGHT = 2;

/* PRIVATE FUNCS */

void eng_RENDERER_BACKEND_VULKAN_create_instance(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    VkApplicationInfo appinfo = {0};
        appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appinfo.pApplicationName = "title";
        appinfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
        appinfo.pEngineName = "untitled engine";
        appinfo.engineVersion = VK_MAKE_VERSION(1,0,0);
        appinfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createinfo = {0};
        createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createinfo.pApplicationInfo = &appinfo;

    uint32_t extensioncount = 0;
    const char** extensions = 0;

    if (platform->backend_api == ENG_PLATFORM_GLFW) {
        uint32_t glfw_extensioncount = 0;
        const char** glfw_extensions;

        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensioncount);

        extensioncount = glfw_extensioncount;
        extensions = glfw_extensions;
    } else {
        // add the following to extensions:
        //  VK_KHR_surface
        //  VK_KHR_swapchain
        //  ... some other shit idfk
    }

    createinfo.enabledExtensionCount = extensioncount;
    createinfo.ppEnabledExtensionNames = extensions;

    if (vkCreateInstance(&createinfo, 0, &vkback->instance) != VK_SUCCESS) {
        printf("failed to create vulkan instance!\n");
        exit(1);
    }
}

typedef struct EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices {
    uint32_t graphics_family;
    uint8_t has_graphics_family;

    uint32_t present_family;
    uint8_t has_present_family;

    uint8_t is_complete;
} EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices;

EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices eng_RENDERER_BACKEND_VULKAN_find_queue_families(EngRendererInterface* this, VkPhysicalDevice device) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices indices = {0};

    uint32_t queuefamilycount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamilycount, 0);

    VkQueueFamilyProperties* queuefamilies = malloc(sizeof(VkQueueFamilyProperties) * queuefamilycount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamilycount, queuefamilies);

    for (uint32_t i = 0; i < queuefamilycount; ++i) {
        if (queuefamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
            indices.has_graphics_family = 1;
        }

        VkBool32 presentsupport = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkback->surface, &presentsupport);

        if (presentsupport) {
            indices.present_family = i;
            indices.has_present_family = 1;
        }

        indices.is_complete = indices.has_graphics_family && indices.has_present_family;

        if (indices.is_complete)
            break;
    }

    free(queuefamilies);

    return indices;
}

uint8_t eng_RENDERER_BACKEND_VULKAN_check_device_extension_support(VkPhysicalDevice device) {
    uint32_t extensioncount;
    vkEnumerateDeviceExtensionProperties(device, 0, &extensioncount, 0);

    VkExtensionProperties* availableextensions = malloc(sizeof(VkExtensionProperties) * extensioncount);
    vkEnumerateDeviceExtensionProperties(device, 0, &extensioncount, availableextensions);

    uint32_t requiredcount = sizeof(requiredextensions) / sizeof(requiredextensions[0]);

    for (uint32_t i = 0; i < extensioncount; ++i) {
        VkExtensionProperties extension = availableextensions[i];
        for (uint32_t j = 0; j < 1; ++j) {
            if (!strcmp(extension.extensionName, requiredextensions[j])) {
                --requiredcount;
                requiredextensions[j] = requiredextensions[requiredcount];
                break;
            }
        }

        if (requiredcount == 0)
            break;
    }

    free(availableextensions);

    return requiredcount == 0;
}

typedef struct EngData_RENDERER_BACKEND_VULKAN_swapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    uint32_t format_count;
    VkPresentModeKHR* present_modes;
    uint32_t present_mode_count;
} EngData_RENDERER_BACKEND_VULKAN_swapchainSupportDetails;

EngData_RENDERER_BACKEND_VULKAN_swapchainSupportDetails eng_RENDERER_BACKEND_VULKAN_query_swapchain_support(EngRendererInterface* this, VkPhysicalDevice device) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    EngData_RENDERER_BACKEND_VULKAN_swapchainSupportDetails details = {0};

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkback->surface, &details.format_count, 0);

    details.formats = malloc(sizeof(VkSurfaceFormatKHR) * details.format_count);

    if (details.format_count != 0)
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkback->surface, &details.format_count, details.formats);

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkback->surface, &details.present_mode_count, 0);

    details.present_modes = malloc(sizeof(VkPresentModeKHR) * details.present_mode_count);

    if (details.present_mode_count != 0)
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkback->surface, &details.present_mode_count, details.present_modes);

    return details;
}


uint8_t eng_RENDERER_BACKEND_VULKAN_is_device_suitable(EngRendererInterface* this, VkPhysicalDevice device) {
    EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices indices = eng_RENDERER_BACKEND_VULKAN_find_queue_families(this, device);

    uint8_t extensions_supported = eng_RENDERER_BACKEND_VULKAN_check_device_extension_support(device);

    uint8_t swapchain_adequate = 0;
    if (extensions_supported) {
        EngData_RENDERER_BACKEND_VULKAN_swapchainSupportDetails swapchain_support = eng_RENDERER_BACKEND_VULKAN_query_swapchain_support(this, device);
        swapchain_adequate = swapchain_support.format_count > 0 && swapchain_support.present_mode_count > 0;
    }

    return indices.is_complete && extensions_supported && swapchain_adequate;
}

void eng_RENDERER_BACKEND_VULKAN_pick_physical_device(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    uint32_t devicecount = 0;
    vkEnumeratePhysicalDevices(vkback->instance, &devicecount, 0);
    if (devicecount == 0) {
        printf("failed to find vulkan supported gpus!\n");
        exit(1);
    }

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * devicecount);
    vkEnumeratePhysicalDevices(vkback->instance, &devicecount, devices);

    vkback->physical_device = VK_NULL_HANDLE;

    for (uint32_t i = 0; i < devicecount; ++i) {
        if (eng_RENDERER_BACKEND_VULKAN_is_device_suitable(this, devices[i]))
            vkback->physical_device = devices[i];
    }

    if (vkback->physical_device == VK_NULL_HANDLE) {
        printf("failed to find a suitable gpu!\n");
        exit(1);
    }

    free(devices);
}

void eng_RENDERER_BACKEND_VULKAN_create_logical_device(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices indices = eng_RENDERER_BACKEND_VULKAN_find_queue_families(this, vkback->physical_device);

    uint32_t uniquequeuefamilies_TEMP[2] = { indices.graphics_family, indices.present_family };
    uint32_t uniquequeuefamilies[2];
    uint32_t unique_count = 0;

    for (uint32_t i = 0; i < 2; ++i) {
        uint32_t f = uniquequeuefamilies_TEMP[i];

        int found = 0;
        for (uint32_t j = 0; j < unique_count; ++j)
            if (uniquequeuefamilies[j] == f) {
                found = 1;
                break;
            }

        if (!found)
            uniquequeuefamilies[unique_count++] = f;
    }

    VkDeviceQueueCreateInfo* queuecreateinfos = malloc(sizeof(VkDeviceQueueCreateInfo) * unique_count);

    float queuepriority = 1.f;
    for (uint32_t i = 0; i < unique_count; ++i) {
        VkDeviceQueueCreateInfo queuecreateinfo = {0};
            queuecreateinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queuecreateinfo.queueFamilyIndex = uniquequeuefamilies[i];
            queuecreateinfo.queueCount = 1;
            queuecreateinfo.pQueuePriorities = &queuepriority;
        queuecreateinfos[i] = queuecreateinfo;
    }

    VkPhysicalDeviceFeatures devicefeatures = {0};

    VkDeviceCreateInfo createinfo = {0};
        createinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createinfo.pQueueCreateInfos = queuecreateinfos;
        createinfo.queueCreateInfoCount = unique_count;
        createinfo.pEnabledFeatures = &devicefeatures;
        createinfo.enabledExtensionCount = sizeof(requiredextensions) / sizeof(requiredextensions[0]);
        createinfo.ppEnabledExtensionNames = requiredextensions;

    /*if (enableValidationLayersOrWhatever) {
        createinfo.enabledLayerCount = [INSERT VALIDATION LAYER SIZE VARIABLE];
        createinfo.ppEnabledLayerNames = [INSERT VALIDATION LAYER ARRAY VARIABLE];
    } else */{
        createinfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(vkback->physical_device, &createinfo, 0, &vkback->device) != VK_SUCCESS) {
        printf("failed to create logical device!\n");
        exit(1);
    }

    vkGetDeviceQueue(vkback->device, indices.graphics_family, 0, &vkback->graphics_queue);
    vkGetDeviceQueue(vkback->device, indices.present_family, 0, &vkback->present_queue);

    free(queuecreateinfos);
}

void eng_RENDERER_BACKEND_VULKAN_create_surface(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    if (platform->backend_api == ENG_PLATFORM_GLFW) {
        EngPlatformInterface_PLATFORM_BACKEND_GLFW* glfwback = platform->backend_data;
        if (glfwCreateWindowSurface(vkback->instance, glfwback->window, 0, &vkback->surface) != VK_SUCCESS) {
            printf("failed to create window surface!\n");
            exit(1);
        }
    } else {
        // todo: add the stuff thats talked about but more platform specific its linked here: https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Window_surface
        // im too lazy to add this in right now, and also it wouldnt do anything/be testable atm since there is only glfw platform (as of Nov 23)
    }
}

// weird ahh c pointer ordering magic that probably doesent even work how its supposed to
VkSurfaceFormatKHR eng_RENDERER_BACKEND_VULKAN_choose_swap_surface_format(VkSurfaceFormatKHR* available_formats, uint32_t available_format_count) {
    for (uint32_t i = 0; i < available_format_count; ++i) {
        VkSurfaceFormatKHR availableformat = available_formats[i];
        if (availableformat.format == VK_FORMAT_B8G8R8A8_SRGB && availableformat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableformat;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR eng_RENDERER_BACKEND_VULKAN_choose_swap_present_mode(VkPresentModeKHR* available_present_modes, uint32_t available_present_mode_count) {
    for (uint32_t i = 0; i < available_present_mode_count; ++i) {
        VkPresentModeKHR availablepresentmode = available_present_modes[i];
        if (availablepresentmode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablepresentmode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D eng_RENDERER_BACKEND_VULKAN_choose_swap_extent(EngPlatformInterface* platform, VkSurfaceCapabilitiesKHR capabilities) {
    if (capabilities.currentExtent.width == UINT32_MAX || capabilities.currentExtent.width == 0) {
        int width, height;
        platform->get_frame_size(platform, &width, &height);
 
        VkExtent2D actualextent = {width, height};

        // aaaaaaa
        // should probably have a cmath.h
        if (actualextent.width < capabilities.minImageExtent.width)
            actualextent.width = capabilities.minImageExtent.width;
        if (actualextent.width > capabilities.maxImageExtent.width)
            actualextent.width = capabilities.maxImageExtent.width;
        if (actualextent.height < capabilities.minImageExtent.height)
            actualextent.height = capabilities.minImageExtent.height;
        if (actualextent.height > capabilities.maxImageExtent.height)
            actualextent.height = capabilities.maxImageExtent.height;

        if (actualextent.width == 0) actualextent.width = width; // fuck it
        if (actualextent.height == 0) actualextent.height = height;

        return actualextent;
    }

    printf("currentextent: %u, %u\n", capabilities.currentExtent.width, capabilities.currentExtent.height);

    return capabilities.currentExtent;
}

void eng_RENDERER_BACKEND_VULKAN_create_swapchain(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    EngData_RENDERER_BACKEND_VULKAN_swapchainSupportDetails swapchainsupport = eng_RENDERER_BACKEND_VULKAN_query_swapchain_support(this, vkback->physical_device);

    VkSurfaceFormatKHR surfaceformat = eng_RENDERER_BACKEND_VULKAN_choose_swap_surface_format(swapchainsupport.formats, swapchainsupport.format_count);
    VkPresentModeKHR presentmode = eng_RENDERER_BACKEND_VULKAN_choose_swap_present_mode(swapchainsupport.present_modes, swapchainsupport.present_mode_count);
    VkExtent2D extent = eng_RENDERER_BACKEND_VULKAN_choose_swap_extent(platform, swapchainsupport.capabilities);

    uint32_t imagecount = swapchainsupport.capabilities.minImageCount + 1;

    if (imagecount == 1)
        imagecount = 2;

    if (swapchainsupport.capabilities.maxImageCount > 0 && imagecount > swapchainsupport.capabilities.maxImageCount)
        imagecount = swapchainsupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createinfo = {0};
        createinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createinfo.surface = vkback->surface;
        createinfo.minImageCount = imagecount;
        createinfo.imageFormat = surfaceformat.format;
        createinfo.imageColorSpace = surfaceformat.colorSpace;
        createinfo.imageExtent = extent;
        createinfo.imageArrayLayers = 1;
        createinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices indices = eng_RENDERER_BACKEND_VULKAN_find_queue_families(this, vkback->physical_device);
    uint32_t queuefamilyindices[] = {indices.graphics_family, indices.present_family};

    if (indices.graphics_family != indices.present_family) {
        createinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createinfo.queueFamilyIndexCount = 2;
        createinfo.pQueueFamilyIndices = queuefamilyindices;
    } else {
        createinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createinfo.queueFamilyIndexCount = 0;
        createinfo.pQueueFamilyIndices = 0;
    }

    createinfo.preTransform = swapchainsupport.capabilities.currentTransform;
    createinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createinfo.presentMode = presentmode;
    createinfo.clipped = VK_TRUE;
    createinfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(vkback->device, &createinfo, 0, &vkback->swapchain) != VK_SUCCESS) {
        printf("failed to create swapchain!\n");
        exit(1);
    }

    vkGetSwapchainImagesKHR(vkback->device, vkback->swapchain, &imagecount, 0);
    vkback->swapchain_image_count = imagecount;
    vkback->swapchain_images = malloc(sizeof(VkImage) * imagecount);
    vkGetSwapchainImagesKHR(vkback->device, vkback->swapchain, &imagecount, vkback->swapchain_images);

    vkback->swapchain_image_format = surfaceformat.format;
    vkback->swapchain_extent = extent;
}

void eng_RENDERER_BACKEND_VULKAN_create_image_views(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    vkback->swapchain_image_view_count = vkback->swapchain_image_count;
    vkback->swapchain_image_views = malloc(sizeof(VkImageView) * vkback->swapchain_image_view_count);

    for (uint32_t i = 0; i < vkback->swapchain_image_view_count; ++i) {
        VkImageViewCreateInfo createinfo = {0};
            createinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createinfo.image = vkback->swapchain_images[i];
            createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createinfo.format = vkback->swapchain_image_format;
            createinfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createinfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createinfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createinfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createinfo.subresourceRange.baseMipLevel = 0;
            createinfo.subresourceRange.levelCount = 1;
            createinfo.subresourceRange.baseArrayLayer = 0;
            createinfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vkback->device, &createinfo, 0, &vkback->swapchain_image_views[i]) != VK_SUCCESS) {
            printf("failed to create image views!\n");
            exit(1);
        }
    }
}

VkShaderModule eng_RENDERER_BACKEND_VULKAN_create_shader_module(EngRendererInterface* this, char* code, size_t code_len) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    VkShaderModuleCreateInfo createinfo = {0};
        createinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createinfo.codeSize = code_len;
        createinfo.pCode = *(uint32_t**)&code;

    VkShaderModule shadermodule;
    if (vkCreateShaderModule(vkback->device, &createinfo, 0, &shadermodule) != VK_SUCCESS) {
        printf("failed to create shader module!\n");
        exit(1);
    }

    return shadermodule;
}

void eng_RENDERER_BACKEND_VULKAN_create_render_pass(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    VkAttachmentDescription colorattachment = {0};
        colorattachment.format = vkback->swapchain_image_format;
        colorattachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorattachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorattachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorattachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorattachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorattachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorattachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorattachmentref = {0};
        colorattachmentref.attachment = 0;
        colorattachmentref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorattachmentref;

    VkSubpassDependency dependency = {0};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderpassinfo = {0};
        renderpassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpassinfo.attachmentCount = 1;
        renderpassinfo.pAttachments = &colorattachment;
        renderpassinfo.subpassCount = 1;
        renderpassinfo.pSubpasses = &subpass;
        renderpassinfo.dependencyCount = 1;
        renderpassinfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vkback->device, &renderpassinfo, 0, &vkback->render_pass) != VK_SUCCESS) {
        printf("failed to create render pass!\n");
        exit(1);
    }
}

void eng_RENDERER_BACKEND_VULKAN_create_framebuffers(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    vkback->swapchain_framebuffer_count = vkback->swapchain_image_view_count;
    vkback->swapchain_framebuffers = malloc(sizeof(VkFramebuffer) * vkback->swapchain_framebuffer_count);
    
    for (uint32_t i = 0; i < vkback->swapchain_image_view_count; ++i) {
        VkImageView attachments[] = {
            vkback->swapchain_image_views[i]
        };

        VkFramebufferCreateInfo framebufferinfo = {0};
            framebufferinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferinfo.renderPass = vkback->render_pass;
            framebufferinfo.attachmentCount = 1;
            framebufferinfo.pAttachments = attachments;
            framebufferinfo.width = vkback->swapchain_extent.width;
            framebufferinfo.height = vkback->swapchain_extent.height;
            framebufferinfo.layers = 1;

        if (vkCreateFramebuffer(vkback->device, &framebufferinfo, 0, &vkback->swapchain_framebuffers[i]) != VK_SUCCESS) {
            printf("failed to create framebuffer!\n");
            exit(1);
        }
    }
}

void eng_RENDERER_BACKEND_VULKAN_create_command_pool(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    // bro ts is used everywhere ong
    EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices queuefamilyindices = eng_RENDERER_BACKEND_VULKAN_find_queue_families(this, vkback->physical_device);

    VkCommandPoolCreateInfo poolinfo = {0};
        poolinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolinfo.queueFamilyIndex = queuefamilyindices.graphics_family;

    if (vkCreateCommandPool(vkback->device, &poolinfo, 0, &vkback->command_pool) != VK_SUCCESS) {
        printf("failed to create command pool!\n");
        exit(1);
    }
}

void eng_RENDERER_BACKEND_VULKAN_create_command_buffers(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    vkback->command_buffer_count = MAX_FRAMES_IN_FLIGHT;
    vkback->command_buffers = malloc(sizeof(VkCommandBuffer) * vkback->command_buffer_count);

    VkCommandBufferAllocateInfo allocinfo = {0};
        allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocinfo.commandPool = vkback->command_pool;
        allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocinfo.commandBufferCount = vkback->command_buffer_count;

    if (vkAllocateCommandBuffers(vkback->device, &allocinfo, vkback->command_buffers) != VK_SUCCESS) {
        printf("failed to allocate command buffers!\n");
        exit(1);
    }
}

void eng_RENDERER_BACKEND_VULKAN_create_sync_objects(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    vkback->image_available_semaphore_count = MAX_FRAMES_IN_FLIGHT;
    vkback->image_available_semaphores = malloc(sizeof(VkSemaphore) * vkback->image_available_semaphore_count);
    vkback->render_finished_semaphore_count = MAX_FRAMES_IN_FLIGHT;
    vkback->render_finished_semaphores = malloc(sizeof(VkSemaphore) * vkback->render_finished_semaphore_count);
    vkback->in_flight_fence_count = MAX_FRAMES_IN_FLIGHT;
    vkback->in_flight_fences = malloc(sizeof(VkFence) * vkback->in_flight_fence_count);

    VkSemaphoreCreateInfo semaphoreinfo = {0};
        semaphoreinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceinfo = {0};
        fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(vkback->device, &semaphoreinfo, 0, &vkback->image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vkback->device, &semaphoreinfo, 0, &vkback->render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vkback->device, &fenceinfo, 0, &vkback->in_flight_fences[i]) != VK_SUCCESS) {
            printf("failed to create synchronization objects for a frame!\n");
            exit(1);
        }
    }
}

void eng_RENDERER_BACKEND_VULKAN_cleanup_swapchain(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    for (uint32_t i = 0; i < vkback->swapchain_framebuffer_count; ++i)
        vkDestroyFramebuffer(vkback->device, vkback->swapchain_framebuffers[i], 0);

    for (uint32_t i = 0; i < vkback->swapchain_image_view_count; ++i)
        vkDestroyImageView(vkback->device, vkback->swapchain_image_views[i], 0);

    vkDestroySwapchainKHR(vkback->device, vkback->swapchain, 0);
}

void eng_RENDERER_BACKEND_VULKAN_recreate_swapchain(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    // hack to pause if width or height is zero (ie. minimizing) until thats not the case
    int w,h;
    platform->get_frame_size(platform, &w,&h);

    vkDeviceWaitIdle(vkback->device);

    eng_RENDERER_BACKEND_VULKAN_cleanup_swapchain(this);

    eng_RENDERER_BACKEND_VULKAN_create_swapchain(this, platform);
    eng_RENDERER_BACKEND_VULKAN_create_image_views(this);
    eng_RENDERER_BACKEND_VULKAN_create_framebuffers(this);
}

void eng_RENDERER_BACKEND_VULKAN_resize_callback(void* data, int width, int height) {
    (void)width;
    (void)height;

    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = data;
    vkback->framebuffer_resized = 1;
}

/* INTERFACE FUNCS */

void eng_RENDERER_BACKEND_VULKAN_constr(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = malloc(sizeof(EngRendererInterface_RENDERER_BACKEND_VULKAN));
    this->backend_data = vkback;

    vkback->framebuffer_resized = 0;
    platform->set_resize_callback(platform, vkback, eng_RENDERER_BACKEND_VULKAN_resize_callback);

    eng_RENDERER_BACKEND_VULKAN_create_instance(this, platform);
    eng_RENDERER_BACKEND_VULKAN_create_surface(this, platform);
    eng_RENDERER_BACKEND_VULKAN_pick_physical_device(this);
    eng_RENDERER_BACKEND_VULKAN_create_logical_device(this);
    eng_RENDERER_BACKEND_VULKAN_create_swapchain(this, platform);
    eng_RENDERER_BACKEND_VULKAN_create_image_views(this);
    eng_RENDERER_BACKEND_VULKAN_create_render_pass(this);
    //eng_RENDERER_BACKEND_VULKAN_create_graphics_pipeline(this);
    eng_RENDERER_BACKEND_VULKAN_create_framebuffers(this);
    eng_RENDERER_BACKEND_VULKAN_create_command_pool(this);
    eng_RENDERER_BACKEND_VULKAN_create_command_buffers(this);
    eng_RENDERER_BACKEND_VULKAN_create_sync_objects(this);
}

void eng_RENDERER_BACKEND_VULKAN_destr(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    vkDeviceWaitIdle(vkback->device);

    eng_RENDERER_BACKEND_VULKAN_cleanup_swapchain(this);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(vkback->device, vkback->image_available_semaphores[i], 0);
        vkDestroySemaphore(vkback->device, vkback->render_finished_semaphores[i], 0);
        vkDestroyFence(vkback->device, vkback->in_flight_fences[i], 0);
    }

    vkDestroyCommandPool(vkback->device, vkback->command_pool, 0);

    vkDestroyRenderPass(vkback->device, vkback->render_pass, 0);

    vkDestroySurfaceKHR(vkback->instance, vkback->surface, 0);
    vkDestroyDevice(vkback->device, 0);
    vkDestroyInstance(vkback->instance, 0);

    free(vkback->command_buffers);
    free(vkback->image_available_semaphores);
    free(vkback->render_finished_semaphores);
    free(vkback->in_flight_fences);

    free(vkback->swapchain_framebuffers);
    free(vkback->swapchain_image_views);
    free(vkback->swapchain_images);

    free(vkback);
    free(this);
}

void eng_RENDERER_BACKEND_VULKAN_frame_begin(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    vkWaitForFences(vkback->device, 1, &vkback->in_flight_fences[vkback->current_frame], VK_TRUE, UINT64_MAX);

    uint32_t imageindex;
    VkResult result = vkAcquireNextImageKHR(vkback->device, vkback->swapchain, UINT64_MAX, vkback->image_available_semaphores[vkback->current_frame], VK_NULL_HANDLE, &imageindex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        eng_RENDERER_BACKEND_VULKAN_recreate_swapchain(this, platform);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        printf("failed to acquire swapchain image!\n");
        exit(1);
    }

    vkResetFences(vkback->device, 1, &vkback->in_flight_fences[vkback->current_frame]);
    vkResetCommandBuffer(vkback->command_buffers[vkback->current_frame], 0);

    vkback->image_index = imageindex;

    VkCommandBufferBeginInfo begininfo = {0};
        begininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begininfo.flags = 0;
        begininfo.pInheritanceInfo = 0;

    if (vkBeginCommandBuffer(vkback->command_buffers[vkback->current_frame], &begininfo) != VK_SUCCESS) {
        printf("failed to begin recording command buffer!\n");
        exit(1);
    }

    VkRenderPassBeginInfo renderpassinfo = {0};
        renderpassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpassinfo.renderPass = vkback->render_pass;
        renderpassinfo.framebuffer = vkback->swapchain_framebuffers[vkback->image_index];
        renderpassinfo.renderArea.offset = (VkOffset2D){0,0};
        renderpassinfo.renderArea.extent = vkback->swapchain_extent;

    // tf is this triple brackets dogshit doing in my code
    // i mean... if it is dogshit... its not out of place in my code
    // BUT WHO CARES! i dont like it
    VkClearValue clearcolor = { .color = { .float32 = {0,0,0,1}}};
    renderpassinfo.clearValueCount = 1;
    renderpassinfo.pClearValues = &clearcolor;

    vkCmdBeginRenderPass(vkback->command_buffers[vkback->current_frame], &renderpassinfo, VK_SUBPASS_CONTENTS_INLINE);
}

void eng_RENDERER_BACKEND_VULKAN_send(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    vkCmdEndRenderPass(vkback->command_buffers[vkback->current_frame]);

    if (vkEndCommandBuffer(vkback->command_buffers[vkback->current_frame]) != VK_SUCCESS) {
        printf("failed to record command buffer!\n");
        exit(1);
    }

    VkSubmitInfo submitinfo = {0};
        submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitsemaphores[] = {vkback->image_available_semaphores[vkback->current_frame]};
    VkPipelineStageFlags waitstages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    // im just going to reindent since this is just silly
        submitinfo.waitSemaphoreCount = 1;
        submitinfo.pWaitSemaphores = waitsemaphores;
        submitinfo.pWaitDstStageMask = waitstages;
        submitinfo.commandBufferCount = 1;
        submitinfo.pCommandBuffers = &vkback->command_buffers[vkback->current_frame];

    VkSemaphore signalsemaphores[] = {vkback->render_finished_semaphores[vkback->current_frame]};
    submitinfo.signalSemaphoreCount = 1;
    submitinfo.pSignalSemaphores = signalsemaphores;

    if (vkQueueSubmit(vkback->graphics_queue, 1, &submitinfo, vkback->in_flight_fences[vkback->current_frame]) != VK_SUCCESS) {
        printf("failed to submit draw command buffer!\n");
        exit(1);
    }

    VkPresentInfoKHR presentinfo = {0};
        presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentinfo.waitSemaphoreCount = 1;
        presentinfo.pWaitSemaphores = signalsemaphores;

    VkSwapchainKHR swapchains[] = {vkback->swapchain};
    presentinfo.swapchainCount = 1;
    presentinfo.pSwapchains = swapchains;
    presentinfo.pImageIndices = &vkback->image_index;
    presentinfo.pResults = 0;

    VkResult result = vkQueuePresentKHR(vkback->present_queue, &presentinfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vkback->framebuffer_resized) {
        eng_RENDERER_BACKEND_VULKAN_recreate_swapchain(this, platform);
    } else if (result != VK_SUCCESS) {
        printf("failed to acquire swapchain image!\n");
        exit(1);
    }

    vkback->current_frame = (vkback->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

typedef struct EngShader_RENDERER_BACKEND_VULKAN {
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
} EngShader_RENDERER_BACKEND_VULKAN;

// rebranded to be "load shader"
EngShader* eng_RENDERER_BACKEND_VULKAN_load_shader(EngRendererInterface* this, const char* vert_path, const char* frag_path) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    EngShader* shader = malloc(sizeof(EngShader));
    EngShader_RENDERER_BACKEND_VULKAN* vkshader = malloc(sizeof(EngShader_RENDERER_BACKEND_VULKAN));
    shader->backend_data = vkshader;

    size_t vert_size = 0;
    size_t frag_size = 0;
    char* vert = engLoadDataFile(vert_path, &vert_size);
    char* frag = engLoadDataFile(frag_path, &frag_size);

    VkShaderModule vert_module = eng_RENDERER_BACKEND_VULKAN_create_shader_module(this, vert, vert_size);
    VkShaderModule frag_module = eng_RENDERER_BACKEND_VULKAN_create_shader_module(this, frag, frag_size);

    VkPipelineShaderStageCreateInfo vert_shaderstageinfo = {0};
        vert_shaderstageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shaderstageinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vert_shaderstageinfo.module = vert_module;
        vert_shaderstageinfo.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shaderstageinfo = {0};
        frag_shaderstageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shaderstageinfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shaderstageinfo.module = frag_module;
        frag_shaderstageinfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderstages[] = {vert_shaderstageinfo, frag_shaderstageinfo};

    VkDynamicState dynamicstates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicstate = {0};
        dynamicstate.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicstate.dynamicStateCount = sizeof(dynamicstates) / sizeof(dynamicstates[0]);
        dynamicstate.pDynamicStates = dynamicstates;

    VkPipelineVertexInputStateCreateInfo vertexinputinfo = {0};
        vertexinputinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexinputinfo.vertexBindingDescriptionCount = 0;
        vertexinputinfo.pVertexBindingDescriptions = 0;
        vertexinputinfo.vertexAttributeDescriptionCount = 0;
        vertexinputinfo.pVertexAttributeDescriptions = 0;

    VkPipelineInputAssemblyStateCreateInfo inputassembly = {0};
        inputassembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputassembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputassembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {0};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = vkback->swapchain_extent.width;
        viewport.height = vkback->swapchain_extent.height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1;

    VkRect2D scissor = {0};
        scissor.offset = (VkOffset2D){0,0};
        scissor.extent = vkback->swapchain_extent;

    VkPipelineViewportStateCreateInfo viewportstate = {0};
        viewportstate.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportstate.viewportCount = 1;
        viewportstate.pViewports = &viewport;
        viewportstate.scissorCount = 1;
        viewportstate.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0;
        rasterizer.depthBiasClamp = 0;
        rasterizer.depthBiasSlopeFactor = 0;

    VkPipelineMultisampleStateCreateInfo multisampling = {0};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1;
        multisampling.pSampleMask = 0;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorblendattachment = {0};
        colorblendattachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT| VK_COLOR_COMPONENT_A_BIT;
        colorblendattachment.blendEnable = VK_FALSE;
        colorblendattachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorblendattachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorblendattachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorblendattachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorblendattachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorblendattachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorblending = {0};
        colorblending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorblending.logicOpEnable = VK_FALSE;
        colorblending.logicOp = VK_LOGIC_OP_COPY;
        colorblending.attachmentCount = 1;
        colorblending.pAttachments = &colorblendattachment;
        colorblending.blendConstants[0] = 0;
        colorblending.blendConstants[1] = 0;
        colorblending.blendConstants[2] = 0;
        colorblending.blendConstants[3] = 0;

    VkPipelineLayoutCreateInfo pipelinelayoutinfo = {0};
        pipelinelayoutinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelinelayoutinfo.setLayoutCount = 0;
        pipelinelayoutinfo.pSetLayouts = 0;
        pipelinelayoutinfo.pushConstantRangeCount = 0;
        pipelinelayoutinfo.pPushConstantRanges = 0;

    if (vkCreatePipelineLayout(vkback->device, &pipelinelayoutinfo, 0, &vkshader->pipeline_layout) != VK_SUCCESS) {
        printf("failed to create pipeline layout!\n");
        exit(1);
    }

    VkGraphicsPipelineCreateInfo pipelineinfo = {0};
        pipelineinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineinfo.stageCount = 2;
        pipelineinfo.pStages = shaderstages;
        pipelineinfo.pVertexInputState = &vertexinputinfo;
        pipelineinfo.pInputAssemblyState = &inputassembly;
        pipelineinfo.pViewportState = &viewportstate;
        pipelineinfo.pRasterizationState = &rasterizer;
        pipelineinfo.pMultisampleState = &multisampling;
        pipelineinfo.pDepthStencilState = 0;
        pipelineinfo.pColorBlendState = &colorblending;
        pipelineinfo.pDynamicState = &dynamicstate;
        pipelineinfo.layout = vkshader->pipeline_layout;
        pipelineinfo.renderPass = vkback->render_pass;
        pipelineinfo.subpass = 0;
        pipelineinfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineinfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(vkback->device, VK_NULL_HANDLE, 1, &pipelineinfo, 0, &vkshader->pipeline) != VK_SUCCESS) {
        printf("failed to create graphics pipeline!\n");
        exit(1);
    }

    vkDestroyShaderModule(vkback->device, frag_module, 0);
    vkDestroyShaderModule(vkback->device, vert_module, 0);

    return shader;
}


void eng_RENDERER_BACKEND_VULKAN_unload_shader(EngRendererInterface* this, EngShader* shader) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;
    EngShader_RENDERER_BACKEND_VULKAN* vkshader = shader->backend_data;

    vkDestroyPipeline(vkback->device, vkshader->pipeline, 0);
    vkDestroyPipelineLayout(vkback->device, vkshader->pipeline_layout, 0);

    free(vkshader);
    free(shader);
}

void eng_RENDERER_BACKEND_VULKAN_bind_shader(EngRendererInterface* this, EngShader* shader) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;
    EngShader_RENDERER_BACKEND_VULKAN* vkshader = shader->backend_data;

    vkCmdBindPipeline(vkback->command_buffers[vkback->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, vkshader->pipeline);
}

void eng_RENDERER_BACKEND_VULKAN_draw(EngRendererInterface* this, int vertices, int instances) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;
    vkCmdDraw(vkback->command_buffers[vkback->current_frame], vertices,instances, 0,0);
}

void eng_RENDERER_BACKEND_VULKAN_bind_frame_viewport(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;
    
    VkViewport viewport = {0};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = vkback->swapchain_extent.width;
        viewport.height = vkback->swapchain_extent.height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1;
    vkCmdSetViewport(vkback->command_buffers[vkback->current_frame], 0,1, &viewport);

    VkRect2D scissor = {0};
        scissor.offset = (VkOffset2D){0,0};
        scissor.extent = vkback->swapchain_extent;
    vkCmdSetScissor(vkback->command_buffers[vkback->current_frame], 0,1, &scissor);
}

void eng_RENDERER_BACKEND_VULKAN_bind_viewport(EngRendererInterface* this, vec4 viewport) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;
    
    VkViewport vp = {0};
        vp.x = viewport.x;
        vp.y = viewport.y;
        vp.width = viewport.z;
        vp.height = viewport.w;
        vp.minDepth = 0;
        vp.maxDepth = 1;
    vkCmdSetViewport(vkback->command_buffers[vkback->current_frame], 0,1, &vp);

    VkRect2D scissor = {0};
        scissor.offset = (VkOffset2D){0,0};
        scissor.extent = vkback->swapchain_extent;
    vkCmdSetScissor(vkback->command_buffers[vkback->current_frame], 0,1, &scissor);
}

/* FUNCS */

EngRendererInterface* eng_RENDERER_BACKEND_VULKAN_make_interface(void) {
    EngRendererInterface* interface = malloc(sizeof(EngRendererInterface));

    interface->backend_api = ENG_RENDERER_VULKAN;

    interface->constr = eng_RENDERER_BACKEND_VULKAN_constr;
    interface->destr = eng_RENDERER_BACKEND_VULKAN_destr;

    interface->frame_begin = eng_RENDERER_BACKEND_VULKAN_frame_begin;
    interface->send = eng_RENDERER_BACKEND_VULKAN_send;

    interface->load_shader = eng_RENDERER_BACKEND_VULKAN_load_shader;
    interface->unload_shader = eng_RENDERER_BACKEND_VULKAN_unload_shader;
    
    interface->draw = eng_RENDERER_BACKEND_VULKAN_draw;

    interface->bind_shader = eng_RENDERER_BACKEND_VULKAN_bind_shader;
    interface->bind_frame_viewport = eng_RENDERER_BACKEND_VULKAN_bind_frame_viewport;
    interface->bind_viewport = eng_RENDERER_BACKEND_VULKAN_bind_viewport;

    return interface;
}
