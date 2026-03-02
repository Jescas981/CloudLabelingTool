#include "LabelingToolScriptable.hpp"
#include "MenuBarScriptable.hpp"
#include "Perceptral/core/AssetManager.h"
#include "SelectionToolScriptable.hpp"
#include <Perceptral/EntryPoint.h>
#include <Perceptral/Perceptral.h>
#include <Perceptral/core/layers/GameLayer.h>

class PointCloudToolApp : public Perceptral::Application {
public:
  PointCloudToolApp() {}

  void onCreate() override {
    // Add root folder
    Perceptral::AssetManager::addRoot("app", PC_ASSETS);

    // Create scene
    auto scene = std::make_shared<Perceptral::Scene>("MainScene");
    getSceneManager().pushScene(scene);

    // Create camera controller
    auto camEnt = scene->getMainCamera();
    camEnt.addComponent<Perceptral::Component::OrbitCameraController>();

    // Collection FIRST — MenuBar depends on it existing
    auto collectionEnt = scene->createEntity("Collection");
    Perceptral::BindNativeScript<CollectionScriptable>(
        collectionEnt.addComponent<Perceptral::Component::NativeScript>());

    auto selectionEnt = scene->createEntity("SelectionTool");
    Perceptral::BindNativeScript<SelectionToolScriptable>(
        selectionEnt.addComponent<Perceptral::Component::NativeScript>());

    auto labelingEnt = scene->createEntity("LabelingTool");
    Perceptral::BindNativeScript<LabelingToolScriptable>(
        labelingEnt.addComponent<Perceptral::Component::NativeScript>());

    // MenuBar second
    auto menuEnt = scene->createEntity("MenuBar");
    Perceptral::BindNativeScript<MenuBarScriptable>(
        menuEnt.addComponent<Perceptral::Component::NativeScript>());

    // Add layer
    getLayerStack().pushLayer(std::make_unique<Perceptral::GameLayer>(*scene));
  }

  Perceptral::ApplicationConfig getDefaultConfig() override {
    Perceptral::ApplicationConfig config;
    config.windowTitle = "PointCloudTool";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    return config;
  }
};

Perceptral::Application *Perceptral::createApplication() {
  return new PointCloudToolApp();
}
