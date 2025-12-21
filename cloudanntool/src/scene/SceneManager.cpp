#include "scene/SceneManager.h"
#include "core/DeltaTime.h"

namespace CloudCore {

SceneManager::~SceneManager() {
    for (auto& scene : sceneStack_) {
        if (scene) {
            scene->onDestroy();
        }
    }
}

void SceneManager::pushScene(std::shared_ptr<Scene> scene) {
    if (scene) {
        sceneStack_.push_back(scene);
        scene->onCreate();
    }
}

void SceneManager::popScene() {
    if (!sceneStack_.empty()) {
        sceneStack_.back()->onDestroy();
        sceneStack_.pop_back();
    }
}

void SceneManager::setScene(std::shared_ptr<Scene> scene) {
    // Clear all scenes and set new one
    for (auto& s : sceneStack_) {
        if (s) s->onDestroy();
    }
    sceneStack_.clear();

    if (scene) {
        sceneStack_.push_back(scene);
        scene->onCreate();
    }
}

Scene* SceneManager::getCurrentScene() const {
    return sceneStack_.empty() ? nullptr : sceneStack_.back().get();
}

void SceneManager::update(DeltaTime deltaTime) {
    if (auto scene = getCurrentScene()) {
        scene->onUpdate(deltaTime);
    }
}

void SceneManager::render() {
    if (auto scene = getCurrentScene()) {
        scene->onRender();
    }
}

} // namespace CloudCore
