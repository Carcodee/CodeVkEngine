#ifndef U_PBR 
#define U_PBR 

#include "../Utils/uMath.glsl"
#include "../Utils/uShading.glsl"
#include "../Utils/uStructs.glsl"

vec3 u_lambert_diffuse(vec3 col){
	return col/PI;
}

vec3 u_cook_torrance(vec3 normal, vec3 view,vec3 light, float d, float g, vec3 f){

	vec3 dgf = d*g*f;
	float dot1 = max(dot(view, normal), 0.0001);
	float dot2 = max(dot(light, normal), 0.0001);
	float dot_products= 4 * dot1 * dot2;
	return dgf/dot_products;
}


float u_ggx(float roughness, vec3 normal, vec3 halfway){
	float dot = max(dot(normal,halfway), 0.0001);
	dot = pow(dot,2.0);
	float roughness_part = pow(roughness,2.0)-1;
	float denom = PI* pow(((dot * roughness_part)+1), 2.0);
	return pow(roughness,2.0)/denom;
}
float u_g1( float rougness, vec3 x_vector, vec3 normal){
	float k = rougness/2;
	float dot1= max(dot(normal, x_vector), 0.0001);
	float denom= (dot1* (1-k)) +k;
	return dot1/denom;
}

float u_g(float alpha, vec3 n, vec3 v, vec3 l){
	return u_g1(alpha,  n, v) * u_g1(alpha,  n, l);
}

vec3 u_get_brdf(vec3 wo, vec3 wi,vec3 wh, u_pbr_context pbr_context){

	float d = u_ggx(pbr_context.roughness, pbr_context.normal, wh);
	float g = u_g(pbr_context.roughness, pbr_context.normal, wo, wi);
	vec3 f = u_fresnel_shilck(wh, wo, pbr_context.f0);
	vec3 cook_torrence = u_cook_torrance(pbr_context.normal, wo, wi, d, g, f);
	vec3 lambert= u_lambert_diffuse(pbr_context.col);
	vec3 ks = pbr_context.f0;
	vec3 kd = (vec3(1.0) - ks) * (1.0 - pbr_context.metallic);
	vec3 brdf =  (kd * lambert) + cook_torrence;
	return brdf;
}

vec3 u_eval_point_light(u_point_light light, vec3 pos, vec3 wo, u_pbr_context pbr_context){
	float d = u_sdf_sphere(light.pos, pos);
	if(d > light.radius){
		return vec3(0.0);
	}

	vec3 wi = normalize(light.pos - pos);
	vec3 wh = normalize(wi + wo);

	float diff = max(0.00, dot(wi, pbr_context.normal));
	float attenuation = 1.0 / (1.0 + (light.l_attenuation * d) + (light.q_attenuation * (d * d)));

	vec3 brdf = u_get_brdf(wo, wi, wh, pbr_context);

	vec3 final_col = brdf * diff * light.col * attenuation * light.intensity;
	return final_col;
}

///TESTING

#endif 
