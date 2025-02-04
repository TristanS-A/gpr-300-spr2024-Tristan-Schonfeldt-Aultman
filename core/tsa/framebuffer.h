#pragma once
#include "../ew/external/glad.h"
#include <GLFW/glfw3.h>

namespace tsa 
{
	struct Framebuffer
	{
		GLuint fbo;
		GLuint rbo;
		GLuint color0;
		GLuint brightness;
		GLuint depthColor;
	};

	Framebuffer createFrameBuffer();
	Framebuffer createHDRFrameBuffer();
	Framebuffer createBloomHDRFrameBuffer();
}