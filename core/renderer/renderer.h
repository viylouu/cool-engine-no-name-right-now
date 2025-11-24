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

    // for now, this is just a single draw frame thing
    // im too lazy to start doing the api right now, so this is all you get
    void (*draw_frame)(
        EngRendererInterface* this
        );
};

/* FUNCS */

EngRendererInterface* eng_make_renderer_interface(EngRendererBackend backend);
