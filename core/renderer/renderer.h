#pragma once

#include <core/platform/platform.h>
#include <core/util/cvec.h>

/* TYPES */

typedef enum EngRendererBackend {
    ENG_RENDERER_VULKAN
} EngRendererBackend;

typedef struct EngUniformBuffer {
    void* backend_data;
} EngUniformBuffer;

typedef struct EngVertexBuffer {
    void* backend_data;
} EngVertexBuffer;

typedef struct EngShader {
    void* backend_data;
    EngUniformBuffer** buffers;
    uint32_t buffer_count;
} EngShader;

typedef enum EngShaderStage {
    ENG_STAGE_VERTEX = 1 << 0,
    ENG_STAGE_FRAGMENT = 1 << 1
} EngShaderStage;

typedef enum EngGPUPrimitive {
    ENG_PRIMITIVE_BYTE,
    ENG_PRIMITIVE_UBYTE,
    ENG_PRIMITIVE_SHORT,
    ENG_PRIMITIVE_USHORT,
    ENG_PRIMITIVE_INT,
    ENG_PRIMITIVE_UINT,
    ENG_PRIMITIVE_FLOAT
} EngGPUPrimitive;

typedef struct EngBufferAttribute {
    char* name;
    uint32_t location;
    uint32_t offset;
    uint32_t components;
    EngGPUPrimitive scalar_type;
    uint8_t normalized;
} EngBufferAttribute;

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
            const char* frag,
            EngUniformBuffer** uniform_buffers,
            uint32_t uniform_buffer_count
            );

        void (*unload_shader)(
            EngRendererInterface* this,
            EngShader* shader
            );
    // } uniform buffers {
        EngUniformBuffer* (*create_uniform_buffer)(
            EngRendererInterface* this,
            char* name,
            uint32_t stage, // uses the EngShaderStage type, but is a uint32_t so the stage may be bitor'd with others (eg: VERTEX | FRAGMENT would mean its used in the vertex and fragment stages) 
            uint32_t size // fuck size_t, i have no fucking clue what the odin equivalent is, so im not using it
            );

        void (*destroy_uniform_buffer)(
            EngRendererInterface* this,
            EngUniformBuffer* uniform_buffer
            );

        void (*update_uniform_buffer)(
            EngRendererInterface* this,
            EngUniformBuffer* uniform_buffer,
            void* new_data
            );
    // } vertex arrays {
        // how tf should this work
        /*EngVertexBuffer* (*create_vertex_buffer)(
            EngRendererInterface* this,
            EngShader* shader,
            uint32_t binding,
            uint32_t total_size,
            uint32_t element_size,
            uint32_t
            );

        void (*destroy_vertex_buffer)(
            EngRendererInterface* this,
            EngVertexBuffer* vertex_buffer
            );*/
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
