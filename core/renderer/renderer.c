#include "renderer.h"

#include <core/renderer/vulkan/vulkan.h>
#include <core/renderer/opengl/opengl.h>

/* FUNCS */

EngRendererInterface* eng_make_renderer_interface(EngRendererBackend backend) {
    switch(backend) {
        case ENG_RENDERER_VULKAN:
            return eng_RENDERER_BACKEND_VULKAN_make_interface();
        case ENG_RENDERER_OPENGL:
            return eng_RENDERER_BACKEND_OPENGL_make_interface();
    }

    return 0;
}
