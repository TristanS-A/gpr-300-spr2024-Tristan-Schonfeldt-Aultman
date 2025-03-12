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
#include <ew/procGen.h>

#include <glm/gtx/transform.hpp>
#include <ew/external//stb_image.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

struct FullscreenQuad 
{
	GLuint vao;
	GLuint vbo;

} fullscreenQuad;

static float quadVertecies[] = {
	// pos (x, y) texcoord (u, v)
	-1.0f,  1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f,  1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
	1.0f,  1.0f, 1.0f, 1.0f,
};

//Depth buffer for shadow map
struct FrameBuffer
{
	GLuint fbo;
	GLuint color;
	GLuint position;
	GLuint normal;
	GLuint lighting;
	GLuint lights;
	GLuint depth;

	//Initiates and generates fram buffer for shadow map
	void init()
	{
		//Bind framebuffer
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		//Create color0 texture attachment
		glGenTextures(1, &color);
		glBindTexture(GL_TEXTURE_2D, color);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Bind color0 attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

		//Create color1 texture attachment
		glGenTextures(1, &position);
		glBindTexture(GL_TEXTURE_2D, position);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Bind color1 attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, position, 0);

		//Create color1 texture attachment
		glGenTextures(1, &normal);
		glBindTexture(GL_TEXTURE_2D, normal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Bind color1 attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normal, 0);

		//Create lighting texture attachment
		glGenTextures(1, &lighting);
		glBindTexture(GL_TEXTURE_2D, lighting);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Bind lighting attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, lighting, 0);

		//Create lighting texture attachment
		glGenTextures(1, &lights);
		glBindTexture(GL_TEXTURE_2D, lights);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Bind lighting attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, lights, 0);

		//Configure drawing to multiple buffers
		GLuint arr[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
		glDrawBuffers(5, arr);

		//Create depth texture attachment
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//Bind depth texture attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		//Check if frame buffer was created
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Failed to bind framebuffer");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
} framebuffer;

struct Material
{
	float ambientK = 0.4;
	float diffuseK = 0.5;
	float specularK = 0.5;
	float shininess = 128;
};

//Caching things
ew::Camera camera;
ew::CameraController camController;
ew::Transform suzanneTransform;
ew::Transform lightSphereTransform;

//Light info
glm::vec3 lightVec = { 2.0f, 20.0f, -2.0f };

//Material info
Material currMat = { 0.0, 1.0, 1.0, 128 };

//Light sphere
ew::Mesh sphere;

void render(ew::Shader shader, ew::Shader lightingShader, ew::Shader lightVisShader, ew::Shader postProcessShader, ew::Model &model, ew::Transform &modelTransform, GLint tex, GLint normalMap, const float dt)
{
	//Pipeline defenitions
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	//GFX Pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	for (int i = 0; i < 30; i++)
	{
		for (int j = 0; j < 300; j++)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, normalMap);

			shader.use();
			modelTransform.rotation = glm::rotate(modelTransform.rotation, dt, glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("camera_viewProj", camera.projectionMatrix() * camera.viewMatrix());
			shader.setInt("_MainTex", 0);
			shader.setMat4("_Model", glm::translate(glm::vec3(i * 2.0f, 0, j * 2.0f)));
			model.draw();

			lightVisShader.use();
			lightVisShader.setMat4("camera_viewProj", camera.projectionMatrix() * camera.viewMatrix());
			lightVisShader.setVec3("_Color", glm::vec3(j / 300.0, j / 300.0, j / 300.0));
			lightVisShader.setMat4("_Model", glm::translate(glm::vec3(i * 2.0f, 5, j * 2.0f)));
			sphere.draw();
		}
	}

	glBindVertexArray(fullscreenQuad.vao);
	glDisable(GL_DEPTH_TEST);
	for (int i = 0; i < 1; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, framebuffer.color);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, framebuffer.position);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, framebuffer.normal);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, framebuffer.lighting);

			lightingShader.use();
			lightingShader.setInt("_PositionTex", 1);
			lightingShader.setInt("_NormalTex", 2);
			lightingShader.setInt("_PrevLightPass", 3);
			lightingShader.setVec3("_CamPos", camera.position);
			lightingShader.setVec3("_Light.color", glm::vec3(1 - j, 0.0f, j));
			lightingShader.setVec3("_Light.pos", glm::vec3(0 , 20, 0));

			lightingShader.setFloat("_Material.ambientK", currMat.ambientK);
			lightingShader.setFloat("_Material.diffuseK", currMat.diffuseK);
			lightingShader.setFloat("_Material.specularK", currMat.specularK);
			lightingShader.setFloat("_Material.shininess", currMat.shininess);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//GFX Pass
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, framebuffer.lighting);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, framebuffer.lights);

	postProcessShader.use();
	postProcessShader.setInt("_Albedo", 0);
	postProcessShader.setInt("_LightingTex", 1);
	postProcessShader.setInt("_Lights", 2);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	camera.position = { 0.0f, 0.0f, 5.0f };
	camera.target = { 0.0f, 0.0f, 0.0f };
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
	camera.farPlane = 1000;

	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader lightingShaderPass = ew::Shader("assets/lightingPass.vert", "assets/lightingPass.frag");
	ew::Shader lightVisShader = ew::Shader("assets/lightVis.vert", "assets/lightVis.frag");
	ew::Shader postProcessShader = ew::Shader("assets/geoShader.vert", "assets/geoShader.frag");

	ew::Model suzanne = ew::Model("assets/suzanne.obj");

	GLint Rock_Color = ew::loadTexture("assets/Rock_Color.png");
	GLint rockNormal = ew::loadTexture("assets/Rock_Normal.png");

	framebuffer.init();

	sphere.load(ew::createSphere(0.5f, 4));

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
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		camController.move(window, &camera, deltaTime);
		render(litShader, lightingShaderPass, lightVisShader, postProcessShader, suzanne, suzanneTransform, Rock_Color, rockNormal, deltaTime);
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

	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.position, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.normal, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.lighting, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.lights, ImVec2(800, 600));

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

