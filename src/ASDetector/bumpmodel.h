/*  Copyright 2021 Philippe Even and Phuc Ngo,
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

#ifndef BUMP_MODEL_H
#define BUMP_MODEL_H


/** 
 * @class BumpModel bumpmodel.h
 * \brief Model of bump features for detection control.
 */
class BumpModel
{
public:

  /** Minimal value for minimal bump width. */
  static const float MIN_MIN_WIDTH;
  /** Maximal value for minimal bump width. */
  static const float MAX_MIN_WIDTH;
  /** Minimal value for minimal bump height. */
  static const float MIN_MIN_HEIGHT;
  /** Maximal value for minimal bump height. */
  static const float MAX_MIN_HEIGHT;
  /** Maximal value for minimal height ratio. */
  static const float MAX_HEIGHT_RATIO;

  /** Minimal shift tolerance. */
  static const float MIN_SHIFT;
  /** Maximal shift tolerance. */
  static const float MAX_SHIFT;
  /** Minimal relative shift tolerance. */
  static const float MIN_REL_SHIFT;
  /** Maximal relative shift tolerance. */
  static const float MAX_REL_SHIFT;

  /** Minimal value for the minimal pinch thickness for trend extension (mm). */
  static const int MIN_TREND_MIN_PINCH;
  /** Maximal value for the minimal pinch thickness for trend extension (mm). */
  static const int MAX_TREND_MIN_PINCH;


  /**
   * \brief Creates a new bump model.
   */
  BumpModel ();

  /**
   * \brief Deletes the bump model.
   */
  ~BumpModel ();

  /**
   * \brief Returns the bump orientation: ridge if true, hollow if false.
   */
  inline bool isOver () const { return over; }

  /**
   * \brief Inverts the bump orientation between ridge or hollow.
   */
  inline void switchOver () { over = ! over; }

  /**
   * \brief Sets the bump orientation.
   * @param status Orientation status : to ridge if true, to hollow otherwise.
   */
  inline void setOver (bool status) { over = status; }

  /**
   * \brief Returns the minimal number of points to detect a bump.
   */
  inline int minCountOfPoints () const { return MIN_PTS; }

  /**
   * \brief Returns the maximal relative length of holes.
   * This hole length is the relative distance (wrt P1P2) between
   *   adjacent points in an input stroke.
   */
  inline float holeMaxRelativeLength () const {
    return HOLE_MAX_RELATIVE_LENGTH; }

  /**
   * \brief Checks if the given position shift is consistent.
   * @param val Position shift value.
   */
  inline bool outOfPositionTolerance (float val) const {
    return (val > pos_tolerance || - val > pos_tolerance); }

  /**
   * \brief Returns the maximal position shift between successive bumps.
   */
  inline float positionShiftTolerance () const { return (pos_tolerance); }

  /**
   * \brief Increments maximal position shift between successive bumps.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incPositionShiftTolerance (int dir);

  /**
   * \brief Sets maximal position shift between successive bumps.
   * @param val New shift tolerance value.
   */
  void setPositionShiftTolerance (float val);

  /**
   * \brief Checks if the given altitude shift is consistent.
   * @param val Altitude shift value.
   */
  inline bool outOfAltitudeTolerance (float val) const {
    return (val > alti_tolerance || - val > alti_tolerance); }

  /**
   * \brief Returns the maximal altitude difference between successive bumps.
   */
  inline float altitudeShiftTolerance () const { return (alti_tolerance); }

  /**
   * \brief Increments maximal altitude difference between successive bumps.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incAltitudeShiftTolerance (int dir);

  /**
   * \brief Sets maximal altitude difference between successive bumps.
   * @param val New shift tolerance value.
   */
  void setAltitudeShiftTolerance (float val);

  /**
   * \brief Checks if the given width shift is consistent.
   * @param val Width shift value.
   */
  inline bool outOfWidthTolerance (float val) const {
    return (val > width_tolerance || - val > width_tolerance); }

  /**
   * \brief Returns the maximal width shift between successive bumps.
   */
  inline float widthShiftTolerance () const { return width_tolerance; }

  /**
   * \brief Increments maximal width shift between successive bumps.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incWidthShiftTolerance (int dir);

  /**
   * \brief Sets maximal width shift between successive bumps.
   * @param val New shift tolerance value.
   */
  void setWidthShiftTolerance (float val);

  /**
   * \brief Checks if the given height shift is consistent.
   * @param val Height shift value.
   */
  inline bool outOfHeightTolerance (float val) const {
    return (val > height_tolerance || - val > height_tolerance); }

  /**
   * \brief Returns the maximal height shift between successive bumps.
   */
  inline float heightShiftTolerance () const { return height_tolerance; }

  /**
   * \brief Increments maximal height shift between successive bumps.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incHeightShiftTolerance (int dir);

  /**
   * \brief Sets maximal height shift between successive bumps.
   * @param val New shift tolerance value.
   */
  void setHeightShiftTolerance (float val);

  /**
   * \brief Checks if the given position shift is consistent wrt width.
   * @param val Position shift value.
   * @param ref Reference width.
   */
  inline bool outOfPositionRelTolerance (float val, float ref) const {
    return (val > ref * pos_rel_tolerance
            || - val > ref * pos_rel_tolerance); }

  /**
   * \brief Returns maximal position relative shift between successive bumps.
   */
  inline float positionRelShiftTolerance () const {
    return (pos_rel_tolerance); }

  /**
   * \brief Increments maximal position relative shift between successive bumps.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incPositionRelShiftTolerance (int dir);

  /**
   * \brief Sets maximal position relative shift between successive bumps.
   * @param val New shift tolerance value.
   */
  void setPositionRelShiftTolerance (float val);

  /**
   * \brief Checks if the given altitude shift is consistent wrt height.
   * @param val Altitude shift value.
   * @param ref Reference height.
   */
  inline bool outOfAltitudeRelTolerance (float val, float ref) const {
    return (val > ref * alti_rel_tolerance
            || - val > ref * alti_rel_tolerance); }

  /**
   * \brief Returns maximal altitude relative shift between successive bumps.
   */
  inline float altitudeRelShiftTolerance () const {
    return (alti_rel_tolerance); }

  /**
   * \brief Increments maximal altitude relative shift between successive bumps.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incAltitudeRelShiftTolerance (int dir);

  /**
   * \brief Sets maximal altitude relative shift between successive bumps.
   * @param val New shift tolerance value.
   */
  void setAltitudeRelShiftTolerance (float val);

  /**
   * \brief Checks if the given width shift is consistent wrt reference width.
   * @param val Width shift value.
   * @param ref Reference width.
   */
  inline bool outOfWidthRelTolerance (float val, float ref) const {
    return (val > ref * width_rel_tolerance
            || - val > ref * width_rel_tolerance); }

  /**
   * \brief Returns maximal width relative shift between successive bumps.
   */
  inline float widthRelShiftTolerance () const { return width_rel_tolerance; }

  /**
   * \brief Increments maximal width relative shift between successive bumps.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incWidthRelShiftTolerance (int dir);

  /**
   * \brief Sets maximal width relative shift between successive bumps.
   * @param val New shift tolerance value.
   */
  void setWidthRelShiftTolerance (float val);

  /**
   * \brief Checks if the given height shift is consistent wrt reference height.
   * @param val Height shift value.
   * @param ref Reference width.
   */
  inline bool outOfHeightRelTolerance (float val, float ref) const {
    return (val > ref * height_rel_tolerance
            || - val > ref * height_rel_tolerance); }

  /**
   * \brief Returns maximal height relative shift between successive bumps.
   */
  inline float heightRelShiftTolerance () const { return height_rel_tolerance; }

  /**
   * \brief Increments maximal height relative shift between successive bumps.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incHeightRelShiftTolerance (int dir);

  /**
   * \brief Sets maximal height relative shift between successive bumps.
   * @param val New shift tolerance value.
   */
  void setHeightRelShiftTolerance (float val);

  /**
   * \brief Returns the minimal width of a bump.
   */
  inline float minWidth () const { return min_width; }

  /**
   * \brief Increments minimal width of a bump.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incMinWidth (int dir);

  /**
   * \brief Sets minimal width of a bump.
   * @param val New width value.
   */
  void setMinWidth (float val);

  /**
   * \brief Returns the minimal height of a bump.
   */
  inline float minHeight () const { return min_height; }

  /**
   * \brief Increments minimal height of a bump.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incMinHeight (int dir);

  /**
   * \brief Sets minimal height of a bump.
   * @param val New length value.
   */
  void setMinHeight (float val);

  /**
   * \brief Indicates whether direction is used to predict next plateau.
   */
  inline bool isDeviationPredictionOn () const {
    return (deviation_prediction_on); }

  /**
   * \brief Switches the deviation addition to the template.
   */
  void switchDeviationPrediction ();

  /**
   * \brief Indicates whether slope is used to predict next plateau position.
   */
  inline bool isSlopePredictionOn () const { return (slope_prediction_on); }

  /**
   * \brief Switches the slope addition to the template.
   */
  void switchSlopePrediction ();

  /**
   * \brief Returns the external end distance when undetected.
   */
  inline float undetectedBoundDistance () const {
    return UNDETECTED_BOUND_DISTANCE; }

  /**
   * \brief Returns if bump reference is mass center (surface otherwise).
   */
  inline bool massReferenced () const { return mass_ref; }

  /**
   * \brief Switches bump reference between mass or surface center.
   */
  inline void switchCenterReference () { mass_ref = ! mass_ref; }

  /**
   * \brief Returns bump position control mode.
   *   Set to 0 if none, 1 if absolute, 2 if relative.
   */
  inline int positionControl () const { return position_control; }

  /**
   * \brief Sets bump position control mode.
   * @param status 0 if none, 1 if absolute, 2 if relative.
   */
  inline void setPositionControl (int status) { position_control = status; }

  /**
   * \brief Returns bump altitude control mode.
   *   Set to 0 if none, 1 if absolute, 2 if relative.
   */
  inline int altitudeControl () const { return altitude_control; }

  /**
   * \brief Sets bump altitude control mode.
   * @param status 0 if none, 1 if absolute, 2 if relative.
   */
  inline void setAltitudeControl (int status) { altitude_control = status; }

  /**
   * \brief Returns bump width control mode.
   *   Set to 0 if none, 1 if absolute, 2 if relative.
   */
  inline int widthControl () const { return width_control; }

  /**
   * \brief Sets bump width control mode.
   * @param status 0 if none, 1 if absolute, 2 if relative.
   */
  inline void setWidthControl (int status) { width_control = status; }

  /**
   * \brief Returns bump height control mode.
   *   Set to 0 if none, 1 if absolute, 2 if relative.
   */
  inline int heightControl () const { return height_control; }

  /**
   * \brief Sets bump height control mode.
   * @param status 0 if none, 1 if absolute, 2 if relative.
   */
  inline void setHeightControl (int status) { height_control = status; }

  /**
   * \brief Returns the status of surrounding trend detection modality.
   */
  inline bool isDetectingTrend () const { return with_trend; }

  /**
   * \brief Switches surrounding trend detection modality.
   */
  inline void switchDetectingTrend () { with_trend = ! with_trend; }

  /**
   * \brief Returns the count of considered trends for measure estimation.
   */
  inline int trendPersistence () const { return TREND_PERSISTENCE; }

  /**
   * \brief Returns the minimal pinch thickness for trend extension (in mm).
   */
  inline int trendMinPinch () const { return trend_min_pinch; }

  /**
   * \brief Increments the minimal pinch thickness for trend extension (in mm).
   * @param dir Increases (+1) or (decreases) (-1) the value.
   */
  void incTrendMinPinch (int dir);

  /**
   * \brief Sets the minimal pinch thickness for trend extension (in mm).
   * @param val New thickness value.
   */
  void setTrendMinPinch (int val);

  /**
   * \brief Returns the status of measures estimation modality.
   */
  inline bool isMeasured () const { return measures_req; }

  /**
   * \brief Switches measures estimation modality.
   */
  inline void switchMeasured () { measures_req = ! measures_req; }


private :

  /** Minimal number of points to detect a bump. */
  static const int MIN_PTS;
  /** Maximal relative length between adjacent points in input stroke. */
  static const float HOLE_MAX_RELATIVE_LENGTH;
  /** Default minimal width of a bump. */
  static const float DEFAULT_MIN_WIDTH;
  /** Default minimal height of a bump. */
  static const float DEFAULT_MIN_HEIGHT;

  /** Default maximal position shift between successive bumps. */
  static const float DEFAULT_POS_SHIFT_TOLERANCE;
  /** Default maximal altitude shift between successive bumps. */
  static const float DEFAULT_ALTI_SHIFT_TOLERANCE;
  /** Default maximal width shift between successive bumps. */
  static const float DEFAULT_WIDTH_SHIFT_TOLERANCE;
  /** Default maximal height shift between successive bumps. */
  static const float DEFAULT_HEIGHT_SHIFT_TOLERANCE;
  /** Default maximal position relative shift between successive bumps. */
  static const float DEFAULT_POS_REL_SHIFT_TOLERANCE;
  /** Default maximal altitude relative shift between successive bumps. */
  static const float DEFAULT_ALTI_REL_SHIFT_TOLERANCE;
  /** Default maximal width relative shift between successive bumps. */
  static const float DEFAULT_WIDTH_REL_SHIFT_TOLERANCE;
  /** Default maximal height relative shift between successive bumps. */
  static const float DEFAULT_HEIGHT_REL_SHIFT_TOLERANCE;

  /** Length tolerance increment. */
  static const float LENGTH_INCREMENT;
  /** Height tolerance increment (for altitudes too). */
  static const float HEIGHT_INCREMENT;
  /** Width tolerance increment. */
  static const float WIDTH_INCREMENT;
  /** Relative value increment. */
  static const float REL_INCREMENT;
  /** Minimal height ratio increment. */
  static const float HEIGHT_RATIO_INCREMENT;
  /** Position tolerance increment. */
  static const float POS_INCREMENT;
  /** External end distance when internal end on interval end. */
  static const float UNDETECTED_BOUND_DISTANCE;

  /** Default minimal pinch thickness for trend extension (millimeters). */
  static const int DEFAULT_TREND_MIN_PINCH;
  /** Trend thickness measure increment (millimeters). */
  static const int THICKNESS_INCREMENT;
  /** Count of considered trends for measure estimation. */
  static const int TREND_PERSISTENCE;


  /** Bump orientation (true for ridge, false for hollow). */
  bool over;
  /** Bump center reference: mass center if true, surface center otherwise. */
  bool mass_ref;
  /** Bump position control mode: 0 = none, 1 = absolute, 2 = relative. */
  int position_control;
  /** Bump altitude control mode: 0 = none, 1 = absolute, 2 = relative. */
  int altitude_control;
  /** Bump width control mode: 0 = none, 1 = absolute, 2 = relative. */
  int width_control;
  /** Bump height control mode: 0 = none, 1 = absolute, 2 = relative. */
  int height_control;
  /** Surrounding trend detection modality. */
  bool with_trend;

  /** Minimal width of a bump. */
  float min_width;
  /** Minimal height of a bump. */
  float min_height;

  /** Maximal position shift between successive bumps. */
  float pos_tolerance;
  /** Maximal altitude shift between successive bumps. */
  float alti_tolerance;
  /** Maximal width shift between successive bumps. */
  float width_tolerance;
  /** Maximal height shift between successive bumps. */
  float height_tolerance;
  /** Maximal position relative shift between successive bumps. */
  float pos_rel_tolerance;
  /** Maximal altitude relative shift between successive bumps. */
  float alti_rel_tolerance;
  /** Maximal width relative shift between successive bumps. */
  float width_rel_tolerance;
  /** Maximal height relative shift between successive bumps. */
  float height_rel_tolerance;

  /** Minimal pinch thickness for trend extension. */
  int trend_min_pinch;

  /** Indicates if the deviation is used to predict next plateau position. */
  bool deviation_prediction_on;
  /** Indicates if the slope is used to predict next plateau position. */
  bool slope_prediction_on;

  /** Indicates if ridge geometrical properties are required. */
  bool measures_req;

};
#endif
