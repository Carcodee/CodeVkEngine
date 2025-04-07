//
// Created by carlo on 2024-10-16.
//

#ifndef CAMERA_HPP
#define CAMERA_HPP

namespace Rendering
{
	class Camera
	{
	public:
		struct matrices
		{
			glm::mat4 perspective;
			glm::mat4 view;
		} matrices;
		struct CameraProperties
		{
			float zNear = 0;
			float zFar = 0;
			float aspect = 0;
			float fov = 0;		
		};
		enum CameraMode
		{
			E_FIXED,
			E_FREE
		};

		Camera(glm::vec3 camPos = glm::vec3(0.0f), CameraMode mode = E_FREE, glm::uvec2 aspect = glm::uvec2(800, 600),
		       glm::vec3 lookAt = glm::vec3(0.0f))
		{
			SetLookAt(lookAt);
			SetPerspective(45.0f, (float)aspect.x/ (float)aspect.y, 0.1f, 512.0f);
			this->position = camPos;
			this->currentMode = mode;
			this->mouseInput = glm::vec2(0.0); 
			// switch (this->currentMode)
			// {
			// case E_FIXED:
			// 	SetLookAt(lookAt);
			// 	break;
			// case E_FREE:
			// 	break;
			// }

		}

		void Move(float deltaTime, glm::vec2 input)
		{
			position += forward * input.y * movementSpeed * deltaTime;
			position += right * input.x * movementSpeed * deltaTime;
		}
		void UpdateCam()
		{
			matrices.view = glm::lookAt(position, position + forward, up);
		}
		void RotateCamera()
		{
			//todo: add input handler
			float currentX = mouseInput.x;
			float currentY = mouseInput.y;

			if (firstMouse)
			{
				lastX = currentX;
				lastY = currentY;
				firstMouse = false;
			}

			float xOffset =  lastX - currentX;
			float yOffset =  lastY - currentY;

			lastX = currentX;
			lastY = currentY;

			xOffset *= sens;
			yOffset *= sens;
			
			yaw += xOffset;
			pitch += yOffset;
			
			pitch = glm::clamp(pitch, -89.0f, 89.0f);

			if (inverseY)
			{
				WorldUp = glm::vec3(0.0f, -1.0f,0.0f);
			}

			glm::vec3 camForward;

			camForward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			camForward.y = sin(glm::radians(pitch));
			camForward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

			forward = normalize(camForward);
			right = normalize(cross(forward, WorldUp));
			up = normalize(cross(right, forward));

			
		}

		void SetPerspective(float fov, float aspect, float znear, float zfar)
		{
			this->fov = fov;
			matrices.perspective = glm::perspective(glm::radians(this->fov), aspect, znear, zfar);
			matrices.perspective[1][1] *=-1;
			
			this->cameraProperties.fov = fov;
			this->cameraProperties.aspect =aspect;
			this->cameraProperties.zNear = znear;
			this->cameraProperties.zFar =zfar;
		}

		void SetLookAt(glm::vec3 targetPosition)
		{
			this->lookAt = targetPosition;
			matrices.view = glm::lookAt(position,
			                            lookAt,
			                            WorldUp);
		}


		glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 right;
		glm::vec3 up;
		glm::vec3 lookAt = glm::vec3(0.0f);

		glm::vec2 mouseInput;
		glm::vec3 position;
		float yaw = -90;
		float pitch = -90;
		float lastX = 0;
		float lastY = 0;
		float sens = 0.4f;

		CameraProperties cameraProperties{};
		
		bool firstMouse = true;
		bool inverseY = false;
		CameraMode currentMode;
		float movementSpeed = 5.0f;
		float fov;
	};
}
#endif //CAMERA_HPP
