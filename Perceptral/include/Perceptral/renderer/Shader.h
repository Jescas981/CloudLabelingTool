#pragma once

#include <string>
#include <memory>
#include <Eigen/Core>

namespace Perceptral {

// Abstract Shader interface
class Shader {
public:
    virtual ~Shader() = default;

    virtual void bind() const = 0;
    virtual void unbind() const = 0;

    virtual void setInt(const std::string& name, int value) = 0;
    virtual void setBool(const std::string& name, bool value) = 0;
    virtual void setIntArray(const std::string& name, int* values, uint32_t count) = 0;
    virtual void setFloat(const std::string& name, float value) = 0;
    virtual void setFloat2(const std::string& name, const Eigen::Vector2f& value) = 0;
    virtual void setFloat3(const std::string& name, const Eigen::Vector3f& value) = 0;
    virtual void setFloat4(const std::string& name, const Eigen::Vector4f& value) = 0;
    virtual void setMat4(const std::string& name, const Eigen::Matrix4f& value) = 0;

    virtual const std::string& getName() const = 0;

    // Factory methods
    static std::shared_ptr<Shader> create(const std::string& filepath);
    static std::shared_ptr<Shader> create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
};

} // namespace Perceptral
