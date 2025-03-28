#ifndef U_MATH 
#define U_MATH 
#define PI 3.1415
#define INVERSE_PI 3.1415



float AbsCosThetaWs(vec3 v1, vec3 v2){
    return abs(dot(v1, v2));
}
float CosThetaWs(vec3 v1, vec3 v2){
    return dot(v1, v2);
}
float AbsCosThetaNs(vec3 v1){
    return v1.z;
}


//float
float u_Lerp(float start, float end, float v){
    return start + v * (end - start);
}
float u_InvLerp(float start, float end, float v){
    return (v - start)/ (end - start);
}
float u_Remap(float iStart, float iEnd, float oStart, float oEnd, float v){
    return oStart + (oEnd - oStart) * (v - iStart)/(iEnd - iStart);
}

//vec2
vec2 u_Lerp(vec2 start, vec2 end, float v){
    return start + v * (end - start);
}
vec2 u_InvLerp(vec2 start, vec2 end, float v){
    return (v - start)/ (end - start);
}
vec2 u_Remap(vec2 iStart, vec2 iEnd, vec2 oStart, vec2 oEnd, float v){
    return oStart + (oEnd - oStart) * (v - iStart)/(iEnd - iStart);
}
vec2 u_Lerp(vec2 start, vec2 end, vec2 v){
    return start + v * (end - start);
}
vec2 u_InvLerp(vec2 start, vec2 end, vec2 v){
    return (v - start)/ (end - start);
}
vec2 u_Remap(vec2 iStart, vec2 iEnd, vec2 oStart, vec2 oEnd, vec2 v){
    return oStart + (oEnd - oStart) * (v - iStart)/(iEnd - iStart);
}

//vec3
vec3 u_Lerp(vec3 start, vec3 end, float v){
    return start + v * (end - start);
}
vec3 u_InvLerp(vec3 start, vec3 end, float v){
    return (v - start)/ (end - start);
}
vec3 u_Remap(vec3 iStart, vec3 iEnd, vec3 oStart, vec3 oEnd, float v){
    return oStart + (oEnd - oStart) * (v - iStart)/(iEnd - iStart);
}
vec3 u_Lerp(vec3 start, vec3 end, vec3 v){
    return start + v * (end - start);
}
vec3 u_InvLerp(vec3 start, vec3 end, vec3 v){
    return (v - start)/ (end - start);
}
vec3 u_Remap(vec3 iStart, vec3 iEnd, vec3 oStart, vec3 oEnd, vec3 v){
    return oStart + (oEnd - oStart) * (v - iStart)/(iEnd - iStart);
}

float u_Min(float v1, float v2){
    return min(v1, v2);
}
vec2 u_Min(vec2 v1, vec2 v2){
    return vec2(min(v1.x, v2.x), min(v1.y, v2.y));
}
vec3 u_Min(vec3 v1,vec3 v2){
    return vec3(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z));
}

float u_Max(float v1, float v2){
    return max(v1, v2);
}
vec2 u_Max(vec2 v1, vec2 v2){
    return vec2(max(v1.x, v2.x), max(v1.y, v2.y));
}
vec3 u_Max(vec3 v1,vec3 v2){
    return vec3(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z));
}

vec3 u_HSLToRGB(float h, float s, float l) {
    float r, g, b;

    float q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
    float p = 2.0 * l - q;

    float hk = mod(h, 1.0);
    float tR = hk + 1.0 / 3.0;
    float tG = hk;
    float tB = hk - 1.0 / 3.0;

    r = tR < 0.0 ? tR + 1.0 : (tR > 1.0 ? tR - 1.0 : tR);
    g = tG < 0.0 ? tG + 1.0 : (tG > 1.0 ? tG - 1.0 : tG);
    b = tB < 0.0 ? tB + 1.0 : (tB > 1.0 ? tB - 1.0 : tB);

    r = r < 1.0 / 6.0 ? p + (q - p) * 6.0 * r : (r < 1.0 / 2.0 ? q : (r < 2.0 / 3.0 ? p + (q - p) * (2.0 / 3.0 - r) * 6.0 : p));
    g = g < 1.0 / 6.0 ? p + (q - p) * 6.0 * g : (g < 1.0 / 2.0 ? q : (g < 2.0 / 3.0 ? p + (q - p) * (2.0 / 3.0 - g) * 6.0 : p));
    b = b < 1.0 / 6.0 ? p + (q - p) * 6.0 * b : (b < 1.0 / 2.0 ? q : (b < 2.0 / 3.0 ? p + (q - p) * (2.0 / 3.0 - b) * 6.0 : p));

    return vec3(r, g, b);
}

//z up in tangent space
void u_GetOrthoBase(vec3 n, out vec3 t, out vec3 b){ 
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
vec4 u_GetPlane(vec3 p0, vec3 p1, vec3 p2){
    vec3 v1 = p1 - p0;
    vec3 v2 = p2 - p0;

    vec4 plane;
    plane.xyz = normalize(cross(v1, v2));
    
    plane.w = -dot(plane.xyz, p0);
    return plane;
}

float u_SDF_Plane(vec3 pos, vec3 n, float dToPlane){
    return dot(pos, n) + dToPlane;
}

bool u_SphereInsidePlane(vec3 pos, float r, vec3 n, float dToPlane){
    bool behindPlane = u_SDF_Plane(pos, n, dToPlane) < -r;
    return !behindPlane;
}
vec3 u_LineIntersectionToZPlane(vec3 A, vec3 B, float zDistance){
    vec3 normal = vec3(0.0, 0.0, 1.0);
    vec3 ab =  B - A;
    float t = (zDistance - dot(normal, A)) / dot(normal, ab);
    vec3 result = A + t * ab;
    return result;
}

bool u_AABB(vec2 minAABB, vec2 maxAABB, vec2 pos){
    bool xCheck = bool(pos.x > minAABB.x && pos.x < maxAABB.x);
    bool yCheck = bool(pos.y > minAABB.y && pos.y < maxAABB.y);
    return xCheck && yCheck;
}
bool u_AABB(vec3 minAABB, vec3 maxAABB, vec3 pos){
    bool xCheck = bool(pos.x > minAABB.x && pos.x < maxAABB.x);
    bool yCheck = bool(pos.y > minAABB.y && pos.y < maxAABB.y);
    bool zCheck = bool(pos.z > minAABB.z && pos.z < maxAABB.z);
    return xCheck && yCheck;
}

bool u_AABB_Sphere(vec2 min, vec2 max, vec2 spherePos, float r){

    float closestX = clamp(spherePos.x, min.x, max.x);
    float closestY = clamp(spherePos.y, min.y, max.y);

    float dx = spherePos.x - closestX;
    float dy = spherePos.y - closestY;

    float distanceSqr = dx * dx + dy * dy;

    return distanceSqr < r * r;
}
bool u_AABB_Sphere(vec3 min, vec3 max, vec3 spherePos, float r){

    float closestX = clamp(spherePos.x, min.x, max.x);
    float closestY = clamp(spherePos.y, min.y, max.y);
    float closestZ = clamp(spherePos.z, min.z, max.z);

    float dx = spherePos.x - closestX;
    float dy = spherePos.y - closestY;
    float dz = spherePos.z - closestZ;

    float distanceSqr = dx * dx + dy * dy + dz * dz ;

    return distanceSqr <= (r * r);
}


float u_SDF_Sphere(vec3 spherePos, vec3 pos){
    return distance(spherePos, pos);
}


#endif 