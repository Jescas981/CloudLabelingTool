#include <Perceptral/renderer/VertexArray.h>
#include <Perceptral/renderer/RenderAPI.h>
#include <Perceptral/core/Macros.h>

#ifdef PC_RENDER_API_OPENGL
#include <Perceptral/platform/opengl/OpenGLVertexArray.h>
#endif


namespace Perceptral {

std::shared_ptr<VertexArray> VertexArray::create() {
    #ifdef PC_RENDER_API_OPENGL
    return std::make_shared<OpenGLVertexArray>();
    #else
    #error "Platform not supported"
    #endif
}

} // namespace Perceptral
