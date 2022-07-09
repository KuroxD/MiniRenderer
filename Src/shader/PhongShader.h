#pragma once
#include "Shader.h"
class PhongShader :public IShader {
public:
	PhongShader(std::shared_ptr<Model> model = nullptr) :IShader(model) {}

	void VertexShader(int fIndex, int vIndex) {
		//MVP
		VertexOut out;
		out.worldCoord = Vec4(model->GetVertex(fIndex, vIndex));

		out.clipCoord = unif.vpMat  * out.worldCoord;

		
		out.uv = model->GetUV(fIndex, vIndex);
		out.normal = model->GetNormal(fIndex, vIndex);
		unif.mtl = model->GetMaterial(fIndex);
		unif.vertexs.push_back(out);
	}

	Vec3 Schlick(float NdotV, const Vec3& f0) {
		return f0 + (Vec3(1,1,1) - f0) * powf((1 - NdotV), 5);
	}


	float GetDepth(Vec3 ndcPos) {
		int x = 0.5f * (800-1) * (ndcPos.x + 1);
		int y = 0.5f * (800-1) * (ndcPos.y + 1);
		
		if (y > 800)	y = 799;
		if (y < 0)	y = 0;
		if (x > 800) x = 799;
		if (x < 0) x = 0;

		int pos = y * 800 + x;
		return unif.shadowMap[pos];
	}

	Vec4 FragmentShader(const Vec3& barycenterCoord, const VertexOut* vertexes[3]) {
		Vec3 weight(barycenterCoord.x / vertexes[0]->clipCoord.w, barycenterCoord.y / vertexes[1]->clipCoord.w, barycenterCoord.z / vertexes[2]->clipCoord.w);

		float normalizer = 1.0 / (weight.x + weight.y + weight.z);
		Vec3 normal = (vertexes[0]->normal * weight.x + vertexes[1]->normal * weight.y + vertexes[2]->normal * weight.z) * normalizer;
		Vec2 uv = (vertexes[0]->uv * weight.x + vertexes[1]->uv * weight.y + vertexes[2]->uv * weight.z) * normalizer;
		Vec4 pos = (vertexes[0]->worldCoord * weight.x + vertexes[1]->worldCoord * weight.y + vertexes[2]->worldCoord * weight.z) * normalizer;
		

		
		float shadow = 1;
		Vec4 lightP = unif.lighVpMat * pos;
		Vec3 shadowPos = lightP.xyz() / lightP.w;
		float depthFromLight = GetDepth(shadowPos);

		if (depthFromLight+0.001 < 0.5*(shadowPos.z+1.0f)) {
			shadow = 0;
		}

		Vec3 kd;
		if (unif.mtl != nullptr) {
			if (unif.mtl->kdMap != nullptr) {
				kd = unif.mtl->kdMap->GetDiffuse(uv);
			}
			else {	//floor
				Vec2 screen_coord[3];
				for (int i = 0; i < 3; i++) {
					screen_coord[i].x = 0.5f * (800 - 1) * (vertexes[i]->ndcCoord.x + 1.0f);
					screen_coord[i].y = 0.5f * (800 - 1) * (vertexes[i]->ndcCoord.y + 1.0f);
				}
				Vec2 screenPos = (screen_coord[0] * barycenterCoord.x + screen_coord[1] * barycenterCoord.y + screen_coord[2] * barycenterCoord.z);
				int pos = (int(screenPos.y) * 800 + int(screenPos.x)) * 4;

				Vec4 reflect(unif.reflectionTexture[pos + 0], unif.reflectionTexture[pos + 1], unif.reflectionTexture[pos + 2], 1);
				return reflect/255.0*shadow;
			}
		}
		

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
		Vec3 diffuse = CwiseProduct(kd, light_intensity/r2) * std::max(0.0f, normal.dot(l));
		Vec3 specular = CwiseProduct(Vec3(0.35,0.35,0.35), light_intensity / r2) * std::max(0.0f, pow(normal.dot(h), p));
		
		Vec3 color = ambient + (diffuse+specular)*shadow;
		
		return color;
	}
};

