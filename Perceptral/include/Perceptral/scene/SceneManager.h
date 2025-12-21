#pragma once

#include <Perceptral/scene/Scene.h>
#include <Perceptral/core/DeltaTime.h>
#include <memory>
#include <vector>

namespace Perceptral {

// Manages scene switching and scene stack
class SceneManager {
public:
    SceneManager() = default;
    ~SceneManager();

    // Scene management
    void pushScene(std::shared_ptr<Scene> scene);
    void popScene();
    void setScene(std::shared_ptr<Scene> scene);

    Scene* getCurrentScene() const;
    bool hasScene() const { return !sceneStack_.empty(); }

    // Update/render current scene
    void update(DeltaTime deltaTime);
    void render();

private:
    std::vector<std::shared_ptr<Scene>> sceneStack_;
};

} // namespace Perceptral
