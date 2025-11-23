#include "glfw.h"

#include <deps/GLFW/glfw3.h>

#if _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
    #define GLFW_EXPOSE_NATIVE_WGL
#elif __linux__
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_WAYLAND
    #define GLFW_EXPOSE_NATIVE_GLX
    #define GLFW_EXPOSE_NATIVE_EGL
#endif
#include <deps/GLFW/glfw3native.h>

#include <stdlib.h>
#include <stdio.h>

/* INTERFACE FUNCS */

void eng_PLATFORM_BACKEND_GLFW_constr(EngPlatformInterface* this) {
    EngPlatformInterface_PLATFORM_BACKEND_GLFW* glfwback = malloc(sizeof(EngPlatformInterface_PLATFORM_BACKEND_GLFW));
    this->backend_data = glfwback;

    if (!glfwInit()) {
        printf("failed to init glfw!\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    glfwback->window = glfwCreateWindow(800,600,"title", 0,0);
    if (!glfwback->window) {
        printf("failed to create glfw window!\n");
        glfwTerminate();
        exit(1);
    }

    this->width = 800;
    this->height = 600;
}

void eng_PLATFORM_BACKEND_GLFW_destr(EngPlatformInterface* this) {
    EngPlatformInterface_PLATFORM_BACKEND_GLFW* glfwback = this->backend_data;

    this->present(this); // fix for segfault

    glfwDestroyWindow(glfwback->window);
    glfwTerminate();

    free(glfwback);
    free(this);
}

uint8_t eng_PLATFORM_BACKEND_GLFW_is_running(EngPlatformInterface* this) {
    EngPlatformInterface_PLATFORM_BACKEND_GLFW* glfwback = this->backend_data;
    return !glfwWindowShouldClose(glfwback->window);
}

void eng_PLATFORM_BACKEND_GLFW_poll(EngPlatformInterface* this) {
    (void)this;
    glfwPollEvents();
}

void eng_PLATFORM_BACKEND_GLFW_present(EngPlatformInterface* this) {
    EngPlatformInterface_PLATFORM_BACKEND_GLFW* glfwback = this->backend_data;
    glfwSwapBuffers(glfwback->window);
}

void* eng_PLATFORM_BACKEND_GLFW_get_handle(EngPlatformInterface* this) {
    EngPlatformInterface_PLATFORM_BACKEND_GLFW* glfwback = this->backend_data;

#if _WIN32
    return (void*)glfwGetWin32Window(glfwback->window);
#elif __linux__
    if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
        return (void*)glfwGetWaylandWindow(glfwback->window);
    else if (glfwGetPlatform() == GLFW_PLATFORM_X11)
        return (void*)glfwGetX11Window(glfwback->window);
#endif

    printf("unsupported platform!\n");
    exit(1);

    // fucking pos compiler warning
    return 0;
}

void eng_PLATFORM_BACKEND_GLFW_get_frame_size(EngPlatformInterface* this, int* width, int* height) {
    EngPlatformInterface_PLATFORM_BACKEND_GLFW* glfwback = this->backend_data;
    glfwGetFramebufferSize(glfwback->window, width, height);
}

uint8_t eng_PLATFORM_BACKEND_GLFW_supports_vulkan(EngPlatformInterface* this) {
    (void)this;
    return glfwVulkanSupported();
}

/* FUNCS */

EngPlatformInterface* eng_PLATFORM_BACKEND_GLFW_make_interface(void) {
    EngPlatformInterface* interface = malloc(sizeof(EngPlatformInterface));

    interface->backend_api = ENG_PLATFORM_GLFW;

    interface->constr = eng_PLATFORM_BACKEND_GLFW_constr;
    interface->destr = eng_PLATFORM_BACKEND_GLFW_destr;
    interface->is_running = eng_PLATFORM_BACKEND_GLFW_is_running;
    interface->poll = eng_PLATFORM_BACKEND_GLFW_poll;
    interface->present = eng_PLATFORM_BACKEND_GLFW_present;
    interface->get_handle = eng_PLATFORM_BACKEND_GLFW_get_handle;
    interface->get_frame_size = eng_PLATFORM_BACKEND_GLFW_get_frame_size;
    interface->supports_vulkan = eng_PLATFORM_BACKEND_GLFW_supports_vulkan;

    return interface;
}
