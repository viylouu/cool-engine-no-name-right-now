#include "vulkan.h"

// only for glfw exts
#include <deps/GLFW/glfw3.h>

#include <deps/vulkan/vulkan.h>

#include <stdlib.h>
#include <stdio.h>

/* TYPES */

typedef struct EngRendererInterface_RENDERER_BACKEND_VULKAN {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
} EngRendererInterface_RENDERER_BACKEND_VULKAN;

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

    uint8_t is_complete;
} EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices;

EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices eng_RENDERER_BACKEND_VULKAN_find_queue_families(VkPhysicalDevice device) {
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

        indices.is_complete = indices.has_graphics_family;

        if (indices.is_complete)
            break;
    }

    free(queuefamilies);

    return indices;
}

uint8_t eng_RENDERER_BACKEND_VULKAN_is_device_suitable(VkPhysicalDevice device) {
    EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices indices = eng_RENDERER_BACKEND_VULKAN_find_queue_families(device);

    return indices.is_complete;
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
        if (eng_RENDERER_BACKEND_VULKAN_is_device_suitable(devices[i]))
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

    EngData_RENDERER_BACKEND_VULKAN_queueFamilyIndices indices = eng_RENDERER_BACKEND_VULKAN_find_queue_families(vkback->physical_device);

    VkDeviceQueueCreateInfo queuecreateinfo = {0};
        queuecreateinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queuecreateinfo.queueFamilyIndex = indices.graphics_family;
        queuecreateinfo.queueCount = 1;

    float queuepriority = 1.f;
    queuecreateinfo.pQueuePriorities = &queuepriority;

    VkPhysicalDeviceFeatures devicefeatures = {0};

    VkDeviceCreateInfo createinfo = {0};
        createinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createinfo.pQueueCreateInfos = &queuecreateinfo;
        createinfo.queueCreateInfoCount = 1;
        createinfo.pEnabledFeatures = &devicefeatures;

    createinfo.enabledExtensionCount = 0;

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
}

/* INTERFACE FUNCS */

void eng_RENDERER_BACKEND_VULKAN_constr(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = malloc(sizeof(EngRendererInterface_RENDERER_BACKEND_VULKAN));
    this->backend_data = vkback;

    eng_RENDERER_BACKEND_VULKAN_create_instance(this, platform);
    eng_RENDERER_BACKEND_VULKAN_pick_physical_device(this);
    eng_RENDERER_BACKEND_VULKAN_create_logical_device(this);
}

void eng_RENDERER_BACKEND_VULKAN_destr(EngRendererInterface* this) {
    EngRendererInterface_RENDERER_BACKEND_VULKAN* vkback = this->backend_data;

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
