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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
int shadowScreenWidth = 500;
int shadowScreenHeight = 500;
float prevFrameTime;
float deltaTime;

struct Material
{
	float ambientK = 0.4;
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

//Depth buffer for shadow map
struct DepthBuffer
{
	GLuint fbo;
	GLuint depth;

	//Initiates and generates fram buffer for shadow map
	void init()
	{
		//Bind framebuffer
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		//Create depth texture attachment
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			shadowScreenWidth, shadowScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//Fixes out of camera view issues (over sampling) for shadows by having no texture wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		//Bind depth texture attachment
		glBindFramebuffer(GL_FRAMEBUFFER, depth);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		//Check if frame buffer was created
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Failed to bind framebuffer");
		}

		glClear(GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
} depthBuffer;

//Debug strruct for shadow
struct ShadowDebug
{
	float bias = 0.0;
	int pcfFactor = 1.0f;
} shadowDebug;

//Caching things
ew::Camera camera;
ew::CameraController camController;
ew::Transform suzanneTransform;
ew::Mesh plane;

//Materials
Material mats[3] = { 
	{0.3, 0.6, 1.0, 30, "Metal"},
	{0.2, 0.8, 0.3, 128, "Rock"},
	{0.4, 0.8, 0.4, 30, "Plastic"}
};
short matIndex = 0;
Material* currMat = &mats[matIndex];

//ImGui control vars
bool paused = false;
bool disableDepthClear = true;
bool deformMesh = true;

//Light info
glm::vec3 lightVec = { 2.0f, 2.0f, -2.0f };

void render(ew::Shader shader, ew::Shader shadowShader, ew::Shader deformShader, ew::Model &model, ew::Transform &modelTransform, GLint tex, GLint normalMap, GLint snowTex, const float dt)
{
	//Pipeline defenitions
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	//glCullFace(GL_FRONT);  //This line causes shadow issues when suzzane is inside the plane

	//Light's view of the scene
	const auto lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
	const auto lightView = glm::lookAt(lightVec, glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	const auto lightViewProj = lightProj * lightView;

	//Shadow pass
	glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer.fbo); //Switches to depth buffer fbo for rendering shadows
	{
		//Begin pass by resizing window
		glViewport(0, 0, shadowScreenWidth, shadowScreenHeight);

		//GFX Pass
		if (!disableDepthClear)
		{
			glClear(GL_DEPTH_BUFFER_BIT);
		}

		shadowShader.use();
		shadowShader.setMat4("_Model", modelTransform.modelMatrix());
		shadowShader.setMat4("_LightViewProj", lightViewProj);
	
		model.draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //Switches back to default framebuffer

	//Render lighting
	{
		//Begin pass
		glViewport(0, 0, screenWidth, screenHeight);

		//GFX Pass
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glCullFace(GL_BACK);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthBuffer.depth);

		shader.use();
		if (!paused)
		{
			modelTransform.rotation = glm::rotate(modelTransform.rotation, dt, glm::vec3(0.0, 1.0, 0.0));
		}
		shader.setMat4("_Model", modelTransform.modelMatrix());
		shader.setMat4("camera_viewProj", camera.projectionMatrix() * camera.viewMatrix());
		shader.setInt("_MainTex", 0);
		shader.setInt("_NormalMap", 1);
		shader.setInt("_ShadowMap", 2);

		shader.setVec3("_CamPos", camera.position);
		shader.setVec3("_Light.color", glm::vec3(1.0f, 1.0f, 1.0f));
		shader.setVec3("_Light.pos", glm::vec3(lightVec));
		shader.setMat4("_LightViewProj", lightViewProj);

		shader.setFloat("_Material.ambientK", currMat->ambientK);
		shader.setFloat("_Material.diffuseK", currMat->diffuseK);
		shader.setFloat("_Material.specularK", currMat->specularK);
		shader.setFloat("_Material.shininess", currMat->shininess);

		shader.setFloat("_ShadowBias", shadowDebug.bias);
		shader.setInt("_PCFFactor", shadowDebug.pcfFactor);

		model.draw();

		//Basically an if statment to render the snow effect
		if (deformMesh)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, snowTex);

			deformShader.use();
			deformShader.setMat4("camera_viewProj", camera.projectionMatrix() * camera.viewMatrix());
			deformShader.setInt("_MainTex", 0);
			deformShader.setInt("_NormalMap", 1);
			deformShader.setInt("_ShadowMap", 2);

			deformShader.setVec3("_CamPos", camera.position);
			deformShader.setVec3("_Light.color", glm::vec3(1.0f, 1.0f, 1.0f));
			deformShader.setVec3("_Light.pos", glm::vec3(lightVec));
			deformShader.setMat4("_LightViewProj", lightViewProj);

			deformShader.setFloat("_Material.ambientK", currMat->ambientK);
			deformShader.setFloat("_Material.diffuseK", currMat->diffuseK);
			deformShader.setFloat("_Material.specularK", currMat->specularK);
			deformShader.setFloat("_Material.shininess", currMat->shininess);

			deformShader.setFloat("_ShadowBias", shadowDebug.bias);
			deformShader.setInt("_PCFFactor", shadowDebug.pcfFactor);
			deformShader.setMat4("_Model", glm::translate(glm::vec3(0.0f, -2.0f, 0.0f)));
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex);

			shader.setMat4("_Model", glm::translate(glm::vec3(0.0f, -2.0f, 0.0f)));
		}
		plane.draw();
	}
}

int main() {
	GLFWwindow* window = initWindow("Assignment 2", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//Camera setup
	camera.position = { 0.0f, 0.0f, 5.0f };
	camera.target = { 0.0f, 0.0f, 0.0f };
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	//Shader, model, and texture loading
	ew::Shader lit_Shader = ew::Shader("assets/blinnPhong.vert", "assets/blinnPhong.frag");
	ew::Shader lit_Deform_Shader = ew::Shader("assets/shadowDeform.vert", "assets/shadowDeform.frag");
	ew::Shader shadowShader = ew::Shader("assets/shadow.vert", "assets/shadow.frag");

	ew::Model suzanne = ew::Model("assets/suzanne.obj");

	GLint Rock_Color = ew::loadTexture("assets/Rock_Color.png");
	GLint rockNormal = ew::loadTexture("assets/Rock_Normal.png");
	GLint snowText = ew::loadTexture("assets/snow.png");

	plane.load(ew::createPlane(10, 10, 100));

	depthBuffer.init();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		camController.move(window, &camera, deltaTime);

		//RENDER
		render(lit_Shader, shadowShader, lit_Deform_Shader, suzanne, suzanneTransform, Rock_Color, rockNormal, snowText, deltaTime);
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
	if (ImGui::BeginCombo("Select Material", currMat->name))
	{
		for (int i = 0; i < sizeof(mats) / sizeof(mats[0]); i++)
		{
			bool is_selected = (currMat->name == mats[i].name);
			if (ImGui::Selectable(mats[i].name, is_selected))
			{
				currMat = &mats[i];
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::CollapsingHeader("Edit Materials"))
	{
		for (int i = 0; i < sizeof(mats) / sizeof(mats[0]); i++)
		{
			if (ImGui::CollapsingHeader(mats[i].name))
			{
				ImGui::SliderFloat("AmbientK", &mats[i].ambientK, 0.0, 1.0);
				ImGui::SliderFloat("DiffuseK", &mats[i].diffuseK, 0.0, 1.0);
				ImGui::SliderFloat("SpecularK", &mats[i].specularK, 0.0, 1.0);
				ImGui::SliderFloat("Shininess", &mats[i].shininess, 2.0, 1024.0);
			}
		}
	}
	ImGui::Dummy({ 2, 20 });

	//Shadow ImGui controls
	ImGui::SliderFloat("Shadow Bias", &shadowDebug.bias, 0.0, 0.01, "%.4f");
	ImGui::SliderInt("Shadow PCF Factor", &shadowDebug.pcfFactor, 0.0, 10);
	ImGui::Dummy({ 2, 20 });
	ImGui::SliderFloat("Suzanne X Val", &suzanneTransform.position.x, -4, 4);
	ImGui::SliderFloat("Suzanne Y Val", &suzanneTransform.position.y, -2, 2);
	ImGui::SliderFloat("Suzanne Z Val", &suzanneTransform.position.z, -4, 4);
	ImGui::Dummy({ 2, 20 });
	ImGui::SliderFloat("Light X Val", &lightVec.x, -4, 4);
	ImGui::SliderFloat("Light Y Val", &lightVec.y, -2, 2);
	ImGui::SliderFloat("Light Z Val", &lightVec.z, -4, 4);
	ImGui::Dummy({ 2, 20 });
	ImGui::Checkbox("Pause", &paused);
	ImGui::Checkbox("Enable Mesh Deform", &deformMesh);
	if (ImGui::Checkbox("Disable Shadow Depth Clear", &disableDepthClear))
	{
		if (disableDepthClear)
		{
			glClear(GL_DEPTH_BUFFER_BIT);
		}
	}

	ImGui::Image((ImTextureID)(intptr_t)depthBuffer.depth, ImVec2(shadowScreenWidth, shadowScreenHeight));
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

