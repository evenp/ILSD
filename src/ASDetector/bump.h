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

#ifndef BUMP_H
#define BUMP_H

#include "pt2f.h"
#include "bumpmodel.h"
#include "digitalstraightsegment.h"
//#include <iostream>


/** 
 * @class Bump bump.h
 * \brief Cross section of a ridge or hollow structure.
 */
class Bump
{
public:

  /** Detection status : not processed. */
  static const int RES_NONE;
  /** Detection status : success. */
  static const int RES_OK;
  /** Detection status : failed, not enough points in input scan. */
  static const int RES_NOT_ENOUGH_INPUT_PTS;
  /** Detection status : failed, presence of hole in input points. */
  static const int RES_HOLE_IN_INPUT_PTS;
  /** Detection status : failed, height lower than minimal awaited value. */
  static const int RES_TOO_LOW;
  /** Detection status : failed, width lower than critical width. */
  static const int RES_TOO_NARROW;
  /** Detection status : failed, no point over the baseline. */
  static const int RES_NO_BUMP_LINE;
  /** Detection status : failed, linear profile. */
  static const int RES_LINEAR;
  /** Detection status : failed, angular profile (break in slope). */
  static const int RES_ANGULAR;

  /** Tracking default: no default. */
  static const int DEF_NONE;
  /** Tracking default: too large shift of center position. */
  static const int DEF_POSITION;
  /** Tracking default: too large shift of center altitude. */
  static const int DEF_ALTITUDE;
  /** Tracking default: too large shift of height. */
  static const int DEF_HEIGHT;
  /** Tracking default: too large shift of widh. */
  static const int DEF_WIDTH;

  /** Measure line maximal translation ratio. */
  static const float MAX_LINE_TRANSLATION_RATIO;
  /** Measure line maximal rotation ratio. */
  static const float MAX_LINE_ROTATION_RATIO;

  
  /**
   * \brief Creates a new bump.
   * @param bmod Bump detection features.
   */
  Bump (BumpModel *bmod);

  /**
   * \brief Deletes the bump.
   */
  ~Bump ();

  /**
   * \brief Detects the bump in a whole scan.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param l12 Input stroke length.
   */
  bool detect (const std::vector<Pt2f> &ptsh, float l12);

  /**
   * \brief Detects the bump specific shape in scan central part.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  bool getBump (const std::vector<Pt2f> &ptsh);

  /**
   * \brief Detects a consistent bump to given template.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param l12 Input stroke length.
   * @param ref Reference bump.
   * @param refdist Reference distance (distance to reference bump).
   */
  bool track (const std::vector<Pt2f> &ptsh, float l12,
              Bump *ref, int refdist);

  /**
   * \brief Provides bump detection status.
   */
  inline int getStatus () const { return status; }

  /**
   * \brief Provides bump tracking default.
   */
  inline int getDefault () const { return def; }

  /**
   * \brief Returns whether the bump is found.
   */
  inline bool isFound () const { return (status == RES_OK); }

  /**
   * \brief Returns whether the bump is accepted.
   */
  inline bool isAccepted () const { return accepted; }

  /**
   * \brief Sets bump status to ok.
   */
  inline void accept () { accepted = true; }

  /**
   * \brief Returns whether the bump is inserted in the ridge.
   * @param smoothed Insertion condition : simply accepted (true) or status ok.
   */
  inline bool inserted (bool smoothed) const {
    return (smoothed ? accepted : status == RES_OK); }

  /**
   * \brief Updates area measures.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  void updateMeasure (const std::vector<Pt2f> *ptsh);

  /**
   * \brief Returns the estimated surface center position.
   */
  inline Pt2f estimatedSurfCenter () const { return scenter_est; }

  /**
   * \brief Returns the estimated mass center position.
   */
  inline Pt2f estimatedMassCenter () const { return mcenter_est; }

  /**
   * \brief Returns the estimated center position.
   */
  inline Pt2f estimatedCenter () const {
    return (bmod->massReferenced () ? mcenter_est : scenter_est); }

  /**
   * \brief Returns the estimated start position.
   */
  inline float estimatedStart () const { return s_est; }

  /**
   * \brief Returns the estimated end position.
   */
  inline float estimatedEnd () const { return e_est; }

  /**
   * \brief Returns the baseline start point index.
   */
  inline int startIndex () const { return s_num; }

  /**
   * \brief Returns the baseline end point index.
   */
  inline int endIndex () const { return e_num; }

  /**
   * \brief Returns the estimated summit position.
   */
  inline Pt2f estimatedSummit () const { return summit_est; }

  /**
   * \brief Returns index of summit point in profile.
   */
  inline int summitIndex () const { return a_num; }

  /**
   * \brief Returns the estimated height at summit.
   */
  inline float estimatedHeight () const { return (h_est); }

  /**
   * \brief Returns the estimated width.
   */
  inline float estimatedWidth () const { return (w_est); }

  /**
   * \brief Returns the position shift.
   */
  inline float positionShift () const {
    return (ref != NULL ? (mcenter_est.x () - ref->mcenter_est.x ()) : 0.0f); }

  /**
   * \brief Returns the altitude shift.
   */
  inline float altitudeShift () const {
    return (ref != NULL ? (mcenter_est.y () - ref->mcenter_est.y ()) : 0.0f); }

  /**
   * \brief Returns the width shift.
   */
  inline float widthShift () const {
    return (ref != NULL ? (w_est - ref->w_est) : 0.0f); }

  /**
   * \brief Returns the height shift.
   */
  inline float heightShift () const {
    return (ref != NULL ? (h_est - ref->h_est) : 0.0f); }

  /**
   * \brief Returns the estimated bump start position.
   */
  inline Pt2f start () const { return start_est; }

  /**
   * \brief Returns the estimated bump end position.
   */
  inline Pt2f end () const { return end_est; }

  /**
   * \brief Returns the maximal start position found.
   */
  inline float internalStart () const { return s_int; }

  /**
   * \brief Returns the minimal end position found.
   */
  inline float internalEnd () const { return e_int; }

  /**
   * \brief Returns the minimal start position found.
   */
  inline float externalStart () const { return s_ext; }

  /**
   * \brief Returns the maximal end position found.
   */
  inline float externalEnd () const { return e_ext; }

  /**
   * \brief Returns the reference center.
   */
  inline Pt2f referenceCenter () const {
    return (ref != NULL ? ref->estimatedCenter () : Pt2f (-1.0f, -1.0f)); }

  /**
   * \brief Returns the reference mass center.
   */
  inline Pt2f referenceMassCenter () const {
    return (ref != NULL ? ref->estimatedMassCenter () : Pt2f (-1.0f, -1.0f)); }

  /**
   * \brief Returns the reference height.
   */
  inline float referenceHeight () const {
    return (ref != NULL ? ref->h_est : -1.0f); }

  /**
   * \brief Returns the reference width.
   */
  inline float referenceWidth () const {
    return (ref != NULL ? ref->w_est : -1.0f); }

  /**
   * \brief Returns the baseline start point.
   */
  inline Pt2f lineStart () const { return line_start; }

  /**
   * \brief Returns the baseline end point.
   */
  inline Pt2f lineEnd () const { return line_end; }

  /**
   * \brief Sets bump estimated deviation.
   */
  inline void setDeviation (float val) { dev_est = val; }

  /**
   * \brief Provides bump estimated deviation.
   */
  inline float estimatedDeviation () const { return dev_est; }

  /**
   * \brief Sets bump estimated slope.
   */
  inline void setSlope (float val) { slope_est = val; }

  /**
   * \brief Provides bump estimated slope.
   */
  inline float estimatedSlope () const { return slope_est; }

  /**
   * \brief Returns whether given position lies within acceptable bounds.
   * @param pos Given position.
   */
  inline bool contains (float pos) const {
    return (pos > (scenter_est.x () + s_est) / 2
            && pos < (scenter_est.x () + e_est) / 2); }

  /**
   * \brief Returns the start trend.
   */
  inline DigitalStraightSegment *startTrend () const {return start_trend; }

  /**
   * \brief Returns the end trend.
   */
  inline DigitalStraightSegment *endTrend () const {return end_trend; }

  /**
   * \brief Inquires if a start trend is found.
   */
  inline bool hasStartTrend () const {return start_trend != NULL; }

  /**
   * \brief Returns start trend vertical thickness.
   */
  float startTrendThickness () const;

  /**
   * \brief Returns start trends vertical thickness.
   * @param Count of considered trends.
   */
  float startTrendsMaxThickness (int nb = 1) const;

  /**
   * \brief Returns start trend start position.
   */
  Pt2f startTrendStart () const;

  /**
   * \brief Returns start trend end position.
   */
  Pt2f startTrendEnd () const;

  /**
   * \brief Returns the last start trend detected since this bump included.
   * @param nb Count of inquired bumps.
   */
  DigitalStraightSegment *lastStartTrend (int nb) const;

  /**
   * \brief Inquires if an end trend is found.
   */
  inline bool hasEndTrend () const {return end_trend != NULL; }

  /**
   * \brief Returns end trend vertical thickness.
   */
  float endTrendThickness () const;

  /**
   * \brief Returns end trends vertical thickness.
   * @param Count of considered trends.
   */
  float endTrendsMaxThickness (int nb = 1) const;

  /**
   * \brief Returns end trend start position.
   */
  Pt2f endTrendStart () const;

  /**
   * \brief Returns end trend end position.
   */
  Pt2f endTrendEnd () const;

  /**
   * \brief Returns the last end trend detected since this bump included.
   * @param nb Count of inquired bumps.
   */
  DigitalStraightSegment *lastEndTrend (int nb) const;

  /**
   * \brief Returns the measure line start point.
   */
  inline bool hasMeasureLine () const { return (mline_p); }

  /**
   * \brief Returns the measure line start point.
   */
  inline Pt2f measureLineStart () const {
    return (mline_p ? mline_start : start_est); }

  /**
   * \brief Returns the measure line end point.
   */
  inline Pt2f measureLineEnd () const {
    return (mline_p ? mline_end : end_est); }

  /**
   * \brief Returns the measure line translation ratio.
   */
  inline float getMeasureLineTranslationRatio () const { return mline_trsl; }

  /**
   * \brief Increments the measure line translation ratio.
   * @param inc Line move increment.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  void incMeasureLineTranslationRatio (int inc,
                                       const std::vector<Pt2f> *ptsh = NULL);

  /**
   * \brief Sets the measure line translation ratio.
   * @param val New translation ratio value.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  void setMeasureLineTranslationRatio (float val,
                                       const std::vector<Pt2f> *ptsh = NULL);

  /**
   * \brief Returns the measure line rotation ratio.
   */
  inline float getMeasureLineRotationRatio () const { return mline_rot; }

  /**
   * \brief Increments the measure line rotation ratio.
   * @param inc Line move increment.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  void incMeasureLineRotationRatio (int inc,
                                    const std::vector<Pt2f> *ptsh = NULL);

  /**
   * \brief Sets the measure line rotation ratio.
   * @param val New rotation ratio value.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  void setMeasureLineRotationRatio (float val,
                                    const std::vector<Pt2f> *ptsh = NULL);

  /**
   * \brief Sets the measure line end points and main index.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  void setMeasureLine (const std::vector<Pt2f> *ptsh);

  /**
   * \brief Returns estimated bump area.
   */
  float estimatedArea () const { return area_est; }

  /**
   * \brief Returns estimated bump area upper bound.
   */
  float estimatedAreaUpperBound () const { return area_up; }

  /**
   * \brief Returns estimated bump area lower bound.
   */
  float estimatedAreaLowerBound () const { return area_low; }

  /**
   * \brief Returns bump height at given height ratio.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  float estimatedWidth (std::vector<Pt2f> *ptsh, float hrat);

  void trace () const;

  void traceExt (const std::vector<Pt2f> &ptsh) const;



private :

  /** Relative assigned thickness wrt stroke length. */
  static const float REL_ASSIGNED_THICKNESS;
  /** Relative pinch distance wrt stroke length. */
  static const float REL_PINCH_LENGTH;
  /** Pinch amount of required points. */
  static const int PINCH_COUNT;
  /** Relative pinch margin wrt measured thickness. */
  static const float REL_PINCH_MARGIN;
  /** Pinch minimal margin value (in mm). */
  static const int PINCH_THRESHOLD;
  /** Minimal amount of points to validate a trend. */
  static const int MIN_TREND_SIZE;
  /** Maximal count of trend interruption. */
  static const int MAX_TREND_INTERRUPT;
  /** Measure line move ratio increment. */
  static const float RATIO_INC;


  /** Detection result. */
  int status;
  /** Tracking default. */
  int def;
  /** Acceptation status. */
  bool accepted;
  /** Ridge orientation status. */
  bool over;

  /** Reference bump. */
  Bump *ref;

  /** Estimated mass center position. */
  Pt2f mcenter_est;
  /** Estimated center position. */
  Pt2f scenter_est;
  /** Estimated start position. */
  Pt2f start_est;
  /** Estimated end position. */
  Pt2f end_est;
  /** Estimated summit position. */
  Pt2f summit_est;
  /** Estimated height at summit (in meter). */
  float h_est;
  /** Estimated width. */
  float w_est;

  /** Index of bump summit impact. */
  int a_num;
  /** Index of bump start impact (or internal start if no trend). */
  int s_num;
  /** Index of bump end impact (or internal end if no trend). */
  int e_num;

  /** Measured max start position (in meter). */
  float s_int;  // UTILE ?
  /** Measured min end position (in meter). */
  float e_int;  // UTILE ?
  /** Measured min start position (in meter). */
  float s_ext;  // UTILE ?
  /** Measured max end position (in meter). */
  float e_ext;  // UTILE ?
  /** Estimated start position (in meter). */
  float s_est;  // UTILE ?
  /** Estimated end position (in meter). */
  float e_est;  // UTILE ?

  /** Start trend. */
  DigitalStraightSegment *start_trend;
  /** End trend. */
  DigitalStraightSegment *end_trend;
  /** Reference start trend. */
  DigitalStraightSegment *strend_ref;
  /** Reference end trend. */
  DigitalStraightSegment *etrend_ref;
  /** Local height reference for trends (to process smaller values). */
  float trends_height;

  /** Baseline start point. */
  Pt2f line_start;
  /** Baseline end point. */
  Pt2f line_end;

  /** Estimated slope. */
  float slope_est;
  /** Estimated direction. */
  float dev_est;

  /** Indicates whether measure line differs from base line. */
  bool mline_p;
  /** Measure line translation ratio. */
  float mline_trsl;
  /** Measure line rotation ratio. */
  float mline_rot;
  /** Measure line start point. */
  Pt2f mline_start;
  /** Measure line end point. */
  Pt2f mline_end;
  /** Measure line interior start point index. */
  int mline_sind;
  /** Measure line interior end point index. */
  int mline_eind;
  /** Measure line top point index. */
  int mline_tind;

  /** Estimated area between reference line and bump surface. */
  float area_est;
  /** Over-estimated area between reference line and bump surface. */
  float area_up;
  /** Under-estimated area between reference line and bump surface. */
  float area_low;

  /** Used plateau model. */
  BumpModel *bmod;


  /**
   * \brief Checks presence of a hole in given point sequence.
   *   A hole is a large gap between adjacent points.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param l12 Scan length (in meters).
   */
  bool hasHole (const std::vector<Pt2f> &ptsh, float l12) const;

  /**
   * \brief Sets a trend on one side of given point sequence.
   *   A trend is a quasi-straight sequence of points.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param limit Count of points at sequence end, that may not be tested.
   * @param l12 Scan length (in meters).
   * @param rev Selected side: end side if true, start side otherwise.
   */
  int setTrend (const std::vector<Pt2f> &ptsh, int limit, float l12, bool rev);

  /**
   * \brief Tracks a trend from a reference on one side of given point sequence.
   *   A trend is a quasi-straight sequence of points.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param tx Reference trend middle position.
   * @param tw Reference trend X-width.
   * @param maxx Position of last possible point.
   * @param rev Selected side: end side if true, start side otherwise.
   */
  int trackTrend (const std::vector<Pt2f> &ptsh, float tx, float tw,
                  float maxx, bool rev);

  /**
   * \brief Finds the bearings of a bump baseline.
   *   The search interval is restricted to [s_num, e_num].
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param s_num In: search start index -> Out: start bearing point index.
   * @param e_num In: search end index -> Out: end bearing point index.
   */
  bool setBaseLine (const std::vector<Pt2f> &ptsh, int &s_num, int &e_num);

  /**
   * \brief Sets the extremum point index and returns its shift to baseline.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param a First baseline parameter (ax + by = c).
   * @param b Second baseline parameter (ax + by = c).
   * @param c Third baseline parameter (ax + by = c).
   */
  float getSummit (const std::vector<Pt2f> &ptsh, float a, float b, float c);

  /**
   * \brief Sets surface center and returns bump area over reference baseline.
   *   Biased computation formerly used.
   *   Bias: product of point position shifts by the heights to baseline.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param a First baseline parameter (ax + by = c).
   * @param b Second baseline parameter (ax + by = c).
   * @param c Third baseline parameter (ax + by = c).
   * @param den Regularization factor (sqrt (a^2 + b^2)).
   */
  float getBiasedAreaAndSurfaceCenter (const std::vector<Pt2f> &ptsh,
                       float a, float b, float c, float den);

  /**
   * \brief Splits the bump orthogonally to baseline in two equal areas.
   *   Finds the orthogonal cut line and the surface center.
   *   Returns according evaluation of bump section area.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param a First baseline parameter (ax + by = c).
   * @param b Second baseline parameter (ax + by = c).
   * @param c Third baseline parameter (ax + by = c).
   * @param den2 Normalization factor to extract area measure (a^2 + b^2).
   */
  float verticalSplit (const std::vector<Pt2f> &ptsh,
                       float a, float b, float c, float den2);

  /**
   * \brief Sets the bump mass center.
   *   Splits the bump at half section area in parallel direction to baseline.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param a First baseline parameter (ax + by = c).
   * @param b Second baseline parameter (ax + by = c).
   * @param c Third baseline parameter (ax + by = c).
   * @param den Normalization factor to extract area measure (sqrt(a^2 + b^2)).
   * @param area_2 Half section area.
   */
  void setMassCenter (const std::vector<Pt2f> &ptsh,
                      float a, float b, float c, float den, float area_2);


  /**
   * \brief Sets an estimation of the area between lower line and surface.
   *   Based on integration of vertical differences between reference line
   *   and surface. Accuracy is related to point integration along the ridge.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  void estimateArea (const std::vector<Pt2f> *ptsh);

  /**
   * \brief Sets section area upper bound.
   *   Area is integrated using parallel cuts to lower line
   *     from the lower line up to the summit.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  void setAreaUpperBound (const std::vector<Pt2f> *ptsh);

  /**
   * \brief Sets section area lower bound.
   *   Area is integrated using parallel cuts to lower line
   *     from the summit down to lower line.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  void setAreaLowerBound (const std::vector<Pt2f> *ptsh);

};
#endif
