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

#ifndef ILSD_KEY_CONTROL_H
#define ILSD_KEY_CONTROL_H

#include "asImGuiWindow.h"

class ASImage;
class GLWindow;
class ILSDDetectionWidget;
class CTrackDetector;
class RidgeDetector;
class TerrainMap;


/**
 * @class ILSDKeyControl ilsdkeycontrol.h
 * \brief Keyboard control of the interactive linear structure detector.
 */
class ILSDKeyControl
{
public:

  /**
   * \brief Creates a ILSD keyboard control.
   */
  ILSDKeyControl ();

  /**
   * \brief Declares the detection widget to call.
   * @param widget Reference to the detection widget controlled.
   */
  void setDetectionWidget (ILSDDetectionWidget* widget);

  /**
   * \brief Handles key pressed event.
   * @param parentWindow Reference to the parent window.
   * @param key Code of the key.
   * @param scancode Unused parameter.
   * @param action Code of hold action on the key.
   * @param mods Code of simultaneous control.
   */
  void processKey (GLWindow* parent,
                   int key, int scancode, int action, int mods);

private:

  /** Reference to ASD detection widget. */
  ILSDDetectionWidget* det_widget;
  /** Reference to the DTM normal map. */
  TerrainMap *dtm_map;
  /** Reference to the carriage track detector. */
  CTrackDetector *tdetector;
  /** Reference to the ridgeor hollow detector. */
  RidgeDetector *rdetector;

};
#endif
