#pragma once
#include "Shader.h"
class ShadowShader :public IShader {
public:
	ShadowShader(std::shared_ptr<Model> model = nullptr) :IShader(model) {}
	void VertexShader(int fIndex, int vIndex) {
		//MVP
		VertexOut out;
		out.worldCoord = Vec4(model->GetVertex(fIndex, vIndex));
		//out.clipCoord = unif.mainCamera->Ortho() * unif.mainCamera->View() * out.worldCoord;
		out.clipCoord = unif.vpMat * out.worldCoord;

		//out.uv = model->GetUV(fIndex, vIndex);
		//out.normal = model->GetNormal(fIndex, vIndex);
		//unif.mtl = model->GetMaterial(fIndex);
		unif.vertexs.push_back(out);
	}

	Vec3 Schlick(float NdotV, const Vec3& f0) {
		return f0 + (Vec3(1, 1, 1) - f0) * powf((1 - NdotV), 5);
	}


	Vec4 FragmentShader(const Vec3& barycenterCoord, const VertexOut* vertexes[3]) {
		float depth=0.0;
		for (int i = 0; i < 3; i++) {
			float vDepth = 0.5f * (vertexes[i]->ndcCoord.z + 1.0f);
			depth += barycenterCoord[i] * vDepth;
		}
		return Vec4(depth,depth,depth,1);
	}
};

