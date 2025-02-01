#pragma once
#include <stdio.h>
#include <math.h>
#include <ew/external/glad.h>
#include <GLFW/glfw3.h>
#include <ew/shader.h>

namespace tsa
{
	struct PPShaderData 
	{
		ew::Shader shaderProgram;
		char* displayName = "Default";

		PPShaderData() {};

		virtual void display(Framebuffer framebuffer) 
		{
			shaderProgram.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
			shaderProgram.setInt("_MainTex", 0);
		}

		virtual void ImGuiDisplay() {}
	};

	struct FogShaderData : PPShaderData
	{
		float nearPlane;
		float farPlane;
		glm::vec3 fogcolor;

		FogShaderData() : PPShaderData() {};

		void display(Framebuffer framebuffer) override
		{
			PPShaderData::display(framebuffer);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, framebuffer.depthColor);
			shaderProgram.setInt("_DepthTex", 1);

			shaderProgram.setFloat("_Near", nearPlane);
			shaderProgram.setFloat("_Far", farPlane);
			shaderProgram.setVec3("_FogColor", fogcolor);
		}

		void ImGuiDisplay() override
		{

			ImGui::SliderFloat("Fog Color Red", &fogcolor.r, 0.0, 1.0);
			ImGui::SliderFloat("Fog Color Green", &fogcolor.g, 0.0, 1.0);
			ImGui::SliderFloat("Fog Color Blue", &fogcolor.b, 0.0, 1.0);
		}
	};

	struct EdgeDetectionData : PPShaderData
	{
		float strength = 16;

		EdgeDetectionData() : PPShaderData() {};

		void display(Framebuffer framebuffer) override
		{
			PPShaderData::display(framebuffer);
			shaderProgram.setFloat("_Strength", strength);
		}

		void ImGuiDisplay() override
		{

			ImGui::SliderFloat("Strength", &strength, 0.0, 100);
		}
	};

	struct ChromaticAborationData : PPShaderData
	{
		glm::vec3 offset = glm::vec3(-0.006, 0.006, -0.006);
		glm::vec2 direction = glm::vec2(1.0, 0.0);

		ChromaticAborationData() : PPShaderData() {};

		void display(Framebuffer framebuffer) override
		{
			PPShaderData::display(framebuffer);
			shaderProgram.setVec3("_Offset", offset);
			shaderProgram.setVec2("_Direction", direction);
		}

		void ImGuiDisplay() override
		{
			ImGui::SliderFloat("X Offset", &offset.x, -1.0, 1.0);
			ImGui::SliderFloat("Y Offset", &offset.y, -1.0, 1.0);
			ImGui::SliderFloat("Z Offset", &offset.z, -1.0, 1.0);
			ImGui::SliderFloat("X Direction", &direction.x, -1.0, 1.0);
			ImGui::SliderFloat("Y Direction", &direction.y, -1.0, 1.0);
		}
	};

	struct BlurrData : PPShaderData
	{
		float strength = 300;

		BlurrData() : PPShaderData() {};

		void display(Framebuffer framebuffer) override
		{
			PPShaderData::display(framebuffer);
			shaderProgram.setFloat("_InvStrrength", strength);
		}

		void ImGuiDisplay() override
		{

			ImGui::SliderFloat("Inverse Blurr Strength", &strength, 0.0, 500);
		}
	};

	struct GrayScaleData : PPShaderData
	{
		bool realistic = true;

		GrayScaleData() : PPShaderData() {};

		void display(Framebuffer framebuffer) override
		{
			PPShaderData::display(framebuffer);

			if (realistic) 
			{
				shaderProgram.setVec3("_RealisticScale", glm::vec3(0.2126f, 0.7152f, 0.0722f));
				shaderProgram.setFloat("_AvarageScale", 1.0f);
			}
			else 
			{
				shaderProgram.setVec3("_RealisticScale", glm::vec3(1.0f, 1.0f, 1.0f));
				shaderProgram.setFloat("_AvarageScale", 3.0f);
			}

		}

		void ImGuiDisplay() override
		{

			ImGui::Checkbox("Use Realistic Grayscale", &realistic);
		}
	};

	struct InverseData : PPShaderData
	{
		float invAmount = 1.0;

		InverseData() : PPShaderData() {};

		void display(Framebuffer framebuffer) override
		{
			PPShaderData::display(framebuffer);

			shaderProgram.setFloat("_InverseSlider", invAmount);

		}

		void ImGuiDisplay() override
		{

			ImGui::SliderFloat("Inverse Amount", &invAmount, 0.0f, 1.0f);
		}
	};
}