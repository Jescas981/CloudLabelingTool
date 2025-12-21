#include <Perceptral/renderer/RenderAPI.h>
#include <Perceptral/core/Macros.h>

#ifdef PC_RENDER_API_OPENGL
#include <Perceptral/platform/opengl/OpenGLRenderAPI.h>
#endif


namespace Perceptral {

RenderAPI::API RenderAPI::s_API = RenderAPI::API::OpenGL;

std::unique_ptr<RenderAPI> RenderAPI::create() {
    #ifdef PC_RENDER_API_OPENGL
    RenderAPI::s_API = RenderAPI::API::OpenGL;
    return std::make_unique<OpenGLRenderAPI>();
    #else
    #error "Platform not supported"
    #endif
}

} // namespace Perceptral
