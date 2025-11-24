#include <core/platform/platform.h>
#include <core/renderer/renderer.h>

int main(void) {
    EngPlatformInterface* platform = eng_make_platform_interface(ENG_PLATFORM_GLFW);
    EngRendererInterface* renderer = eng_make_renderer_interface(ENG_RENDERER_VULKAN);

    platform->constr(platform);
    renderer->constr(renderer, platform);

    EngShader* shader = renderer->load_shader(renderer, "data/eng/shaders/vulkan/tri.vert.spv", "data/eng/shaders/vulkan/tri.frag.spv");

    while (platform->is_running(platform)) {
        platform->poll(platform);
        renderer->frame_begin(renderer, platform);

        renderer->bind_shader(renderer, shader);
        renderer->bind_frame_viewport(renderer);
        renderer->draw(renderer, 6,1);

        renderer->send(renderer, platform);
        platform->swap(platform);
    }

    renderer->unload_shader(renderer, shader);

    renderer->destr(renderer);
    platform->destr(platform);
    return 0;
}
