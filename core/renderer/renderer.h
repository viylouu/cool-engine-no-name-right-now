#pragma once

#include <core/platform/platform.h>

/* TYPES */

typedef enum EngRendererBackend {
    ENG_RENDERER_VULKAN
} EngRendererBackend;

typedef struct EngRendererInterface EngRendererInterface;
struct EngRendererInterface {
    void* backend_data;
    EngRendererBackend backend_api;

    void (*constr)(
        EngRendererInterface* this,
        EngPlatformInterface* platform
        );

    void (*destr)(
        EngRendererInterface* this
        );

    void (*frame_begin)(
        EngRendererInterface* this,
        EngPlatformInterface* platform
        );

    void (*send)(
        EngRendererInterface* this,
        EngPlatformInterface* platform
        );
};

/* FUNCS */

EngRendererInterface* eng_make_renderer_interface(EngRendererBackend backend);
