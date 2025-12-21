#include <Perceptral/scene/Components.h>
#include <Eigen/Geometry>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Perceptral {

Eigen::Matrix4f TransformComponent::getTransform() const {
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();

    // Translation
    transform(0, 3) = position.x();
    transform(1, 3) = position.y();
    transform(2, 3) = position.z();

    // Rotation (Euler angles to rotation matrix)
    float cosX = std::cos(rotation.x() * M_PI / 180.0f);
    float sinX = std::sin(rotation.x() * M_PI / 180.0f);
    float cosY = std::cos(rotation.y() * M_PI / 180.0f);
    float sinY = std::sin(rotation.y() * M_PI / 180.0f);
    float cosZ = std::cos(rotation.z() * M_PI / 180.0f);
    float sinZ = std::sin(rotation.z() * M_PI / 180.0f);

    Eigen::Matrix4f rotX = Eigen::Matrix4f::Identity();
    rotX(1, 1) = cosX; rotX(1, 2) = -sinX;
    rotX(2, 1) = sinX; rotX(2, 2) = cosX;

    Eigen::Matrix4f rotY = Eigen::Matrix4f::Identity();
    rotY(0, 0) = cosY; rotY(0, 2) = sinY;
    rotY(2, 0) = -sinY; rotY(2, 2) = cosY;

    Eigen::Matrix4f rotZ = Eigen::Matrix4f::Identity();
    rotZ(0, 0) = cosZ; rotZ(0, 1) = -sinZ;
    rotZ(1, 0) = sinZ; rotZ(1, 1) = cosZ;

    Eigen::Matrix4f rotation_matrix = rotZ * rotY * rotX;

    // Scale
    Eigen::Matrix4f scale_matrix = Eigen::Matrix4f::Identity();
    scale_matrix(0, 0) = scale.x();
    scale_matrix(1, 1) = scale.y();
    scale_matrix(2, 2) = scale.z();

    // Combine: T * R * S
    transform = transform * rotation_matrix * scale_matrix;

    return transform;
}

} // namespace Perceptral
