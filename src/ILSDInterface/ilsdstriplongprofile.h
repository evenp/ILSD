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

#ifndef ILSD_STRIP_LONG_PROFILE_H
#define ILSD_STRIP_LONG_PROFILE_H

#include "ilsdlongprofileitem.h"

class ASImage;
class GLWindow;
struct ASCanvasPos;


/**
 * @class ILSDStripLongProfile ilsdstriplongprofile.h
 * \brief Line strip longitudinal profile widget.
 */
class ILSDStripLongProfile : public ILSDLongProfileItem
{
public:

  /**
   * \brief Creates a line strip longitudinal profile widget.
   * @param subdiv DTM map subdivision factor.
   * @param item_ctrl Reference to applicable item controls.
   */
  ILSDStripLongProfile (int subdiv, ILSDItemControl *item_ctrl);

  /**
   * \brief \brief Returns the structure profile name.
   */
  std::string profileName () const;

  /**
   * \brief Declares data to be analysed.
   * @param image Reference to DTM view.
   * @param pdata Reference to point cloud.
   */
  void setData (ASImage* image, IPtTileSet* pdata);

  /**
   * \brief Sets longitudinal profile from an initial scan.
   * @param pt1 Scan start point.
   * @param pt2 Scan end point.
   */
  void buildProfile (Pt2i pt1, Pt2i pt2);

  /**
   * \brief Updates the widget display contents.
   * @params parentWindow Reference to parent widget.
   */
  void paint (GLWindow* parentWindow);

  /**
   * \brief Updates background image contents
   */
  void update ();

  /**
   * \brief Saves background image to given file (directory must exist).
   * @param fname Image file name.
   */
  void save (std::string fname);


protected:

  /**
   * \brief Updates drawing parameters.
   */
  void updateDrawing ();

  /**
   * \brief Displays selected scan detection status.
   */
  void paintStatus ();


private:

  /** Point grid / Dtm image ratio. */
  int subdiv;

  /** Point cloud central and left scans. */
  std::vector <std::vector <Pt2i> > leftscan;
  /** Point cloud right scans. */
  std::vector <std::vector <Pt2i> > rightscan;

  /** Scanner provider on point data */
  ScannerProvider scanp;

  /** Collected points on current stripe. */
  std::vector<Pt2f> current_points;
  /** Absolute reference altitude for profile drawing. */
  float href0;
  /** Scan strip reference altitude for profile drawing. */
  float hrefc;
  /** Local position reference point. */
  Pt2i np1;

};
#endif
