#include "vulkan.h"

// only for glfw exts
#include <deps/GLFW/glfw3.h>

#include <deps/vulkan/vulkan.h>

#include <stdlib.h>
#include <stdio.h>

/* TYPES */

typedef struct EngRendererInterface_BACKEND_VULKAN {
    VkInstance instance;
    VkPhysicalDevice physical_device;
} EngRendererInterface_BACKEND_VULKAN;

/* PRIVATE FUNCS */

void eng_RENDERER_BACKEND_VULKAN_create_instance(EngRendererInterface* this, EngPlatformInterface* platform) {
    EngRendererInterface_BACKEND_VULKAN* vkback = this->backend_data;

    VkApplicationInfo appinfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "title",
        .applicationVersion = VK_MAKE_VERSION(1,0,0),
        .pEngineName = "untitled engine",
        .engineVersion = VK_MAKE_VERSION(1,0,0),
        .apiVersion = VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo createinfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appinfo
    };

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

int32_t eng_RENDERER_BACKEND_VULKAN_get_device_suitability(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceproperties;
    vkGetPhysicalDeviceProperties(device, &deviceproperties);
    //VkPhysicalDeviceFeatures devicefeatures;
    //vkGetPhysicalDeviceFeatures(device, &devicefeatures);

    int score = 0;

    if (deviceproperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 64;

    // todo: more checks and shit

    return score;
}

void eng_RENDERER_BACKEND_VULKAN_pick_physical_device(EngRendererInterface* this) {
    EngRendererInterface_BACKEND_VULKAN* vkback = this->backend_data;

    uint32_t devicecount = 0;
    vkEnumeratePhysicalDevices(vkback->instance, &devicecount, 0);
    if (devicecount == 0) {
        printf("failed to find vulkan supported gpus!\n");
        exit(1);
    }

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * devicecount);
    vkEnumeratePhysicalDevices(vkback->instance, &devicecount, devices);

    vkback->physical_device = VK_NULL_HANDLE;

    int32_t max_score = -1;
    int32_t max_index = -1;

    for (uint32_t i = 0; i < devicecount; ++i) {
        int32_t score = eng_RENDERER_BACKEND_VULKAN_get_device_suitability(devices[i]);
        if (score > max_score) {
            max_score = score;
            max_index = i;
        }
    }

    if (max_score < 0) {
        printf("failed to find a suitable gpu!\n");
        exit(1);
    }

    vkback->physical_device = devices[max_index];

    free(devices);
}

/* INTERFACE FUNCS */

void eng_RENDERER_BACKEND_VULKAN_constr(EngRendererInterface* this, EngPlatformInterface* platform) {
    (void)platform;

    EngRendererInterface_BACKEND_VULKAN* vkback = malloc(sizeof(EngRendererInterface_BACKEND_VULKAN));
    this->backend_data = vkback;

    eng_RENDERER_BACKEND_VULKAN_create_instance(this, platform);
    eng_RENDERER_BACKEND_VULKAN_pick_physical_device(this);
}

void eng_RENDERER_BACKEND_VULKAN_destr(EngRendererInterface* this) {
    EngRendererInterface_BACKEND_VULKAN* vkback = this->backend_data;

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
