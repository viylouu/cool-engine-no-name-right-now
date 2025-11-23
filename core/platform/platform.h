#pragma once

#include <stdint.h>

/* TYPES */

typedef enum EngPlatformBackend {
    ENG_PLATFORM_GLFW
} EngPlatformBackend;

typedef struct EngPlatformInterface EngPlatformInterface;
struct EngPlatformInterface {
    void* backend_data;

    void (*constr)(
        EngPlatformInterface* this
        );

    void (*destr)(
        EngPlatformInterface* this
        );

    int (*is_running)(
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

// renderer specific shit

    int (*supports_vulkan)(
        EngPlatformInterface* this
        );
};

/* FUNCS */

EngPlatformInterface* eng_make_platform_interface(EngPlatformBackend backend);
