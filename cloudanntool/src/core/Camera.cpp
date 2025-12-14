#include "core/Camera.h"
#include <iostream>
#include "core/Log.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace CloudCore {

Camera::Camera()
    : position_(0.0f, 0.0f, 0.0f)
    , front_(0.0f, 0.0f, 1.0f)
    , up_(0.0f, 1.0f, 0.0f)
    , right_(1.0f, 0.0f, 0.0f)
    , worldUp_(0.0f, 1.0f, 0.0f)
    , yaw_(0.0f)
    , pitch_(0.0f)
    , orbitTarget_(0.0f, 0.0f, 0.0f)
    , orbitDistance_(5.0f)
    , orbitYaw_(0.0f)
    , orbitPitch_(0.0f)
    , isPerspective_(true)
    , fov_(60.0f)
    , aspectRatio_(16.0f / 9.0f)
    , nearPlane_(0.1f)
    , farPlane_(1000.0f)
    , orthoLeft_(-1.0f)
    , orthoRight_(1.0f)
    , orthoBottom_(-1.0f)
    , orthoTop_(1.0f)
    , movementSpeed_(5.0f)
    , mouseSensitivity_(0.1f)
    , zoomSensitivity_(2.5f)
    , orbitSensitivity_(0.5f)
    , panSensitivity_(0.01f)
{
    // Note: right_ vector will be updated by updateCameraVectors()
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
    // yaw is the angle in the XZ plane (horizontal rotation)
    // pitch is the angle up/down
    yaw_ = std::atan2(direction.x(), -direction.y()) * 180.0f / M_PI;
    pitch_ = std::asin(direction.z()) * 180.0f / M_PI;
    updateCameraVectors();
}

void Camera::setUp(const Eigen::Vector3f& up)
{
    worldUp_ = up.normalized();
    updateCameraVectors();
}

void Camera::setPerspective(float fov, float aspect, float near, float far)
{
    isPerspective_ = true;
    fov_ = fov;
    aspectRatio_ = aspect;
    nearPlane_ = near;
    farPlane_ = far;
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float near, float far)
{
    isPerspective_ = false;
    orthoLeft_ = left;
    orthoRight_ = right;
    orthoBottom_ = bottom;
    orthoTop_ = top;
    nearPlane_ = near;
    farPlane_ = far;
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
    // xoffset *= 0.01;
    yoffset *= mouseSensitivity_;

    // yaw_ += xoffset;
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
    // Create 4x4 transformation matrix
    Eigen::Matrix4f Z_UP_TO_OPENGL = Eigen::Matrix4f::Identity();
    
    // Get 3x3 rotation and embed it in the 4x4 matrix
    Eigen::Matrix3f rotation = (Eigen::AngleAxisf(-M_PI/2.0f, Eigen::Vector3f::UnitX()))
                                .toRotationMatrix();
    Z_UP_TO_OPENGL.block<3,3>(0,0) = rotation;
    
    Eigen::Matrix4f glView = getViewMatrix() * Z_UP_TO_OPENGL; 
    Eigen::Matrix4f projection = getProjectionMatrix();
    Eigen::Matrix4f PV = projection * glView;
    return PV;
}

Eigen::Matrix4f Camera::getViewMatrix() const
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Vector3f f = front_.normalized();
    Eigen::Vector3f r = right_.normalized();
    Eigen::Vector3f u = up_.normalized();

    // Column-major for OpenGL: each column is a basis vector
    // Column 0: right vector
    view(0, 0) = r.x();
    view(1, 0) = r.y();
    view(2, 0) = r.z();
    view(3, 0) = 0.0f;

    // Column 1: up vector
    view(0, 1) = u.x();
    view(1, 1) = u.y();
    view(2, 1) = u.z();
    view(3, 1) = 0.0f;

    // Column 2: front vector
    view(0, 2) = -f.x();
    view(1, 2) = -f.y();
    view(2, 2) = -f.z();
    view(3, 2) = 0.0f;

    // Column 3: translation
    view(0, 3) = -r.dot(position_);
    view(1, 3) = -u.dot(position_);
    view(2, 3) = f.dot(position_);  // Note: positive f.dot since we're using -f in the basis
    view(3, 3) = 1.0f;

    return view;
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
        projection(2, 3) = -(2.0f * farPlane_ * nearPlane_) / (farPlane_ - nearPlane_);
        projection(3, 2) = -1.0f;             // W = -Z (perspective divide)
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
    float yawRad = yaw_ * M_PI / 180.0f;
    float pitchRad = pitch_ * M_PI / 180.0f;

    front.x() = std::sin(yawRad) * std::cos(pitchRad);
    front.y() = std::sin(pitchRad);
    front.z() = std::cos(yawRad) * std::cos(pitchRad);

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
    orbitPitch_ += yoffset;

    // Constrain pitch to avoid gimbal lock
    orbitPitch_ = std::clamp(orbitPitch_, -89.0f, 89.0f);

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
    // Set distance to frame the object (add some padding)
    orbitDistance_ = radius * 2.5f;
    // Reset orbit angles to front view
    orbitYaw_ = 0.0f;
    orbitPitch_ = 20.0f;

    updateOrbitPosition();
}

void Camera::updateOrbitPosition()
{
    // Convert spherical coordinates to Cartesian
    float yawRad = orbitYaw_ * M_PI / 180.0f;
    float pitchRad = orbitPitch_ * M_PI / 180.0f;

    // Calculate position relative to target
    Eigen::Vector3f offset;
    offset.x() = orbitDistance_ * std::cos(pitchRad) * std::sin(yawRad);
    offset.y() = orbitDistance_ * std::sin(pitchRad);
    offset.z() = orbitDistance_ * std::cos(pitchRad) * std::cos(yawRad); 

    position_ = orbitTarget_ + offset;

    // Update camera vectors to look at target
    front_ = (orbitTarget_ - position_).normalized();
    right_ = front_.cross(worldUp_).normalized();
    up_ = right_.cross(front_).normalized();
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
    orbitYaw_ = 180.0f;
    orbitPitch_ = 0.0f;
    updateOrbitPosition();
}

void Camera::setViewTop()
{
    orbitYaw_ = 0.0f;
    orbitPitch_ = 89.0f; // Almost straight down
    updateOrbitPosition();
}

void Camera::setViewBottom()
{
    orbitYaw_ = 0.0f;
    orbitPitch_ = -89.0f; // Almost straight up
    updateOrbitPosition();
}

void Camera::setViewLeft()
{
    orbitYaw_ = -90.0f;
    orbitPitch_ = 0.0f;
    updateOrbitPosition();
}

void Camera::setViewRight()
{
    orbitYaw_ = 90.0f;
    orbitPitch_ = 0.0f;
    updateOrbitPosition();
}

} // namespace CloudCore
