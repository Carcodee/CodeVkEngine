//
// Created by carlo on 2024-10-28.
//

#ifndef LIGHTS_HPP
#define LIGHTS_HPP

struct DirectionalLight
{
	glm::vec3 pos;
	glm::vec3 col;
	float     intensity;
};
struct PointLight
{
	glm::vec3 pos          = glm::vec3(0.0f);
	glm::vec3 col          = glm::vec3(1.0f);
	float     radius       = 0.0f;
	float     intensity    = 1.0f;
	float     lAttenuation = 0.01f;
	float     qAttenuation = 1.0f;
	PointLight(glm::vec3 pos, glm::vec3 col, float intensity, float lAttenuation, float qAttenuation)
	{
		this->pos          = pos;
		this->col          = col;
		this->intensity    = intensity;
		this->lAttenuation = lAttenuation;
		this->qAttenuation = qAttenuation;
		CalculateRadiusFromParams();
	}

	PointLight(glm::vec3 pos, glm::vec3 col, float radius, float intensity, float lAttenuation, float qAttenuation)
	{
		this->pos          = pos;
		this->col          = col;
		this->intensity    = intensity;
		this->lAttenuation = lAttenuation;
		this->radius       = radius;
		CalculateQAttenuationFromRadius();
	}
	void CalculateRadiusFromParams()
	{
		float a = qAttenuation;
		float b = lAttenuation;
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

	void CalculateQAttenuationFromRadius()
	{
		float threshold = 0.01f;        // Define the threshold
		qAttenuation    = (1.0f / threshold - 1.0f - lAttenuation * radius) / (radius * radius);
	}
};

#endif        // LIGHTS_HPP
