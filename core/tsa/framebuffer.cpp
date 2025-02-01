#include "framebuffer.h"

namespace tsa
{
	Framebuffer tsa::createFrameBuffer(GLuint fbo, GLuint color0, GLuint color1)
	{
		Framebuffer frameBuffer;
		frameBuffer.fbo = fbo;
		frameBuffer.rbo = fbo;
		frameBuffer.color0 = color0;
		frameBuffer.color1 = color1;
		return frameBuffer;
	}
}
