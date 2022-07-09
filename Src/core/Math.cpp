#include "Math.h"


float radians(float angle) {
	return angle / 180.0 * PI;
}

//ÖØÐÄ·¨
bool InTriangle(const Vec2& v0, const Vec2& v1, const Vec2& v2, const Vec2& p,Vec3& barycenter_coord) {
	
	float u = (p.x * (v1.y - v2.y) + (v2.x - v1.x) * p.y + v1.x * v2.y - v2.x * v1.y ) / (v0.x * (v1.y - v2.y) + (v2.x - v1.x) * v0.y + v1.x * v2.y - v2.x * v1.y);
	float v = (p.x * (v2.y - v0.y) + (v0.x - v2.x) * p.y + v2.x * v0.y - v0.x * v2.y) / (v1.x * (v2.y - v0.y) + (v0.x - v2.x) * v1.y + v2.x * v0.y - v0.x * v2.y);
	
	barycenter_coord.x = u;
	barycenter_coord.y = v;
	barycenter_coord.z = 1 - u - v;
	return u >= 0 && v >= 0 && 1 - u - v >= 0;
}

Vec3 CwiseProduct(const Vec3& lh, const Vec3& rh){
	return Vec3(lh.x * rh.x, lh.y * rh.y, lh.z * rh.z);
}
