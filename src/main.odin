package main

import "../intf"

main :: proc() {
    platf := intf.make_platform_interface(.GLFW)
    platf->constr()

    render := intf.make_renderer_interface(.OPENGL)
    render->constr(platf)

    shader := render->load_shader("data/eng/shaders/opengl/rect.vert", "data/eng/shaders/opengl/rect.frag", nil,0,nil,0)

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
