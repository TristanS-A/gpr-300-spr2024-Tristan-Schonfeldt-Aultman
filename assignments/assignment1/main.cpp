#include <stdio.h>
#include <math.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/cameraController.h>
#include <ew/transform.h>
#include <ew/texture.h>
#include <tuple>
#include <unordered_map>

#include <tsa/framebuffer.h>
#include <tsa/shaderStructs.h>

namespace ToonShading
{
	typedef struct
	{
		glm::vec3 highlight;
		glm::vec3 shadow;
	} Palette;
}

enum PostProcessShaders
{
	FOG,
	CHROMATIC_ABORATION,
	GRAYSCALE,
	INVERSE,
	EDGE_DETECTION,
	BLURR,
	GAUSSEN_BLUR
};

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

struct Material
{
	float ambientK = 1.0;
	float diffuseK = 0.5;
	float specularK = 0.5;
	float shininess = 128;
	char* name = "Default";
};

struct FullscreenQuad 
{
	GLuint vao;
	GLuint vbo;

} fullscreenQuad;

enum Materials {
	METAL,
	ROCK,
	PLASTIC
};

//Caching things
ew::Camera camera;
ew::CameraController camController;
ew::Transform suzanneTransform;
Material mats[3] = {
	{1.0, 0.6, 1.0, 30, "Metal"},
	{1.0, 0.8, 0.3, 128, "Rock"},
	{1.0, 0.8, 0.4, 30, "Plastic"}
};
short matIndex = 0;
Material* currMat = &mats[matIndex];
bool usingNormalMap = true;
tsa::Framebuffer framebuffer;
tsa::Framebuffer pingPongBuffers[2];
int pingPongIndex = 0;
PostProcessShaders currPPShader = FOG;
std::unordered_map<PostProcessShaders, tsa::PPShaderData*> ppShaderMap;

static int palette_index = 0;
std::vector<std::tuple<std::string, ToonShading::Palette>> palette = {
	{"Sunny Day", {{1.00f, 1.00f, 1.00f}, {0.60f, 0.54f, 0.52f}}},
	{"Bright Night", {{0.47f, 0.58f, 0.68f}, {0.32f, 0.39f, 0.57f}}},
	{"Rainy Day", {{0.62f, 0.69f, 0.67f}, {0.50f, 0.55f, 0.50f}}},
	{"Rainy Night", {{0.24f, 0.36f, 0.54f}, {0.25f, 0.31f, 0.31f}}},
};

static float quadVertecies[] = {
	// pos (x, y) texcoord (u, v)
	-1.0f,  1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f,  1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
	1.0f,  1.0f, 1.0f, 1.0f,
};

void render(ew::Shader shader, ew::Model& model, ew::Transform& modelTransform, GLint tex, GLint normalMap, GLint zaToon, const float dt)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	//Pipeline defenitions
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//GFX Pass
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalMap);

	shader.use();
	modelTransform.rotation = glm::rotate(modelTransform.rotation, dt, glm::vec3(0.0, 1.0, 0.0));
	shader.setMat4("_Model", modelTransform.modelMatrix());
	shader.setMat4("camera_viewProj", camera.projectionMatrix() * camera.viewMatrix());
	shader.setInt("_MainTex", 0);
	shader.setInt("_NormalMap", 1);
	shader.setInt("_ZaToon", 2);
	shader.setBool("_Use_NormalMap", usingNormalMap);
	shader.setVec3("_EyePos", camera.position);
	shader.setVec3("_Highlight", std::get<1>(palette[palette_index]).highlight);
	shader.setVec3("_Shadow", std::get<1>(palette[palette_index]).shadow);

	shader.setFloat("_Material.ambientK", currMat->ambientK);
	shader.setFloat("_Material.diffuseK", currMat->diffuseK);
	shader.setFloat("_Material.specularK", currMat->specularK);
	shader.setFloat("_Material.shininess", currMat->shininess);

	model.draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cleanupShaders()
{
	auto it = ppShaderMap.begin();
	while (it != ppShaderMap.end())
	{
		delete it->second;
		it++;
	}

	ppShaderMap.clear();
}

int main() {
	GLFWwindow* window = initWindow("Work Session 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	camera.position = { 0.0f, 0.0f, 5.0f };
	camera.target = { 0.0f, 0.0f, 0.0f };
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
	camController.sprintMoveSpeed = 20;

	tsa::FogShaderData* fogShader = new tsa::FogShaderData();
	fogShader->shaderProgram = ew::Shader("assets/fog.vert", "assets/fog.frag");
	fogShader->farPlane = camera.farPlane;
	fogShader->nearPlane = camera.nearPlane;
	fogShader->fogcolor = glm::vec3(0.8f, 0.8f, 0.8f);
	fogShader->displayName = "Fog Effect";
	ppShaderMap[FOG] = fogShader;

	tsa::EdgeDetectionData* edgeDetectionShader = new tsa::EdgeDetectionData();
	edgeDetectionShader->shaderProgram = ew::Shader("assets/edgeDetection.vert", "assets/edgeDetection.frag");
	edgeDetectionShader->displayName = "Edge Detection Effect";
	ppShaderMap[EDGE_DETECTION] = edgeDetectionShader;

	tsa::ChromaticAborationData* chromaticAborationShader = new tsa::ChromaticAborationData();
	chromaticAborationShader->shaderProgram = ew::Shader("assets/chromaticAboration.vert", "assets/chromaticAboration.frag");
	chromaticAborationShader->displayName = "Chromatic Aboration Effect";
	ppShaderMap[CHROMATIC_ABORATION] = chromaticAborationShader;

	tsa::BlurrData* blurrShader = new tsa::BlurrData();
	blurrShader->shaderProgram = ew::Shader("assets/blurr.vert", "assets/blurr.frag");
	blurrShader->displayName = "Blurr Effect";
	ppShaderMap[BLURR] = blurrShader;

	tsa::GaussenBlurrData* gaussenShader = new tsa::GaussenBlurrData();
	gaussenShader->shaderProgram = ew::Shader("assets/gaussenBlur.vert", "assets/gaussenBlur.frag");
	gaussenShader->displayName = "Gaussen Blurr Effect";
	ppShaderMap[GAUSSEN_BLUR] = gaussenShader;

	tsa::GrayScaleData* grayScaleShader = new tsa::GrayScaleData();
	grayScaleShader->shaderProgram = ew::Shader("assets/grayScale.vert", "assets/grayScale.frag");
	grayScaleShader->displayName = "Grayscale Effect";
	ppShaderMap[GRAYSCALE] = grayScaleShader;

	tsa::InverseData* inverseShader = new tsa::InverseData();
	inverseShader->shaderProgram = ew::Shader("assets/inverse.vert", "assets/inverse.frag");
	inverseShader->displayName = "Inverse Effect";
	ppShaderMap[INVERSE] = inverseShader;

	ew::Shader hdrShader = ew::Shader("assets/hdr.vert", "assets/hdr.frag");

	ew::Shader fullShader = ew::Shader("assets/fullscreen.vert", "assets/fullscreen.frag");
	ew::Shader lit_Shader = ew::Shader("assets/lit.vert", "assets/lit.frag");

	ew::Model suzanne = ew::Model("assets/Suzanne.fbx");
	suzanneTransform.scale = glm::vec3(0.1f);

	GLint Rock_Color = ew::loadTexture("assets/Txo_dokuo.png");
	GLint rockNormal = ew::loadTexture("assets/Rock_Normal.png");
	GLint zaToon = ew::loadTexture("assets/ZAtoon.png");

	framebuffer = tsa::createHDRFrameBuffer();
	pingPongBuffers[0] = tsa::createHDRFrameBuffer();
	pingPongBuffers[1] = tsa::createHDRFrameBuffer();

	//Check if frame buffer was created
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Failed to bind framebuffer");
		return 0;
	}

	//Initialize fullscreen quad
	glGenVertexArrays(1, &fullscreenQuad.vao);
	glGenBuffers(1, &fullscreenQuad.vbo);

	//Bind vao and vbo
	glBindVertexArray(fullscreenQuad.vao);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuad.vbo);

	//Buffer data to vbo
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertecies), &quadVertecies, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); //Position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
	glEnableVertexAttribArray(1); //Text coords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*) (sizeof(float) * 2));

	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		camController.move(window, &camera, deltaTime);

		render(lit_Shader, suzanne, suzanneTransform, Rock_Color, rockNormal, zaToon, deltaTime);

		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		glBindVertexArray(fullscreenQuad.vao);

		pingPongBuffers[1].color0 = framebuffer.brightness;

		pingPongIndex = 0;
		for (int i = 0; i < 1; i++)
		{
			if (pingPongIndex % 2 == 0)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, pingPongBuffers[0].fbo);

				ew::Shader gBlurrShader = ppShaderMap[GAUSSEN_BLUR]->shaderProgram;

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, pingPongBuffers[1].color0);

				gBlurrShader.use();
				gBlurrShader.setInt("_MainTex", 0);

				glDrawArrays(GL_TRIANGLES, 0, 6);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
			else 
			{
				glBindFramebuffer(GL_FRAMEBUFFER, pingPongBuffers[1].fbo);

				ew::Shader gBlurrShader = ppShaderMap[GAUSSEN_BLUR]->shaderProgram;

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, pingPongBuffers[0].color0);

				gBlurrShader.use();
				gBlurrShader.setInt("_MainTex", 0);

				glDrawArrays(GL_TRIANGLES, 0, 6);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			pingPongIndex++;
		}

		framebuffer.color0 = pingPongBuffers[pingPongIndex % 2 == 0].color0;

		ppShaderMap[currPPShader]->display(framebuffer);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);


		drawUI();
		glfwSwapBuffers(window);
	}

	cleanupShaders();
	printf("Shutting down...");
}


void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");

	if (ImGui::CollapsingHeader("Select Post Process Effect"))
	{
		auto it = ppShaderMap.begin();
		while (it != ppShaderMap.end())
		{
			bool is_selected = (currPPShader == it->first);
			if (ImGui::Selectable(it->second->displayName, is_selected))
			{
				currPPShader = it->first;
			}

			it++;
		}
	}

	if (ImGui::CollapsingHeader("Edit Post Process Effect"))
	{
		ppShaderMap[currPPShader]->ImGuiDisplay();
	}

	ImGui::Dummy(ImVec2(0.0f, 20.0f));

	ImGui::Checkbox("Using Normal Map", &usingNormalMap);

	if (ImGui::BeginCombo("Palette", std::get<std::string>(palette[palette_index]).c_str()))
	{
		for (auto n = 0; n < palette.size(); ++n)
		{
			auto is_selected = (std::get<0>(palette[palette_index]) == std::get<0>(palette[n]));
			if (ImGui::Selectable(std::get<std::string>(palette[n]).c_str(), is_selected))
			{
				palette_index = n;
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::ColorEdit3("Highlight", &std::get<ToonShading::Palette>(palette[palette_index]).highlight[0]);
	ImGui::ColorEdit3("Shadow", &std::get<ToonShading::Palette>(palette[palette_index]).shadow[0]);

	ImGui::Begin("OpenGL Texture Text");
	ImGui::Text("pointer = %x", framebuffer.color0);
	ImGui::Text("size = %d x %d", 800, 600);
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color0, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.brightness, ImVec2(800, 600));
	ImGui::End();

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}