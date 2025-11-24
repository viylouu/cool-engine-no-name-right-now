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
    if (capabilities.currentExtent.width == UINT32_MAX) {
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

        return actualextent;
    }

    return capabilities.currentExtent;
}

void eng_RENDERER_BACKEND_VULKAN_create_swapchain(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    EngData_RENDERER_BACKEND_VULKAN_swapchainSupportDetails swapchainsupport = eng_RENDERER_BACKEND_VULKAN_query_swapchain_support(this, vkback->physical_device);

    VkSurfaceFormatKHR surfaceformat = eng_RENDERER_BACKEND_VULKAN_choose_swap_surface_format(swapchainsupport.formats, swapchainsupport.format_count);
    VkPresentModeKHR presentmode = eng_RENDERER_BACKEND_VULKAN_choose_swap_present_mode(swapchainsupport.present_modes, swapchainsupport.present_mode_count);
    VkExtent2D extent = eng_RENDERER_BACKEND_VULKAN_choose_swap_extent(platform, swapchainsupport.capabilities);

    uint32_t imagecount = swapchainsupport.capabilities.minImageCount + 1;

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

void eng_RENDERER_BACKEND_VULKAN_create_graphics_pipeline(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    size_t tri_vert_size = 0;
    size_t tri_frag_size = 0;
    char* tri_vert = engLoadDataFile("data/eng/shaders/vulkan/tri.vert.spv", &tri_vert_size);
    char* tri_frag = engLoadDataFile("data/eng/shaders/vulkan/tri.frag.spv", &tri_frag_size);

    VkShaderModule tri_vert_module = eng_RENDERER_BACKEND_VULKAN_create_shader_module(this, tri_vert, tri_vert_size);
    VkShaderModule tri_frag_module = eng_RENDERER_BACKEND_VULKAN_create_shader_module(this, tri_frag, tri_frag_size);

    VkPipelineShaderStageCreateInfo tri_vert_shaderstageinfo = {0};
        tri_vert_shaderstageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        tri_vert_shaderstageinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        tri_vert_shaderstageinfo.module = tri_vert_module;
        tri_vert_shaderstageinfo.pName = "main";

    VkPipelineShaderStageCreateInfo tri_frag_shaderstageinfo = {0};
        tri_frag_shaderstageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        tri_frag_shaderstageinfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        tri_frag_shaderstageinfo.module = tri_frag_module;
        tri_frag_shaderstageinfo.pName = "main";

    VkPipelineShaderStageCreateInfo tri_shaderstages[] = {tri_vert_shaderstageinfo, tri_frag_shaderstageinfo};

    (void)tri_shaderstages;

    vkDestroyShaderModule(vkback->device, tri_frag_module, 0);
    vkDestroyShaderModule(vkback->device, tri_vert_module, 0);
}

/* INTERFACE FUNCS */

void eng_RENDERER_BACKEND_VULKAN_constr(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = malloc(sizeof(EngRendererInterface_RENDERER_BACKEND_VULKAN));
    this->backend_data = vkback;

    eng_RENDERER_BACKEND_VULKAN_create_instance(this, platform);
    eng_RENDERER_BACKEND_VULKAN_create_surface(this, platform);
    eng_RENDERER_BACKEND_VULKAN_pick_physical_device(this);
    eng_RENDERER_BACKEND_VULKAN_create_logical_device(this);
    eng_RENDERER_BACKEND_VULKAN_create_swapchain(this, platform);
    eng_RENDERER_BACKEND_VULKAN_create_image_views(this);
    eng_RENDERER_BACKEND_VULKAN_create_graphics_pipeline(this);
}

void eng_RENDERER_BACKEND_VULKAN_destr(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    for (uint32_t i = 0; i < vkback->swapchain_image_view_count; ++i)
        vkDestroyImageView(vkback->device, vkback->swapchain_image_views[i], 0);

    vkDestroySwapchainKHR(vkback->device, vkback->swapchain, 0);
    vkDestroySurfaceKHR(vkback->instance, vkback->surface, 0);
    vkDestroyDevice(vkback->device, 0);
    vkDestroyInstance(vkback->instance, 0);

    free(vkback->swapchain_image_views);
    free(vkback->swapchain_images);

    free(vkback);
    free(this);
}

/* FUNCS */

EngRendererInterface* eng_RENDERER_BACKEND_VULKAN_make_interface(void) {
    EngRendererInterface* interface = malloc(sizeof(EngRendererInterface));

    interface->backend_api = ENG_RENDERER_VULKAN;

    interface->constr = eng_RENDERER_BACKEND_VULKAN_constr;
    interface->destr = eng_RENDERER_BACKEND_VULKAN_destr;

    return interface;
}
