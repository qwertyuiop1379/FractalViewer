#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

class SandboxLayer : public GLCore::Layer
{
private:
	GLuint program;
	GLuint computeProgram;
	GLuint texture;

	GLCore::Utils::Shader* m_Shader;
	GLCore::Utils::Shader* m_ComputeShader;
	GLCore::Utils::OrthographicCameraController m_CameraController;
	
	GLuint m_QuadVA, m_QuadVB, m_QuadIB;

	double iter_max;
	double zoom;
	double ir;
	double im;

	bool ctrl_down;
	bool shift_down;
	bool click_down;

	unsigned int width;
	unsigned int height;

	unsigned int needsDisplay;
	unsigned int mode;

public:
	SandboxLayer();
	virtual ~SandboxLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
};