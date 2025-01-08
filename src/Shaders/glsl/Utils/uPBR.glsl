#ifndef U_PBR 
#define U_PBR 

#include "../Utils/uMath.glsl"
#include "../Utils/uShading.glsl"

vec3 U_LambertDiffuse(vec3 col){
	return col/PI;
}

vec3 U_CookTorrance(vec3 normal, vec3 view,vec3 light, float D, float G, vec3 F){

	vec3 DGF = D*G*F;
	float dot1 = max(dot(view, normal), 0.0001);
	float dot2 = max(dot(light, normal), 0.0001);
	float dotProducts= 4 * dot1 * dot2;
	return DGF/dotProducts;
}

float U_GGX(float roughness, vec3 normal, vec3 halfway){
	float dot = max(dot(normal,halfway), 0.0001);
	dot = pow(dot,2.0);
	float roughnessPart = pow(roughness,2.0)-1;
	float denom = PI* pow(((dot * roughnessPart)+1), 2.0);
	return pow(roughness,2.0)/denom;
}
float u_G1( float rougness, vec3 xVector, vec3 normal){
	
	float k = rougness/2;
	float dot1= max(dot(normal, xVector), 0.0001);
	float denom= (dot1* (1-k)) +k;
	return dot1/denom;
}

float u_G(float alpha, vec3 N, vec3 V, vec3 L){
	return u_G1(alpha,  N, V) * u_G1(alpha,  N, L);
}

vec3 u_GetBRDF(vec3 normal, vec3 wo, vec3 wi,vec3 wh, vec3 col, vec3 FO, float metallic,  float roughness){

	float D = U_GGX(roughness, normal, wh);
	float G = u_G(roughness, normal, wo, wi);
	vec3 F = u_FresnelShilck(wh, wo, FO);
	vec3 cookTorrence = U_CookTorrance(normal, wo, wi, D, G, F);
	vec3 lambert= U_LambertDiffuse(col);
	vec3 ks = F;
	vec3 kd = (vec3(1.0) - ks) * (1 - metallic);
	vec3 BRDF =  (kd * lambert) + cookTorrence;
	return BRDF;
}
///TESTING

#endif 
