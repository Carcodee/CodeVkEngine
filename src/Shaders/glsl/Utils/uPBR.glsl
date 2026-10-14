#ifndef U_PBR 
#define U_PBR 

#include "../Utils/uMath.glsl"
#include "../Utils/uShading.glsl"
#include "../Utils/uStructs.glsl"

vec3 U_Lambert_Diffuse(vec3 col){
	return col/PI;
}

vec3 U_Cook_Torrance(vec3 normal, vec3 view,vec3 light, float d, float g, vec3 f){

	vec3 dgf = d*g*f;
	float dot1 = max(dot(view, normal), 0.0001);
	float dot2 = max(dot(light, normal), 0.0001);
	float dot_products= 4 * dot1 * dot2;
	return dgf/dot_products;
}


float U_Ggx(float roughness, vec3 normal, vec3 halfway){
	float dot = max(dot(normal,halfway), 0.0001);
	dot = pow(dot,2.0);
	float roughness_part = pow(roughness,2.0)-1;
	float denom = PI* pow(((dot * roughness_part)+1), 2.0);
	return pow(roughness,2.0)/denom;
}
float U_G1( float rougness, vec3 x_vector, vec3 normal){
	float k = rougness/2;
	float dot1= max(dot(normal, x_vector), 0.0001);
	float denom= (dot1* (1-k)) +k;
	return dot1/denom;
}

float U_G(float alpha, vec3 n, vec3 v, vec3 l){
	return U_G1(alpha,  n, v) * U_G1(alpha,  n, l);
}

vec3 U_Get_Brdf(vec3 wo, vec3 wi,vec3 wh, U_Pbr_Context pbr_context){

	float d = U_Ggx(pbr_context.roughness, pbr_context.normal, wh);
	float g = U_G(pbr_context.roughness, pbr_context.normal, wo, wi);
	vec3 f = U_Fresnel_Shilck(wh, wo, pbr_context.f0);
	vec3 cook_torrence = U_Cook_Torrance(pbr_context.normal, wo, wi, d, g, f);
	vec3 lambert= U_Lambert_Diffuse(pbr_context.col);
	vec3 ks = pbr_context.f0;
	vec3 kd = (vec3(1.0) - ks) * (1.0 - pbr_context.metallic);
	vec3 brdf =  (kd * lambert) + cook_torrence;
	return brdf;
}

vec3 U_Eval_Point_Light(U_Point_Light light, vec3 pos, vec3 wo, U_Pbr_Context pbr_context){
	float d = U_Sdf_Sphere(light.pos, pos);
	if(d > light.radius){
		return vec3(0.0);
	}

	vec3 wi = normalize(light.pos - pos);
	vec3 wh = normalize(wi + wo);

	float diff = max(0.00, dot(wi, pbr_context.normal));
	float attenuation = 1.0 / (1.0 + (light.l_attenuation * d) + (light.q_attenuation * (d * d)));

	vec3 brdf = U_Get_Brdf(wo, wi, wh, pbr_context);

	vec3 final_col = brdf * diff * light.col * attenuation * light.intensity;
	return final_col;
}

///TESTING

#endif 
