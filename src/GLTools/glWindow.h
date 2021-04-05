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

#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <vector>
#include <string>

using namespace std;

/* Forward declarations */
struct GLFWwindow;
class AsImGuiWindow;
struct ASCanvasPos;
struct ImGuiContext;
struct ImFont;

/**
 * @brief Manage window creation and destruction. Also handle render loop and children imgui windows.
*/
class GLWindow
{
public:

	/**
	 * @brief Create a glfw window. Never create two GLWindow at the same time.
	*/
	GLWindow(const char* windowTitle, const ASCanvasPos& size);

	/**
	 * @brief Destroy glfw window
	*/
	virtual ~GLWindow();

	/**
	 * @brief get glfw context for this window
	*/
	inline GLFWwindow* getGlfwContext() const { return glfwContext; }

	/**
	 * @brief get imgui context for this window 
	*/
	inline ImGuiContext* getImGuiContext() const { return imguiContext; }

	/**
	 * @brief set window size (pixels)
	*/
	void resize(const ASCanvasPos& size);

	/**
	 * @brief Start render loop
	*/
	static void run();

	/**
	 * @brief Get reference to main window
	*/
	inline static GLWindow* getMainWindow() { return MainWindow; }

	/**
	 * @brief register new imgui window
	*/
	void RegisterImGuiWindow(AsImGuiWindow* inWindow);

	/**
	 * @brief unregister destroyed imgui window
	*/
	void UnRegisterImGuiWindow(AsImGuiWindow* inWindow);

	/**
	 * @brief tell if anything is focused, when cursor is on background
	 */
	inline bool IsBackgroundHovered() { return bIsBackgroundHovered; }

protected:

	/**
	 * @brief abstract, rendering update
	*/
	virtual void drawWindow() = 0;

	/**
	 * @brief abstract, key pressed event
	*/
	virtual void processKey(int key, int scancode, int action, int mods) = 0;

	/**
	 * @brief abstract, mouse button pressed event
	*/
	virtual void processMouseButtonKey(int button, int action, int mods) = 0;

	/**
	 * @brief abstract, mouse movement event
	*/
	virtual void moveCursor(double posX, double posY) = 0;

	/**
	 * @brief Warns about child window imminent closing.
	*/
    virtual void onChildWindowToBeDeleted (AsImGuiWindow *child) = 0;

private:

	/**
	 * @brief Glfw render context
	*/
	GLFWwindow* glfwContext;

	/**
	 * @brief ImGui render context
	*/
	ImGuiContext* imguiContext;

	/**
	 * @brief Imgui current font
	*/
	ImFont* Font;

	/**
	 * @brief displayed children imgui windows
	*/
	vector<AsImGuiWindow*> childWindows;

	/**
	 * @brief tel if background is hovered
	 */
	bool bIsBackgroundHovered;

	/**
	 * @brief draw frame
	*/
	void cycleRender();

	/**
	 * @brief output framebuffer to png
	 */
	void capture(const char* filePath);

	/**
	 * @brief should the window be destroyed for the next frame
	*/
	bool shouldWindowBeClosed() const;

	/**
	 * @brief currently displayed window
	*/
	static GLWindow* MainWindow;

	/**
	 * @brief try to draw next frame
	*/
	static void update();

	/**
	 * @brief should the application be closed for the next frame
	*/
	static bool shouldExitProgram();

	/**
	 * @brief glfw errors callback
	*/
	static void glfwErrorCallback(int error, const char* description);

	/**
	 * @brief window resize callback
	*/
	static void glfwFramebuffer_size_callback(GLFWwindow* window, int width, int height) {}

	/**
	 * @brief key press callnack
	*/
	static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	/**
	 * @brief mouse button callback
	*/
	static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	/**
	 * @brief mouse movement callback
	*/
	static void glfwCursorPosCallback(GLFWwindow* window, double posX, double posY);
};
#endif
