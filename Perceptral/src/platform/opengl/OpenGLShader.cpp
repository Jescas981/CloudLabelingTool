#include <Perceptral/platform/opengl/OpenGLShader.h>
#include <Perceptral/core/Log.h>
#include <glad/glad.h>
#include <fstream>
#include <iomanip>

namespace Perceptral {

static GLenum shaderTypeFromString(const std::string& type) {
    if (type == "vertex")
        return GL_VERTEX_SHADER;
    if (type == "fragment" || type == "pixel")
        return GL_FRAGMENT_SHADER;
    
    PC_CORE_ASSERT(false, "Unknown shader type!");
    return 0;
}

OpenGLShader::OpenGLShader(const std::string& filepath) {
    std::string source = readFile(filepath);
    auto shaderSources = preProcess(source);
    compile(shaderSources);
    
    // Extract name from filepath
    auto lastSlash = filepath.find_last_of("/\\");
    lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
    auto lastDot = filepath.rfind('.');
    auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
    name_ = filepath.substr(lastSlash, count);
}

OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
    : name_(name) {
    std::unordered_map<uint32_t, std::string> sources;
    sources[GL_VERTEX_SHADER] = vertexSrc;
    sources[GL_FRAGMENT_SHADER] = fragmentSrc;
    compile(sources);
}

OpenGLShader::~OpenGLShader() {
    glDeleteProgram(rendererID_);
}

std::string OpenGLShader::readFile(const std::string& filepath) {
    std::string result;
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    if (in) {
        in.seekg(0, std::ios::end);
        int size = in.tellg();
        if (size != -1) {
            result.resize(size);
            in.seekg(0, std::ios::beg);
            in.read(&result[0], size);
        } else {
            PC_CORE_ERROR("Could not read from file '{0}'", filepath);
        }
    } else {
        PC_CORE_ERROR("Could not open file '{0}'", filepath);
    }
    return result;
}

std::unordered_map<uint32_t, std::string> OpenGLShader::preProcess(const std::string& source) {
    std::unordered_map<uint32_t, std::string> shaderSources;
    
    const char* typeToken = "#type";
    size_t typeTokenLength = strlen(typeToken);
    size_t pos = source.find(typeToken, 0);
    
    while (pos != std::string::npos) {
        size_t eol = source.find_first_of("\r\n", pos);
        PC_CORE_ASSERT(eol != std::string::npos, "Syntax error");
        size_t begin = pos + typeTokenLength + 1;
        std::string type = source.substr(begin, eol - begin);
        PC_CORE_ASSERT(shaderTypeFromString(type), "Invalid shader type specified");
        
        size_t nextLinePos = source.find_first_not_of("\r\n", eol);
        PC_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
        pos = source.find(typeToken, nextLinePos);
        
        shaderSources[shaderTypeFromString(type)] = (pos == std::string::npos) ? 
            source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
    }
    
    return shaderSources;
}

void OpenGLShader::compile(const std::unordered_map<uint32_t, std::string>& shaderSources) {
    GLuint program = glCreateProgram();
    PC_CORE_ASSERT(shaderSources.size() <= 2, "Only 2 shaders supported for now");
    std::array<GLenum, 2> glShaderIDs;
    int glShaderIDIndex = 0;
    
    for (auto& kv : shaderSources) {
        GLenum type = kv.first;
        const std::string& source = kv.second;
        
        GLuint shader = glCreateShader(type);
        const GLchar* sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, 0);
        glCompileShader(shader);
        
        GLint isCompiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
            
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
            glDeleteShader(shader);
            
            PC_CORE_ERROR("{0}", infoLog.data());
            PC_CORE_ASSERT(false, "Shader compilation failure!");
            break;
        }
        
        glAttachShader(program, shader);
        glShaderIDs[glShaderIDIndex++] = shader;
    }
    
    glLinkProgram(program);
    
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
        glDeleteProgram(program);
        
        for (auto id : glShaderIDs)
            glDeleteShader(id);
        
        PC_CORE_ERROR("{0}", infoLog.data());
        PC_CORE_ASSERT(false, "Shader link failure!");
        return;
    }
    
    for (auto id : glShaderIDs) {
        glDetachShader(program, id);
        glDeleteShader(id);
    }
    
    rendererID_ = program;
}

void OpenGLShader::bind() const {
    glUseProgram(rendererID_);
}

void OpenGLShader::unbind() const {
    glUseProgram(0);
}

void OpenGLShader::setInt(const std::string& name, int value) {
    uploadUniformInt(name, value);
}

void OpenGLShader::setBool(const std::string& name, bool value) {
    uploadUniformInt(name, value ? 1 : 0);
}

void OpenGLShader::setIntArray(const std::string& name, int* values, uint32_t count) {
    uploadUniformIntArray(name, values, count);
}

void OpenGLShader::setFloat(const std::string& name, float value) {
    uploadUniformFloat(name, value);
}

void OpenGLShader::setFloat2(const std::string& name, const Eigen::Vector2f& value) {
    uploadUniformFloat2(name, value);
}

void OpenGLShader::setFloat3(const std::string& name, const Eigen::Vector3f& value) {
    uploadUniformFloat3(name, value);
}

void OpenGLShader::setFloat4(const std::string& name, const Eigen::Vector4f& value) {
    uploadUniformFloat4(name, value);
}

void OpenGLShader::setMat4(const std::string& name, const Eigen::Matrix4f& value) {
    uploadUniformMat4(name, value);
}

void OpenGLShader::uploadUniformInt(const std::string& name, int value) {
    GLint location = glGetUniformLocation(rendererID_, name.c_str());
    glUniform1i(location, value);
}

void OpenGLShader::uploadUniformIntArray(const std::string& name, int* values, uint32_t count) {
    GLint location = glGetUniformLocation(rendererID_, name.c_str());
    glUniform1iv(location, count, values);
}

void OpenGLShader::uploadUniformFloat(const std::string& name, float value) {
    GLint location = glGetUniformLocation(rendererID_, name.c_str());
    glUniform1f(location, value);
}

void OpenGLShader::uploadUniformFloat2(const std::string& name, const Eigen::Vector2f& value) {
    GLint location = glGetUniformLocation(rendererID_, name.c_str());
    glUniform2f(location, value.x(), value.y());
}

void OpenGLShader::uploadUniformFloat3(const std::string& name, const Eigen::Vector3f& value) {
    GLint location = glGetUniformLocation(rendererID_, name.c_str());
    glUniform3f(location, value.x(), value.y(), value.z());
}

void OpenGLShader::uploadUniformFloat4(const std::string& name, const Eigen::Vector4f& value) {
    GLint location = glGetUniformLocation(rendererID_, name.c_str());
    glUniform4f(location, value.x(), value.y(), value.z(), value.w());
}

void OpenGLShader::uploadUniformMat3(const std::string& name, const Eigen::Matrix3f& matrix) {
    GLint location = glGetUniformLocation(rendererID_, name.c_str());
    glUniformMatrix3fv(location, 1, GL_FALSE, matrix.data());
}

void OpenGLShader::uploadUniformMat4(const std::string& name, const Eigen::Matrix4f& matrix) {
    GLint location = glGetUniformLocation(rendererID_, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());
}

} // namespace Perceptral