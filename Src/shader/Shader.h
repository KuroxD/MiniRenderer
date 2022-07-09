#pragma once
#include "../core/Render.h"
#include "../core/Camera.h"
#include "../core/Model.h"
#include "../core/Math.h"


struct VertexOut {
	Vec3 normal;
	Vec2 uv;
	Vec4 worldCoord;
	Vec4 clipCoord;
	Vec3 ndcCoord;
};

struct uniform {
	std::shared_ptr<Camera> mainCamera=nullptr;
	bool isReflection=false;


	//VP
	Mat4 viewMat;
	Mat4 perspectMat;
	Mat4 vpMat;

	//VertexShader Output
	std::vector<VertexOut> vertexs;

	const Material* mtl=nullptr;
	float* shadowMap = nullptr;
	Mat4 lighVpMat;
	unsigned char* reflectionTexture = nullptr;
};

class IShader {
public:
	uniform unif;
	std::shared_ptr<Model> model;
	IShader(std::shared_ptr<Model> model=nullptr) :model(model) {}
	virtual void VertexShader(int fIndex, int vIndex) = 0;
	virtual Vec4 FragmentShader(const Vec3& barycenterCoord, const VertexOut* vertexes[3]) = 0;
	virtual ~IShader() {};
};

