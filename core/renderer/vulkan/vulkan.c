#include "vulkan.h"

// only for glfw helpers and shit
#include <deps/GLFW/glfw3.h>
#include <core/platform/glfw/glfw.h>

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

uint8_t eng_RENDERER_BACKEND_VULKAN_is_device_suitable(EngRendererInterface* this, VkPhysicalDevice device) {
    EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices indices = eng_RENDERER_BACKEND_VULKAN_find_queue_families(this, device);

    uint8_t extensions_supported = eng_RENDERER_BACKEND_VULKAN_check_device_extension_support(device);

    return indices.is_complete && extensions_supported;
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

/* INTERFACE FUNCS */

void eng_RENDERER_BACKEND_VULKAN_constr(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = malloc(sizeof(EngRendererInterface_RENDERER_BACKEND_VULKAN));
    this->backend_data = vkback;

    eng_RENDERER_BACKEND_VULKAN_create_instance(this, platform);
    eng_RENDERER_BACKEND_VULKAN_create_surface(this, platform);
    eng_RENDERER_BACKEND_VULKAN_pick_physical_device(this);
    eng_RENDERER_BACKEND_VULKAN_create_logical_device(this);
}

void eng_RENDERER_BACKEND_VULKAN_destr(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

    vkDestroySurfaceKHR(vkback->instance, vkback->surface, 0);
    vkDestroyDevice(vkback->device, 0);
    vkDestroyInstance(vkback->instance, 0);

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
