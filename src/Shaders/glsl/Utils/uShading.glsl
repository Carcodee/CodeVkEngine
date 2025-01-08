#ifndef U_SHADING
#define U_SHADING 

vec3 u_FresnelShilck(vec3 halfway, vec3 view, vec3 FO){
    float powPart= 1- max(dot(view, halfway),0.0001);
    powPart =pow(powPart,5);
    vec3 vecPow = powPart * (vec3(1.0)-FO);
    return FO + vecPow;
}

#endif 
