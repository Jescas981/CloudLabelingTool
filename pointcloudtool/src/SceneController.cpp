#include "SceneController.h"
#include "core/Log.h"
#include "io/PLYLoader.h"
#include "io/PointCloudExporter.h"
#include <filesystem>

namespace PointCloudTool {

SceneController::SceneController(std::shared_ptr<CloudCore::Scene> scene, CloudCore::Camera* camera)
    : scene_(scene)
    , camera_(camera)
{
    CC_CORE_INFO("SceneController initialized");
}

PointCloudObject* SceneController::addPointCloudObject(const std::string& name, std::shared_ptr<CloudCore::PointCloud> pointCloud)
{
    // Check if object with this name already exists
    if (objectMap_.find(name) != objectMap_.end()) {
        CC_CORE_WARN("Object with name '{}' already exists, skipping", name);
        return nullptr;
    }

    // Compute bounds for the point cloud
    pointCloud->computeBounds();

    // Create the object
    auto object = std::make_unique<PointCloudObject>(name, pointCloud);

    // Create entity in scene
    auto entity = scene_->createEntity(name);
    object->onCreate(entity);

    // Store pointer before moving
    PointCloudObject* objectPtr = object.get();
    objectMap_[name] = objectPtr;
    objects_.push_back(std::move(object));

    CC_CORE_INFO("Added point cloud object '{}' with {} points", name, pointCloud->size());

    return objectPtr;
}

void SceneController::removePointCloudObject(const std::string& name)
{
    auto it = objectMap_.find(name);
    if (it == objectMap_.end()) {
        CC_CORE_WARN("Object '{}' not found", name);
        return;
    }

    // Call destroy lifecycle method
    it->second->onDestroy();

    // Remove from vector
    objects_.erase(
        std::remove_if(objects_.begin(), objects_.end(),
            [&name](const std::unique_ptr<PointCloudObject>& obj) {
                return obj->getName() == name;
            }),
        objects_.end()
    );

    // Remove from map
    objectMap_.erase(it);

    CC_CORE_INFO("Removed point cloud object '{}'", name);
}

void SceneController::removeAllObjects()
{
    for (auto& object : objects_) {
        object->onDestroy();
    }
    objects_.clear();
    objectMap_.clear();
    CC_CORE_INFO("Removed all objects from scene");
}

PointCloudObject* SceneController::getObject(const std::string& name)
{
    auto it = objectMap_.find(name);
    return (it != objectMap_.end()) ? it->second : nullptr;
}

PointCloudObject* SceneController::getObjectAt(size_t index)
{
    if (index < objects_.size()) {
        return objects_[index].get();
    }
    return nullptr;
}

void SceneController::frameCamera(const std::string& objectName)
{
    auto* object = getObject(objectName);
    if (!object) {
        CC_CORE_WARN("Cannot frame camera: object '{}' not found", objectName);
        return;
    }

    Eigen::Vector3f center = object->getCenter();
    float radius = object->getBoundingSphereRadius();

    camera_->frameTarget(center, radius);
    CC_CORE_INFO("Camera framed on object '{}'", objectName);
}

void SceneController::frameCameraAll()
{
    if (objects_.empty()) {
        CC_CORE_WARN("No objects to frame camera on");
        return;
    }

    // Calculate combined bounding box of all objects
    Eigen::Vector3f globalMin = objects_[0]->getMin();
    Eigen::Vector3f globalMax = objects_[0]->getMax();

    for (size_t i = 1; i < objects_.size(); ++i) {
        Eigen::Vector3f objMin = objects_[i]->getMin();
        Eigen::Vector3f objMax = objects_[i]->getMax();

        globalMin = globalMin.cwiseMin(objMin);
        globalMax = globalMax.cwiseMax(objMax);
    }

    // Calculate center and radius
    Eigen::Vector3f center = (globalMin + globalMax) * 0.5f;
    float radius = (globalMax - center).norm();

    camera_->frameTarget(center, radius);
    CC_CORE_INFO("Camera framed on all {} objects", objects_.size());
}

void SceneController::resetCamera()
{
    if (!objects_.empty()) {
        frameCameraAll();
    } else {
        camera_->frameTarget(defaultCameraTarget_, defaultCameraDistance_);
    }
}

void SceneController::update(CloudCore::DeltaTime deltaTime)
{
    // Update all objects
    for (auto& object : objects_) {
        object->onUpdate(deltaTime);
    }
}

bool SceneController::loadPointCloudFromFile(const std::string& filepath)
{
    std::shared_ptr<CloudCore::PointCloud> pointCloud;

    // Detect file type by extension
    std::filesystem::path path(filepath);
    std::string extension = path.extension().string();

    // Convert extension to lowercase for case-insensitive comparison
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    CC_CORE_INFO("Loading point cloud from: {}", filepath);

    if (extension == ".ply") {
        pointCloud = CloudCore::PLYLoader::load(filepath);
    } else {
        CC_CORE_ERROR("Unsupported file format: {}", extension);
        return false;
    }

    if (!pointCloud || pointCloud->empty()) {
        CC_CORE_ERROR("Failed to load point cloud or point cloud is empty");
        return false;
    }

    // Extract filename (without extension) for object name
    std::string name = path.stem().string();

    // Add loaded point cloud to scene
    auto* object = addPointCloudObject(name, pointCloud);
    if (!object) {
        CC_CORE_ERROR("Failed to add point cloud object to scene");
        return false;
    }

    // Frame camera on newly loaded object
    frameCamera(name);

    CC_CORE_INFO("Successfully loaded point cloud '{}' with {} points", name, pointCloud->size());
    return true;
}

bool SceneController::savePointCloudToFile(const std::string& objectName, const std::string& filepath, const std::string& format)
{
    // Find the object
    auto* object = getObject(objectName);
    if (!object) {
        CC_CORE_ERROR("Cannot save: object '{}' not found", objectName);
        return false;
    }

    auto pointCloud = object->getPointCloud();
    if (!pointCloud || pointCloud->empty()) {
        CC_CORE_ERROR("Cannot save: point cloud is empty");
        return false;
    }

    auto cloud = pointCloud->getCloud();
    // Get labels from component
    // Get labels from component
    auto* comp = object->getComponent();
    if (comp && !comp->labels.empty()) {
        // Ensure label count matches point count
        if (comp->labels.size() != cloud->points.size()) {
            CC_ERROR("Label count ({}) doesn't match point count ({})", 
                            comp->labels.size(), cloud->points.size());
        } else {
            // Assign labels to each point
            for (size_t i = 0; i < cloud->points.size(); ++i) {
                cloud->points[i].label = comp->labels[i];
            }
            CC_ERROR("Assigned {} labels to point cloud", comp->labels.size());
        }
    }

    // =

    CC_CORE_INFO("Saving point cloud '{}' to: {} (format: {})", objectName, filepath, format);

    bool success = false;
    if (format == "ply") {
        success = CloudCore::PLYLoader::save(filepath, *pointCloud, true);
    } else if (format == "ply_ascii") {
        success = CloudCore::PLYLoader::save(filepath, *pointCloud, false);
    } else {
        CC_CORE_ERROR("Unsupported save format: {}", format);
        return false;
    }

    if (success) {
        CC_CORE_INFO("Successfully saved point cloud to '{}'", filepath);
    } else {
        CC_CORE_ERROR("Failed to save point cloud to '{}'", filepath);
    }

    return success;
}

} // namespace PointCloudTool
