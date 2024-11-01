#include <glad/glad.h>
#include "utils.h"
#include "Timer.h"

GLuint createFrameBufferByBinding(int vWidth, int vHeight, GLuint& voColorTexID, GLuint& voRenderBufferID)
{
	GLuint FrameBufferID;
	glGenFramebuffers(1, &FrameBufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferID);

	glGenTextures(1, &voColorTexID);
	glBindTexture(GL_TEXTURE_2D, voColorTexID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, vWidth, vHeight);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vWidth, vHeight, vInternalFormat, GL_FLOAT, nullptr);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, voColorTexID, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &voRenderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, voRenderBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, vWidth, vHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, voRenderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	_ASSERTE(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	return FrameBufferID;
}

GLuint createFrameBufferByUnBinding(int vWidth, int vHeight, GLuint& voColorTexID, GLuint& voRenderBufferID)
{
	GLuint FrameBufferID;
	glGenFramebuffers(1, &FrameBufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferID);

	glCreateTextures(GL_TEXTURE_2D, 1, &voColorTexID);
	glTextureParameteri(voColorTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(voColorTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(voColorTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(voColorTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(voColorTexID, 1, GL_RGBA8, vWidth, vHeight);
	//glTextureSubImage2D(TexID, 0, 0, 0, vWidth, vHeight, vInternalFormat, GL_FLOAT, nullptr);
	glNamedFramebufferTexture(FrameBufferID, GL_COLOR_ATTACHMENT0, voColorTexID, 0);

	glCreateRenderbuffers(1, &voRenderBufferID);
	glNamedRenderbufferStorage(voRenderBufferID, GL_DEPTH24_STENCIL8, vWidth, vHeight);
	glNamedFramebufferRenderbuffer(FrameBufferID, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, voRenderBufferID);

	_ASSERTE(glCheckNamedFramebufferStatus(FrameBufferID, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	return FrameBufferID;
}

void copyAttachmentByBinding(GLuint vReadFrameBufferID, GLuint vDrawFrameBufferID, int vWidth, int vHeight)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, vReadFrameBufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, vDrawFrameBufferID);
	glBlitFramebuffer(
		0, 0, vWidth, vHeight, 
		0, 0, vWidth, vHeight, 
		GL_COLOR_BUFFER_BIT, GL_LINEAR
	);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void copyAttachmentByUnBinding(GLuint vReadFrameBufferID, GLuint vDrawFrameBufferID, int vWidth, int vHeight)
{
	glBlitNamedFramebuffer(
		vReadFrameBufferID, vDrawFrameBufferID,
		0, 0, vWidth, vHeight,
		0, 0, vWidth, vHeight,
		GL_COLOR_BUFFER_BIT, GL_LINEAR
	);
}

void testBinding(int vWidth, int vHeight)
{
	GLuint ColorTexID1, RenderBufferID1;
	GLuint ColorTexID2, RenderBufferID2;
	const GLuint FrameBufferID1 = createFrameBufferByBinding(vWidth, vHeight, ColorTexID1, RenderBufferID1);
	const GLuint FrameBufferID2 = createFrameBufferByBinding(vWidth, vHeight, ColorTexID2, RenderBufferID2);
	copyAttachmentByBinding(FrameBufferID1, FrameBufferID2, vWidth, vHeight);
	glDeleteFramebuffers(1, &FrameBufferID1);
	glDeleteFramebuffers(1, &FrameBufferID2);
	glDeleteTextures(1, &ColorTexID1);
	glDeleteTextures(1, &ColorTexID2);
	glDeleteRenderbuffers(1, &RenderBufferID1);
	glDeleteRenderbuffers(1, &RenderBufferID2);
}

void testUnBinding(int vWidth, int vHeight)
{
	GLuint ColorTexID1, RenderBufferID1;
	GLuint ColorTexID2, RenderBufferID2;
	const GLuint FrameBufferID1 = createFrameBufferByUnBinding(vWidth, vHeight, ColorTexID1, RenderBufferID1);
	const GLuint FrameBufferID2 = createFrameBufferByUnBinding(vWidth, vHeight, ColorTexID2, RenderBufferID2);
	copyAttachmentByUnBinding(FrameBufferID1, FrameBufferID2, vWidth, vHeight);
	glDeleteFramebuffers(1, &FrameBufferID1);
	glDeleteFramebuffers(1, &FrameBufferID2);
	glDeleteTextures(1, &ColorTexID1);
	glDeleteTextures(1, &ColorTexID2);
	glDeleteRenderbuffers(1, &RenderBufferID1);
	glDeleteRenderbuffers(1, &RenderBufferID2);
}

int main()
{
	validation_utils::createWindow(800, 600, "Direct State Access");
	constexpr int Count = 1000;
	constexpr int Width = 512;
	constexpr int Height = 512;
	validation_utils::CTimer Timer;
	Timer.setMaxRecordCount(Count);
	std::cout << "Not use DSA\n";
	for (int i = 0; i < Count; ++i)
	{
		Timer.start();
		testBinding(Width, Height);
		Timer.getDeltaTime();
	}
	std::cout << "Use DSA\n";
	for (int i = 0; i < Count; ++i)
	{
		Timer.start();
		testUnBinding(Width, Height);
		Timer.getDeltaTime();
	}
	std::cout << "OpenGL Error: " << glGetError() << '\n';
	return 0;
}