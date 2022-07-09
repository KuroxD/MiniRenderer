#pragma once
#include "Shader.h"

class PBRShader :public IShader {
public:
	PBRShader(std::shared_ptr<Model> model = nullptr):IShader(model){}

	void VertexShader(int f_index, int v_index) {
		//MVP
		VertexOut out;
		out.worldCoord= Vec4(model->GetVertex(f_index, v_index));
		out.clipCoord = unif.vpMat * out.worldCoord;
		out.uv = model->GetUV(f_index, v_index);
		out.normal = model->GetNormal(f_index, v_index);

		unif.vertexs.push_back(out);
	}

	Vec3 SchlickRoughness(float NdotV, float roughness, const Vec3& f0) {
		float r1 = 1 - roughness;
		if (r1 < f0.x)
			r1 = f0.x;

		return f0 + (Vec3(r1,r1,r1) - f0)*powf((1 - NdotV), 5);
	}

	Vec3 CalcuNormal(Vec3& normal,const Vec2& uv, const VertexOut* vertexes[3]) {
		Vec3 sample = model->GetNormal(uv);

		Vec2 duv1 = vertexes[1]->uv - vertexes[0]->uv;
		Vec2 duv2 = vertexes[2]->uv - vertexes[0]->uv;
		float delta = duv1.x * duv2.y - duv2.x * duv1.y;

		Vec3 e1 = vertexes[1]->worldCoord.xyz() - vertexes[0]->worldCoord.xyz();
		Vec3 e2 = vertexes[2]->worldCoord.xyz() - vertexes[0]->worldCoord.xyz();

		Vec3 u = e1 * duv2.y - e2 * duv1.y;
		Vec3 v = e1 * -duv2.x + e2 * duv1.x;
		u = u / delta;
		v = v / delta;

		normal = normal.normalized();
		Vec3 t = (u - normal * u.dot(normal)).normalized();
		Vec3 b = (v - normal * v.dot(normal) - t * v.dot(t)).normalized();

		Vec3 normal_new = t * sample.x + b * sample.y + normal * sample.z;
		return normal_new;

	}

	void Reinhard(Vec4& color) {
		float a = 2.51f;
		float b = 0.03f;
		float c = 2.43f;
		float d = 0.59f;
		float e = 0.14f;

		for (int i = 0; i < 3; i++) {
			color[i] = (color[i] * (a * color[i] + b)) / (color[i] * (c * color[i] + d) + e);
			if (color[i] > 1)
				color[i] = 1.0;
			if (color[i] < 0)
				color[i] = 0;
		}
	}


	Vec4 FragmentShader(const Vec3& barycenterCoord, const VertexOut* vertexes[3]) {
		Vec3 weight(barycenterCoord.x / vertexes[0]->clipCoord.w, barycenterCoord.y / vertexes[1]->clipCoord.w, barycenterCoord.z / vertexes[2]->clipCoord.w);
		
		float normalizer = 1.0 / (weight.x + weight.y + weight.z);
		Vec3 normal = (vertexes[0]->normal * weight.x + vertexes[1]->normal * weight.y + vertexes[2]->normal * weight.z) * normalizer;
		Vec2 uv = (vertexes[0]->uv * weight.x + vertexes[1]->uv * weight.y + vertexes[2]->uv * weight.z) * normalizer;
		Vec4 pos = (vertexes[0]->worldCoord * weight.x + vertexes[1]->worldCoord * weight.y + vertexes[2]->worldCoord * weight.z) * normalizer;

		if (model->normalMap) {
			normal = CalcuNormal(normal, uv, vertexes);
		}


		Vec3 n = normal.normalized();
		Vec3 v = (unif.mainCamera->dPos - pos.xyz()).normalized();
		float n_dot_v = fmax(n.dot(v), 0);
		
		Vec4 color(0, 0, 0, 1);

		if (n_dot_v > 0) {
			float roughness = model->GetRoughness(uv);
			float metalness = model->GetMetalness(uv);
			float occlusion = model->GetOcclusion(uv);
			Vec3 emission = model->GetEmision(uv);

			Vec3 albedo = model->GetDiffuse(uv);


			//f0
			Vec3 temp(0.04, 0.04, 0.04);
			Vec3 f0 = temp + (albedo - temp) * metalness;

			//diffuse
			Vec3 kd = (Vec3(1, 1, 1) - SchlickRoughness(n_dot_v, roughness, f0)) * (1 - metalness);
			CubeMap* irradianceMap = model->iblMap->irradianceMap;
			Vec3 irradiance = irradianceMap->Sample(n);
			//irradiance = irradiance * irradiance;
			Vec3 diffuse = irradiance * kd * albedo;

			//specular
			Vec3 r = (n * 2.0 * v.dot(n) - v).normalized();
			Vec2 lutUV(n_dot_v, roughness);
			Vec3 lutSample = model->iblMap->brdfLut->GetDiffuse(lutUV);
			float specularScale = lutSample.x;
			float specularBias = lutSample.y;
			Vec3 specular = f0 * specularScale + specularBias;
			float maxMipLevel = model->iblMap->mipLevel - 1;
			int mipLevel = roughness * maxMipLevel + 0.5;
			Vec3 prefilterColor = model->iblMap->prefilterMaps[mipLevel]->Sample(r);
			//prefilterColor = prefilterColor * prefilterColor;
			specular = prefilterColor * specular;

			color = (diffuse+specular)* occlusion +emission;
		}
		
		
		Reinhard(color);
		color.x = powf(color.x, 1.0 / 2.2);
		color.y = powf(color.y, 1.0 / 2.2);
		color.z = powf(color.z, 1.0 / 2.2);
		color.w = 1;
		return color;
	}
};