#pragma once

#include <core/platform/platform.h>

/* TYPES */

typedef enum EngRendererBackend {
    ENG_RENDERER_VULKAN
} EngRendererBackend;

typedef struct EngRendererInterface EngRendererInterface;
struct EngRendererInterface {
    void* backend_data;

    void (*constr)(
        EngRendererInterface* this,
        EngPlatformInterface* platform
        );

    void (*destr)(
        EngRendererInterface* this
        );
};

/* FUNCS */

EngRendererInterface* eng_make_renderer_interface(EngRendererBackend backend);
