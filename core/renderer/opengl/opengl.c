#include "opengl.h"

#include <stdlib.h>

/* INTERFACE FUNCS */

void eng_RENDERER_BACKEND_OPENGL_constr(EngRendererInterface* this, EngPlatformInterface* platform) {
    (void)this;
    (void)platform;
}

void eng_RENDERER_BACKEND_OPENGL_destr(EngRendererInterface* this) {
    free(this);
}

/* FUNCS */

EngRendererInterface* eng_RENDERER_BACKEND_OPENGL_make_interface(void) {
    EngRendererInterface* interface = malloc(sizeof(EngRendererInterface));

    interface->constr = eng_RENDERER_BACKEND_OPENGL_constr;
    interface->destr = eng_RENDERER_BACKEND_OPENGL_destr;

    return interface;
}
