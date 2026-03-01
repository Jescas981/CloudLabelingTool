#pragma once

#include "PointCloudObject.h"
#include <Perceptral/core/DeltaTime.h>
#include <Perceptral/scene/Scene.h>
#include <Perceptral/core/Camera.h>
#include <memory>
#include <vector>
#include <unordered_map>

namespace PointCloudTool {

/**
 * @brief Controls the scene and manages all point cloud objects
 *
 * Responsibilities:
 * - Add/remove point cloud objects
 * - Update all objects each frame
 * - Manage object lifecycle
 * - Provide object access by name or index
 */
class SceneController {
public:
    SceneController(std::shared_ptr<Perceptral::Scene> scene, Perceptral::Camera* camera);
    ~SceneController() = default;

    // Object management
    PointCloudObject* addPointCloudObject(const std::string& name, std::shared_ptr<Perceptral::PointCloud> pointCloud);
    void removePointCloudObject(const std::string& name);
    void removeAllObjects();

    // File loading and saving
    bool loadPointCloudFromFile(const std::string& filepath);
    bool savePointCloudToFile(const std::string& objectName, const std::string& filepath, const std::string& format = "ply");

    // Access
    PointCloudObject* getObject(const std::string& name);
    PointCloudObject* getObjectAt(size_t index);
    const std::vector<std::unique_ptr<PointCloudObject>>& getObjects() const { return objects_; }
    size_t getObjectCount() const { return objects_.size(); }

    // Camera management
    void frameCamera(const std::string& objectName);
    void frameCameraAll();
    void resetCamera();

    // Update loop
    void update(Perceptral::DeltaTime deltaTime);

    // Scene access
    std::shared_ptr<Perceptral::Scene> getScene() const { return scene_; }
    Perceptral::Camera* getCamera() const { return camera_; }

private:
    std::shared_ptr<Perceptral::Scene> scene_;
    Perceptral::Camera* camera_;  // Non-owning pointer
    std::vector<std::unique_ptr<PointCloudObject>> objects_;
    std::unordered_map<std::string, PointCloudObject*> objectMap_;

    Eigen::Vector3f defaultCameraTarget_{0.0f, 0.0f, 0.0f};
    float defaultCameraDistance_ = 10.0f;
};

} // namespace PointCloudTool
