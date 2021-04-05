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

#ifndef ILSD_CROSS_PROFILE_VIEW_H
#define ILSD_CROSS_PROFILE_VIEW_H

#include "asImGuiWindow.h"
#include "ilsditemcontrol.h"
#include "ilsdcrossprofileitem.h"
#include "ridgedetector.h"
#include "ctrackdetector.h"

class ASImage;
class GLWindow;
class ILSDDetectionWidget;


/**
 * @class ILSDCrossProfileView ilsdcrossprofileview.h
 * \brief A window to analyse cross profiles of extracted structures.
 */
class ILSDCrossProfileView : public AsImGuiWindow
{

public:

  /**
   * \brief Creates an analysis window for linear structure cross profiles.
   * @param context GL context.
   * @param fpos Flag indicating whether a position is fixed for the view.
   * @param pos View position if fixed.
   * @param detector Reference to current carriage track detector.
   * @param ictrl Reference to applicable item controls.
   * @param inwidget Parent window controller.
   */
  ILSDCrossProfileView (GLWindow* context, bool fpos, ASCanvasPos &pos,
                        CTrackDetector* detector, ILSDItemControl *ictrl,
                        ILSDDetectionWidget* inwidget);

  /**
   * \brief Creates an analysis window for ridge cross profiles.
   * @param context GL context.
   * @param fpos Flag indicating whether a position is fixed for the view.
   * @param pos View position if fixed.
   * @param detector Reference to current ridge detector.
   * @param ictrl Reference to applicable item controls.
   * @param inwidget Parent window controller.
   */
  ILSDCrossProfileView (GLWindow* context, bool fpos, ASCanvasPos &pos,
                        RidgeDetector* detector, ILSDItemControl *ictrl,
                        ILSDDetectionWidget* inwidget);

  /**
   * \brief Creates an analysis window for straight line cross profiles.
   * @param context GL context.
   * @param fpos Flag indicating whether a position is fixed for the view.
   * @param pos View position if fixed.
   * @param subdiv DTM map subdivision factor.
   * @param ictrl Reference to applicable item controls.
   * @param inwidget Parent window controller.
   */
  ILSDCrossProfileView (GLWindow* context, bool fpos, ASCanvasPos &pos,
                        int subdiv, ILSDItemControl *ictrl,
                        ILSDDetectionWidget* inwidget);

  /**
   * \brief Deletes the analysis window.
   */
  virtual ~ILSDCrossProfileView ();

  /**
   * \brief Declares data to be analysed.
   * @param image DTM map.
   * @param pdata Point cloud.
   */
  void setData (ASImage* image, IPtTileSet* pdata);

  /**
   * \brief Resets the viewer for a new display.
   */
  void reset ();

  /**
   * \brief Updates the widget.
   */
  void update ();

  /**
   * \brief Resets viewer aspect.
   */
  void resetControls ();

  /**
   * \brief Resets viewer size on widget size.
   */
  void resetSize ();

  /**
   * \brief Gets currently analyzed DTM scan.
   */
  std::vector<Pt2i> *getCurrentScan () const {
    return item->getCurrentScan (); }

  /**
   * \brief Gets a specific analyzed DTM scan.
   * @param index Index of required scan.
   */
  std::vector<Pt2i> *getDisplayScan (int index) const {
    return item->getDisplayScan (index); }

  /**
   * \brief Sets the image scan area from an initial scan.
   * @param p1 Scan start point.
   * @param p2 Scan end point.
   */
  void buildScans (Pt2i p1, Pt2i p2);

  /**
   * \brief Requires scan building in analyzed widget.
   */
  void rebuildScans ();

  /**
   * \brief Returns the currently displayed scan.
   */
  inline int getScan () const { return item_ctrl->scan (); }

  /**
   * \brief Sets the currently displayed scan.
   * @param val Number of the scan to display.
   */
  void setScan (int val);

  /**
   * \brief Gets the measure line translation ratio.
   */
  inline float getMeasureLineTranslationRatio () const {
    return (item->getMeasureLineTranslationRatio ()); }

  /**
   * \brief Sets the measure line translation ratio.
   * @param val New translation ratio value.
   */
  inline void setMeasureLineTranslationRatio (float val) {
    item->setMeasureLineTranslationRatio (val); }

  /**
   * \brief Gets the measure line rotation ratio.
   */
  inline float getMeasureLineRotationRatio () const {
    return (item->getMeasureLineRotationRatio ()); }

  /**
   * \brief Sets the measure line rotation ratio.
   * @param val New rotation ratio value.
   */
  inline void setMeasureLineRotationRatio (float val) {
    item->setMeasureLineRotationRatio (val); }


protected:

  /**
   * \brief Draws imgui content.
   * @paral Window where the event was detected.
   */
  virtual void DrawContent (GLWindow* windowContext);

  /**
   * \brief Handles key event.
   * @param key Code of the key.
   * @param scancode Unused parameter.
   * @param action Code of hold action on the key.
   * @param mods Code of simultaneous control.
   */
  virtual void keyPressed (int key, int scancode, int action, int mods);


private:

  /** Cross profile analysis widget. */
  ILSDCrossProfileItem* item;
  /** Detection widget handling this view. */
  ILSDDetectionWidget* detection_widget;
  /** Reference to applicable analysis widget controls. */
  ILSDItemControl* item_ctrl;
  /** Count of captures. */
  int igrab;

};
#endif
