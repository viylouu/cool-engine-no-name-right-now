package intf

PlatformBackend :: enum {
    GLFW
}

PlatformInterface :: struct {
    backend_data: rawptr,
    backend_api: PlatformBackend,

    width, height: u32,

    constr: proc "c" (this: ^PlatformInterface),
    destr: proc "c" (this: ^PlatformInterface),

    is_running: proc "c" (this: ^PlatformInterface) -> bool,

    poll: proc "c" (this: ^PlatformInterface),
    swap: proc "c" (this: ^PlatformInterface),

// renderapi helpers, dont call these
// only included to keep correct struct size

    get_handle: proc "c" (this: ^PlatformInterface) -> rawptr,
    get_frame_size: proc "c" (this: ^PlatformInterface, width, height: ^i32),
    set_resize_callback: proc "c" (this: ^PlatformInterface, callback_data: rawptr, callback: proc "c" (data: rawptr, width, height: i32)),

// renderer specific shit

    supports_vulkan: proc "c" (this: ^PlatformInterface) -> bool
}

RendererBackend :: enum {
    VULKAN
}

Shader :: struct {
    backend_data: rawptr
}

UniformBuffer :: struct {
    backend_data: rawptr,
    shader: ^Shader
}

ShaderStage :: enum {
    VERTEX = 1 << 0,
    FRAGMENT = 1 << 1
}

RendererInterface :: struct {
    backend_data: rawptr,
    backend_api: RendererBackend,

    constr: proc "c" (this: ^RendererInterface, platform: ^PlatformInterface),
    destr: proc "c" (this: ^RendererInterface),

    frame_begin: proc "c" (this: ^RendererInterface, platform: ^PlatformInterface),
    send: proc "c" (this: ^RendererInterface, platform: ^PlatformInterface),

// draw stuff

    // shaders {
        load_shader: proc "c" (this: ^RendererInterface, vert, frag: cstring, uniform_buffers: ^^UniformBuffer, uniform_buffer_count: u32) -> ^Shader, // not sure what the underlying data of a multipointer is, so its this for now. you can just do like &ARR[0] for now
        unload_shader: proc "c" (this: ^RendererInterface, shader: ^Shader),
    // } uniform buffers {
        create_uniform_buffer: proc "c" (
            this: ^RendererInterface, 
            name: cstring,
            stage: u32, // uses the ShaderStage type, but is a u32 so the stage may be bitor'd with others (eg: VERTEX | FRAGMENT would mean its used in the vertex and fragment stages) 
            size: u32 // no clue what size_of returns (and what the c equivalent is)
        ) -> ^UniformBuffer,
        destroy_uniform_buffer: proc "c" (this: ^RendererInterface, uniform_buffer: ^UniformBuffer),
        update_uniform_buffer: proc "c" (this: ^RendererInterface, uniform_buffer: ^UniformBuffer, new_data: rawptr),
    // } commands {
        draw: proc "c" (this: ^RendererInterface, vertices,indices: i32),
    // } bindings {
        bind_shader: proc "c" (this: ^RendererInterface, shader: ^Shader),
        bind_frame_viewport: proc "c" (this: ^RendererInterface),
        bind_viewport: proc "c" (this: ^RendererInterface, viewport: [4]f32)
    // }
}

when ODIN_OS == .Linux {
    foreign import eng "lib/libceng.a"
} else {
    foreign import eng "lib/ceng.lib"
}


@(default_calling_convention = "c")
foreign eng {
    @(link_name="eng_make_platform_interface") make_platform_interface :: proc (backend: PlatformBackend) -> ^PlatformInterface ---
    @(link_name="eng_make_renderer_interface") make_renderer_interface :: proc (backend: RendererBackend) -> ^RendererInterface ---
}
