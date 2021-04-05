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

#include "asImGuiWindow.h"
#include "imgui.h"
#include "glWindow.h"
#include "imgui_internal.h"
#include <iostream>
#ifdef __APPLE__
	#include "GL/glew.h"
#else
	#include "glad/glad.h"
#endif
#include "GLFW/glfw3.h"
#include "asImage.h"

constexpr int G_IMGUI_WINDOW_TITLE_BAR_HEIGHT = 25;

AsImGuiWindow::AsImGuiWindow(GLWindow* context, string name, bool bForceInitialPos, const ASCanvasPos& initialPos)
	: windowContext(context), windowName(name), windowPos(initialPos), bSetWindowPos(!bForceInitialPos) {
	context->RegisterImGuiWindow(this);
	windowSize = ASCanvasPos(800, 600);
	bKeepOpen = true;
	bIsFocused = false;
}

void AsImGuiWindow::Draw()
{
//	if (IsFocused()) ImGui::SetNextWindowFocus();
	ImGui::SetNextWindowSize(ImVec2((float)windowSize.x, (float)windowSize.y + G_IMGUI_WINDOW_TITLE_BAR_HEIGHT));
	if (!bSetWindowPos)
	{
		bSetWindowPos = true;
		ImGui::SetNextWindowPos(ImVec2((float)windowPos.x, (float)windowPos.y));

	}
	if (ImGui::Begin(windowName.data(), &bKeepOpen, ImGuiWindowFlags_NoResize))
	{
		bIsFocused = ImGui::IsWindowHovered();
		windowPos.x = (uint32_t)ImGui::GetWindowPos().x;
		windowPos.y = (uint32_t)ImGui::GetWindowPos().y;
		DrawContent(windowContext);
		ImGui::End();
	}
}

bool AsImGuiWindow::IsFocused() const
{	
	return bIsFocused;
}

void AsImGuiWindow::processKey(int key, int scancode, int action, int mods)
{
	if (IsFocused())
	{
		if (key == GLFW_KEY_ESCAPE && mods == 0 && action == GLFW_PRESS)
		{
			bKeepOpen = false;
		}
		else if (key == GLFW_KEY_P && mods & GLFW_MOD_SHIFT && action == GLFW_PRESS)
		{
			capture(string(string("captures/screenshot_") + windowName + ".png").data());
		}
		else
		{
			keyPressed(key, scancode, action, mods);
		}
	}
}

void AsImGuiWindow::processMouseButton(int button, int action, int mods)
{
	if (IsFocused()) mouseButtonPressed(button, action, mods);
}

void AsImGuiWindow::processMouseMovement(double posX, double posY)
{
	if (IsFocused()) mouseMoved(posX, posY);
}

void AsImGuiWindow::capture(const char* filePath)
{
	int fX, fY;
	glfwGetWindowSize(GLWindow::getMainWindow()->getGlfwContext(), &fX, &fY);

	int x = windowSize.x;
	int y = windowSize.y + G_IMGUI_WINDOW_TITLE_BAR_HEIGHT;
	int posX = windowPos.x;
	int posY = windowPos.y;

	uint8_t* data = new uint8_t[sizeof(uint8_t) * 3 * x * y];
	glReadPixels(posX, fY - posY - y, x, y, GL_RGB, GL_UNSIGNED_BYTE, data);
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

void AsImGuiWindow::setPosition(int wposx, int wposy)
{
	windowPos.set(wposx, wposy);
}
