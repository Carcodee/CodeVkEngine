#ifndef U_PBR 
#define U_PBR 

#include "../Utils/uMath.glsl"
#include "../Utils/uShading.glsl"
#include "../Utils/uStructs.glsl"

vec3 u_LambertDiffuse(vec3 col){
	return col/PI;
}

vec3 u_CookTorrance(vec3 normal, vec3 view,vec3 light, float D, float G, vec3 F){

	vec3 DGF = D*G*F;
	float dot1 = max(dot(view, normal), 0.0001);
	float dot2 = max(dot(light, normal), 0.0001);
	float dotProducts= 4 * dot1 * dot2;
	return DGF/dotProducts;
}


float u_GGX(float roughness, vec3 normal, vec3 halfway){
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

vec3 u_GetBRDF(vec3 wo, vec3 wi,vec3 wh, u_PBRContext pbrContext){

	float D = u_GGX(pbrContext.roughness, pbrContext.normal, wh);
	float G = u_G(pbrContext.roughness, pbrContext.normal, wo, wi);
	vec3 F = u_FresnelShilck(wh, wo, pbrContext.F0);
	vec3 cookTorrence = u_CookTorrance(pbrContext.normal, wo, wi, D, G, F);
	vec3 lambert= u_LambertDiffuse(pbrContext.col);
	vec3 ks = pbrContext.F0;
	vec3 kd = (vec3(1.0) - ks) * (1.0 - pbrContext.metallic);
	vec3 BRDF =  (kd * lambert) + cookTorrence;
	return BRDF;
}

vec3 u_EvalPointLight(u_PointLight light, vec3 pos, vec3 wo, u_PBRContext pbrContext){
	float d = u_SDF_Sphere(light.pos, pos);
	if(d > light.radius){
		return vec3(0.0);
	}

	vec3 wi = normalize(light.pos - pos);
	vec3 wh = normalize(wi + wo);

	float diff = max(0.00, dot(wi, pbrContext.normal));
	float attenuation = 1.0 / (1.0 + (light.lAttenuation * d) + (light.qAttenuation * (d * d)));

	vec3 brdf = u_GetBRDF(wo, wi, wh, pbrContext);

	vec3 finalCol = brdf * diff * light.col * attenuation * light.intensity;
	return finalCol;
}

///TESTING

#endif 
