#ifndef U_MATH 
#define U_MATH 
#define PI 3.1415
#define INVERSE_PI 3.1415

#define MAGIC 0x4d415449u

#define MAX_SH_COEFFS 16

const float sh_c0 = 0.28209479177387814f;
const float sh_c1 = 0.4886025119029199f;
const float sh_c2[] = {
1.0925484305920792f,
-1.0925484305920792f,
0.31539156525252005f,
-1.0925484305920792f,
0.5462742152960396f
};
const float sh_c3[] = {
-0.5900435899266435f,
2.890611442640554f,
-0.4570457994644658f,
0.3731763325901154f,
-0.4570457994644658f,
1.445305721320277f,
-0.5900435899266435f
};

float Abs_Cos_Theta_Ws(vec3 v1, vec3 v2){
    return abs(dot(v1, v2));
}
float Cos_Theta_Ws(vec3 v1, vec3 v2){
    return dot(v1, v2);
}
float Abs_Cos_Theta_Ns(vec3 v1){
    return v1.z;
}


//float
float U_Lerp(float start, float end, float v){
    return start + v * (end - start);
}
float U_Inv_Lerp(float start, float end, float v){
    return (v - start)/ (end - start);
}
float U_Remap(float i_start, float i_end, float o_start, float o_end, float v){
    return o_start + (o_end - o_start) * (v - i_start)/(i_end - i_start);
}

//vec2
vec2 U_Lerp(vec2 start, vec2 end, float v){
    return start + v * (end - start);
}
vec2 U_Inv_Lerp(vec2 start, vec2 end, float v){
    return (v - start)/ (end - start);
}
vec2 U_Remap(vec2 i_start, vec2 i_end, vec2 o_start, vec2 o_end, float v){
    return o_start + (o_end - o_start) * (v - i_start)/(i_end - i_start);
}
vec2 U_Lerp(vec2 start, vec2 end, vec2 v){
    return start + v * (end - start);
}
vec2 U_Inv_Lerp(vec2 start, vec2 end, vec2 v){
    return (v - start)/ (end - start);
}
vec2 U_Remap(vec2 i_start, vec2 i_end, vec2 o_start, vec2 o_end, vec2 v){
    return o_start + (o_end - o_start) * (v - i_start)/(i_end - i_start);
}

//vec3
vec3 U_Lerp(vec3 start, vec3 end, float v){
    return start + v * (end - start);
}
vec3 U_Inv_Lerp(vec3 start, vec3 end, float v){
    return (v - start)/ (end - start);
}
vec3 U_Remap(vec3 i_start, vec3 i_end, vec3 o_start, vec3 o_end, float v){
    return o_start + (o_end - o_start) * (v - i_start)/(i_end - i_start);
}
vec3 U_Lerp(vec3 start, vec3 end, vec3 v){
    return start + v * (end - start);
}
vec3 U_Inv_Lerp(vec3 start, vec3 end, vec3 v){
    return (v - start)/ (end - start);
}
vec3 U_Remap(vec3 i_start, vec3 i_end, vec3 o_start, vec3 o_end, vec3 v){
    return o_start + (o_end - o_start) * (v - i_start)/(i_end - i_start);
}

float U_Min(float v1, float v2){
    return min(v1, v2);
}
vec2 U_Min(vec2 v1, vec2 v2){
    return vec2(min(v1.x, v2.x), min(v1.y, v2.y));
}
vec3 U_Min(vec3 v1,vec3 v2){
    return vec3(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z));
}

float U_Max(float v1, float v2){
    return max(v1, v2);
}
vec2 U_Max(vec2 v1, vec2 v2){
    return vec2(max(v1.x, v2.x), max(v1.y, v2.y));
}
vec3 U_Max(vec3 v1,vec3 v2){
    return vec3(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z));
}

vec3 U_Hsl_To_Rgb(float h, float s, float l) {
    float r, g, b;

    float q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
    float p = 2.0 * l - q;

    float hk = mod(h, 1.0);
    float t_r = hk + 1.0 / 3.0;
    float t_g = hk;
    float t_b = hk - 1.0 / 3.0;

    r = t_r < 0.0 ? t_r + 1.0 : (t_r > 1.0 ? t_r - 1.0 : t_r);
    g = t_g < 0.0 ? t_g + 1.0 : (t_g > 1.0 ? t_g - 1.0 : t_g);
    b = t_b < 0.0 ? t_b + 1.0 : (t_b > 1.0 ? t_b - 1.0 : t_b);

    r = r < 1.0 / 6.0 ? p + (q - p) * 6.0 * r : (r < 1.0 / 2.0 ? q : (r < 2.0 / 3.0 ? p + (q - p) * (2.0 / 3.0 - r) * 6.0 : p));
    g = g < 1.0 / 6.0 ? p + (q - p) * 6.0 * g : (g < 1.0 / 2.0 ? q : (g < 2.0 / 3.0 ? p + (q - p) * (2.0 / 3.0 - g) * 6.0 : p));
    b = b < 1.0 / 6.0 ? p + (q - p) * 6.0 * b : (b < 1.0 / 2.0 ? q : (b < 2.0 / 3.0 ? p + (q - p) * (2.0 / 3.0 - b) * 6.0 : p));

    return vec3(r, g, b);
}

//z up in tangent space
void U_Get_Ortho_Base(vec3 n, out vec3 t, out vec3 b){
    if(n.z < 0.99){
        vec3 arbitrary = vec3(0.0, 0.0, 1.0);
        t = normalize(cross(n, arbitrary));
        b = normalize(cross(n, t));
    }else{
        vec3 arbitrary = vec3(0.0, 1.0, 0.0);
        t = normalize(cross(n, arbitrary));
        b = normalize(cross(n, t));
    }
}
vec4 U_Get_Plane(vec3 p0, vec3 p1, vec3 p2){
    vec3 v1 = p1 - p0;
    vec3 v2 = p2 - p0;

    vec4 plane;
    plane.xyz = normalize(cross(v1, v2));
    
    plane.w = -dot(plane.xyz, p0);
    return plane;
}

float U_Sdf_Plane(vec3 pos, vec3 n, float d_to_plane){
    return dot(pos, n) + d_to_plane;
}

bool U_Sphere_Inside_Plane(vec3 pos, float r, vec3 n, float d_to_plane){
    bool behind_plane = U_Sdf_Plane(pos, n, d_to_plane) < -r;
    return !behind_plane;
}
vec3 U_Line_Intersection_To_Z_Plane(vec3 a, vec3 b, float z_distance){
    vec3 normal = vec3(0.0, 0.0, 1.0);
    vec3 ab =  b - a;
    float t = (z_distance - dot(normal, a)) / dot(normal, ab);
    vec3 result = a + t * ab;
    return result;
}

bool U_Aabb(vec2 min_aabb, vec2 max_aabb, vec2 pos){
    bool x_check = bool(pos.x > min_aabb.x && pos.x < max_aabb.x);
    bool y_check = bool(pos.y > min_aabb.y && pos.y < max_aabb.y);
    return x_check && y_check;
}
bool U_Aabb(vec3 min_aabb, vec3 max_aabb, vec3 pos){
    bool x_check = bool(pos.x > min_aabb.x && pos.x < max_aabb.x);
    bool y_check = bool(pos.y > min_aabb.y && pos.y < max_aabb.y);
    bool z_check = bool(pos.z > min_aabb.z && pos.z < max_aabb.z);
    return x_check && y_check;
}

bool U_Aabb_Sphere(vec2 min, vec2 max, vec2 sphere_pos, float r){

    float closest_x = clamp(sphere_pos.x, min.x, max.x);
    float closest_y = clamp(sphere_pos.y, min.y, max.y);

    float dx = sphere_pos.x - closest_x;
    float dy = sphere_pos.y - closest_y;

    float distance_sqr = dx * dx + dy * dy;

    return distance_sqr < r * r;
}
bool U_Aabb_Sphere(vec3 min, vec3 max, vec3 sphere_pos, float r){

    float closest_x = clamp(sphere_pos.x, min.x, max.x);
    float closest_y = clamp(sphere_pos.y, min.y, max.y);
    float closest_z = clamp(sphere_pos.z, min.z, max.z);

    float dx = sphere_pos.x - closest_x;
    float dy = sphere_pos.y - closest_y;
    float dz = sphere_pos.z - closest_z;

    float distance_sqr = dx * dx + dy * dy + dz * dz ;

    return distance_sqr <= (r * r);
}


float U_Sdf_Sphere(vec3 sphere_pos, vec3 pos){
    return distance(sphere_pos, pos);
}


vec3 U_Compute_Sh(vec3 ray_dir,in vec3 sh_coef[MAX_SH_COEFFS], int band_size) {
    float x = ray_dir.x, y = ray_dir.y, z = ray_dir.z;

    vec3 c = sh_c0 * sh_coef[0];

    c -= sh_c1 * sh_coef[1] * y;
    c += sh_c1 * sh_coef[2] * z;
    c -= sh_c1 * sh_coef[3] * x;
    if (band_size >= 2){
        c += sh_c2[0] * sh_coef[4] * x * y;
        c += sh_c2[1] * sh_coef[5] * y * z;
        c += sh_c2[2] * sh_coef[6] * (2.0 * z * z - x * x - y * y);
        c += sh_c2[3] * sh_coef[7] * z * x;
        c += sh_c2[4] * sh_coef[8] * (x * x - y * y);
        if(band_size >= 3){
            c +=sh_c3[0] * sh_coef[9] * (3.0 * x * x - y * y) * y;
            c +=sh_c3[1] * sh_coef[10] * x * y * z;
            c +=sh_c3[2] * sh_coef[11] * (4.0 * z * z - x * x - y * y) * y;
            c +=sh_c3[3] * sh_coef[12] * z * (2.0 * z * z - 3.0 * x * x - 3.0 * y * y);
            c +=sh_c3[4] * sh_coef[13] * x * (4.0 * z * z - x * x - y * y);
            c +=sh_c3[5] * sh_coef[14] * (x * x - y * y) * z;
            c +=sh_c3[6] * sh_coef[15] * x * (x * x - 3.0 * y * y);
        }
    }
    c += 0.5;
    if (c.x < 0.0) {
        c.x = 0.0;
    }

    //    assert(all(lessThanEqual(c, vec3(159.0))), "invalid sh: %f %f %f\n", c);
    return c;
}
mat4 Grid_3d_Pos_From_1d(float cell_size, uvec3 dimensions, vec3 origin, uint cell_id){

    uvec3 pos_in_grid = uvec3(cell_id % dimensions.x, (cell_id / dimensions.x) % dimensions.y, cell_id / (dimensions.x * dimensions.y));
    vec3 offset= vec3(pos_in_grid.x, pos_in_grid.y, pos_in_grid.z) * cell_size;

    mat4 model = mat4(1.0);
    model[0][0] = cell_size;
    model[1][1] = cell_size;
    model[2][2] = cell_size;

    model[3][0] = origin.x + offset.x;
    model[3][1] = origin.z + offset.y;
    model[3][2] = origin.y + offset.z;
    return model;
}
#endif 