#pragma once
#include <math.h>
#define EPSILON 1e-5f
constexpr float PI = 3.1415926;



class Vec2 {
public:
	Vec2() = default;
	~Vec2() = default;
	Vec2(float x_, float y_) :x(x_), y(y_) {};
	float x=0, y=0;

	Vec2 operator-(const Vec2& rh) const { return Vec2(x - rh.x, y - rh.y); }
	Vec2 operator*(float d)const { return Vec2(x * d, y * d); }
	Vec2 operator/(float d)const { return Vec2(x / d, y / d); }
	Vec2 operator+(const Vec2& rh) const { return Vec2(x + rh.x, y + rh.y); }
	Vec2 operator+(int d) const { return Vec2(x + d, y + d); }
	float dot(const Vec2& rh) const { return x * rh.x + y * rh.y; }
	bool edge(Vec2 start, Vec2 end, Vec2 point) { return (end.x-start.x)*(point.y-start.y)-(point.x-start.x)*(end.y-start.y)>0; }
};

class Vec3 {
public:
	Vec3() = default;
	~Vec3() = default;
	Vec3(float x_, float y_, float z_) :x(x_), y(y_), z(z_) {};
	explicit Vec3(float d) :x(d), y(d), z(d) {};
	float x = 0, y = 0, z = 0;

	float& operator[](int index) {
		return *(&x + index);
	}

	float operator[](int index) const {
		return *(&x + index);
	}

	float dot(const Vec3& rh) {
		return x * rh.x + y * rh.y + z * rh.z;
	}

	Vec3& normalized() {
		float m = sqrtf(x * x + y * y + z * z);
		x /= m;
		y /= m;
		z /= m;
		return *this;
	}

	Vec3 cross(const Vec3& rh) {
		return Vec3(y * rh.z - z * rh.y, z * rh.x - x * rh.z, x * rh.y - y * rh.x);
	}

	Vec3 operator-(const Vec3& rh) const { return Vec3(x - rh.x, y - rh.y, z - rh.z); }
	Vec3 operator-(float d)const { return Vec3(x - d, y - d, z - d); }
	Vec3 operator-()const { return Vec3(-x, -y, -z); }
	Vec3 operator+(float d)const { return Vec3(x + d, y + d, z + d); }
	Vec3 operator*(float d) const { return Vec3(x * d, y * d, z * d); }
	Vec3 operator*(const Vec3& rh) { return Vec3(x * rh.x, y * rh.y, z * rh.z); }
	Vec3 operator/(float d) const { return Vec3(x / d, y / d, z / d); }
	Vec3 operator+(const Vec3& rh) const { return Vec3(x + rh.x, y + rh.y, z + rh.z); }
	float norm() { return sqrtf(x * x + y * y + z * z); }

	Vec2 xy() const { return Vec2(x, y); }
};

class Vec4 {
public:
	Vec4() = default;
	Vec4(float x_, float y_, float z_, float w_=0.0f) :x(x_), y(y_), z(z_), w(w_) {};
	Vec4(const Vec3& p, float w_=1.0f) :x(p.x), y(p.y), z(p.z), w(w_) {};
	~Vec4() = default;
	float x=0, y=0, z=0, w=0;

	float& operator[](int index){
		return *(&x + index);
	}

	float operator[](int index) const {
		return *(&x + index);
	}

	float dot(const Vec4& rh) const {
		return x * rh.x + y * rh.y + z * rh.z+w*rh.w;
	}

	Vec4 cross(const Vec4& rh) {
		return Vec4(y * rh.z - z * rh.y, z * rh.x - x * rh.z, x * rh.y - y * rh.x, 0);
	}

	Vec4& normalized() {
		float m = sqrtf(x * x + y * y + z * z + w * w);
		x /= m;
		y /= m;
		z /= m;
		w /= m;
		return *this;
	}
	Vec4& uniform() { 
		x /= w;
		y /= w;
		z /= w;
		w = 1;
		return *this;
	}
	Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }
	Vec4 operator-(const Vec4& rh) const { return Vec4(x - rh.x, y - rh.y, z - rh.z, w - rh.w); }
	Vec4 operator+(const Vec4& rh) const { return Vec4(x + rh.x, y + rh.y, z + rh.z, w + rh.w); }
	Vec4 operator*(float d) const { return Vec4(x * d, y * d, z * d, w * d); }
	Vec4 operator/(float div) const { return Vec4(x / div, y / div, z / div, w / div); }

	Vec2 xy() const { return Vec2(x, y); }
	Vec3 xyz() const { return Vec3(x, y, z); }
};

class Mat4 {
public:
	Mat4() {
		v[0] = Vec4(1, 0, 0, 0);
		v[1] = Vec4(0, 1, 0, 0);
		v[2] = Vec4(0, 0, 1, 0);
		v[3] = Vec4(0, 0, 0, 1);
	}

	Mat4(const Vec4& v0, const Vec4& v1, const Vec4& v2, const Vec4& v3) {
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		v[3] = v3;
	}

	~Mat4()=default;

	void Translate(float d_x, float d_y, float d_z) {
		Mat4 translation(Vec4(1, 0, 0, d_x), Vec4(0, 1, 0, d_y), Vec4(0, 0, 1, d_z), Vec4(0, 0, 0, 1));
		*this = translation * (*this);
	}

	void Scale(float d_x, float d_y, float d_z) {
		Mat4 scale(Vec4(d_x, 0, 0, 0), Vec4(0, d_y, 0, 0), Vec4(0, 0, d_z, 0), Vec4(0, 0, 0, 1));
		*this = scale * (*this);
	}


	static Mat4 LookAt(Vec3 pos, Vec3 target, Vec3 updir) {
		Vec3 z = (pos-target).normalized();
		Vec3 x = updir.cross(z).normalized();
		Vec3 y = z.cross(x);

		Mat4 rotation(Vec4(x, 0), Vec4(y, 0), Vec4(z, 0), Vec4(0, 0, 0, 1));
		Mat4 translation;
		translation.Translate(-pos.x, -pos.y, -pos.z);

		return  rotation * translation;
	}

	static Mat4 Reflect(Vec3 pos, Vec3 normal) {
		float d = -pos.dot(normal);
		Vec4 temp = Vec4(normal, d)*2;
		Mat4 m = Mat4(temp * normal.x, temp * normal.y, temp * normal.z, Vec4(0, 0, 0, 0));
		return Mat4() - m;
	}

	static Mat4 Ortho(float top,float bottom,float left,float right,float near_z,float far_z) {
		Mat4 e;
		e.Translate(-(right + left) / 2, -(top + bottom) / 2, (near_z + far_z) / 2);
		e.Scale(2 / (right - left), 2 / (top - bottom), 2 / (near_z - far_z));
		return e;
	}

	static Mat4 Perspect(float top, float bottom, float left, float right, float near_z, float far_z) {
		Mat4 persp(Vec4(2*near_z/(right-left), 0, (right+left)/(right-left), 0),
								Vec4(0, 2*near_z/(top-bottom), (top+bottom)/(top-bottom), 0),
								Vec4(0, 0, -(near_z + far_z)/(far_z-near_z), -2*near_z * far_z/(far_z-near_z)),
								Vec4(0, 0, -1, 0));
		return persp;
		//Ortho(top, bottom, left, right, near_z, far_z);
	}

	static Mat4 Perspect(float fov, float aspect, float near_z, float far_z) {
		fov = fov / 180.0 * PI;
		//float t = near_z / fabs(near_z);
		float theta = fov / 2.0;
		return Mat4(Vec4(1.0f/ (aspect * tanf(theta)), 0, 0, 0),
			Vec4(0, 1.0f / tanf(theta), 0, 0),
			Vec4(0, 0, -(far_z + near_z) / (far_z - near_z), -2 * far_z * near_z / (far_z - near_z)),
			Vec4(0, 0, -1.0f, 0));
	}


	Vec4 operator*(const Vec4& rh) const {
		return Vec4(v[0].dot(rh), v[1].dot(rh), v[2].dot(rh), v[3].dot(rh));
	}

	const Mat4 operator*(const Mat4& rh) const {
		Vec4 colum_v[4];
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				colum_v[i][j] = rh.v[j][i];
			}
		}

		Vec4 res[4];
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				res[i][j] = v[i].dot(colum_v[j]);
			}
		}
		 
		return Mat4(res[0],res[1],res[2],res[3]);
	}

	Mat4 operator-(const Mat4& rh) const {
		Mat4 res;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				res.v[i][j] = v[i][j] - rh.v[i][j];
			}
		}
		return res;
	}

	Vec4 v[4];		//ÐÐÏòÁ¿
};


float radians(float angle);
bool InTriangle(const Vec2& v0, const Vec2& v1, const Vec2& v2, const Vec2& p, Vec3& barycenter_coord);
Vec3 CwiseProduct(const Vec3& lh, const Vec3& rh);
