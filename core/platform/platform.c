#include "platform.h"

#include <core/platform/glfw/glfw.h>

/* FUNCS */

EngPlatformInterface* eng_make_platform_interface(EngPlatformBackend backend) {
    switch(backend) {
        case ENG_PLATFORM_GLFW:
            return eng_PLATFORM_BACKEND_GLFW_make_interface();
    }

    return 0;
}
