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

#ifndef ILSD_RIDGE_LONG_PROFILE_H
#define ILSD_RIDGE_LONG_PROFILE_H

#include "ilsdlongprofileitem.h"
#include "ridgedetector.h"

class ASImage;
class GLWindow;
struct ASCanvasPos;


/**
 * @class ILSDRidgeLongProfile ilsdridgelongprofile.h
 * \brief Ridge longitudinal profile widget.
 */
class ILSDRidgeLongProfile : public ILSDLongProfileItem
{
public:

  /**
   * \brief Creates a ridge longitudinal profile widget.
   * @param detector Reference to current ridge detector.
   * @param item_ctrl Reference to applicable item controls.
   */
  ILSDRidgeLongProfile (RidgeDetector* detector, ILSDItemControl *item_ctrl);

  /**
   * \brief Sets longitudinal profile from an initial scan.
   * @param pt1 Scan start point.
   * @param pt2 Scan end point.
   */
  void buildProfile (Pt2i pt1, Pt2i pt2);

  /**
   * \brief Updates the widget display.
   * @params parentWindow Reference to parent widget.
   */
  void paint (GLWindow* context);

  /**
   * \brief \brief Returns the structure profile name.
   */
  std::string profileName () const;

  /**
   * \brief Updates background image content.
   */
  void update ();

  /**
   * \brief Saves background image in given file (directory must exist).
   * @param fname Output image file name.
   */
  void save (std::string fname);


protected:

  /**
   * \brief Updates drawing parameters.
   */
  void updateDrawing ();

  /**
   * \brief Displays informations about selected plateau.
   */
//  void paintInfo ();

  /**
   * \brief Displays selected scan detection status.
   */
  void paintStatus ();


private:

  /** Reference to used detector. */
  RidgeDetector* det;


  /**
   * \brief Gets ridge point at distance pos on scan num.
   * @param num Scan index.
   * @param pos Point position coordinate on the scan.
   */
  Pt2f localize (int num, float pos) const;

};
#endif
