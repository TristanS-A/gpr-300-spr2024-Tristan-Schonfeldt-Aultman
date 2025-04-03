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

#include <cstdlib>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
int shadowScreenWidth = 512;
int shadowScreenHeight = 512;
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
	GLuint material;
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

		//Create material texture attachment
		glGenTextures(1, &material);
		glBindTexture(GL_TEXTURE_2D, material);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Bind material attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, material, 0);

		//Configure drawing to multiple buffers
		GLuint arr[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, arr);

		//Create depth texture attachment
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//Bind depth texture attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		//Check if frame buffer was created
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Failed to bind framebuffer");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
} framebuffer;

//Depth buffer for shadow map
struct LightVolumeFrameBuffer
{
	GLuint fbo;
	GLuint color;
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

		//Create depth texture attachment
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//Bind depth texture attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		//Check if frame buffer was created
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Failed to bind framebuffer");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
} lightVolumeFBO;

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

//Plane stuff
ew::Mesh plane;

//Debug strruct for shadow
struct ShadowDebug
{
	float bias = 0.0;
	int pcfFactor = 1.0f;
} shadowDebug;

//Light info
glm::vec3 lightVec = { 8.0f, 20.0f, 8.0f };

//Material info
Material currMat = { 0.0, 1.0, 1.0, 128 };

//Light spheres
ew::Mesh sphere;
ew::Mesh debugSphere;

//Light data arrays
int lightsLength = 100;
glm::vec3 lightsPos[100];
glm::vec3 lightsCol[100];
float lightRadius = 6;

void renderSuzannes(ew::Shader shader, GLuint tex, ew::Transform& modelTransform, ew::Model& model, const float dt)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);

		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				modelTransform.rotation = glm::rotate(modelTransform.rotation, dt, glm::vec3(0.0, 1.0, 0.0));

				shader.use();
				shader.setFloat("_Material.ambientK", currMat.ambientK);
				shader.setFloat("_Material.diffuseK", currMat.diffuseK);
				shader.setFloat("_Material.specularK", currMat.specularK);
				shader.setFloat("_Material.shininess", currMat.shininess);

				shader.setMat4("camera_viewProj", camera.projectionMatrix() * camera.viewMatrix());
				shader.setMat4("_Model", glm::translate(glm::vec3(i * 2.0f, 0, j * 2.0f)));
				shader.setInt("_MainTex", 0);

				model.draw();
			}
		}
	}

	shader.setMat4("_Model", glm::translate(glm::vec3(0.0f, -2.0f, 0.0f)));

	plane.draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderLightVolumes(ew::Shader lighhtVolumeShader, const glm::highp_mat4 shadowLightViewProj)
{
	glBindFramebuffer(GL_FRAMEBUFFER, lightVolumeFBO.fbo);

	//GFX Pass
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE); //Additive blending
	glCullFace(GL_FRONT); //Front face culling - we want to render back faces so that the light volumes don't disappear when we enter them.
	glDepthMask(GL_FALSE); //Disable writing to depth buffer

	lighhtVolumeShader.use();
	lighhtVolumeShader.setMat4("camera_viewProj", camera.projectionMatrix() * camera.viewMatrix());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, framebuffer.position);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, framebuffer.normal);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, framebuffer.material);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, depthBuffer.depth);

	lighhtVolumeShader.setInt("_Albedo", 0);
	lighhtVolumeShader.setInt("_PositionTex", 1);
	lighhtVolumeShader.setInt("_NormalTex", 2);
	lighhtVolumeShader.setInt("_MaterialTex", 3);
	lighhtVolumeShader.setInt("_ShadowMap", 4);

	lighhtVolumeShader.setFloat("_ShadowBias", shadowDebug.bias);
	lighhtVolumeShader.setInt("_PCFFactor", shadowDebug.pcfFactor);

	lighhtVolumeShader.setVec3("_ShadowLight.color", glm::vec3(1.0));
	lighhtVolumeShader.setVec3("_ShadowLight.pos", lightVec);
	lighhtVolumeShader.setMat4("_LightViewProj", shadowLightViewProj);

	for (int i = 0; i < lightsLength; i++)
	{
		lighhtVolumeShader.setVec3("_CamPos", camera.position);

		lighhtVolumeShader.setVec3("_Light.color", lightsCol[i]);
		lighhtVolumeShader.setFloat("_Light.radius", lightRadius);
		lighhtVolumeShader.setVec3("_Light.pos", lightsPos[i]);

		lighhtVolumeShader.setMat4("_Model", glm::translate(lightsPos[i]));
		sphere.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void applyGeoShader(ew::Shader geoShader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//GFX Pass
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(fullscreenQuad.vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightVolumeFBO.color);

	geoShader.use();

	geoShader.setInt("_Albedo", 0);
	geoShader.setInt("_Lighting", 1);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void renderDebugLights(ew::Shader lightVisShader)
{
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	lightVisShader.use();
	lightVisShader.setMat4("camera_viewProj", camera.projectionMatrix() * camera.viewMatrix());

	for (int i = 0; i < lightsLength; i++)
	{
		lightVisShader.setVec3("_Color", lightsCol[i]);
		lightVisShader.setMat4("_Model", glm::translate(lightsPos[i]));
		debugSphere.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setLightData()
{
	srand(0);
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			glm::vec3 randColor = glm::vec3(rand() % 2, rand() % 2, rand() % 2);
			glm::vec3 lightPos = glm::vec3(i * 2.0f, 1.5, j * 2.0f);

			lightsPos[i * 10 + j] = lightPos;
			lightsCol[i * 10 + j] = randColor;
		}
	}
}

void renderShadowMap(ew::Shader shadowMapShader, GLuint tex, ew::Transform& modelTransform, ew::Model& model, const float dt, const glm::highp_mat4 shadowLightViewProj)
{
	//Pipeline defenitions
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	//glCullFace(GL_FRONT);  //This line causes shadow issues when suzzane is inside the plane

	//Shadow pass
	glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer.fbo); //Switches to depth buffer fbo for rendering shadows
	{
		//Begin pass by resizing window
		glViewport(0, 0, shadowScreenWidth, shadowScreenHeight);

		//GFX Pass
		glClear(GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				shadowMapShader.use();
				shadowMapShader.setMat4("_Model", glm::translate(glm::vec3(i * 2.0f, 0, j * 2.0f)));
				shadowMapShader.setMat4("_LightViewProj", shadowLightViewProj);

				model.draw();
			}
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //Switches back to default framebuffer

	glViewport(0, 0, screenWidth, screenHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
}

void MegaRender(ew::Shader shader, ew::Shader lightVisShader, ew::Shader postProcessShader, ew::Shader lightVolumeShader, ew::Shader shadowMapShader, ew::Shader planeShader, ew::Model& model, ew::Transform& modelTransform, GLint tex, GLint normalMap, const float dt)
{	
	//Pipeline defenitions
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);


	//GFX Pass
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Light's view of the scene
	const auto lightProj = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.1f, 100.0f);
	const auto lightView = glm::lookAt(lightVec, glm::vec3(8.0f, 0.0f, 8.0f), glm::vec3(0.0f, -1.0f, 1.0f));
	const glm::highp_mat4 shadowLightViewProj = lightProj * lightView;

	renderSuzannes(shader, tex, modelTransform, model, dt);

	renderShadowMap(shadowMapShader, tex, modelTransform, model, dt, shadowLightViewProj);

	renderLightVolumes(lightVolumeShader, shadowLightViewProj);

	applyGeoShader(postProcessShader);

	renderDebugLights(lightVisShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthBuffer.depth);
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
	//ew::Shader lightingShaderPass = ew::Shader("assets/lightingPass.vert", "assets/lightingPass.frag");
	ew::Shader lightVisShader = ew::Shader("assets/lightVis.vert", "assets/lightVis.frag");
	ew::Shader postProcessShader = ew::Shader("assets/geoShader.vert", "assets/geoShader.frag");
	ew::Shader lightVolumeShader = ew::Shader("assets/lightVolume.vert", "assets/lightVolume.frag");
	ew::Shader shadowShader = ew::Shader("assets/shadow.vert", "assets/shadow.frag");
	ew::Shader planeShader = ew::Shader("assets/plane.vert", "assets/plane.frag");

	ew::Model suzanne = ew::Model("assets/suzanne.obj");

	GLint Rock_Color = ew::loadTexture("assets/snow.png");
	GLint rockNormal = ew::loadTexture("assets/Rock_Normal.png");

	plane.load(ew::createPlane(100, 100, 100));

	framebuffer.init();
	lightVolumeFBO.init();
	depthBuffer.init();

	setLightData();

	sphere.load(ew::createSphere(6, 10));
	debugSphere.load(ew::createSphere(0.5f, 4));

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

		// deferred; render all geo data of scene (albedo, position, normla)
		MegaRender(litShader, lightVisShader, postProcessShader, lightVolumeShader, shadowShader, planeShader, suzanne, suzanneTransform, Rock_Color, rockNormal, deltaTime);

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

	ImGui::Text("Material Settings");
	ImGui::SliderFloat("AmbientK", &currMat.ambientK, 0.0, 1.0);
	ImGui::SliderFloat("DiffuseK", &currMat.diffuseK, 0.0, 1.0);
	ImGui::SliderFloat("SpecularK", &currMat.specularK, 0.0, 1.0);
	ImGui::SliderFloat("Shininess", &currMat.shininess, 2.0, 1024.0);

	ImGui::Dummy({ 2, 20 });

	ImGui::Text("Shadow Settings");
	ImGui::SliderFloat("Shadow Bias", &shadowDebug.bias, 0.0, 0.01, "%.4f");
	ImGui::SliderInt("Shadow PCF Factor", &shadowDebug.pcfFactor, 0.0, 10);

	ImGui::Dummy({ 2, 20 });

	ImGui::Text("G-Buffer");
	ImGui::Text("Color");
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color, ImVec2(800, 600));
	ImGui::Text("Position");
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.position, ImVec2(800, 600));
	ImGui::Text("Normal");
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.normal, ImVec2(800, 600));
	ImGui::Text("Lighting");
	ImGui::Image((ImTextureID)(intptr_t)lightVolumeFBO.color, ImVec2(800, 600));
	ImGui::Text("Material");
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.material, ImVec2(800, 600));
	ImGui::Text("Shadow Depth");
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