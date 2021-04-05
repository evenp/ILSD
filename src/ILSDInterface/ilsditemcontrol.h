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

#ifndef ILSD_ITEM_CONTROL_H
#define ILSD_ITEM_CONTROL_H

#include <string>


/**
 * @class ILSDItemControl ilsditemcontrol.h
 * \brief Structure analysis widget controls.
 */
class ILSDItemControl
{
public:

  /** Analysis parameter : default width for straight strip analysis. */
  static const int DEFAULT_STRAIGHT_STRIP_WIDTH;
  /** Analysis parameter : minimal width of analyzed straight strip. */
  static const int MIN_STRAIGHT_STRIP_WIDTH;
  /** Analysis parameter : maximal width of analyzed straight strip. */
  static const int MAX_STRAIGHT_STRIP_WIDTH;
  /** Drawing parameter : default profile view size. */
  static const int DEFAULT_PROF_WIDTH;
  /** Drawing parameter : minimal profile view size. */
  static const int MIN_PROF_WIDTH;
  /** Drawing parameter : maximal profile view size. */
  static const int MAX_PROF_WIDTH;
  /** Drawing parameter : default scan view size. */
  static const int DEFAULT_SCAN_WIDTH;
  /** Drawing parameter : minimal scan view size. */
  static const int MIN_SCAN_WIDTH;
  /** Drawing parameter : maximal scan view size. */
  static const int MAX_SCAN_WIDTH;
  /** Drawing parameter : default analysis view height. */
  static const int DEFAULT_VIEW_HEIGHT;
  /** Drawing parameter : minimal analysis view height. */
  static const int MIN_VIEW_HEIGHT;
  /** Drawing parameter : maximal analysis view height. */
  static const int MAX_VIEW_HEIGHT;
  /** Drawing parameter : measure line maximal translation ratio. */
  static const float MAX_LINE_TRANSLATION_RATIO;
  /** Drawing parameter : measure line maximal rotation ratio. */
  static const float MAX_LINE_ROTATION_RATIO;
  /** Drawing parameter : default longitudinal profile view size. */
  static const int DEFAULT_LONG_VIEW_WIDTH;
  /** Drawing parameter : default longitudinal profile view height. */
  static const int DEFAULT_LONG_VIEW_HEIGHT;
  /** Drawing parameter : minimal longitudinal profile view width. */
  static const int MIN_LONG_VIEW_WIDTH;
  /** Drawing parameter : maximal longitudinal profile view width. */
  static const int MAX_LONG_VIEW_WIDTH;
  /** Drawing parameter : minimal longitudinal profile view height. */
  static const int MIN_LONG_VIEW_HEIGHT;
  /** Drawing parameter : maximal longitudinal profile view height. */
  static const int MAX_LONG_VIEW_HEIGHT;


  /**
   * \brief Creates structure analysis widget controls.
   */
  ILSDItemControl ();

  /**
   * \brief Returns the width of analyzed straight strip.
   */
  inline int straightStripWidth () const { return straight_strip_width; }

  /**
   * \brief Increments the width of analyzed straight strip.
   * @param inc Increment step.
   */
  void incStraightStripWidth (int inc);

  /**
   * \brief Sets the width of analyzed straight strip.
   * @param val New width.
   */
  void setStraightStripWidth (int val);

  /**
   * \brief Returns the profile view width.
   */
  inline int profileWidth () const { return prof_width; }

  /**
   * \brief Increments the profile view width.
   * @param inc Increment step.
   */
  void incProfileWidth (int inc);

  /**
   * \brief Sets the profile view width.
   * @param val New width.
   */
  void setProfileWidth (int val);

  /**
   * \brief Returns the scan view width.
   */
  inline int scanWidth () const { return scan_width; }

  /**
   * \brief Increments the scan view width.
   * @param inc Increment step.
   */
  void incScanWidth (int inc);

  /**
   * \brief Sets the scan view width.
   * @param val New width.
   */
  void setScanWidth (int val);

  /**
   * \brief Returns the analysis view height.
   */
  inline int viewHeight () const { return view_height; }

  /**
   * \brief Increments the analysis view height.
   * @param inc Increment step.
   */
  void incViewHeight (int inc);

  /**
   * \brief Sets the analysis view height.
   * @param val New height.
   */
  void setViewHeight (int val);

  /**
   * \brief Returns the longitudinal profile view width.
   */
  inline int longViewWidth () const { return long_view_width; }

  /**
   * \brief Increments the longitudinal profile view width.
   * @param inc Increment step.
   */
  void incLongViewWidth (int inc);

  /**
   * \brief Sets the longitudinal profile view width.
   * @param val New width.
   */
  void setLongViewWidth (int val);

  /**
   * \brief Returns the longitudinal profile view height.
   */
  inline int longViewHeight () const { return long_view_height; }

  /**
   * \brief Increments the longitudinal profile view height.
   * @param inc Increment step.
   */
  void incLongViewHeight (int inc);

  /**
   * \brief Sets the longitudinal profile view height.
   * @param val New height.
   */
  void setLongViewHeight (int val);

  /**
   * \brief Gets current scan resolution.
   */
  inline int scanResolution () const { return scan_resol; }

  /**
   * \brief Gets current scan resolution level (as a power of 2).
   */
  int scanResolutionLevel () const;

  /**
   * \brief Increments the current scan resolution.
   * @param inc Direction (1 for zooming, -1 for unzooming).
   */
  void incScanResolution (int inc);

  /**
   * \brief Sets the current scan resolution.
   * @param val Power of 2 for the new resolution value.
   */
  void setScanResolution (int val);

  /**
   * \brief Returns the cloud points size.
   */
  inline int pointSize () const { return point_size; }

  /**
   * \brief Increments the cloud points size.
   * @param inc Direction (1 for leftwards, -1 for rightwards).
   */
  void incPointSize (int inc);

  /**
   * \brief Sets the cloud points size.
   * @param val New size.
   */
  void setPointSize (int val);

  /**
   * \brief Returns the current profile display shift.
   */
  inline int profileShift () const { return profile_shift; }

  /**
   * \brief Shifts the profile display.
   * @param inc Direction (1 for leftwards, -1 for rightwards)
   */
  inline void shiftProfile (int inc) { profile_shift += inc * PROF_SHIFT_INC; }

  /**
   * \brief Resets the current profile display shift.
   */
  inline void resetProfileShift () { profile_shift = 0; }

  /**
   * \brief Returns the scan alignment modality status.
   */
  inline bool isAligned () const { return aligned; }

  /**
   * \brief Switches scan alignment modality.
   */
  inline void switchAligned () { aligned = ! aligned; }

  /**
   * \brief Returns the status of initial detection display modality.
   */
  inline bool isInitialDetection () const { return initial_detection; }

  /**
   * \brief Switches initial detection display modality.
   */
  inline void switchInitialDetection () {
    initial_detection = ! initial_detection; }

  /**
   * \brief Inquires whether height reference is static.
   */
  inline bool isStaticHeight () const { return static_height; }

  /**
   * \brief Switches profile reference between input stroke or current scan.
   */
  inline void switchStaticHeight () { static_height = !static_height; }

  /**
   * \brief Inquires if template is displayed.
   */
  inline bool isTemplateDisplay () const { return display_template; }

  /**
   * \brief Switches template display modality.
   */
  inline void switchTemplateDisplay () {
    display_template = ! display_template; }

  /**
   * \brief Inquires if reference template is displayed.
   */
  inline bool isRefDisplay () const { return display_reference; }

  /**
   * \brief Switches reference template display modality.
   */
  inline void switchRefDisplay () { display_reference = ! display_reference; }

  /**
   * \brief Inquires if estimated position is displayed.
   */
  inline bool isEstimDisplay () const { return display_estimation; }

  /**
   * \brief Switches estimated position display modality.
   */
  inline void switchEstimDisplay () {
    display_estimation = ! display_estimation; }

  /**
   * \brief Inquires if estimated direction is displayed.
   */
  inline bool isDirDisplay () const { return display_direction; }

  /**
   * \brief Switches estimated direction display modality.
   */
  inline void switchDirDisplay () {
    display_direction = ! display_direction; }

  /**
   * \brief Inquires if next position is displayed.
   */
  inline bool isPredDisplay () const { return display_prediction; }

  /**
   * \brief Switches next position display modality.
   */
  inline void switchPredDisplay () {
    display_prediction = ! display_prediction; }

  /**
   * \brief Inquires if legend is displayed.
   */
  inline bool isLegendDisplay () const { return display_legend; }

  /**
   * \brief Switches legend display modality.
   */
  inline void switchLegendDisplay () { display_legend = ! display_legend; }

  /**
   * \brief Returns measure computation and display modality status.
   */
  inline bool isMeasuring () const { return measuring; }

  /**
   * \brief Switches measure computation and display modality status.
   */
  inline void switchMeasuring () { measuring = ! measuring; }

  /**
   * \brief Returns measure computation and display modality status.
   */
  inline bool isThinLongStrip () const { return thin_long_strip; }

  /**
   * \brief Switches measure computation and display modality status.
   */
  inline void switchThinLongStrip () { thin_long_strip = ! thin_long_strip; }

  /**
   * \brief Returns the structure minimal scan index.
   */
  inline int minScan () const { return min_scan; }

  /**
   * \brief Sets a minimal value for scan index.
   * @param val New scan index.
   */
  void setMinScan (int val);

  /**
   * \brief Returns the structure maximal scan index.
   */
  inline int maxScan () const { return max_scan; }

  /**
   * \brief Sets a maximal value for scan index.
   * @param val New scan index.
   */
  void setMaxScan (int val);

  /**
   * \brief Returns the current scan index.
   */
  inline int scan () const { return cur_scan; }

  /**
   * \brief Increments the current scan index.
   * @param inc Increment value.
   */
  void incScan (int inc);

  /**
   * \brief Sets the current scan index.
   * @param val New index value.
   */
  void setScan (int val);

  /**
   * \brief Resets the current scan interval.
   */
  void resetScan ();

  /**
   * \brief Checks if the measure interval setting has started.
   */
  inline bool isSetMeasure () const {return (mstart_on || mstop_on); }

  /**
   * \brief Returns start index of complete measure interval.
   */
  inline int measureStart () const {return mstart; }

  /**
   * \brief Returns start index of current measure interval.
   */
  inline int measureIntervalStart () const {
    return (mstart_on ? mstart : cur_scan); }

  /**
   * \brief Returns stop index of complete measure interval.
   */
  inline int measureStop () const {return mstop; }

  /**
   * \brief Returns stop index of current measure interval.
   */
  inline int measureIntervalStop () const {
    return (mstop_on ? mstop : cur_scan); }

  /**
   * \brief Resets measure interval.
   */
  inline void resetMeasure () { mstart_on = false; mstop_on = false;
                                mstart = 0; mstop = 0; }

  /**
   * \brief Checks if the measure interval start is completely set.
   */
  inline bool isSetMeasureStart () const { return mstart_on; }

  /**
   * \brief Sets measure interval start.
   */
  inline void setMeasureStart () { mstart_on = true;  mstart = cur_scan; }

  /**
   * \brief Checks if the measure interval stop is completely set.
   */
  inline bool isSetMeasureStop () const { return mstop_on; }

  /**
   * \brief Sets measure interval stop.
   */
  inline void setMeasureStop () { mstop_on = true;  mstop = cur_scan; }

  /**
   * \brief Checks whether the current scan lies within the measure interval.
   */
  bool isCurrentScanMeasured () const;

  /**
   * \brief Returns the decimal resolution used for floating point values.
   */
  inline int decimalResolution () const { return decimal_resolution; }

  /**
   * \brief Switches legend display modality.
   * @param inc Direction (1 for upper, -1 for lower).
   */
  void incDecimalResolution (int inc);


protected:

  /** Drawing parameter : view size increment. */
  static const int SIZE_INC;
  /** Drawing parameter : default cloud points size. */
  static const int DEFAULT_POINT_SIZE;
  /** Drawing parameter : default scan point resolution. */
  static const int DEFAULT_SCAN_RESOL;
  /** Drawing parameter : profile shift increment. */
  static const int PROF_SHIFT_INC;
  /** Drawing parameter : line move ratio increment. */
  static const float RATIO_INC;


  /** Width of analyzed straight strip. */
  int straight_strip_width;

  /** Profile view width. */
  int prof_width;
  /** Scan view width. */
  int scan_width;
  /** Analysis view height. */
  int view_height;
  /** Longitudinal profile view width. */
  int long_view_width;
  /** Longitudinal profile view height. */
  int long_view_height;
  /** Longitudinal profile view height. */
  int long_prof_height;
  /** Scan points zoom factor. */
  int scan_resol;

  /** Paints aligned scans if true, dynamic ones otherwise. */
  bool aligned;
  /** Displays initial detection if true, final detection otherwise. */
  bool initial_detection;
  /** Indicates if height reference is static (first profile). */
  bool static_height;

  /** Template display modality. */
  bool display_template;
  /** Reference template display modality. */
  bool display_reference;
  /** Estimated position display modality. */
  bool display_estimation;
  /** Estimated direction display modality. */
  bool display_direction;
  /** Next position display modality. */
  bool display_prediction;
  /** Legend display modality. */
  bool display_legend;
  /** Cloud points size. */
  int point_size;
  /** Controlable side profile shift. */
  int profile_shift;
  /** Measure computation and display modality. */
  bool measuring;
  /** Longitudinal profile thin resolution modality for straight strips. */
  bool thin_long_strip;
  /** Decimal resolution of displayed floating point values. */
  int decimal_resolution;

  /** Current scan index. */
  int cur_scan;
  /** Minimal scan index (right scan size). */
  int min_scan;
  /** Maximal scan index (left scan size). */
  int max_scan;

  /** Measure start status. */
  bool mstart_on;
  /** Measure start scan index. */
  int mstart;
  /** Measure stop status. */
  bool mstop_on;
  /** Measure start scan index. */
  int mstop;


  /**
   * \brief Returns a displayable text for floating point values.
   */
  std::string format (float val) const;

};
#endif
