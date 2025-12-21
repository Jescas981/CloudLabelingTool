#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace Perceptral {

enum class CameraMovement {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class Camera {
public:
    Camera();
    ~Camera();

    // Camera setup
    void setPosition(const Eigen::Vector3f& position);
    void setTarget(const Eigen::Vector3f& target);
    void setUp(const Eigen::Vector3f& up);
    void setPerspective(float fov, float aspect, float near, float far);
    void setOrthographic(float left, float right, float bottom, float top, float near, float far);

    // Camera control - FPS style
    void processKeyboard(CameraMovement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);
    void rotate(float pitch, float yaw);
    void zoom(float amount);

    // Orbit camera controls
    void orbitRotate(float xoffset, float yoffset);
    void orbitPan(float xoffset, float yoffset);
    void orbitZoom(float amount);
    void setOrbitTarget(const Eigen::Vector3f& target);
    void setOrbitDistance(float distance);
    void frameTarget(const Eigen::Vector3f& target, float radius);

    // View presets (maintain current orbit distance and target)
    void setViewFront();
    void setViewBack();
    void setViewTop();
    void setViewBottom();
    void setViewLeft();
    void setViewRight();

    // Getters
    const Eigen::Vector3f& getPosition() const { return position_; }
    const Eigen::Vector3f& getFront() const { return front_; }
    const Eigen::Vector3f& getUp() const { return up_; }
    const Eigen::Vector3f& getRight() const { return right_; }
    const Eigen::Vector3f& getOrbitTarget() const { return orbitTarget_; }
    float getOrbitDistance() const { return orbitDistance_; }
    Eigen::Matrix4f getViewProjectionMatrix() const;

    float getFOV() const { return fov_; }
    float getAspectRatio() const { return aspectRatio_; }
    float getNear() const { return nearPlane_; }
    float getFar() const { return farPlane_; }

    // Settings
    void setMovementSpeed(float speed) { movementSpeed_ = speed; }
    void setMouseSensitivity(float sensitivity) { mouseSensitivity_ = sensitivity; }
    void setZoomSensitivity(float sensitivity) { zoomSensitivity_ = sensitivity; }
    void setOrbitSensitivity(float sensitivity) { orbitSensitivity_ = sensitivity; }
    void setPanSensitivity(float sensitivity) { panSensitivity_ = sensitivity; }

private:
    void updateCameraVectors();
    void updateOrbitPosition();
    Eigen::Matrix4f getViewMatrix() const;
    Eigen::Matrix4f getProjectionMatrix() const;

    // Camera attributes
    Eigen::Vector3f position_;
    Eigen::Vector3f front_;
    Eigen::Vector3f up_;
    Eigen::Vector3f right_;
    Eigen::Vector3f worldUp_;

    // Euler angles
    float yaw_;
    float pitch_;

    // Orbit camera attributes
    Eigen::Vector3f orbitTarget_;
    // Eigen::Quaternionf orbitRotation_;  
    float orbitYaw_{0.0};
    float orbitPitch_{0.0};
    float orbitDistance_;

    // Projection parameters
    bool isPerspective_;
    float fov_;
    float aspectRatio_;
    float nearPlane_;
    float farPlane_;

    // Orthographic projection parameters
    float orthoLeft_;
    float orthoRight_;
    float orthoBottom_;
    float orthoTop_;

    // Camera options
    float movementSpeed_;
    float mouseSensitivity_;
    float zoomSensitivity_;
    float orbitSensitivity_;
    float panSensitivity_;
};

} // namespace Perceptral
