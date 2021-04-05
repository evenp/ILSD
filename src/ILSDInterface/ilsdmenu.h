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

#ifndef ILSD_MENU_H
#define ILSD_MENU_H

#include "asImGuiWindow.h"
#include <vector>

class ASImage;
class GLWindow;
class ILSDDetectionWidget;
class ILSDItemControl;
class CTrackDetector;
class RidgeDetector;
class TerrainMap;


/**
 * @class ILSDMenu ilsdmenu.h
 * \brief Main menu of the interactive linear structure detection.
 */
class ILSDMenu
{
public:

  /**
   * \brief Creates a ILSD main menu.
   */
  ILSDMenu ();

  /**
   * \brief Declares the detection widget to call.
   * @param widget Reference to the detection widget controlled.
   */
  void setDetectionWidget (ILSDDetectionWidget* widget);

  /**
   * \brief Draws ASD user interface.
   * @param parentWindow Reference to the parent window.
   */
  virtual void drawUI (GLWindow* parentWindow);


private:

  /** Reference to ASD detection widget. */
  ILSDDetectionWidget* det_widget;
  /** Reference to the DTM normal map. */
  TerrainMap *dtm_map;
  /** Reference to an analysis controler. */
  ILSDItemControl *ictrl;
  /** Reference to the carriage track detector. */
  CTrackDetector *tdetector;
  /** Reference to the ridgeor hollow detector. */
  RidgeDetector *rdetector;

  /** Enable imgui demo windows */
  bool showDemoWindow;
  /** Import menu parent window. */
  GLWindow* import_parent;
  /** Import point tile name. */
  std::string import_tile;


  /**
   * \brief Draws ASD file menu.
   * Returns whether other menus should be displayed.
   */
  bool drawFileMenu (GLWindow* parent);

  /**
   * \brief Draws ASD display menu.
   * @param parent Reference to the parent window.
   */
  void drawDisplayMenu (GLWindow* parent);

  /**
   * \brief Draws ASD selection menu.
   * @param parent Reference to the parent window.
   */
  void drawSelectionMenu (GLWindow* parent);

  /**
   * \brief Draws ASD detection menu.
   * @param parent Reference to the parent window.
   */
  void drawDetectionMenu (GLWindow* parent);

  /**
   * \brief Draws ASD carriage track detection sub-menu.
   * @param parent Reference to the parent window.
   */
  void drawCTrackDetectionSubmenu (GLWindow* parent);

  /**
   * \brief Draws ASD ridge or hollow structure detection sub-menu.
   * @param parent Reference to the parent window.
   */
  void drawRidgeDetectionSubmenu (GLWindow* parent);

  /**
   * \brief Draws ASD cross profile menu.
   * @param parent Reference to the parent window.
   */
  void drawCrossProfileMenu (GLWindow* parent);

  /**
   * \brief Draws ASD longitudinal profile menu.
   * @param parent Reference to the parent window.
   */
  void drawLongProfileMenu (GLWindow* parent);

  /**
   * \brief Draws ASD preference menu.
   * @param parent Reference to the parent window.
   */
  void drawPrefMenu (GLWindow* parent);

  /**
   * \brief Draws information about mouse position.
   * @param parent Reference to the parent window.
   * @param sx Cursor X-position in main window.
   */
  void drawMouseInfo (GLWindow* parent, int sx);

  /**
   * \brief Draws information about input selection.
   * @param parent Reference to the parent window.
   * @param sy Cursor Y-position in main window.
   */
  void drawSelectionInfo (GLWindow* parent, int sy);

  /**
   * \brief Imports a point tile from xyz format to local til format.
   * @param path Names of selected files.
   */
  void importPointTile (const std::vector<std::string>& paths);

  /**
   * \brief Imports a DTM tile from asc format to local nvm format.
   * @param path Names of selected files.
   */
  void importDTMTiles (const std::vector<std::string>& paths);

};
#endif
