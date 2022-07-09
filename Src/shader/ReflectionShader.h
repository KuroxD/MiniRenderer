#pragma once
#include "Shader.h"
class ReflectionShader :public IShader {
public:
	ReflectionShader(std::shared_ptr<Model> model = nullptr) :IShader(model) {}

	void VertexShader(int fIndex, int vIndex) {
		//MVP
		VertexOut out;
		out.worldCoord = Vec4(model->GetVertex(fIndex, vIndex));

		Mat4 reflectionMat = Mat4::Reflect(Vec3(0, 0, 0), Vec3(0, 1, 0));
		out.clipCoord = unif.vpMat * reflectionMat * out.worldCoord;
		

		out.uv = model->GetUV(fIndex, vIndex);
		out.normal = model->GetNormal(fIndex, vIndex);
		unif.mtl = model->GetMaterial(fIndex);
		unif.vertexs.push_back(out);
	}

	Vec3 Schlick(float NdotV, const Vec3& f0) {
		return f0 + (Vec3(1, 1, 1) - f0) * powf((1 - NdotV), 5);
	}


	Vec4 FragmentShader(const Vec3& barycenterCoord, const VertexOut* vertexes[3]) {
		Vec3 weight(barycenterCoord.x / vertexes[0]->clipCoord.w, barycenterCoord.y / vertexes[1]->clipCoord.w, barycenterCoord.z / vertexes[2]->clipCoord.w);

		float normalizer = 1.0 / (weight.x + weight.y + weight.z);
		Vec3 normal = (vertexes[0]->normal * weight.x + vertexes[1]->normal * weight.y + vertexes[2]->normal * weight.z) * normalizer;
		Vec2 uv = (vertexes[0]->uv * weight.x + vertexes[1]->uv * weight.y + vertexes[2]->uv * weight.z) * normalizer;
		Vec4 pos = (vertexes[0]->worldCoord * weight.x + vertexes[1]->worldCoord * weight.y + vertexes[2]->worldCoord * weight.z) * normalizer;

		Vec3 kd;
		kd = unif.mtl->kdMap->GetDiffuse(uv);
			

		float p = 150.0;
		Vec3 l_pos(0, 15, -15);
		Vec3 light_intensity(250);
		Vec3 l = l_pos;

		float r = l.norm();
		float r2 = r * r;
		Vec3 v = unif.mainCamera->pos - pos.xyz();
		Vec3 h = l.normalized() + v.normalized();
		h.normalized();
		l.normalized();

		normal.normalized();
		//ambient+diffuse+specular
		Vec3 ambient = CwiseProduct(kd, light_intensity / r2);
		Vec3 diffuse = CwiseProduct(kd, light_intensity / r2) * std::max(0.0f, normal.dot(l));
		Vec3 specular = CwiseProduct(Vec3(0.35, 0.35, 0.35), light_intensity / r2) * std::max(0.0f, pow(normal.dot(h), p));

		Vec3 color = ambient + diffuse;

		Vec3 n(0, 1, 0);
		v = (unif.mainCamera->dPos - pos.xyz()).normalized();
		float n_dot_v = fmax(n.dot(v), 0);
		float f0 = powf((1 - n_dot_v), 1.1);

		return color * f0 + Vec3(0.4, 0.4, 0.4) * (1 - f0);
	}
};

