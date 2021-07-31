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

#ifndef PLATEAU_MODEL_H
#define PLATEAU_MODEL_H


/** 
 * @class PlateauModel plateaumodel.h
 * \brief Model of plateau features for detection control.
 */
class PlateauModel
{
public:

  /** Minimal value for minimal length tolerance. */
  static const float MIN_MIN_LENGTH_TOLERANCE;
  /** Maximal value for minimal length tolerance. */
  static const float MAX_MIN_LENGTH_TOLERANCE;
  /** Minimal value for maximal length tolerance. */
  static const float MIN_MAX_LENGTH_TOLERANCE;
  /** Maximal value for maximal length tolerance. */
  static const float MAX_MAX_LENGTH_TOLERANCE;

  /** Minimal thickness tolerance value. */
  static const float MIN_THICKNESS_TOLERANCE;
  /** Maximal thickness tolerance value. */
  static const float MAX_THICKNESS_TOLERANCE;
  /** Minimal slope tolerance value. */
  static const float MIN_SLOPE_TOLERANCE;
  /** Maximal slope tolerance value. */
  static const float MAX_SLOPE_TOLERANCE;
  /** Minimal value for maximal side shift between successive plateaux bound. */
  static const float MIN_SIDE_SHIFT_TOLERANCE;
  /** Maximal value for maximal side shift between successive plateaux bound. */
  static const float MAX_SIDE_SHIFT_TOLERANCE;


  /**
   * \brief Creates a new plateau model.
   */
  PlateauModel ();

  /**
   * \brief Deletes the plateau model.
   */
  ~PlateauModel ();

  /**
   * \brief Returns the minimal number of points to detect a plateau.
   */
  inline int minCountOfPoints () const { return PLATEAU_MIN_PTS; }

  /**
   * \brief Returns the maximal thickness of a plateau.
   */
  inline float thicknessTolerance () const { return thickness_tolerance; }

  /**
   * \brief Increments the maximal thickness of a plateau.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incThicknessTolerance (int dir);

  /**
   * \brief Sets the maximal thickness of a plateau.
   * @param val New thickness value.
   */
  void setThicknessTolerance (float val);

  /**
   * \brief Returns the thickness variation if let flexible.
   */
  inline float heightFlexibility () const { return HEIGHT_FLEXIBILITY; }

  /**
   * \brief Returns the maximal height difference between successive plateaux.
   */
  inline float slopeTolerance () const { return slope_tolerance; }

  /**
   * \brief Increments maximal height difference between successive plateaux.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incSlopeTolerance (int dir);

  /**
   * \brief Sets maximal height difference between successive plateaux.
   * @param val New slope value.
   */
  void setSlopeTolerance (float val);

  /**
   * \brief Returns the critical length to detect a plateau.
   */
  inline float criticalLength () const { return PLATEAU_CRITICAL_LENGTH; }

  /**
   * \brief Returns the minimal length of a plateau.
   */
  inline float minLength () const { return min_length; }

  /**
   * \brief Increments minimal length of a plateau.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incMinLength (int dir);

  /**
   * \brief Sets minimal length of a plateau.
   * @param val New length value.
   */
  void setMinLength (float val);

  /**
   * \brief Returns the maximal length of a plateau.
   */
  inline float maxLength () const { return max_length; }

  /**
   * \brief Increments maximal length of a plateau.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incMaxLength (int dir);

  /**
   * \brief Sets maximal length of a plateau.
   * @param val New length value.
   */
  void setMaxLength (float val);

  /**
   * \brief Returns a start length in case of a lacking plateau bound.
   */
  inline float startLength () const { return PLATEAU_START_LENGTH; }

  /**
   * \brief Returns the required accuracy for bounds detection.
   */
  inline float boundAccuracy () const { return PLATEAU_BOUND_ACCURACY; }

  /**
   * \brief Returns the maximal side shift between successive plateaux bounds.
   */
  inline float sideShiftTolerance () const { return side_shift_tolerance; }

  /**
   * \brief Increments maximal side shift between successive plateaux bounds.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incSideShiftTolerance (int dir);

  /**
   * \brief Sets maximal side shift between successive plateaux bounds.
   * @param val New side shift value.
   */
  void setSideShiftTolerance (float val);

  /**
   * \brief Returns the minimal count of optimal height points used.
   */
  inline float optHeightMinUse () const { return opt_height_min_use; }

  /**
   * \brief Returns the maximal width difference between successive plateaux.
   */
  inline float widthMoveTolerance () const { return width_move_tolerance; }

  /**
   * \brief Increments maximal width difference between successive plateaux.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incWidthMoveTolerance (int dir);

  /**
   * \brief Returns the maximal blurred segment tilt (%).
   */
  inline int bsMaxTilt () const { return bs_max_tilt; }

  /**
   * \brief Increments maximal blurred segment tilt (%).
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incBSmaxTilt (int dir);

  /**
   * \brief Sets maximal blurred segment tilt (%).
   * @param val New value for the cotangent of the maximal road tilt.
   */
  void setBSmaxTilt (int val);

  /**
   * \brief Returns the maximal number of successive outliers in a plateau.
   */
  inline int maxInterruption () const { return PLATEAU_MAX_INTERRUPTION; }

  /**
   * \brief Returns the blurred segment thickness margin after pinching.
   */
  inline int bsPinchMargin () const { return BS_PINCH_MARGIN; }

  /**
   * \brief Returns the minimal size of ending succesive plateaux.
   */
  inline int tailMinSize () const { return tail_min_size; }

  /**
   * \brief Increments the minimal size of ending succesive plateaux.
   * @param inc Increment value.
   */
  void incTailMinSize (int inc);

  /**
   * \brief Sets the minimal size of ending succesive plateaux.
   * @param val New size value.
   */
  void setTailMinSize (int val);

  /**
   * \brief Returns the search distance for lost plateau.
   */
  inline float plateauSearchDistance () const {
    return DEFAULT_PLATEAU_SEARCH_DISTANCE; }

  /**
   * \brief Indicates whether direction is used to predict next plateau.
   */
  inline bool isDeviationPredictionOn () const {
    return (deviation_prediction_on); }

  /**
   * \brief Switches deviation addition to the template.
   */
  void switchDeviationPrediction ();

  /**
   * \brief Indicates whether slope is used to predict next plateau position.
   */
  inline bool isSlopePredictionOn () const { return (slope_prediction_on); }

  /**
   * \brief Switches slope addition to the template.
   */
  void switchSlopePrediction (); 


private :

  /** Minimal number of points to detect a plateau. */
  static const int PLATEAU_MIN_PTS;
  /** Default maximal thickness of a plateau. */
  static const float DEFAULT_THICKNESS_TOLERANCE;
  /** Thickness variation when let flexible. */
  static const float HEIGHT_FLEXIBILITY;
  /** Default maximal height difference between successive plateaux. */
  static const float DEFAULT_SLOPE_TOLERANCE;
  /** Height tolerance increment. */
  static const float HEIGHT_INCREMENT;
  /** Critical length to detect a plateau. */
  static const float PLATEAU_CRITICAL_LENGTH;
  /** Default minimal length of a plateau. */
  static const float DEFAULT_MIN_LENGTH;
  /** Default maximal length of a plateau. */
  static const float DEFAULT_MAX_LENGTH;
  /** Awaited length of a plateau in case of a bound undetected. */
  static const float PLATEAU_START_LENGTH;
  /** Length tolerance increment. */
  static const float LENGTH_INCREMENT;
  /** Default maximal side shift between successive plateaux bound. */
  static const float DEFAULT_SIDE_SHIFT_TOLERANCE;
  /** Default maximal width difference between successive plateaux. */
  static const float DEFAULT_PLATEAU_WIDTH_MOVE_TOLERANCE;
  /** Default accuracy threshold for bounds detection. */
  static const float PLATEAU_BOUND_ACCURACY;
  /** Position tolerance increment. */
  static const float POS_INCREMENT;
  /** Minimal position tolerance value. */
  static const float MIN_POS_TOLERANCE;
  /** Default minimal count of optimal height points used. */
  static const float OPT_HEIGHT_MIN_USE;
  /** Maximal number of successive interruptions (height artefacts). */
  static const int PLATEAU_MAX_INTERRUPTION;
  /** Pinch margin for blurred segment extension. */
  static const int BS_PINCH_MARGIN;
  /** Default value for maximal blurred segment tilt. */
  static const int DEFAULT_BS_MAX_TILT;
  /** Default value for lost plateau search distance. */
  static const float DEFAULT_PLATEAU_SEARCH_DISTANCE;
  /** Default value for the minimal size of ending successive plateaux. */
  static const int DEFAULT_TAIL_MIN_SIZE;

  /** Maximal thickness of a plateau. */
  float thickness_tolerance;
  /** Maximal height difference between successive plateaux. */
  float slope_tolerance;
  /** Minimal length of a plateau. */
  float min_length;
  /** Maximal length of a plateau. */
  float max_length;
  /** Maximal side shift between successive plateaux bounds. */
  float side_shift_tolerance;
  /** Minimal width difference between successive plateaux. */
  float width_move_tolerance;
  /** Minimal count of optimal height points used. */
  float opt_height_min_use;
  /** Maximal blurred segment tilt (%). */
  int bs_max_tilt;
  /** Minimal size of ending successive plateaux. */
  int tail_min_size;

  /** Indicates if the deviation is used to predict next plateau position. */
  bool deviation_prediction_on;
  /** Indicates if the slope is used to predict next plateau position. */
  bool slope_prediction_on;

};
#endif
