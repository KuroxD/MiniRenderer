#pragma once
#include "Math.h"
#include "Model.h"
#include "../shader/Shader.h"
#include "Camera.h"



class Renderer
{
public:
	Renderer(int width,int height);
	~Renderer();

	void DrawTriangle(int face_index, IShader* shader);
	
	void Clear() { ClearZBuffer(); ClearFrameBuffer(); }
	void AddModels(std::shared_ptr<Model> model) { models.push_back(model); }
	void SetShader(IShader* shader) { this->shader=shader; }
	IShader* GetShader() const { return shader; }
	void SetCamera(std::shared_ptr<Camera> mainCamera) { this->mainCamera = mainCamera; }
	void Draw();

	void SetFrameBuffer(std::shared_ptr<unsigned char> frameBuffer) { m_frameBuffer = frameBuffer; }
	void SetZBuffer(std::shared_ptr<float> zBuffer) { m_zBuffer = zBuffer; }
	std::shared_ptr<unsigned char> GetFrameBuffer() const { return m_frameBuffer; }
	std::shared_ptr<float> GetZBuffer() const { return m_zBuffer; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	bool backClip = true;
private:
	//buffer
	int m_width, m_height;
	std::shared_ptr<unsigned char> m_frameBuffer;
	void SetColor(int x, int y, Vec4 rgba);
	void ClearFrameBuffer();

	std::shared_ptr<float> m_zBuffer;
	bool TrySetDepth(int x, int y, float depth);
	void ClearZBuffer();

	IShader* shader;
	std::vector<std::shared_ptr<Model>> models;
	std::shared_ptr<Camera> mainCamera;
	//pipeline
	void Rasterize(IShader* shader, const VertexOut** vertexes);
};




