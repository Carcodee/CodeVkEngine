//
// Created by carlo on 2024-10-16.
//

#ifndef CAMERA_HPP
#define CAMERA_HPP

namespace rendering
{
class camera
{
  public:
	struct matrices
	{
		glm::mat4 perspective;
		glm::mat4 view;
	} matrices;
	struct camera_properties
	{
		float z_near  = 0;
		float z_far   = 0;
		float aspect = 0;
		float fov    = 0;
	};
	enum camera_mode
	{
		e_fixed,
		e_free
	};

	camera(glm::vec3 cam_pos = glm::vec3(0.0f), camera_mode mode = e_free, glm::uvec2 aspect = glm::uvec2(800, 600),
	       glm::vec3 look_at = glm::vec3(0.0f))
	{
		set_look_at(look_at);
		set_perspective(45.0f, (float) aspect.x / (float) aspect.y, 0.1f, 512.0f);
		this->position    = cam_pos;
		this->current_mode = mode;
		this->mouse_input  = glm::vec2(0.0);
		// switch (this->current_mode)
		// {
		// case E_FIXED:
		// 	SetLookAt(lookAt);
		// 	break;
		// case E_FREE:
		// 	break;
		// }
	}

	void move(float delta_time, glm::vec2 input)
	{
		position += forward * input.y * movement_speed * delta_time;
		position += right * input.x * movement_speed * delta_time;
	}
	void update_cam()
	{
		matrices.view = glm::lookAt(position, position + forward, up);
	}
	void rotate_camera()
	{
		// todo: add input handler
		float current_x = mouse_input.x;
		float current_y = mouse_input.y;

		if (first_mouse)
		{
			last_x      = current_x;
			last_y      = current_y;
			first_mouse = false;
		}

		float x_offset = last_x - current_x;
		float y_offset = last_y - current_y;

		last_x = current_x;
		last_y = current_y;

		x_offset *= sens;
		y_offset *= sens;

		yaw += x_offset;
		pitch += y_offset;

		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		if (inverse_y)
		{
			world_up = glm::vec3(0.0f, -1.0f, 0.0f);
		}

		glm::vec3 cam_forward;

		cam_forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		cam_forward.y = sin(glm::radians(pitch));
		cam_forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		forward = normalize(cam_forward);
		right   = normalize(cross(forward, world_up));
		up      = normalize(cross(right, forward));
	}

	void set_perspective(float fov, float aspect, float znear, float zfar)
	{
		matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
		matrices.perspective[1][1] *= -1;

		this->camera_properties.fov    = fov;
		this->camera_properties.aspect = aspect;
		this->camera_properties.z_near  = znear;
		this->camera_properties.z_far   = zfar;
	}

	void set_look_at(glm::vec3 target_position)
	{
		this->look_at  = target_position;
		matrices.view = glm::lookAt(position,
		                            look_at,
		                            world_up);
	}
	glm::mat4 get_view_proj()
	{
		return matrices.perspective * matrices.view;
	}

	glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 look_at = glm::vec3(0.0f);

	glm::vec2 mouse_input;
	glm::vec3 position;
	float     yaw   = -90;
	float     pitch = -90;
	float     last_x = 0;
	float     last_y = 0;
	float     sens  = 0.4f;

	camera_properties camera_properties{};

	bool       first_mouse = true;
	bool       inverse_y   = false;
	camera_mode current_mode;
	float      movement_speed = 5.0f;
};
}        // namespace rendering
#endif        // CAMERA_HPP
