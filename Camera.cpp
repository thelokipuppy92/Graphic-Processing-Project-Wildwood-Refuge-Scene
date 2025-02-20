#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        this->cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
        return glm::lookAt(cameraPosition, cameraFrontDirection + cameraPosition, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction) {
        case MOVE_FORWARD:
            if ((cameraPosition + (cameraFrontDirection * speed)).y < 2.40348) {
                cameraPosition.y = cameraPosition.y;
                /*  if ((cameraPosition + cameraFrontDirection * speed).z < -16.381378 ||
                      (cameraPosition + cameraFrontDirection * speed).z > 33.466751) {
                      cameraPosition.z += cameraPosition.z;
                  }
                  else {*/
                cameraPosition.z += (cameraFrontDirection * speed).z;
                //}
                cameraPosition.x += (cameraFrontDirection * speed).x;

            }
            /* else if ((cameraPosition + cameraFrontDirection * speed).z < -16.381378 ||
                 (cameraPosition + cameraFrontDirection * speed).z > 33.466751) {
                 cameraPosition.z = cameraPosition.z;
                 cameraPosition.x += (cameraFrontDirection * speed).x;
                 if ((cameraPosition + cameraFrontDirection * speed).y < 2.40348) {
                     cameraPosition.y = cameraPosition.y;
                 }
                 else {
                     cameraPosition.y += (cameraFrontDirection * speed).y;
                 }

             }*/
            else {
                cameraPosition += cameraFrontDirection * speed;
            }
            break;

        case MOVE_BACKWARD:
            if ((cameraPosition - (cameraFrontDirection * speed)).y < 2.40348) {
                cameraPosition.y = cameraPosition.y;
                cameraPosition.z -= (cameraFrontDirection * speed).z;
                cameraPosition.x -= (cameraFrontDirection * speed).x;

            }
            else
                cameraPosition -= cameraFrontDirection * speed;
            break;

        case MOVE_RIGHT:
            cameraPosition += cameraRightDirection * speed;
            break;

        case MOVE_LEFT:
            cameraPosition -= cameraRightDirection * speed;
            break;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation aroud the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraFrontDirection = glm::normalize(front);
    }
    glm::vec3 Camera::getCameraPosition()
    {
        return cameraPosition;
    }

    void Camera::setCameraPosition(glm::vec3 pos)
    {
        this->cameraPosition = pos;
    }

    glm::vec3 Camera::getCameraFront() {
        return cameraFrontDirection;
    }

    void Camera::setCameraFrontDirection(glm::vec3 direction) {
        this->cameraFrontDirection = direction;
    }

    glm::vec3 Camera::getCameraTarget() {
        return this->cameraTarget;
    }

    void Camera::scenePreview(float angle) {
        this->cameraPosition = glm::vec3(-0.389868f, 4.84547f, -7.08406f);
        

        glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        this->cameraPosition = glm::vec4(rotationMat * glm::vec4(this->cameraPosition, 1.0f));
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    }

}