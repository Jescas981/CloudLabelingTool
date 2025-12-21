#include <Perceptral/core/Camera.h>
#include <Eigen/src/Core/Matrix.h>
#include <iostream>
#include <Perceptral/core/Log.h>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

namespace Perceptral {

Camera::Camera()
    : position_(0.0f, 0.0f, 0.0f)
    , front_(0.0f, 1.0f, 0.0f)
    , up_(0.0f, 0.0f, 1.0f)
    , right_(1.0f, 0.0f, 0.0f)
    , worldUp_(0.0f, 0.0f, 1.0f)
    , yaw_(0.0f)
    , pitch_(0.7068f)
    , orbitTarget_(0.0f, 0.0f, 0.0f)
    , orbitDistance_(5.0f)
    , isPerspective_(true)
    , fov_(60.0f)
    , aspectRatio_(16.0f / 9.0f)
    , nearPlane_(0.1f)
    , farPlane_(50.0f)
    , orthoLeft_(-1.0f)
    , orthoRight_(1.0f)
    , orthoBottom_(-1.0f)
    , orthoTop_(1.0f)
    , movementSpeed_(5.0f)
    , mouseSensitivity_(0.1f)
    , zoomSensitivity_(2.5f)
    , orbitSensitivity_(0.01f)
    , panSensitivity_(0.01f)
{
    updateCameraVectors();
}

Camera::~Camera()
{
}

void Camera::setPosition(const Eigen::Vector3f& position)
{
    position_ = position;
}

void Camera::setTarget(const Eigen::Vector3f& target)
{
    // Direction should point FROM camera TO target (not away from it)
    Eigen::Vector3f direction = (target - position_).normalized();

    // Calculate yaw and pitch from the direction vector
    yaw_ = std::atan2(direction.x(), -direction.y()) * 180.0f / M_PI;
    pitch_ = std::asin(direction.z()) * 180.0f / M_PI;
    updateCameraVectors();
}

void Camera::setUp(const Eigen::Vector3f& up)
{
    worldUp_ = up.normalized();
    updateCameraVectors();
}

void Camera::setPerspective(float fov, float aspect, float nearPlane, float farPlane)
{
    isPerspective_ = true;
    fov_ = fov;
    aspectRatio_ = aspect;
    nearPlane_ = nearPlane;
    farPlane_ = farPlane;
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    isPerspective_ = false;
    orthoLeft_ = left;
    orthoRight_ = right;
    orthoBottom_ = bottom;
    orthoTop_ = top;
    nearPlane_ = nearPlane;
    farPlane_ = farPlane;
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = movementSpeed_ * deltaTime;

    switch (direction) {
        case CameraMovement::Forward:
            position_ += front_ * velocity;
            break;
        case CameraMovement::Backward:
            position_ -= front_ * velocity;
            break;
        case CameraMovement::Left:
            position_ -= right_ * velocity;
            break;
        case CameraMovement::Right:
            position_ += right_ * velocity;
            break;
        case CameraMovement::Up:
            position_ += worldUp_ * velocity;
            break;
        case CameraMovement::Down:
            position_ -= worldUp_ * velocity;
            break;
    }
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= mouseSensitivity_;
    yoffset *= mouseSensitivity_;

    yaw_ += xoffset;
    pitch_ += yoffset;

    if (constrainPitch) {
        pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset)
{
    fov_ -= yoffset * zoomSensitivity_;
    fov_ = std::clamp(fov_, 1.0f, 90.0f);
}

void Camera::rotate(float pitch, float yaw)
{
    pitch_ += pitch;
    yaw_ += yaw;
    pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
    updateCameraVectors();
}

void Camera::zoom(float amount)
{
    fov_ -= amount * zoomSensitivity_;
    fov_ = std::clamp(fov_, 1.0f, 90.0f);
}

Eigen::Matrix4f Camera::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

Eigen::Matrix4f Camera::getViewMatrix() const
{
    Eigen::Vector3f f = front_.normalized();
    Eigen::Vector3f r = right_.normalized();
    Eigen::Vector3f u = up_.normalized();

    Eigen::Matrix4f view_matrix;
    view_matrix << r.x(), r.y(), r.z(), -r.dot(position_),
                   u.x(), u.y(), u.z(), -u.dot(position_),
                   -f.x(), -f.y(), -f.z(), f.dot(position_),
                   0.0f, 0.0f, 0.0f, 1.0f;
   
    return view_matrix;
}

Eigen::Matrix4f Camera::getProjectionMatrix() const
{
    Eigen::Matrix4f projection = Eigen::Matrix4f::Zero();

    if (isPerspective_) {
        // Perspective projection matrix (column-major for OpenGL)
        float tanHalfFov = std::tan(fov_ * M_PI / 360.0f);  // fov_/2 in radians
        float f = 1.0f / tanHalfFov;

        projection(0, 0) = f / aspectRatio_;  // X scaling
        projection(1, 1) = f;                 // Y scaling
        projection(2, 2) = -(farPlane_ + nearPlane_) / (farPlane_ - nearPlane_);
        projection(3, 2) = -1.0f;             // W = -Z (perspective divide)
        projection(2, 3) = -(2.0f * farPlane_ * nearPlane_) / (farPlane_ - nearPlane_);
        // projection(3,3) remains 0.0f
    } else {
        // Orthographic projection matrix (column-major for OpenGL)
        projection(0, 0) = 2.0f / (orthoRight_ - orthoLeft_);
        projection(1, 1) = 2.0f / (orthoTop_ - orthoBottom_);
        projection(2, 2) = -2.0f / (farPlane_ - nearPlane_);  // Negative for right-handed system
        projection(0, 3) = -(orthoRight_ + orthoLeft_) / (orthoRight_ - orthoLeft_);
        projection(1, 3) = -(orthoTop_ + orthoBottom_) / (orthoTop_ - orthoBottom_);
        projection(2, 3) = -(farPlane_ + nearPlane_) / (farPlane_ - nearPlane_);
        projection(3, 3) = 1.0f;
    }

    return projection;
}

void Camera::updateCameraVectors()
{
    Eigen::Vector3f front;
    front.x() = std::sin(yaw_) * std::cos(pitch_);
    front.y() = std::cos(yaw_) * std::cos(pitch_);
    front.z() = std::sin(pitch_);

    front_ = front.normalized();
    right_ = front_.cross(worldUp_).normalized();
    up_ = right_.cross(front_).normalized();
}

// Orbit camera controls
void Camera::orbitRotate(float xoffset, float yoffset)
{
    xoffset *= orbitSensitivity_;
    yoffset *= orbitSensitivity_;

    orbitYaw_ += xoffset;
    orbitPitch_ -= yoffset;

    // Prevent flipping
    orbitPitch_ = std::clamp(orbitPitch_, -1.55f, 1.55f);

    updateOrbitPosition();
}

void Camera::orbitPan(float xoffset, float yoffset)
{
    xoffset *= panSensitivity_ * orbitDistance_;
    yoffset *= panSensitivity_ * orbitDistance_;

    // Pan in camera space (right and up directions)
    // Positive xoffset = move mouse right = pan right
    // Positive yoffset = move mouse up = pan up
    orbitTarget_ += right_ * xoffset;
    orbitTarget_ += up_ * yoffset;

    updateOrbitPosition();
}

void Camera::orbitZoom(float amount)
{
    orbitDistance_ -= amount * zoomSensitivity_;
    orbitDistance_ = std::max(0.1f, orbitDistance_);

    updateOrbitPosition();
}

void Camera::setOrbitTarget(const Eigen::Vector3f& target)
{
    orbitTarget_ = target;
    updateOrbitPosition();
}

void Camera::setOrbitDistance(float distance)
{
    orbitDistance_ = std::max(0.1f, distance);
    updateOrbitPosition();
}

void Camera::frameTarget(const Eigen::Vector3f& target, float radius)
{
    orbitTarget_ = target;
    orbitDistance_ = radius * 2.5f;
    
    orbitPitch_ = 0.7853f;
    orbitYaw_ = -0.4712f;

    updateOrbitPosition();
}


void Camera::updateOrbitPosition()
{
    Eigen::Vector3f offset;
    offset.x() = orbitDistance_ * std::cos(orbitPitch_) * std::sin(orbitYaw_);
    offset.z() = orbitDistance_ * std::sin(orbitPitch_);
    offset.y() = orbitDistance_ * std::cos(orbitPitch_) * std::cos(orbitYaw_);
    
    position_ = orbitTarget_ + offset;

    front_ = (orbitTarget_ - position_).normalized();
    right_ = front_.cross(worldUp_).normalized();
    up_    = right_.cross(front_).normalized();
}

// View presets
void Camera::setViewFront()
{
    orbitYaw_ = 0.0f;
    orbitPitch_ = 0.0f;
    updateOrbitPosition();
}

void Camera::setViewBack()
{
    orbitYaw_ = -1.5708f;
    orbitPitch_ = 0.0f;
    updateOrbitPosition();
}

void Camera::setViewTop()
{
    orbitYaw_ = 0.0f;
    orbitPitch_ = 1.5708f;
    updateOrbitPosition();
}

void Camera::setViewBottom()
{
    orbitYaw_ = 0.0f;
    orbitPitch_ = -1.5708f;
    updateOrbitPosition();
}

void Camera::setViewLeft()
{
    orbitYaw_ = -1.5708f;
    orbitPitch_ = 0.0f;
    updateOrbitPosition();
}

void Camera::setViewRight()
{
    orbitYaw_ = 1.5708f;
    orbitPitch_ = 0.0f;
    updateOrbitPosition();
}

} // namespace Perceptral
