#pragma once
#include <GLFW/glfw3.h>

namespace tsa 
{
	struct Framebuffer
	{
		GLuint fbo;
		GLuint rbo;
		GLuint color0;
		GLuint color1;
		GLuint depthColor;
	};

	Framebuffer createFrameBuffer(GLuint fbo = 0, GLuint color0 = 0, GLuint color1 = 0);
}