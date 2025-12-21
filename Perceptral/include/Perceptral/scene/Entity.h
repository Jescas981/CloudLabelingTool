#pragma once

#include <entt/entt.hpp>

namespace Perceptral {

class Scene;

/**
 * @brief Handle to an entity in the scene
 *
 * Wraps an EnTT entity handle. EnTT is currently exposed as a transitive
 * dependency of CloudCore.
 *
 * TODO: Consider hiding EnTT from public API in future version using Pimpl pattern.
 */
class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene);
    Entity(const Entity& other) = default;
    Entity& operator=(const Entity& other) = default;

    // Component management (template methods defined in Scene.h)
    template<typename T, typename... Args>
    T& addComponent(Args&&... args);

    template<typename T>
    T& getComponent();

    template<typename T>
    const T& getComponent() const;

    template<typename T>
    bool hasComponent() const;

    template<typename T>
    void removeComponent();

    // Validity check
    operator bool() const { return entityHandle_ != entt::null && scene_ != nullptr; }
    operator entt::entity() const { return entityHandle_; }
    operator uint32_t() const { return static_cast<uint32_t>(entityHandle_); }

    // Comparison operators
    bool operator==(const Entity& other) const {
        return entityHandle_ == other.entityHandle_ && scene_ == other.scene_;
    }

    bool operator!=(const Entity& other) const {
        return !(*this == other);
    }

private:
    entt::entity entityHandle_{ entt::null };
    Scene* scene_ = nullptr;

    friend class Scene;
};

} // namespace Perceptral
