#pragma once
#include "Shader.h"
class SkyBoxShader :public IShader {
public:
	SkyBoxShader(std::shared_ptr<Model> model = nullptr) :IShader(model) {}

	void VertexShader(int fIndex, int vIndex) {
		//MVP
		Mat4 modelMat;
		modelMat.Translate(unif.mainCamera->pos.x, unif.mainCamera->pos.y, unif.mainCamera->pos.z);
		
		VertexOut out;
		out.worldCoord =  Vec4(model->GetVertex(fIndex, vIndex));
		Mat4 view=Mat4::LookAt(Vec3(0,0,0), unif.mainCamera->dPos, unif.mainCamera->updir);
		out.clipCoord= unif.perspectMat * view * out.worldCoord;
		out.clipCoord.z = out.clipCoord.w;
		unif.vertexs.push_back(out);
	
	}


	Vec4 FragmentShader(const Vec3& barycenterCoord, const VertexOut* vertexes[3]) {
		Vec3 weight(barycenterCoord.x / vertexes[0]->clipCoord.w, barycenterCoord.y / vertexes[1]->clipCoord.w, barycenterCoord.z / vertexes[2]->clipCoord.w);
		float normalizer = 1.0 / (weight.x + weight.y + weight.z);
		Vec4 pos = (vertexes[0]->worldCoord * weight.x + vertexes[1]->worldCoord * weight.y + vertexes[2]->worldCoord * weight.z) * normalizer;


		//return pos.xyz() - var.camera->pos;
		//return pos.xyz();
		return model->enviromentMap->Sample(pos.xyz());
	}
};

