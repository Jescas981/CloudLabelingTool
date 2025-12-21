#include "renderer/VertexArray.h"
#include "platform/opengl/OpenGLVertexArray.h"
#include "renderer/RenderAPI.h"
#include <iostream>
#include <core/Macros.h>

namespace CloudCore {

std::shared_ptr<VertexArray> VertexArray::create() {
    #ifdef CC_RENDER_API_OPENGL
    return std::make_shared<OpenGLVertexArray>();
    #else
    #error "Platform not supported"
    #endif
}

} // namespace CloudCore
