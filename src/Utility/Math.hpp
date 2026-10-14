//
// Created by carlo on 2025-04-06.
//

#ifndef MATH_HPP
#define MATH_HPP

namespace util
{

inline float M_Next_Float(float min, float max)
{
	return 0;
}
inline glm::vec4 M_Normalize_Rotation(const glm::vec4 rot)
{
	float sum_of_sqaures  = rot.x * rot.x + rot.y * rot.y + rot.z * rot.z + rot.w * rot.w;
	float normalized_val = std::sqrt(sum_of_sqaures);
	return glm::vec4(rot.x / normalized_val, rot.y / normalized_val, rot.z / normalized_val, rot.w / normalized_val);
};
inline float M_Sigmoid(float val)
{
	return 1 / (1.0 + std::exp(-val));
}

constexpr float  c0 = 0.28209479177387814f;
inline glm::vec3 M_Sh2_Rgb(const glm::vec3 color)
{
	return 0.5f + c0 * color;
};

inline float M_Exp(float val)
{
	return std::exp(val);
}

inline glm::vec2 M_Exp(glm::vec2 val)
{
	return glm::vec2(std::exp(val.x), std::exp(val.y));
}
inline glm::vec3 M_Exp(glm::vec3 val)
{
	return glm::vec3(std::exp(val.x), std::exp(val.y), std::exp(val.z));
}

}        // namespace util

#endif        // MATH_HPP
