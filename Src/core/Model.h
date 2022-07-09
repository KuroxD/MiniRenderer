#pragma once
#include "Math.h"
#include <vector>
#include "TGAImage.h"
#include <iostream>


struct Trianlge {
	int v[3];
	int normal[3];
	int uv[3];
	int material;
};

struct CubeMap {
	Vec3 Sample(const Vec3& direction);
	TGAImage* face[6];
	~CubeMap() {
		for (int i = 0; i < 6; i++) {
			if (face[i])	delete face[i];
		}
	}
};

struct IBLMap {
	int mipLevel;

	CubeMap* irradianceMap;
	CubeMap* prefilterMaps[20];
	TGAImage* brdfLut;
	~IBLMap() {
		if (irradianceMap) delete irradianceMap;
		if (brdfLut) delete brdfLut;
		for (int i = 0; i < mipLevel; i++)	delete prefilterMaps[i];
	}
};

struct Material {
	std::string tag;
	Vec3 Ka;
	Vec3 Kd;
	Vec3 Ks;

	TGAImage* kdMap=nullptr;
};


class Model
{
public:
	Mat4 model_mat;

	std::vector<Trianlge> fBuffer;
	std::vector<Vec3> vertexBuffer;
	std::vector<Vec3> normalBuffer;
	std::vector<Vec2> uvBuffer;
	//PhongMtl
	std::vector<Material> materialBuffer;


	inline const Vec3& GetVertex(int fIndex, int vIndex) const { 	return vertexBuffer[fBuffer[fIndex].v[vIndex]]; }
	inline const Vec3& GetNormal(int fIndex, int vIndex) const { return normalBuffer[fBuffer[fIndex].normal[vIndex]]; }
	inline const Vec2& GetUV(int fIndex, int vIndex) const { return uvBuffer[fBuffer[fIndex].uv[vIndex]]; }
	inline const Material* GetMaterial(int fIndex) const { return &materialBuffer[fBuffer[fIndex].material]; }

	//Vec3 GetDiffuse(const Vec2& uv);
	
	//PBR mtl

	IBLMap* iblMap;
	enum PBRTexture {
		diffuse=0,normal=1,roughness=2,metalness=3,occlusion=4,emision=5
	};


	TGAImage* diffuseMap;
	TGAImage* normalMap;
	TGAImage* roughnessMap;
	TGAImage* metalnessMap;
	TGAImage* occlusionMap;
	TGAImage* emisionMap;


	Vec3 GetDiffuse(const Vec2& uv);
	Vec3 GetNormal(const Vec2& uv);
	float GetRoughness(const Vec2& uv);
	float GetMetalness(const Vec2& uv);
	float GetOcclusion(const Vec2& uv);
	Vec3 GetEmision(const Vec2& uv);


	//Skybox
	CubeMap* enviromentMap;



	Model();
	~Model();
	void LoadModel(const char* filePath,bool isPBR=false,bool isSkyBox=false);
	void LoadMap(const std::string& mapPath);
	void LoadMaterial(const std::string& mtlPath);
	void LoadIBLMap(const std::string& iblPath);
};
