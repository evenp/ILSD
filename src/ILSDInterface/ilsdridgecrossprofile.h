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

#ifndef ILSD_RIDGE_CROSS_PROFILE_H
#define ILSD_RIDGE_CROSS_PROFILE_H

#include "ilsdcrossprofileitem.h"
#include "ridgedetector.h"

class ASImage;
class GLWindow;
struct ASCanvasPos;


/**
 * @class ILSDRidgeCrossProfile ilsdridgecrossprofile.h
 * \brief Ridge cross profile widget.
 */
class ILSDRidgeCrossProfile : public ILSDCrossProfileItem
{
public:

  /**
   * \brief Creates a ridge cross profile widget.
   * @param detector Reference to current ridge detector.
   * @param item_ctrl Reference to applicable item controls.
   */
  ILSDRidgeCrossProfile (RidgeDetector* detector, ILSDItemControl *item_ctrl);

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
   * @params parentWindow Reference to parent widget.
   */
  void paint (GLWindow* context);

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
   * \brief Gets point profile to be displayed.
   * @param num Scan number.
   */
  std::vector<Pt2f> *getProfile (int num);

  /**
   * \brief Updates drawing parameters.
   */
  void updateDrawing ();

  /**
   * \brief Returns measure line translation ratio.
   */
  float getMeasureLineTranslationRatio () const;

  /**
   * \brief Increments measure line translation ratio.
   * @param inc Increment direction.
   */
  void incMeasureLineTranslationRatio (int inc);

  /**
   * \brief Sets measure line translation ratio.
   * @param val New translation ratio value.
   */
  void setMeasureLineTranslationRatio (float val);

  /**
   * \brief Returns measure line rotation ratio.
   */
  float getMeasureLineRotationRatio () const;

  /**
   * \brief Increments measure line rotation ratio.
   * @param inc Increment direction.
   */
  void incMeasureLineRotationRatio (int inc);

  /**
   * \brief Sets measure line rotation ratio.
   * @param val New rotation ratio value.
   */
  void setMeasureLineRotationRatio (float val);

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
   * \brief Displays informations about selected plateau.
   */
  void paintInfo ();

  /**
   * \brief Displays selected scan detection status.
   */
  void paintStatus ();


private:

  /** Reference to used detector. */
  RidgeDetector* det;

  /** Estimated measure to be displayed. */
  float meas_est;
  /** Estimated measure lower bound to be displayed. */
  float meas_low;
  /** Estimated measure upper bound to be displayed. */
  float meas_up;


  /**
   * \brief Updates measure features.
   */
  void updateMeasure ();

};
#endif
