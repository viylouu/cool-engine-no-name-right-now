#include <core/platform/platform.h>
#include <core/renderer/renderer.h>

#include <stdio.h>

int main(void) {
    EngPlatformInterface* platform = eng_make_platform_interface(ENG_PLATFORM_GLFW);
    EngRendererInterface* renderer = eng_make_renderer_interface(ENG_RENDERER_VULKAN);

    platform->constr(platform);
    renderer->constr(renderer, platform);

    printf("window!\n");

    while (platform->is_running(platform)) {
        platform->poll(platform);
        renderer->draw_frame(renderer, platform);
        platform->present(platform);
    }

    renderer->destr(renderer);
    platform->destr(platform);
    return 0;
}
