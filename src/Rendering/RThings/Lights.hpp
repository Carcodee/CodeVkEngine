//
// Created by carlo on 2024-10-28.
//

#ifndef LIGHTS_HPP
#define LIGHTS_HPP

struct Directional_Light
{
	glm::vec3 pos;
	glm::vec3 col;
	float     intensity;
};
struct Point_Light
{
	glm::vec3 pos          = glm::vec3(0.0f);
	glm::vec3 col          = glm::vec3(1.0f);
	float     radius       = 0.0f;
	float     intensity    = 1.0f;
	float     l_attenuation = 0.01f;
	float     q_attenuation = 1.0f;
	Point_Light(glm::vec3 pos, glm::vec3 col, float intensity, float l_attenuation, float q_attenuation)
	{
		this->pos          = pos;
		this->col          = col;
		this->intensity    = intensity;
		this->l_attenuation = l_attenuation;
		this->q_attenuation = q_attenuation;
		Calculate_Radius_From_Params();
	}

	Point_Light(glm::vec3 pos, glm::vec3 col, float radius, float intensity, float l_attenuation, float q_attenuation)
	{
		this->pos          = pos;
		this->col          = col;
		this->intensity    = intensity;
		this->l_attenuation = l_attenuation;
		this->radius       = radius;
		Calculate_Q_Attenuation_From_Radius();
	}
	void Calculate_Radius_From_Params()
	{
		float a = q_attenuation;
		float b = l_attenuation;
		float c = 1.0 - (1.0 / 0.01);
		// Calculate the discriminant
		float discriminant = (b * b) - (4 * a * c);
		if (discriminant < 0)
		{
			std::cout << "Invalid parameters for attenuation or intensity threshold" << std::endl;
		}

		// Calculate possible solutions for the radius
		float r1 = (-b + sqrt(discriminant)) / (2.0f * a);
		float r2 = (-b - sqrt(discriminant)) / (2.0f * a);

		// Choose the positive radius
		radius = std::max(r1, r2);
	}

	void Calculate_Q_Attenuation_From_Radius()
	{
		float threshold = 0.01f;        // Define the threshold
		q_attenuation    = (1.0f / threshold - 1.0f - l_attenuation * radius) / (radius * radius);
	}
};

#endif        // LIGHTS_HPP
