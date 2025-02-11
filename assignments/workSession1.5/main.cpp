#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
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

#include <ew/procGen.h>

namespace ToonShading
{
	typedef struct
	{
		glm::vec3 highlight;
		glm::vec3 shadow;
	} Palette;
}

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

enum Materials {
	METAL,
	ROCK,
	PLASTIC
};

struct WaterDebug
{
	glm::vec3 waterColor = glm::vec3(0.0, 0.0, 1.0);
	ew::Mesh plane;
	float tiling = 1.0;
	float blend1 = 0.5f;
	float blend2 = 0.5f;
	float waterLevStrength = 1.0f;
	float warpStrength = 0.2f;
	float warpScale = 1.0f;
	float specScale = 1.0f;
	float texScale = 1.0f;
	float lowerCuttoff = 0.2;
	float upperCuttoff = 0.8;
} waterDebug;

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

static int palette_index = 0;
std::vector<std::tuple<std::string, ToonShading::Palette>> palette = {
	{"Sunny Day", {{1.00f, 1.00f, 1.00f}, {0.60f, 0.54f, 0.52f}}},
	{"Bright Night", {{0.47f, 0.58f, 0.68f}, {0.32f, 0.39f, 0.57f}}},
	{"Rainy Day", {{0.62f, 0.69f, 0.67f}, {0.50f, 0.55f, 0.50f}}},
	{"Rainy Night", {{0.24f, 0.36f, 0.54f}, {0.25f, 0.31f, 0.31f}}},
};

void render(ew::Shader shader, ew::Model& model, ew::Transform& modelTransform, GLint tex, GLint warp, GLint spec, const float dt)
{
	//Pipeline defenitions
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	//GFX Pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, warp);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, spec);

	shader.use();
	modelTransform.rotation = glm::rotate(modelTransform.rotation, dt, glm::vec3(0.0, 1.0, 0.0));
	shader.setMat4("_Model", glm::mat4(1.0));
	shader.setMat4("camera_viewProj", camera.projectionMatrix() * camera.viewMatrix());
	shader.setInt("_MainTex", 0);
	shader.setInt("_WaterWarpTex", 1);
	shader.setInt("_WaterSpecTex", 2);
	shader.setVec3("_CamPos", camera.position);
	shader.setVec3("_WaterColor", waterDebug.waterColor);
	shader.setFloat("_Tiling", waterDebug.tiling);
	shader.setFloat("_Time", glfwGetTime());
	shader.setFloat("_Blend1", waterDebug.blend1);
	shader.setFloat("_Blend2", waterDebug.blend2);
	shader.setFloat("_Strength", waterDebug.waterLevStrength);
	shader.setFloat("_TexScale", waterDebug.texScale);
	shader.setFloat("_SpecScale", waterDebug.specScale);
	shader.setFloat("_WarpScale", waterDebug.warpScale);
	shader.setFloat("_WarpStrength", waterDebug.warpStrength);
	shader.setFloat("lowCuttoff", waterDebug.lowerCuttoff);
	shader.setFloat("upperCuttoff", waterDebug.upperCuttoff);

	waterDebug.plane.draw();
}

int main() {
	GLFWwindow* window = initWindow("Work Session 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	camera.position = { 0.0f, 1.0f, 5.0f };
	camera.target = { 0.0f, 0.0f, 0.0f };
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
	camController.sprintMoveSpeed = 20;

	ew::Shader lit_Shader = ew::Shader("assets/toon.vert", "assets/toon.frag");
	ew::Shader water_Shader = ew::Shader("assets/water.vert", "assets/water.frag");

	ew::Model suzanne = ew::Model("assets/skull.obj");
	suzanneTransform.scale = glm::vec3(0.1f);

	GLint waterPNG = ew::loadTexture("assets/wave_tex.png");
	GLint waterWarpPNG = ew::loadTexture("assets/wave_warp.png");
	GLint waterSpecPNG = ew::loadTexture("assets/wave_spec.png");

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	waterDebug.plane.load(ew::createPlane(10, 10, 20));

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		camController.move(window, &camera, deltaTime);
		render(water_Shader, suzanne, suzanneTransform, waterPNG, waterWarpPNG, waterSpecPNG, deltaTime);
		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}


void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	
	ImGui::SliderFloat("Water Tiling", &waterDebug.tiling, 0.0, 10.0);
	ImGui::ColorEdit3("Water Color", (float*)&waterDebug.waterColor);
	ImGui::SliderFloat("Water Blend1", &waterDebug.blend1, 0.0, 10.0);
	ImGui::SliderFloat("Water Blend2", &waterDebug.blend2, 0.0, 1.0);
	ImGui::SliderFloat("Water Level Strength", &waterDebug.waterLevStrength, 0.0, 10.0);
	ImGui::SliderFloat("Water Warp Strength", &waterDebug.warpStrength, 0.0, 10.0);

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

