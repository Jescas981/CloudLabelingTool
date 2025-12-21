#include "renderer/Shader.h"
#include "core/Macros.h"
#include "platform/opengl/OpenGLShader.h"

namespace CloudCore {

std::shared_ptr<Shader> Shader::create(const std::string &filepath) {
    #ifdef CC_RENDER_API_OPENGL
        return std::make_shared<OpenGLShader>(filepath);
    #else
    #error "Not API available"
    #endif
}

std::shared_ptr<Shader> Shader::create(const std::string &name,
                                       const std::string &vertexSrc,
                                       const std::string &fragmentSrc) {
   #ifdef CC_RENDER_API_OPENGL
        return std::make_shared<OpenGLShader>(name,vertexSrc,fragmentSrc);
    #else
    #error "Not API available"
    #endif
}

} // namespace CloudCore
