#include "scene/Scene.h"
#include "core/DeltaTime.h"
#include "scene/Components.h"
#include "rendering/BackgroundRenderer.h"
#include "core/Macros.h"


namespace CloudCore {

Scene::Scene(const std::string& name)
    : name_(name) {
    registry_.ctx().emplace<BackgroundSettings>();
}

Scene::~Scene() {
    clear();
}

void Scene::onCreate(){
}

void Scene::onUpdate(DeltaTime deltaTime) {
    UNUSED(deltaTime);
    // Update all systems
}

void Scene::onRender() {
    if (camera_) {
    }
}

Entity Scene::createEntity(const std::string& name) {
    Entity entity = { registry_.create(), this };
    entity.addComponent<TagComponent>(name);
    entity.addComponent<TransformComponent>();
    return entity;
}

void Scene::destroyEntity(Entity entity) {
    registry_.destroy(entity);
}

void Scene::clear() {
    registry_.clear();
}

Entity Scene::findEntityByName(const std::string& name) {
    auto view = registry_.view<TagComponent>();
    for (auto entity : view) {
        const TagComponent& tag = view.get<TagComponent>(entity);
        if (tag.tag == name) {
            return { entity, this };
        }
    }
    return {};
}

} // namespace CloudCore
