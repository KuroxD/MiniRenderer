#pragma once
#include "Model.h"
#include "Render.h"
#include "../shader/Shader.h"
#include <vector>

struct Scence
{
	const char* scence_name;
	std::shared_ptr<Camera>(*BuildScence)(std::vector<Renderer*>& renderPasses);
};

std::shared_ptr<Camera> BuildQin(std::vector<Renderer*>& renderPasses);
std::shared_ptr<Camera> BuildHelmet(std::vector<Renderer*>& renderPasses);

const Scence SCENCES[] = {
	"helmet",BuildHelmet,
	"qin",BuildQin
};

