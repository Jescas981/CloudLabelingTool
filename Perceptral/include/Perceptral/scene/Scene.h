#pragma once

#include "Entity.h"
#include <Perceptral/core/DeltaTime.h>
#include <entt/entt.hpp>
#include <string>

namespace Perceptral {

class Camera;

// Scene using EnTT ECS
class Scene {
public:
    Scene(const std::string& name = "Scene");
    ~Scene();

    // Scene lifecycle
    virtual void onCreate();
    virtual void onUpdate(DeltaTime deltaTime);
    virtual void onRender();
    virtual void onDestroy() {}

    // Entity management
    Entity createEntity(const std::string& name = "Entity");
    void destroyEntity(Entity entity);
    void clear();

    // Named entity lookup
    Entity findEntityByName(const std::string& name);

    // Camera management
    void setCamera(Camera* camera) { camera_ = camera; }
    Camera* getCamera() const { return camera_; }

    // Getters
    const std::string& getName() const { return name_; }
    entt::registry& getRegistry() { return registry_; }

private:
    friend class Entity;

    std::string name_;
    entt::registry registry_;
    Camera* camera_ = nullptr;
};

// Entity template method implementations (must be after Scene is fully defined)
template<typename T, typename... Args>
inline T& Entity::addComponent(Args&&... args) {
    return scene_->registry_.template emplace<T>(entityHandle_, std::forward<Args>(args)...);
}

template<typename T>
inline T& Entity::getComponent() {
    return scene_->registry_.template get<T>(entityHandle_);
}

template<typename T>
inline bool Entity::hasComponent() const {
    return scene_->registry_.template all_of<T>(entityHandle_);
}

template<typename T>
inline void Entity::removeComponent() {
    scene_->registry_.template erase<T>(entityHandle_);
}

} // namespace Perceptral
