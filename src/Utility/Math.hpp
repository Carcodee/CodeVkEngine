//
// Created by carlo on 2025-04-06.
//

#ifndef MATH_HPP
#define MATH_HPP

namespace UTIL{

     inline float M_NextFloat(float min, float max)
     {
          return 0;
     }
     inline glm::vec4 M_NormalizeRotation(const glm::vec4 rot) {
          float sumOfSqaures = rot.x * rot.x + rot.y * rot.y + rot.z * rot.z + rot.w * rot.w;
          float normalizedVal = std::sqrt(sumOfSqaures);
          return glm::vec4(rot.x / normalizedVal, rot.y / normalizedVal, rot.z / normalizedVal, rot.w / normalizedVal);
     };
     inline float M_Sigmoid(float val)
     {
          return 1/ (1.0 + std::exp(-val));
     }

     constexpr float C0 = 0.28209479177387814f;
     inline glm::vec3 M_SH2RGB(const glm::vec3 color) {
          return 0.5f + C0 * color;
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

     


}

#endif //MATH_HPP
