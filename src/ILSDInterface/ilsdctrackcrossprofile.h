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

#ifndef ILSD_CARRIAGE_TRACK_CROSS_PROFILE_H
#define ILSD_CARRIAGE_TRACK_CROSS_PROFILE_H

#include "ilsdcrossprofileitem.h"
#include "ctrackdetector.h"

class ASImage;
class ASPainter;
struct ASCanvasPos;
class GLWindow;


/**
 * @class ILSDCTrackCrossProfile ilsdctrackcrossprofile.h
 * \brief Carriage track cross profile widget.
 */
class ILSDCTrackCrossProfile : public ILSDCrossProfileItem
{
public:

  /**
   * \brief Creates a carriage track cross profile widget.
   * @param detector Reference to current carriage track detector.
   * @param item_ctrl Reference to applicable item controls.
   */
  ILSDCTrackCrossProfile (CTrackDetector* detector, ILSDItemControl *item_ctrl);

  /**
   * \brief \brief Returns the structure profile name.
   */
  std::string profileName () const;

  /**
   * \brief Gets DTM scan to be displayed.
   * @param num Scan number.
   */
  std::vector<Pt2i> *getDisplayScan (int num);

  /**
   * \brief Updates the widget display.
   */
  void paint (GLWindow* parentWindow);

  /**
   * \brief Updates background image contents.
   */
  void update ();

  /**
   * @brief Saves background image in given file (directory must exist).
   * @param name Image file name.
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
   * \brief Draws the scans used for the track detection.
   */
  void paintScans ();

  /**
   * \brief Draws aligned the scans used for the track detection.
   */
  void paintAlignedScans ();

  /**
   * \brief Draws the altitude profile of the selected scan.
   */
  void paintProfile ();

  /**
   * \brief Displays information about selected plateau.
   */
  void paintInfo ();

  /**
   * \brief Displays selected scan detection status.
   */
  void paintStatus ();


private:

  /** Reference to used detector. */
  CTrackDetector* det;

};
#endif
