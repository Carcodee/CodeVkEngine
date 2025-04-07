//
// Created by carlo on 2025-04-06.
//

#ifndef MATH_HPP
#define MATH_HPP

namespace UTIL{

     static float M_NextFloat(float min, float max)
     {
          return 0;
     }
     glm::vec4 NormalizeRotation(glm::vec4& rot) {
          float sumOfSqaures = rot.x * rot.x + rot.y * rot.y + rot.z * rot.z + rot.w * rot.w;
          float normalizedVal = std::sqrt(sumOfSqaures);
          return glm::vec4(rot.x / normalizedVal, rot.y / normalizedVal, rot.z / normalizedVal, rot.w / normalizedVal);
     };
     float Sigmoid(float val)
     {
          return 1/ (1.0 + std::exp(-val));
     }
     


}

#endif //MATH_HPP
