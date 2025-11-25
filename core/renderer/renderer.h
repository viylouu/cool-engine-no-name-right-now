#pragma once

#include <core/platform/platform.h>
#include <core/util/cvec.h>

/* TYPES */

typedef enum EngRendererBackend {
    ENG_RENDERER_VULKAN
} EngRendererBackend;

typedef struct EngUniform {
    void* backend_data;
} EngUniform;

typedef struct EngShader {
    void* backend_data;
    EngUniform* uniforms;
    uint32_t uniform_count;
} EngShader;

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

    // draw stuff

    // shaders {
        EngShader* (*load_shader)(
            EngRendererInterface* this,
            const char* vert,
            const char* frag
            );

        void (*unload_shader)(
            EngRendererInterface* this,
            EngShader* shader
            );

        void (*describe_ubo)(
            EngRendererInterface* this,
            EngShader* shader
            );
    // } commands {
        void (*draw)(
            EngRendererInterface* this,
            int vertices,
            int instances
            );
    // } bindings {
        void (*bind_shader)(
            EngRendererInterface* this,
            EngShader* shader
            );

        void (*bind_frame_viewport)(
            EngRendererInterface* this
            );

        void (*bind_viewport)(
            EngRendererInterface* this,
            vec4 viewport
            );
    // )
};

/* FUNCS */

EngRendererInterface* eng_make_renderer_interface(EngRendererBackend backend);
