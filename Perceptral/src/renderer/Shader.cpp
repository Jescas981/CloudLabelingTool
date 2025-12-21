#include <Perceptral/renderer/Shader.h>
#include <Perceptral/core/Macros.h>
#include <Perceptral/platform/opengl/OpenGLShader.h>

namespace Perceptral {

std::shared_ptr<Shader> Shader::create(const std::string &filepath) {
    #ifdef PC_RENDER_API_OPENGL
        return std::make_shared<OpenGLShader>(filepath);
    #else
    #error "Not API available"
    #endif
}

std::shared_ptr<Shader> Shader::create(const std::string &name,
                                       const std::string &vertexSrc,
                                       const std::string &fragmentSrc) {
   #ifdef PC_RENDER_API_OPENGL
        return std::make_shared<OpenGLShader>(name,vertexSrc,fragmentSrc);
    #else
    #error "Not API available"
    #endif
}

} // namespace Perceptral
