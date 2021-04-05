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

#ifndef AS_IMGUIWINDOWS_H
#define AS_IMGUIWINDOWS_H

#include <string>
#include "asCanvasPos.h"

using namespace std;

class GLWindow;

/**
 * @brief 'Virtual' window handled by imgui displayed inside a parent glfw window.
*/
class AsImGuiWindow
{
	/** 
	 * @brief GLWindow must be the only class that can delete this one
	*/
	friend class GLWindow;

public:

	/**
	 * @brief Create a virtual ImGui window inside given glfw context
	*/
	AsImGuiWindow(GLWindow* context, string name, bool bForceInitialPos = false, const ASCanvasPos& initialPos = ASCanvasPos(50,50));

	/**
	 * @brief set window title
	*/
	inline void setWindowTitle(string title) { windowName = title; }

	/**
	 * @brief draw imgui content. (called from GLWindow)
	*/
	void Draw();

	/**
	 * @brief does window be closed next frame
	*/
	inline bool ShouldClose() const { return !bKeepOpen; }

	/**
	 * @brief request closure of the window
	*/
	inline void CloseView() { bKeepOpen = false; }

	/**
	 * @brief get GLWindow who handle this window.
	*/
	GLWindow* getContext() const { return windowContext; }

	/**
	 * @brief set display resolution
	*/
	void resize(ASCanvasPos size) { windowSize = size; }

	/**
	 * @brief is window focused
	*/
	bool IsFocused() const;

	/**
	 * @brief When keyboard key is pressed (called from GLWindow)
	*/
	void processKey(int key, int scancode, int action, int mods);

	/**
	 * @brief When mouse button is pressed (called from GLWindow)
	*/
	void processMouseButton(int button, int action, int mods);

	/**
	 * @brief When mouse is moved (called from GLWindow)
	*/
	void processMouseMovement(double posX, double posY);

	/**
	 * @brief Return last knowed window position
	*/
	inline const ASCanvasPos& GetWindowPos() const { return windowPos; }

	/**
	 * @brief Set window position
	*/
	void setPosition(int wposx, int wposy);

protected:

	/**
	 * @brief delete the window (never call manually)
	*/
	virtual ~AsImGuiWindow() {}

	/**
	 * @brief Implement to add you own imgui content
	*/
	virtual void DrawContent(GLWindow* windowContext) = 0;

	/**
	 * @brief Implement to handle keyboard inputs (not called when not focused)
	*/
	virtual void keyPressed(int key, int scancode, int action, int mods) {}

	/**
	 * @brief Implement to handle mouse button inputs (not called when not focused)
	*/
	virtual void mouseButtonPressed(int button, int action, int mods) {}

	/**
	 * @brief Implement to handle mouse movements (not called when not focused)
	*/
	virtual void mouseMoved(double posX, double posY) {}

private:

	/**
	 * @brief The window will be closed when set to false
	*/
	bool bKeepOpen;

	/**
	 * @brief Is the window currently focused
	*/
	bool bIsFocused;

	/**
	* @brief Set window position for the first frame
	*/
	bool bSetWindowPos = false;

	/**
	 * @brief current window name
	*/
	string windowName;

	/**
	 * @brief Parent GLWindow
	*/
	GLWindow* windowContext;

	/**
	 * @brief current display resolution
	*/
	ASCanvasPos windowSize;

	/**
	 * @brief current display position
	*/
	ASCanvasPos windowPos;

	/**
	 * @brief output framebuffer to png
	 */
	void capture(const char* filePath);
};
#endif
