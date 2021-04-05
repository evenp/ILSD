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

#ifndef ILSD_STRIP_CROSS_PROFILE_H
#define ILSD_STRIP_CROSS_PROFILE_H

#include "ilsdcrossprofileitem.h"

class ASImage;
class GLWindow;
struct ASCanvasPos;


/**
 * @class ILSDStripCrossProfile ilsdstripcrossprofile.h
 * \brief Line strip cross profile widget.
 */
class ILSDStripCrossProfile : public ILSDCrossProfileItem
{
public:

  /**
   * \brief Creates a line strip cross profile widget.
   * @param subdiv DTM map subdivision factor.
   * @param item_ctrl Reference to applicable item controls.
   */
  ILSDStripCrossProfile (int subdiv, ILSDItemControl *item_ctrl);

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
   * \brief Sets the image scan area from an initial scan.
   * @param pt1 Scan start point.
   * @param pt2 Scan end point.
   */
  void buildScans (Pt2i pt1, Pt2i pt2);

  /**
   * \brief Sets the image scan area after width has changed.
   */
  virtual void rebuildScans ();

  /**
   * \brief Updates profile for a new stripe.
   */
  void updateProfile ();

  /**
   * \brief Checks is the analyse strip can be reset.
   */
  virtual bool isStripAdaptable () const;

  /**
   * \brief Gets currently analyzed DTM scan.
   */
  virtual std::vector<Pt2i> *getCurrentScan ();

  /**
   * \brief Gets DTM scan to be displayed.
   * @param num Scan number.
   */
  std::vector<Pt2i> *getDisplayScan (int num);

  /**
   * \brief Updates the widget display contents.
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
   * \brief Gets point profile to be displayed.
   * @param num Scan number.
   */
  std::vector<Pt2f> *getProfile (int num);

  /**
   * \brief Updates drawing parameters.
   */
  void updateDrawing ();

  /**
   * \brief Draws the scan strip.
   */
  void paintScans ();

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

  /** Image central and left scans. */
  std::vector <std::vector <Pt2i> > leftiscan;
  /** Image right scans. */
  std::vector <std::vector <Pt2i> > rightiscan;

  /** Scanner provider on point data */
  ScannerProvider scanp;
  /** Scanner provider on image */
  ScannerProvider scani;

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
