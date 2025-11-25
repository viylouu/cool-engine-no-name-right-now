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

Uniform :: struct {
    backend_data: rawptr
}

Shader :: struct {
    backend_data: rawptr,
    uniforms: ^Uniform,
    uniform_count: u32
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
        load_shader: proc "c" (this: ^RendererInterface, vert, frag: cstring) -> ^Shader,
        unload_shader: proc "c" (this: ^RendererInterface, shader: ^Shader),

        describe_ubo: proc "c" (this: ^RendererInterface, shader: ^Shader),
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
