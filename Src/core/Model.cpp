#include "Model.h"
#include <fstream>
#include <io.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>

Vec3 Model::GetDiffuse(const Vec2& uv)
{
	int uv_x = fmod(uv.x, 1) * diffuseMap->get_width();
	int uv_y = fmod(uv.y, 1) * diffuseMap->get_height();
	TGAColor c = diffuseMap->get(uv_x, uv_y);
	return Vec3((float)c[2] / 255.f, (float)c[1] / 255.f, (float)c[0] / 255.f);
}

Vec3 Model::GetNormal(const Vec2& uv)
{
	if (normalMap == nullptr)
		return Vec3(0, 0, 0);
	int u = fmod(uv.x, 1) * normalMap->get_width();
	int v = fmod(uv.y, 1) * normalMap->get_height();
	TGAColor c = normalMap->get(u, v);
	Vec3 res((float)c[2] / 255.f, (float)c[1] / 255.f, (float)c[0] / 255.f);
	return res * 2.0 - 1;
}

float Model::GetRoughness(const Vec2& uv)
{
	return roughnessMap->Sample(uv)[0]/255.f;
}

float Model::GetMetalness(const Vec2& uv)
{
	return metalnessMap->Sample(uv)[0]/255.f;
}

float Model::GetOcclusion(const Vec2& uv)
{
	return occlusionMap->Sample(uv)[0]/255.f;
}

Vec3 Model::GetEmision(const Vec2& uv)
{
	TGAColor c = emisionMap->Sample(uv);

	return Vec3((float)c[2] / 255.f, (float)c[1] / 255.f, (float)c[0] / 255.f);
}

CubeMap* LoadCubeMap(char files[][256]) {
	CubeMap* cubeMap = new CubeMap();
	for (int i = 0; i < 6; i++) {
		cubeMap->face[i] = new TGAImage();
		cubeMap->face[i]->read_tga_file(files[i]);
		cubeMap->face[i]->flip_vertically();
	}
	return cubeMap;
}

Model::Model()  {
	diffuseMap = nullptr;
	normalMap = nullptr;
	roughnessMap = nullptr;
	metalnessMap = nullptr;
	occlusionMap = nullptr;
	emisionMap = nullptr;
	iblMap = nullptr;
	enviromentMap = nullptr;
}





Model::~Model()
{
	for (int i = 0; i < materialBuffer.size(); i++) {
		if (materialBuffer[i].kdMap != nullptr) {
			delete materialBuffer[i].kdMap;
		}
	}


	if (diffuseMap) delete diffuseMap;
	if (normalMap) delete normalMap;
	if (roughnessMap) delete roughnessMap;
	if (metalnessMap) delete metalnessMap;
	
}

void Model::LoadModel(const char* filepath,bool isPBR,bool isSkyBox) {
	std::ifstream in;
	in.open(filepath, std::ifstream::in);
	std::string line;
	assert(in.good());

	int cur_mtl = 0;
	while (!in.eof()) {
		std::getline(in, line);
		if (!line.compare(0, 2, "v "))		//vertex
		{
			std::istringstream iss(line.substr(2));
			Vec3 v;
			iss >> v.x>>v.y>>v.z;

			vertexBuffer.push_back(v);
		}
		else if (!line.compare(0, 3, "vn "))	//normal
		{
			std::istringstream iss(line.substr(3));
			Vec3 n;
			iss >> n.x >> n.y >> n.z;

			normalBuffer.push_back(n);
		}
		else if (!line.compare(0, 3, "vt "))		//texture coord
		{
			std::istringstream iss(line.substr(3));
			Vec2 uv;
			iss >> uv.x>>uv.y;
			uv.x = fmod(uv.x, 1.0);
			uv.y = fmod(uv.y, 1.0);
			uv.y = 1 - uv.y;

			
			uvBuffer.push_back(uv);
		}
		else if (!line.compare(0, 2, "f "))	//face
		{
			std::istringstream iss(line.substr(2));
			Trianlge f;
			char trash;
			for (int i = 0; i < 3; i++) {
				iss >> f.v[i] >> trash >> f.uv[i] >> trash >> f.normal[i];
			}
			for (int i = 0; i < 3; i++) {
				f.v[i]--;
				f.uv[i]--;
				f.normal[i]--;
			}
			f.material = cur_mtl;
			fBuffer.push_back(f);
		}
		else if (!line.compare(0, 7, "mtllib ")) {		//Load Material
			std::string path(filepath);
			size_t slash = path.find_last_of('/');
			std::string mtl_path = path.substr(0, slash) + std::string("/") + line.substr(7);
			std::cout <<"LOAD MTL:"<< mtl_path << std::endl;
			LoadMaterial(mtl_path);
		}
		else if (!line.compare(0, 7, "usemtl ")) {	//specify Material
			std::istringstream iss(line.substr(7));
			std::string material_tag;
			iss >> material_tag;
			for (int i = 0; i < materialBuffer.size(); i++) {
				if (materialBuffer[i].tag == material_tag) {
					cur_mtl = i;
					break;
				}
			}
		}
	}
	in.close();

	if (isPBR) {
		LoadMap(filepath);
	}

	if (isSkyBox) {
		char paths[6][256];
		const char* suffix[] = {
			"right","left","top","bottom","back","front"
		};
		std::string path(filepath);
		size_t dot = path.find_last_of(".");
		path = path.substr(0, dot);

		for (int i = 0; i < 6; i++) {
			sprintf_s(paths[i], "%s_%s.tga", path.c_str(), suffix[i]);
		}
		enviromentMap=LoadCubeMap(paths);
	}
}

void Model::LoadMaterial(const std::string& mtlpath)
{
	std::ifstream in;
	in.open(mtlpath, std::ifstream::in);
	std::string line;
	assert(in.good());

	while (!in.eof()) {
		std::getline(in, line);
		if (!line.compare(0, 7, "newmtl "))		//new mtl
		{
			std::istringstream iss(line.substr(7));

			materialBuffer.push_back(Material());
			iss >> materialBuffer.back().tag;
		}
		else if (!line.compare(0, 3, "Ka ")) {
			std::istringstream iss(line.substr(3));

			Vec3& Ka = materialBuffer.back().Ka;
			iss >> Ka.x >> Ka.y >> Ka.z;
		}
		else if (!line.compare(0, 3, "Ks ")) {
			std::istringstream iss(line.substr(3));

			Vec3& Ks = materialBuffer.back().Ks;
			iss >> Ks.x >> Ks.y >> Ks.z;
		}
		else if (!line.compare(0, 3, "Kd ")) {
			std::istringstream iss(line.substr(3));

			Vec3& Kd = materialBuffer.back().Kd;
			iss >> Kd.x >> Kd.y >> Kd.z;
		}
		else if (!line.compare(0, 7, "map_Kd ")) {
			std::istringstream iss(line.substr(7));

			size_t slash = mtlpath.find_last_of('/');
			std::string map_path = mtlpath.substr(0, slash) + std::string("/") + line.substr(7);
			TGAImage* texture = new TGAImage();
			texture->read_tga_file(map_path.c_str());
			//texture->flip_horizontally();

			materialBuffer.back().kdMap = texture;
		}
	}
}



void Model::LoadIBLMap(const std::string& iblpath) {
	iblMap = new IBLMap();
	const char* face[6] = { "px", "nx", "py", "ny", "pz", "nz" };
	char paths[6][256];

	iblMap->mipLevel = 10;


	for (int j = 0; j < 6; j++) {
		sprintf_s(paths[j], "%s/i_%s.tga", iblpath.c_str(), face[j]);
	}
	iblMap->irradianceMap = LoadCubeMap(paths);

	for (int i = 0; i < iblMap->mipLevel; i++) {
		for (int j = 0; j < 6; j++) {
			sprintf_s(paths[j], "%s/m%d_%s.tga", iblpath.c_str(), i, face[j]);
		}
		iblMap->prefilterMaps[i] = LoadCubeMap(paths);
	}

	iblMap->brdfLut = new TGAImage();
	iblMap->brdfLut->read_tga_file("Assets/common/BRDF_LUT.tga");
}

void Model::LoadMap(const std::string& mappath) {
	size_t dot = mappath.find_last_of('.');
	std::string prefix = mappath.substr(0, dot);

	std::string tgaPath = prefix + "_diffuse.tga";
	if (_access(tgaPath.c_str(), 0) != -1) {
		diffuseMap = new TGAImage();
		diffuseMap->read_tga_file(tgaPath.c_str());
	}

	tgaPath = prefix + "_normal.tga";
	if (_access(tgaPath.c_str(), 0) != -1) {
		normalMap = new TGAImage();
		normalMap->read_tga_file(tgaPath.c_str());
	}

	tgaPath = prefix + "_roughness.tga";
	if (_access(tgaPath.c_str(), 0) != -1) {
		roughnessMap = new TGAImage();
		roughnessMap->read_tga_file(tgaPath.c_str());
	}

	tgaPath = prefix + "_metalness.tga";
	if (_access(tgaPath.c_str(), 0) != -1) {
		metalnessMap = new TGAImage();
		metalnessMap->read_tga_file(tgaPath.c_str());
	}

	tgaPath = prefix + "_emission.tga";
	if (_access(tgaPath.c_str(), 0) != -1) {
		emisionMap = new TGAImage();
		emisionMap->read_tga_file(tgaPath.c_str());
	}

	tgaPath = prefix + "_occlusion.tga";
	if (_access(tgaPath.c_str(), 0) != -1) {
		occlusionMap = new TGAImage();
		occlusionMap->read_tga_file(tgaPath.c_str());
	}
}

Vec3 CubeMap::Sample(const Vec3& direction)
{
	int face_index = -1;
	float ma = 0, sc = 0, tc = 0;
	float abs_x = fabs(direction.x), abs_y = fabs(direction.y), abs_z = fabs(direction.z);
	if (abs_x > abs_y && abs_x > abs_z) {
		ma = abs_x;
		if (direction.x > 0) {
			face_index = 0;
			sc = +direction.z;
			tc = +direction.y;
		}
		else {
			face_index = 1;
			sc = -direction.z;
			tc = +direction.y;
		}
	}
	else if (abs_y > abs_z) {
		ma = abs_y;
		if (direction.y > 0) {
			face_index = 2;
			sc = +direction.x;
			tc = +direction.z;
		}
		else {
			face_index = 3;
			sc = +direction.x;
			tc = -direction.z;
		}
	}
	else {
		ma = abs_z;
		if (direction.z > 0) {
			face_index = 4;
			sc = -direction.x;
			tc = +direction.y;
		}
		else {
			face_index = 5;
			sc = +direction.x;
			tc = +direction.y;
		}
	}

	float u = (sc / ma + 1.0f) / 2.0f;
	float v = (tc / ma + 1.0f) / 2.0f;

	TGAColor c = face[face_index]->Sample(Vec2(u,v));
	//return Vec3((face_index+1)/6.0, (face_index + 1) / 6.0, (face_index + 1) / 6.0);
	return Vec3((float)c[2] / 255.f, (float)c[1] / 255.f, (float)c[0] / 255.f);
}
