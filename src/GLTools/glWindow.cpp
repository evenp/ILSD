/*  Copyright 2021 Pierre Even,
      co-author of paper:
      Even, P., Grzesznik, A., Gebhardt, A., Chenal, T., Even, P. and Ngo, P.,
      2021,
      Fast extraction of linear structures fromLiDAR raw data
      for archaeomorphological structure prospection.
      In the International Archives of the Photogrammetry, Remote Sensing
      and Spatial Information Sciences (proceedings of the 2021 edition
      of the XXIVth ISPRS Congress).

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "glWindow.h"

#ifdef __APPLE__
	#include "GL/glew.h"
#else
	#include "glad/glad.h"
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"
#include "asImGuiWindow.h"
#include "stb_image_write.h"
#include "asImage.h"
#include <chrono>
#include <thread>

#define FONTS_PATH "font/"
#define USED_FONT "Roboto-Medium.ttf"

GLWindow* GLWindow::MainWindow = nullptr;
std::chrono::steady_clock::time_point G_LAST_FRAME_TIME = std::chrono::steady_clock::now();
bool G_SLEEP_HIDLE_THREADS = true; //Can affect framerate stability, but highly reduce cpu usage.
uint64_t G_MAX_FRAMERATE = 60; //Max image per second

GLWindow::GLWindow(const char* windowTitle, const ASCanvasPos& size)
{
	if (GLWindow::MainWindow)
	{
		cerr << "Cannot create multiple application windows" << endl;
		return;
	}

	/** try to initialize glfw */
	if (!glfwInit())
	{
		cerr << "Failed to initialize glfw" << endl;
		return;
	}

	/** Set GLFW window propeties */
	std::string glsl_version = "";

	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	// GL 3.2 + GLSL 150
	glsl_version = "#version 150";
	glfwWindowHint( // required on Mac OS
		GLFW_OPENGL_FORWARD_COMPAT,
		GL_TRUE
	);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#elif __linux__
	// GL 3.2 + GLSL 150
	glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#elif _WIN32
	// GL 3.0 + GLSL 130
	glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif

#ifdef _WIN32
	// if it's a HighDPI monitor, try to scale everything
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	float xscale, yscale;
	glfwGetMonitorContentScale(monitor, &xscale, &yscale);
	if (xscale > 1 || yscale > 1)
	{
		glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
	}
#elif __APPLE__
	// to prevent 1200x800 from becoming 2400x1600
	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif


	/** Try to create window */
	if (!(glfwContext = glfwCreateWindow(size.x, size.y, windowTitle, NULL, NULL)))
	{
		glfwTerminate();
		cerr << "Failed to create window " << windowTitle << " (" << size.x << "x" << size.y << ")" << endl;
	}
	glfwSwapInterval(1);

	/** Bind glfw callbacks */
	glfwMakeContextCurrent(glfwContext);
	glfwSetErrorCallback(glfwErrorCallback);
	glfwSetKeyCallback(glfwContext, glfwKeyCallback);
	glfwSetFramebufferSizeCallback(glfwContext, glfwFramebuffer_size_callback);
	glfwSetMouseButtonCallback(glfwContext, glfwMouseButtonCallback);
	glfwSetCursorPosCallback(glfwContext, glfwCursorPosCallback);

#ifdef __APPLE__
	/** Glew initialization (openGL loader) */
	glewExperimental = GL_TRUE;
	glewInit();
#else
	/** Glad initialization (openGL loader) */
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cerr << "Failed to initialize GLAD" << std::endl;
	}
#endif


	glClearColor(0, 0, 0, 0);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	imguiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(imguiContext);
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Game pad Controls

	Font = io.Fonts->AddFontFromFileTTF(string(string(FONTS_PATH) + "/" + USED_FONT).data(), 16.f);

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(glfwContext, true);
	ImGui_ImplOpenGL3_Init(glsl_version.data());

	GLWindow::MainWindow = this;
}

GLWindow::~GLWindow()
{	
	/** shutdown imgui */
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	/** shutdown glfw */
	glfwDestroyWindow(GLWindow::MainWindow->glfwContext);
	glfwTerminate();

	GLWindow::MainWindow = nullptr;

	for (int64_t i = childWindows.size() - 1; i >= 0; --i)
	{
		delete childWindows[i];
	}
}

bool GLWindow::shouldWindowBeClosed() const
{
	return glfwWindowShouldClose(glfwContext);
}

void GLWindow::resize(const ASCanvasPos& size)
{
	glfwSetWindowSize(glfwContext, size.x, size.y);
}

void GLWindow::update()
{
	/** clamp framerate to 60 fps */

	double deltaTime;
	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - G_LAST_FRAME_TIME).count() / 1000000.0;

	if (G_MAX_FRAMERATE != 0 && (1.0 / deltaTime > G_MAX_FRAMERATE))
	{
		if (G_SLEEP_HIDLE_THREADS) std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<uint64_t>((1.0 / G_MAX_FRAMERATE - deltaTime) * 1000000000)));
		return;
	}
	G_LAST_FRAME_TIME = std::chrono::steady_clock::now();

	GLWindow::MainWindow->cycleRender();
}

void GLWindow::run()
{
	/** start render loop */
	while (!GLWindow::shouldExitProgram()) {
		GLWindow::update();
	}
}

void GLWindow::RegisterImGuiWindow(AsImGuiWindow* inWindow)
{
	childWindows.push_back(inWindow);
}

void GLWindow::UnRegisterImGuiWindow(AsImGuiWindow* inWindow)
{
	bool bCanDelete = false;
	for (unsigned int i = 0; i < childWindows.size(); ++i)
	{
		if (childWindows[i] == inWindow)
		{
			childWindows.erase(childWindows.begin() + i);
			bCanDelete = true;
		}
	}
	onChildWindowToBeDeleted (inWindow);
	if (bCanDelete) delete inWindow;
}

bool GLWindow::shouldExitProgram()
{
	return GLWindow::MainWindow->shouldWindowBeClosed();
}

void GLWindow::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/* process inputs for main window */
	GLWindow::MainWindow->processKey(key, scancode, action, mods);

	/* close main windows if any subwindows is open */
	if (action == GLFW_PRESS && GLWindow::getMainWindow()->IsBackgroundHovered())
	{
		if (key == GLFW_KEY_ESCAPE)
		{
			glfwSetWindowShouldClose(GLWindow::MainWindow->getGlfwContext(), 1);
		}
		if (key == GLFW_KEY_P && mods & GLFW_MOD_SHIFT)
		{
			GLWindow::MainWindow->capture("captures/Screenshot_Window.png");
		}
	}
	else
	{
		/* process inputs for childs windows */
		for (int64_t i = GLWindow::MainWindow->childWindows.size() - 1; i >= 0; --i)
			GLWindow::MainWindow->childWindows[i]->processKey(key, scancode, action, mods);
	}
}

void GLWindow::glfwErrorCallback(int error, const char* description)
{
	cerr << "glfw Error (" << error << ") : " << description << std::endl;
}

void GLWindow::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (GLWindow::getMainWindow()->IsBackgroundHovered())
		GLWindow::MainWindow->processMouseButtonKey(button, action, mods);

	for (int64_t i = GLWindow::MainWindow->childWindows.size() - 1; i >= 0; --i)
		GLWindow::MainWindow->childWindows[i]->processMouseButton(button, action, mods);
}

void GLWindow::glfwCursorPosCallback(GLFWwindow* window, double posX, double posY)
{
	if (GLWindow::getMainWindow()->IsBackgroundHovered() && !ImGui::IsAnyWindowFocused())
		GLWindow::MainWindow->moveCursor(posX, posY);

	for (int64_t i = GLWindow::MainWindow->childWindows.size() - 1; i >= 0; --i)
		GLWindow::MainWindow->childWindows[i]->processMouseMovement(posX, posY);
}

void GLWindow::cycleRender()
{
	/* switch to current windows context (should alway be the same) */
	glfwMakeContextCurrent(glfwContext);
	ImGui::SetCurrentContext(imguiContext);

	/* event manager */
	glfwPollEvents();

	/* Initialize new imgui frame */
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	glClear(GL_COLOR_BUFFER_BIT);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 13));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5, 0.5  ));
	ImGui::PushFont(Font);

	/* Draw content */
	drawWindow();

	/* Draw glfw windows */
	for (int64_t i = childWindows.size() - 1; i >= 0; --i)
	{
		if (childWindows[i]->ShouldClose())
		{
//			delete childWindows[i];
			UnRegisterImGuiWindow(childWindows[i]);
		}
		else
		{
			childWindows[i]->Draw();
		}
	}

	ImGui::PopFont();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGuiContext& g = *GImGui;
	bIsBackgroundHovered = !ImGui::IsAnyItemHovered() && !ImGui::IsAnyWindowHovered() && g.OpenPopupStack.Size == 0;

	ImGui::EndFrame();

	/* Render imgui draw data */
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(glfwContext);
}

void GLWindow::capture(const char* filePath)
{
	int x, y;
	glfwGetWindowSize(getGlfwContext(), &x, &y);
	uint8_t* data = new uint8_t[sizeof(uint8_t) * 3 * x * y];
	glReadPixels(0, 0, x, y, GL_RGB, GL_UNSIGNED_BYTE, data);
	ASImage img(ASCanvasPos(x, y));
	img.clear(ASColor::BLACK);

	for (int i = 0; i < x; ++i)
	{
		for (int j = 0; j < y; ++j)
		{
			img.setPixel(i, y - 1 - j, data[(i + j * x) * 3], data[(i + j * x) * 3 + 1], data[(i + j * x) * 3 + 2], 255); // flip image (no alpha)
		}
	}
	img.save(filePath);
	cout << "captured window to " << filePath << endl;
}
