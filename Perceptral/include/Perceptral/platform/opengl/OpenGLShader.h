#pragma once

#include <Perceptral/renderer/Shader.h>
#include <string>
#include <unordered_map>

namespace Perceptral {

class OpenGLShader : public Shader {
public:
    OpenGLShader(const std::string& filepath);
    OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
    virtual ~OpenGLShader();

    virtual void bind() const override;
    virtual void unbind() const override;

    virtual void setInt(const std::string& name, int value) override;
    virtual void setBool(const std::string& name, bool value) override;
    virtual void setIntArray(const std::string& name, int* values, uint32_t count) override;
    virtual void setFloat(const std::string& name, float value) override;
    virtual void setFloat2(const std::string& name, const Eigen::Vector2f& value) override;
    virtual void setFloat3(const std::string& name, const Eigen::Vector3f& value) override;
    virtual void setFloat4(const std::string& name, const Eigen::Vector4f& value) override;
    virtual void setMat4(const std::string& name, const Eigen::Matrix4f& value) override;

    virtual const std::string& getName() const override { return name_; }

    void uploadUniformInt(const std::string& name, int value);
    void uploadUniformIntArray(const std::string& name, int* values, uint32_t count);
    void uploadUniformFloat(const std::string& name, float value);
    void uploadUniformFloat2(const std::string& name, const Eigen::Vector2f& value);
    void uploadUniformFloat3(const std::string& name, const Eigen::Vector3f& value);
    void uploadUniformFloat4(const std::string& name, const Eigen::Vector4f& value);
    void uploadUniformMat3(const std::string& name, const Eigen::Matrix3f& matrix);
    void uploadUniformMat4(const std::string& name, const Eigen::Matrix4f& matrix);

private:
    std::string readFile(const std::string& filepath);
    std::unordered_map<uint32_t, std::string> preProcess(const std::string& source);
    void compile(const std::unordered_map<uint32_t, std::string>& shaderSources);

    uint32_t rendererID_;
    std::string name_;
};

} // namespace Perceptral
