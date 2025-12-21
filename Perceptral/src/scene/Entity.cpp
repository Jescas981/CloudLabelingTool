#include <Perceptral/scene/Entity.h>
#include <Perceptral/scene/Scene.h>

namespace Perceptral {

Entity::Entity(entt::entity handle, Scene* scene)
    : entityHandle_(handle), scene_(scene) {
}

} // namespace Perceptral
