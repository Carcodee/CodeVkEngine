#ifndef U_SHADING
#define U_SHADING 

vec3 u_fresnel_shilck(vec3 halfway, vec3 view, vec3 fo){
    float pow_part= 1- max(dot(view, halfway),0.0001);
    pow_part =pow(pow_part,5);
    vec3 vec_pow = pow_part * (vec3(1.0)-fo);
    return fo + vec_pow;
}



#endif 
