/*  Copyright 2021 Philippe Even, Phuc Ngo and Pierre Even,
      co-authors of paper:
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

#ifndef ILSD_WINDOW
#define ILSD_WINDOW

#include "glWindow.h"
#include "ilsddetectionwidget.h"

class ILSDDetectionWidget;
class ILSDMenu;
class ILSDKeyControl;


class ILSDWindow : public GLWindow
{
public:

  /**
   * \brief Creates a main window for interactive linear structure detection.
   */
  ILSDWindow ();

  /**
   * \brief Deletes the main ILSD window.
   */
  virtual ~ILSDWindow ();

  /**
   * \brief Returns the point cloud access type.
   */
  inline int cloudAccess () const { return detection_widget->cloudAccess (); }

  /**
   * \brief Sets the point cloud access type.
   * @param type New access type.
   */
  inline void setCloudAccess (int type) {
    detection_widget->setCloudAccess (type); }

  /**
   * \brief Loads tiles and resizes the window accordingly.
   * Returns whether loading succeeded.
   */
  bool loadTiles ();

  /**
   * \brief Sets detection mode (RIDGE, HOLLOW, CTRACK or NONE).
   * @param mode Selected detection mode.
   */
  void setDetectionMode (int mode);

  /**
   * \brief Switches analysis window on or off.
   */
  inline void switchAnalysisWindow () { showAnal = ! showAnal; }

  /**
   * \brief Takes into account the option (after image load).
   */
  void runOptions ();

  /**
   * \brief Performs a simple test.
   */
  void runTest ();


protected:

  /**
   * \brief Updates rendering (abstract)
   */
  virtual void drawWindow ();

  /**
   * \brief Handles key pressed event (abstract).
   * @param key Code of the key.
   * @param scancode Unused parameter.
   * @param action Code of hold action on the key.
   * @param mods Code of simultaneous control.
   */
  virtual void processKey (int key, int scancode, int action, int mods);

  /**
   * \brief Handles mouse button pressed event (abstract).
   * @param button Code of the button.
   * @param action Code of hold action on the button.
   * @param mods Code of simultaneous control.
   */
  virtual void processMouseButtonKey (int button, int action, int mods);

  /**
   * \brief Handles mouse movement event (abstract).
   * @param posX X-coordinate of mouse position.
   * @param posY Y-coordinate of mouse position.
   */
  virtual void moveCursor (double posX, double posY);

  /**
   * \brief Warns about child window imminent closing.
   * @param child Child window that will be closed.
   */
  void onChildWindowToBeDeleted (AsImGuiWindow *child);


private:

  /** ASD detection widget. */
  ILSDDetectionWidget* detection_widget;
  /** ASD menu. */
  ILSDMenu* menu;
  /** ASD control. */
  ILSDKeyControl* control;
  /** Analysis window displayed option. */
  bool showAnal;

};
#endif
