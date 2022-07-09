#include "Render.h"
#include <algorithm>


const std::vector<Vec4> clipLines = {
	Vec4(0,0,0,1),	//wplane
	Vec4(0,0,1,1),	//near
	Vec4(0,0,-1,1),	//far
	Vec4(0,1,0,1),	//top
	Vec4(0,-1,0,1),	//bottom
	Vec4(1,0,0,1),	//left
	Vec4(-1,0,0,1)	//right
	
};


bool Inside(const Vec4& p, const Vec4& line) {
	return p.dot(line) >= 0;
}

VertexOut Intersect(const VertexOut& v1, const VertexOut& v2, const Vec4& line) {
	float d1 = v1.clipCoord.dot(line);
	float d2 = v2.clipCoord.dot(line);

	float weight = d1 / (d1 - d2);
	VertexOut out; 
	out.clipCoord = v1.clipCoord  + (v2.clipCoord-v1.clipCoord) *  weight;
	out.normal = v1.normal + (v2.normal - v1.normal) * weight;
	out.uv = v1.uv + (v2.uv - v1.uv) * weight;
	out.worldCoord = v1.worldCoord + (v2.worldCoord - v1.worldCoord) * weight;
	return out;
}


void SutherlandHodgeman(std::vector<VertexOut>& vertexOut) {
	bool allInside = true;
	for (int i = 0; allInside && i < 3; i++) {
		auto& clipCoord = vertexOut[i].clipCoord;
		allInside &= fabs(clipCoord.x) <= clipCoord.w && fabs(clipCoord.y) <= clipCoord.w && fabs(clipCoord.z) <= clipCoord.w;
	}

	if (allInside) return;

	std::vector<VertexOut> output(vertexOut);
	for (int i = 0; i < clipLines.size(); i++) {
		std::vector<VertexOut> input(output);
		output.clear();
		for (int j = 0; j < input.size(); j++) {
			VertexOut& current = input[j];
			VertexOut& last = input[(j + input.size() - 1) % input.size()];
			if (Inside(clipLines[i], current.clipCoord)) {
				if (!Inside(clipLines[i], last.clipCoord)) {
					output.push_back(Intersect(last, current, clipLines[i]));
				}
				output.push_back(current);
			}
			else if (Inside(clipLines[i], last.clipCoord)) {
				output.push_back(Intersect(last, current, clipLines[i]));
			}
		}
	}

	vertexOut = std::move(output);
}




Renderer::Renderer(int width_, int height_)
	:m_width(width_), m_height(height_), 
	m_frameBuffer(new unsigned char[m_width * m_height * 4], [](unsigned char* p) {delete[] p; }),
	m_zBuffer(new float[m_width * m_height], [](float* p) {delete[] p; })
{
	mainCamera = nullptr;
}



Renderer::~Renderer()
{

}

bool BackClip(const VertexOut* vOuts[3]) {
	Vec2 v0 = vOuts[1]->ndcCoord.xy() - vOuts[0]->ndcCoord.xy();
	Vec2 v1 = vOuts[2]->ndcCoord.xy() - vOuts[0]->ndcCoord.xy();

	return v0.x * v1.y - v0.y * v1.x<0;
}


void Renderer::DrawTriangle(int fIndex,IShader* shader)
{
	auto& vertexOut = shader->unif.vertexs;
	//Vertex shader
	vertexOut.clear();
	for (int vIndex = 0; vIndex < 3; vIndex++) {
		shader->VertexShader(fIndex, vIndex);
	}

	//Clip
	SutherlandHodgeman(vertexOut);
	
	//NDC space
	for (int i = 0; i < vertexOut.size(); i++) {
		vertexOut[i].ndcCoord = vertexOut[i].clipCoord.xyz() / vertexOut[i].clipCoord.w;
	}


	//Triangle reassemble
	if (vertexOut.size()>=3) {
		for (int i = 0; i < vertexOut.size() - 2; i++) {
			const VertexOut* vOuts[] = { &vertexOut[0],&vertexOut[i + 1],&vertexOut[i + 2] };
			if (!shader->model->enviromentMap && BackClip(vOuts)!=shader->unif.isReflection) {	//Not SkyBox
				continue;
			}
			Rasterize(shader, vOuts);
		}
	}
}

void Renderer::Draw()
{
	auto& uniform = shader->unif;
	uniform.viewMat = mainCamera->viewMat;
	uniform.perspectMat = mainCamera->perspectMat;
	uniform.mainCamera = mainCamera;
	uniform.vpMat = uniform.perspectMat * uniform.viewMat;
	
	
	for (int i = 0; i < models.size(); i++) {
		shader->model = models[i];
		for (int j = 0; j < models[i]->fBuffer.size(); j++) {
			DrawTriangle(j, shader);
		}
	}
}

void Renderer::Rasterize(IShader* shader,const VertexOut** vertexes)
{
	//viewport
	Vec3 screen_coord[3];
	for (int i = 0; i < 3; i++) {
		screen_coord[i].x = 0.5f * (m_width-1) * (vertexes[i]->ndcCoord.x + 1.0f);
		screen_coord[i].y = 0.5f * (m_height-1) * (vertexes[i]->ndcCoord.y + 1.0f);
		screen_coord[i].z = 0.5f * (vertexes[i]->ndcCoord.z + 1.0f);
	}

	//bounding box
	float min_x = m_width-1, max_x = 0;
	float min_y = m_height-1, max_y = 0;
	for (int i = 0; i < 3; i++) {
		min_x = std::min(screen_coord[i].x, min_x);
		max_x = std::max(screen_coord[i].x, max_x);
		min_y = std::min(screen_coord[i].y, min_y);
		max_y = std::max(screen_coord[i].y, max_y);
	}

	min_x = std::max(0.0f, min_x);
	max_x = std::min<float>(m_width - 1, max_x);
	min_y = std::max(0.0f, min_y);
	max_y = std::min<float>(m_height - 1, max_y); 

	//rasterize
	for (int x = (int)min_x ; x <= (int)max_x; x++) {
		for (int y = (int)min_y; y <= (int)max_y; y++) {
			Vec2	 screen_pos(x + 0.5f, y + 0.5f);
			Vec3 barycenter_coord;
			if (InTriangle(screen_coord[0].xy(), screen_coord[1].xy(), screen_coord[2].xy(), screen_pos, barycenter_coord)) {
				Vec3 weight(barycenter_coord.x / vertexes[0]->clipCoord.w, barycenter_coord.y / vertexes[1]->clipCoord.w, barycenter_coord.z / vertexes[2]->clipCoord.w);
				float normalizer = 1.0 / (weight.x+ weight.y+ weight.z);
					
				float z = barycenter_coord.x * screen_coord[0].z + barycenter_coord.y * screen_coord[1].z + barycenter_coord.z * screen_coord[2].z;
				
				if (TrySetDepth(x, y, z) && m_frameBuffer!=nullptr) {
					SetColor(x, y, shader->FragmentShader(barycenter_coord, vertexes));
				}
			}
		}
	}
}

void Renderer::ClearZBuffer()
{
	std::fill(m_zBuffer.get(), m_zBuffer.get() + m_width * m_height, 10000.0f);
}

void Renderer::ClearFrameBuffer()
{
	if (m_frameBuffer != nullptr) {
		std::fill(m_frameBuffer.get(), m_frameBuffer.get() + m_width * m_height * 4 * sizeof(unsigned char), 102.0f);
	}
	
	//memset(m_frameBuffer, 0, m_width * m_height * 4 * sizeof(unsigned char));
}

void Renderer::SetColor(int x, int y, Vec4 rgba)
{
	for (int i = 0; i < 4; i++) {
		if (rgba[i] > 1) rgba[i] = 1.0;
		if (rgba[i] < 0) rgba[i] = 0;
	}

	//int pos = ((m_height - 1 - y) * m_width + x) * 4;
	int pos = (y * m_width + x) * 4;
	m_frameBuffer.get()[pos + 0] = rgba.x * 255.0;
	m_frameBuffer.get()[pos + 1] = rgba.y * 255.0;
	m_frameBuffer.get()[pos + 2] = rgba.z * 255.0;
	m_frameBuffer.get()[pos + 3] = rgba.w * 255.0;
}

bool Renderer::TrySetDepth(int x, int y, float depth_)
{
	int pos = y * m_width + x;
	if (depth_  <= m_zBuffer.get()[pos]) {
		m_zBuffer.get()[pos] = depth_;
		return true;
	}
	return false;
}
