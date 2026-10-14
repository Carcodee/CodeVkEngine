//
// Created by carlo on 2025-04-06.
//

#ifndef MATH_HPP
#define MATH_HPP

namespace util
{

inline float m_next_float(float min, float max)
{
	return 0;
}
inline glm::vec4 m_normalize_rotation(const glm::vec4 rot)
{
	float sum_of_sqaures  = rot.x * rot.x + rot.y * rot.y + rot.z * rot.z + rot.w * rot.w;
	float normalized_val = std::sqrt(sum_of_sqaures);
	return glm::vec4(rot.x / normalized_val, rot.y / normalized_val, rot.z / normalized_val, rot.w / normalized_val);
};
inline float m_sigmoid(float val)
{
	return 1 / (1.0 + std::exp(-val));
}

constexpr float  c0 = 0.28209479177387814f;
inline glm::vec3 m_sh2_rgb(const glm::vec3 color)
{
	return 0.5f + c0 * color;
};

inline float m_exp(float val)
{
	return std::exp(val);
}

inline glm::vec2 m_exp(glm::vec2 val)
{
	return glm::vec2(std::exp(val.x), std::exp(val.y));
}
inline glm::vec3 m_exp(glm::vec3 val)
{
	return glm::vec3(std::exp(val.x), std::exp(val.y), std::exp(val.z));
}

}        // namespace util

#endif        // MATH_HPP
