#include "vulkan.h"

// only for glfw exts
#include <deps/GLFW/glfw3.h>

#include <deps/vulkan/vulkan.h>

#include <stdlib.h>
#include <stdio.h>

/* TYPES */

typedef struct EngRendererInterface_BACKEND_VULKAN {
    VkInstance instance;
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

/* INTERFACE FUNCS */

void eng_RENDERER_BACKEND_VULKAN_constr(EngRendererInterface* this, EngPlatformInterface* platform) {
    (void)platform;

    EngRendererInterface_BACKEND_VULKAN* vkback = malloc(sizeof(EngRendererInterface_BACKEND_VULKAN));
    this->backend_data = vkback;

    eng_RENDERER_BACKEND_VULKAN_create_instance(this, platform);
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
