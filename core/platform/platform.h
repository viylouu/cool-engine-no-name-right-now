#pragma once

#include <stdint.h>

/* TYPES */

typedef enum EngPlatformBackend {
    ENG_PLATFORM_GLFW
} EngPlatformBackend;

typedef struct EngPlatformInterface EngPlatformInterface;
struct EngPlatformInterface {
    void* backend_data;
    EngPlatformBackend backend_api;

    uint32_t width;
    uint32_t height;

    void (*constr)(
        EngPlatformInterface* this
        );

    void (*destr)(
        EngPlatformInterface* this
        );

    uint8_t (*is_running)(
        EngPlatformInterface* this
        );

    void (*poll)(
        EngPlatformInterface* this
        );

    void (*present)(
        EngPlatformInterface* this
        );

// renderapi helpers

    // casts to void* if not already
    void* (*get_handle)(
        EngPlatformInterface* this
        );

    void (*get_frame_size)(
        EngPlatformInterface* this,
        int* width,
        int* height
        );

// renderer specific shit

    uint8_t (*supports_vulkan)(
        EngPlatformInterface* this
        );
};

/* FUNCS */

EngPlatformInterface* eng_make_platform_interface(EngPlatformBackend backend);
