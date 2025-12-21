#pragma once

#include <Perceptral/renderer/Shader.h>
#include <string>
#include <unordered_map>
#include <memory>

namespace Perceptral {

class ShaderLibrary {
public:
    void add(const std::string& name, const std::shared_ptr<Shader>& shader);
    void add(const std::shared_ptr<Shader>& shader);

    std::shared_ptr<Shader> load(const std::string& filepath);
    std::shared_ptr<Shader> load(const std::string& name, const std::string& filepath);

    std::shared_ptr<Shader> get(const std::string& name);

    bool exists(const std::string& name) const;

private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders_;
};

} // namespace Perceptral
