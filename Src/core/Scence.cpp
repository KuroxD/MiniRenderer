#include "Scence.h"

#include <iostream>

#include "../shader/PhongShader.h"
#include "../shader/PBRShader.h"
#include "../shader/SkyboxShader.h"
#include "../shader/ShadowShader.h"
#include "../shader/ReflectionShader.h"
#include "../core/Math.h"



std::shared_ptr<Camera> BuildQin(std::vector<Renderer*>& renderPasses) {
	float width = 800;
	float height = 800;
	std::shared_ptr<Camera> mainCamera(new Camera(Vec3(0, 10.5, -25), Vec3(0, 10.5, 0), width/height));

	std::shared_ptr<Model> qin(new Model());
	qin->LoadModel("Assets/qin/qin.obj");
	std::shared_ptr<Model> floor(new Model());
	floor->LoadModel("Assets/qin/floor.obj");
	
	std::shared_ptr<Camera> rayCamera(new Camera(Vec3(0, 15, -15), Vec3(0,5,0), width / height));
	rayCamera->perspectMat = Mat4::Ortho(20.0, -5.0, -10.0, 10.0, 1, 200);
	//shadow pass
	Renderer* shadowPass = new Renderer(width,height);
	shadowPass->SetFrameBuffer(nullptr);
	shadowPass->SetShader(new ShadowShader());
	shadowPass->AddModels(qin);
	shadowPass->SetCamera(rayCamera);

	//reflection pass
	Renderer* reflectionPass = new Renderer(width, height);
	reflectionPass->SetShader(new ReflectionShader());
	reflectionPass->GetShader()->unif.isReflection = true;
	reflectionPass->AddModels(qin);
	reflectionPass->SetCamera(mainCamera);

	//phong pass
	Renderer* phongPass = new Renderer(width, height);
	phongPass->SetCamera(mainCamera);
	phongPass->AddModels(qin);
	phongPass->AddModels(floor);
	phongPass->SetShader(new PhongShader());
	IShader* phongShader = phongPass->GetShader();
	phongShader->unif.shadowMap = shadowPass->GetZBuffer().get();
	phongShader->unif.reflectionTexture = reflectionPass->GetFrameBuffer().get();
	phongShader->unif.lighVpMat = rayCamera->perspectMat * rayCamera->viewMat;



	//renderPasses = { shadowPass,phongPass };
	renderPasses = { shadowPass,reflectionPass,phongPass };
	return mainCamera;
}

std::shared_ptr<Camera> BuildHelmet(std::vector<Renderer*>& renderPasses) {
	float width = 800;
	float height = 800;
	std::shared_ptr<Camera> mainCamera(new Camera(Vec3(0, 0, -5), Vec3(0, 0, 0), width / height));

	std::shared_ptr<Model> helmet(new Model());
	helmet->LoadModel("Assets/helmet/helmet.obj", true);
	helmet->LoadIBLMap("Assets/common");

	std::shared_ptr<Model> skyBox(new Model());
	skyBox->LoadModel("Assets/workshop/box.obj", false, true);
	skyBox->model_mat.Translate(0, 0, 5);

	Renderer* pbrPass = new Renderer(width, height);
	pbrPass->SetCamera(mainCamera);
	pbrPass->AddModels(helmet);
	pbrPass->SetShader(new PBRShader());


	Renderer* skyBoxPass = new Renderer(width, height);
	skyBoxPass->SetCamera(mainCamera);
	skyBoxPass->SetFrameBuffer(pbrPass->GetFrameBuffer());
	skyBoxPass->SetZBuffer(pbrPass->GetZBuffer());
	skyBoxPass->AddModels(skyBox);
	skyBoxPass->SetShader(new SkyBoxShader());

	renderPasses = { pbrPass,skyBoxPass };
	return mainCamera;
}
