#include "SandboxLayer.h"

using namespace GLCore;
using namespace GLCore::Utils;

GLubyte indices[] = { 0, 1, 2, 0, 2, 3 };
GLfloat vertices[] = { -1, -1, 0, // bottom left corner
					  -1,  1, 0, // top left corner
					   1,  1, 0, // top right corner
					   1, -1, 0 }; // bottom right corner

GLuint vao, vbo;

bool CheckShader(GLuint shader)
{
	GLint compiled;
	GLint blen = 0;
	GLsizei slen = 0;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &blen);

		if (blen)
		{
			GLchar *log = (GLchar *)malloc(blen);
			glGetShaderInfoLog(shader, blen, &slen, log);
			printf("Compile Error:\n%s\n", log);
			free(log);

			return false;
		}
	}

	return true;
}

bool CheckLinked(GLuint program)
{
	GLint linked;
	GLint blen = 0;
	GLsizei slen = 0;

	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &blen);

		if (blen)
		{
			GLchar *log = (GLchar *)malloc(blen);
			glGetProgramInfoLog(program, blen, &slen, log);
			printf("Link Error:\n%s\n", log);
			free(log);

			return false;
		}
	}

	return true;
}

SandboxLayer::SandboxLayer() : m_CameraController(16.0f / 9.0f)
{
}

SandboxLayer::~SandboxLayer()
{
}

void SandboxLayer::OnAttach()
{
	EnableGLDebugging();

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_Shader = Shader::FromGLSLTextFiles("vertex.glsl", "fragment.glsl");

	glCreateVertexArrays(1, &m_QuadVA);
	glBindVertexArray(m_QuadVA);

	float vertices[] =
	{
		-1, -1, 0,
		-1,  1, 0,
		 1,  1, 0,
		 1, -1, 0
	};

	glCreateBuffers(1, &m_QuadVB);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };
	glCreateBuffers(1, &m_QuadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	iter_max = 100;
	zoom = 500;
	ir = 0;
	im = 0;

	ctrl_down = false;
	shift_down = false;
	click_down = false;

	GLint v[4];

	glGetIntegerv(GL_VIEWPORT, v);

	width = v[2];
	height = v[3];

	needsDisplay = 1;
	mode = 0;
}

void SandboxLayer::OnDetach()
{
	glDeleteVertexArrays(1, &m_QuadVA);
	glDeleteBuffers(1, &m_QuadVB);
	glDeleteBuffers(1, &m_QuadIB);
}

void SandboxLayer::OnEvent(Event& event)
{
	m_CameraController.OnEvent(event);

	EventDispatcher dispatcher(event);

	dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent &e)
	{
		if (ctrl_down)
		{
			if (e.GetYOffset() < 0)
				iter_max *= 0.9;
			else
				iter_max = (iter_max < 20) ? iter_max + 1 : iter_max * 1.1;

			if (iter_max < 1)
				iter_max = 1;
		}
		else
		{
			if (e.GetYOffset() < 0)
				zoom *= 0.75;
			else
				zoom *= 1.25;

			if (zoom < 1)
				zoom = 1;

			if (zoom > 4000000000000000)
				zoom = 4000000000000000;
		}

		needsDisplay = 1;
		return false;
	});

	dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e)
	{
		switch (e.GetKeyCode())
		{
			case 341:
			case 345:
			{
				ctrl_down = true;
				break;
			}

			case 340:
			case 344:
			{
				shift_down = true;
				break;
			}
		}

		return false;
	});

	dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent &e)
	{
		switch (e.GetKeyCode())
		{
			case 341:
			case 345:
			{
				ctrl_down = false;
				break;
			}

			case 340:
			case 344:
			{
				shift_down = false;
				break;
			}

			case 67:
			{
				if (ctrl_down)
				{
					printf("R: ");
					scanf("%lf", &ir);
					printf("M: ");
					scanf("%lf", &im);

					needsDisplay = 1;
				}
				else
				{
					printf("R: %f\nI: %f\n", ir, im);
				}

				break;
			}

			case 77:
			{
				mode = ++mode == 2 ? 0 : mode;
				needsDisplay = 1;
				break;
			}

			case 90:
			{
				printf("Zoom: %f\n", zoom);
				break;
			}
		}

		return false;
	});

	dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent &e)
	{
		if (e.GetMouseButton() == 0)
			click_down = true;

		return false;
	});

	dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent &e)
	{
		if (e.GetMouseButton() == 0)
			click_down = false;

		return false;
	});

	dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent &e)
	{
		static double mx, my;

		if (click_down)
		{
			double dx = (mx - e.GetX()) / zoom;
			double dy = (e.GetY() - my) / zoom;

			ir += dx;
			im += dy;

			needsDisplay = 1;
		}

		mx = e.GetX();
		my = e.GetY();

		return false;
	});

	dispatcher.Dispatch<WindowResizeEvent>([&](WindowResizeEvent &e)
	{
		width = e.GetWidth();
		height = e.GetHeight();

		glViewport(0, 0, width, height);

		needsDisplay = 1;
		return false;
	});
}

void SandboxLayer::OnUpdate(Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	if (needsDisplay)
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_Shader->GetRendererID());

		glUniform1ui(glGetUniformLocation(m_Shader->GetRendererID(), "iter_max"), (unsigned int)iter_max);
		glUniform1ui(glGetUniformLocation(m_Shader->GetRendererID(), "mode"), mode);
		glUniform2ui(glGetUniformLocation(m_Shader->GetRendererID(), "screen"), width, height);
		glUniform2d(glGetUniformLocation(m_Shader->GetRendererID(), "center"), ir , im);
		glUniform1d(glGetUniformLocation(m_Shader->GetRendererID(), "zoom"), zoom);

		glBindVertexArray(m_QuadVA);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		needsDisplay = needsDisplay == 1 ? 2 : 0;
	}
}

void SandboxLayer::OnImGuiRender()
{
	
}
