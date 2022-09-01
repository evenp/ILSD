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

#ifndef PLATEAU_H
#define PLATEAU_H

#include "pt2f.h"
#include "plateaumodel.h"
#include "digitalstraightsegment.h"


/** 
 * @class Plateau plateau.h
 * \brief Cross section of a carriage track.
 */
class Plateau
{
public:

  /** Detection status : not processed. */
  static const int PLATEAU_RES_NONE;
  /** Detection status : success. */
  static const int PLATEAU_RES_OK;
  /** Detection status : failed, not enough points in input scan. */
  static const int PLATEAU_RES_NOT_ENOUGH_INPUT_PTS;
  /** Detection status : failed, length lower than minimal awaited value. */
  static const int PLATEAU_RES_TOO_NARROW;
  /** Detection status : failed, not enough points with same height. */
  static const int PLATEAU_RES_NOT_ENOUGH_ALT_PTS;
  /** Detection status : failed, not enough successive points. */
  static const int PLATEAU_RES_NOT_ENOUGH_CNX_PTS;
  /** Detection status : failed, both bounds undetected. */
  static const int PLATEAU_RES_NO_BOUND_POS;
  /** Detection status : failed, too few optimal height points used. */
  static const int PLATEAU_RES_OPTIMAL_HEIGHT_UNDER_USED;
  /** Detection status : failed, too large width narrowing. */
  static const int PLATEAU_RES_TOO_LARGE_NARROWING;
  /** Detection status : failed, too large width widening. */
  static const int PLATEAU_RES_TOO_LARGE_WIDENING;
  /** Detection status : failed, no blurred segment built. */
  static const int PLATEAU_RES_NO_BS;
  /** Detection status : failed, too large blurred segment tilt. */
  static const int PLATEAU_RES_TOO_LARGE_BS_TILT;
  /** Detection status : failed, central point out of height reference. */
  static const int PLATEAU_RES_OUT_OF_HEIGHT_REF;

  
  /**
   * \brief Creates a new plateau.
   * @param pmod Plateau detection features.
   */
  Plateau (PlateauModel *pmod);

  /**
   * \brief Deletes the plateau.
   */
  ~Plateau ();

  /**
   * \brief Sets the input scan center shift.
   * @param ct_shift Center shift value (in pixels).
   */
  inline void setScanShift (int ct_shift) { scan_shift = ct_shift; }

  /**
   * \brief Returns the input scan center shift.
   */
  inline int scanShift () const { return scan_shift; }

  /**
   * \brief Detects the plateau in a scan.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param all Indicates whether all heights should be processed.
   * @param exh Lower value of excluded height interval.
   */
  bool detect (const std::vector<Pt2f> &ptsh,
               bool all = true, float exh = 0.0f);

  /**
   * \brief Detects the plateau in a scan knowing the neighboring plateau.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   * @param lstart Awaited start position.
   * @param lend Awaited end position.
   * @param lheight Awaited altitude.
   * @param cshift Reference center shift.
   * @param confdist Distance to last reliable plateau (in count of stripes).
   */
  bool track (const std::vector<Pt2f> &ptsh,
              float lstart, float lend, float lheight,
              float cshift, int confdist);

  /**
   * \brief Provides plateau detection status.
   */
  inline int getStatus () const { return status; }

  /**
   * \brief Checks if detection failure does not come from a low point density.
   */
  inline int hasEnoughPoints () const {
    return (status != PLATEAU_RES_NOT_ENOUGH_INPUT_PTS
            && status != PLATEAU_RES_NOT_ENOUGH_ALT_PTS); }

  /**
   * \brief Sets plateau detection status.
   * @param newstatus New status value.
   */
  inline void setStatus (int newstatus) { status = newstatus; }

  /**
   * \brief Sets plateau status to ok.
   */
  inline void acceptResult () { status = PLATEAU_RES_OK; }

  /**
   * \brief Returns whether the plateau was accepted.
   */
  inline bool isAccepted () const { return accepted; }

  /**
   * \brief Accepts the plateau for integration into the carriage track.
   */
  inline void accept () { accepted = true; }

  /**
   * \brief Refuses the plateau for integration into the carriage track.
   */
  inline void prune () { accepted = false; }

  /**
   * \brief Returns whether the plateau is inserted in the track.
   * @param smoothed Insertion condition : simply accepted (true) or status ok.
   */
  inline bool inserted (bool smoothed) const {
    return (smoothed ? accepted : status == PLATEAU_RES_OK); }

  /**
   * \brief Indicates whether the start position lies in side tolerance.
   */
  inline bool consistentStart () const { return s_ok; }

  /**
   * \brief Indicates whether the end position lies in side tolerance.
   */
  inline bool consistentEnd () const { return e_ok; }

  /**
   * \brief Indicates whether the width lies in assigned tolerance.
   */
  inline bool consistentWidth () const { return w_ok; }

  /**
   * \brief Indicates whether the height lies in assigned tolerance.
   */
  inline bool consistentHeight () const { return status == PLATEAU_RES_OK; }

  /**
   * \brief Indicates whether position and width lies in assigned tolerance.
   */
  inline bool reliable () const { return (s_ok && e_ok && w_ok); }

  /**
   * \brief Returns whether bounds are finely bounded.
   */
  inline bool bounded () const { return (s_ok && e_ok); }

  /**
   * \brief Indicates whether at least one bound lies in assigned tolerance.
   */
  inline bool possible () const { return (s_ok || e_ok); }

  /**
   * \brief Indicates whether tested height is not successful.
   */
  inline bool noOptimalHeight () const { return (
    status == PLATEAU_RES_NOT_ENOUGH_INPUT_PTS
    || status == PLATEAU_RES_NOT_ENOUGH_ALT_PTS); }

  /**
   * \brief Returns the estimated start position.
   */
  inline float estimatedStart () const { return s_est; }

  /**
   * \brief Returns the estimated end position.
   */
  inline float estimatedEnd () const { return e_est; }

  /**
   * \brief Returns the estimated center position.
   */
  inline float estimatedCenter () const { return (s_est + e_est) / 2; }

  /**
   * \brief Returns the estimated width.
   */
  inline float estimatedWidth () const { return (e_est - s_est); }

  /**
   * \brief Returns the estimated width confidence level.
   */
  inline float widthConfidence () const {
    return ((s_int - s_ext) + (e_ext - e_int)); }

  /**
   * \brief Returns the start point index in the profile sequence.
   */
  inline int startIndex () const { return s_num; }

  /**
   * \brief Returns the end point index in the profile sequence.
   */
  inline int endIndex () const { return e_num; }

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
   * \brief Returns the distance from reference start to start interval.
   */
  inline float distStart () const { return sdist; }

  /**
   * \brief Returns the distance from reference end to end interval.
   */
  inline float distEnd () const { return edist; }

  /**
   * \brief Returns the reference start position.
   */
  inline float referenceStart () const { return s_ref; }

  /**
   * \brief Returns the reference end position.
   */
  inline float referenceEnd () const { return e_ref; }

  /**
   * \brief Returns the reference center position.
   */
  inline float referenceCenter () const { return (s_ref + e_ref) / 2; }

  /**
   * \brief Returns the reference min height.
   */
  inline float referenceHeight () const { return h_ref; }

  /**
   * \brief Provides a plateau smallest height.
   */
  inline float getMinHeight () const { return h_min; }

  /**
   * \brief Sets plateau estimated direction.
   */
  inline int getWidthChange () const { return width_change; }

  /**
   * \brief Sets plateau estimated deviation.
   */
  inline void setDeviation (float val) { dev_est = val; }

  /**
   * \brief Provides plateau estimated deviation.
   */
  inline float estimatedDeviation () const { return dev_est; }

  /**
   * \brief Sets plateau estimated slope.
   */
  inline void setSlope (float val) { slope_est = val; }

  /**
   * \brief Provides plateau estimated slope.
   */
  inline float estimatedSlope () const { return slope_est; }

  /**
   * \brief Returns whether given position lies within external bounds.
   * @param pos Given position.
   */
  inline bool contains (float pos) const {
    return (pos > s_ext && pos < e_ext); }

  /**
   * \brief Returns the vertical width of the plateau enclosing DSS.
   */
  float getDSSwidth () const;

  /**
   * \brief Returns the start point of the plateau enclosing DSS.
   */
  Pt2f getDSSstart () const;

  /**
   * \brief Returns the end point of the plateau enclosing DSS.
   */
  Pt2f getDSSend () const;

  /** Returns whether the plateau is less tilted than another one.
   * @param pl The other plateau to compare with.
   */
  bool lessTiltedThan (Plateau *pl) const;

  /** Returns whether the plateau is thiner than another one.
   * @param pl The other plateau to compare with.
   */
  bool thinerThan (Plateau *pl) const;

//  void trace () const;

//  void traceExt (const std::vector<Pt2f> &ptsh) const;



private :

  /** Minimal number of points to detect a plateau. */
  static const int PLATEAU_MIN_PTS;
  /** Maximal number of successive interruptions (height artefacts). */
  static const int PLATEAU_MAX_INTERRUPTION;
  /** Maximal number of undetected plateau accepted. */
  static const int PLATEAU_LACK_TOLERANCE;
  /** Default maximal thickness of a plateau. */
  static const float DEFAULT_PLATEAU_HEIGHT_TOLERANCE;
  /** Default maximal height difference between successive plateaux. */
  static const float DEFAULT_PLATEAU_SLOPE_TOLERANCE;
  /** Height tolerance increment. */
  static const float HEIGHT_INCREMENT;
  /** Minimal height tolerance value. */
  static const float MIN_HEIGHT_TOLERANCE;
  /** Default minimal length of a plateau. */
  static const float DEFAULT_PLATEAU_MIN_LENGTH;
  /** Length tolerance increment. */
  static const float LENGTH_INCREMENT;
  /** Minimal length tolerance value. */
  static const float MIN_LENGTH_TOLERANCE;
  /** Default maximal width shift between successive plateaux. */
  static const float DEFAULT_PLATEAU_SHIFT_TOLERANCE;
  /** Position tolerance increment. */
  static const float POS_INCREMENT;
  /** Minimal position tolerance value. */
  static const float MIN_POS_TOLERANCE;

  /** Detection result. */
  int status;
  /** Acceptation status. */
  bool accepted;

  /** Used plateau model. */
  PlateauModel *pmod;
  /** Enclosing digital straight segment of plateau blurred segment. */
  DigitalStraightSegment *dss;

  /** Local height (for relative calculations). */
  float locheight;
  /** Reference altitude (in meter). */
  float h_ref;
  /** Minimal value of height interval. */
  float h_min;

  /** Reference start position (in meter). */
  float s_ref;
  /** Reference end position (in meter). */
  float e_ref;
  /** Input scan center shift (in pixels). Invalid for first scan. */
  int scan_shift;
  /** Index of plateau start impact. */
  int s_num;
  /** Index of plateau end impact. */
  int e_num;
  /** Measured max start position (in meter). */
  float s_int;
  /** Measured min end position (in meter). */
  float e_int;
  /** Measured min start position (in meter). */
  float s_ext;
  /** Measured max end position (in meter). */
  float e_ext;
  /** Estimated start position (in meter). */
  float s_est;
  /** Estimated end position (in meter). */
  float e_est;

  /** Distance of start reference to start bound interval. */
  float sdist;
  /** Distance of end reference to end bound interval. */
  float edist;
  /** Status of plateau start detection. */
  bool s_ok;
  /** Status of plateau end detection. */
  bool e_ok;
  /** Status of plateau width detection. */
  bool w_ok;

  /** Width change detection: 0 = no change, -1 = narrowing, +1 = widening. */
  int width_change;
  /** Estimated track slope. */
  float slope_est;
  /** Estimated track deviation. */
  float dev_est;


  /**
   * \brief Sets first plateau bounds and suitability from tracking result.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  void setFirstBounds (const std::vector<Pt2f> &ptsh);

  /**
   * \brief Sets plateau bounds and suitability from tracking result.
   * @param ptsh Scan points sorted by increasing distance to scan start bound.
   */
  void setBounds (const std::vector<Pt2f> &ptsh);

  /**
   * \brief Estimates the plateau position.
   * @param wmt Maximal bounds shift tolered.
   */
  void setPosition (float wmt);

  /**
   * \brief Compares points by increasing altitude.
   * @param p1 First point.
   * @param p2 Second point.
   */
  static bool compHigher (Pt2f p1, Pt2f p2);

};
#endif
