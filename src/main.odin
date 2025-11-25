package main

import "../intf"

main :: proc() {
    platf := intf.make_platform_interface(.GLFW)
    platf->constr()

    render := intf.make_renderer_interface(.VULKAN)
    render->constr(platf)

    shader := render->load_shader("data/eng/shaders/vulkan/rect.vert.spv", "data/eng/shaders/vulkan/rect.frag.spv")

    for platf->is_running() {
        platf->poll()
        render->frame_begin(platf)

        render->bind_shader(shader)
        render->bind_frame_viewport()
        render->draw(6,1)

        render->send(platf)
        platf->swap()
    }

    render->unload_shader(shader)

    render->destr()
    platf->destr()
}
