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

#ifndef CARRIAGE_TRACK_DETECTOR_H
#define CARRIAGE_TRACK_DETECTOR_H

#include "carriagetrack.h"
#include "ipttileset.h"
#include "scannerprovider.h"


/** 
 * @class CTrackDetector ctrackdetector.h
 * \brief Carriage track detector.
 * All calculations in meters, but not input ip1, ip2, fp1, fp2.
 */
class CTrackDetector
{
public:

  /** Extraction result : no extraction. */
  static const int RESULT_NONE;
  /** Extraction result : successful extraction. */
  static const int RESULT_OK;
  /** Extraction failure : two narrow input segment. */
  static const int RESULT_FAIL_TOO_NARROW_INPUT;
  /** Extraction failure : no input scan. */
  static const int RESULT_FAIL_NO_AVAILABLE_SCAN;
  /** Extraction failure : no central plateau detected. */
  static const int RESULT_FAIL_NO_CENTRAL_PLATEAU;
  /** Extraction failure : no long enough sequence of plateaux. */
  static const int RESULT_FAIL_NO_CONSISTENT_SEQUENCE;
  /** Extraction failure : no bounds for the section plateaux. */
  static const int RESULT_FAIL_NO_BOUNDS;
  /** Extraction failure : maximal track absolute shift length. */
  static const int RESULT_FAIL_TOO_HECTIC_PLATEAUX;
  /** Extraction failure : minimal plateaux density not fillfiled. */
  static const int RESULT_FAIL_TOO_SPARSE_PLATEAUX;


  /**
   * \brief Creates a new carriage track detector.
   */
  CTrackDetector ();

  /**
   * \brief Deletes the carriage track detector.
   */
  ~CTrackDetector ();

  /**
   * \brief Sets the points grid.
   * @param data Cloud points grid.
   * @param width Dtm grid width.
   * @param height Dtm grid height.
   * @param subdiv Cloud grid / Dtm grid ratio.
   * @param cellsize Cell size: DTM pixel to cloud point (meter) ratio.
   */
  void setPointsGrid (IPtTileSet *data, int width, int height,
                      int subdiv, float cellsize);

  /**
   * \brief Clears stored detected feature.
   */
  void clear ();

  /**
   * \brief Avoids former detection clearance.
   */
  void preserveDetection ();

  /**
   * \brief Returns the profile model used.
   */
  inline PlateauModel *model () { return &pfeat; }

  /**
   * \brief Avoids the deletion of the last extracted carriage track.
   */
  inline void preserveFormerTrack () { fct = NULL; }

  /**
   * \brief Detects a carriage track between input points.
   * Returns the detected carriage track.
   * @param p1 First input point.
   * @param p2 Second input point.
   */
  CarriageTrack *detect (const Pt2i &p1, const Pt2i &p2);

  /**
   * \brief Returns the detection status.
   * @param initial Initial detection if true, final otherwise.
   */
  inline int getStatus (bool initial = false) {
    return (initial ? istatus : fstatus); }

  /**
   * \brief Returns the detected carriage track.
   */
  inline CarriageTrack *getCarriageTrack (bool initial = false) {
    return (initial ? ict : fct); }

  /**
   * \brief Returns the final detection input stroke points.
   */
  inline void getInputStroke (Pt2i &p1, Pt2i &p2, bool initial = false) const {
    p1.set (initial ? ip1 : fp1); p2.set (initial ? ip2 : fp2); }

  /**
   * \brief Returns the status of automatic extraction modality.
   */
  inline bool isAutomatic () const { return auto_p; }

  /**
   * \brief Switches on or off the automatic detection modality.
   */
  inline void switchAutomatic () { auto_p = ! auto_p; }

  /**
   * \brief Sets the status of automatic detection modality.
   */
  inline void setAutomatic (bool status) { auto_p = status; }

  /**
   * \brief Sets the profile registration status on or off.
   * @param status New status for profile registration modality.
   */
  inline void recordProfile (bool status) { profileRecordOn = status; }

  /**
   * \brief Checks whether no successful detection is stored.
   */
  inline bool noFinalDetection () const { return (fct == NULL); }

  /**
   * \brief Checks whether no successful detection is stored.
   */
  inline bool noInitialDetection () const { return (ict == NULL); }

  /**
   * \brief Returns accepted count of successive plateau detection failures.
   */
  inline int getPlateauLackTolerance () const {
    return plateau_lack_tolerance; }

  /**
   * \brief Increments accepted count of successive plateau detection failures.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incPlateauLackTolerance (int dir);

  /**
   * \brief Sets accepted count of successive plateau detection failures.
   * @param nb New count value.
   */
  void setPlateauLackTolerance (int nb);

  /**
   * \brief Switches on or off the initialization step.
   */
  inline void switchInitialization () {
    if (initial_track_extent != 0) initial_track_extent = 0;
    else initial_track_extent = INITIAL_TRACK_EXTENT; }

  /**
   * \brief Indicates whether initialization step is on.
   */
  inline bool isInitializationOn () const {
    return (initial_track_extent != 0); }

  /**
   * \brief Returns the status of track absolute shift length pruning modality.
   */
  inline bool isShiftLengthPruning () const { return shift_length_pruning; }

  /**
   * \brief Switches on or off the track absolute shift length pruning modality.
   */
  inline void switchShiftLengthPruning () {
    shift_length_pruning = ! shift_length_pruning; }

  /**
   * \brief Returns the maximal absolute shift length.
   */
  inline float maxShiftLength () const { return max_shift_length; }

  /**
   * \brief Increments the maximal absolute shift length.
   * @param Increment value.
   */
  void incMaxShiftLength (int inc);

  /**
   * \brief Sets the maximal absolute shift length.
   * @param New length value.
   */
  void setMaxShiftLength (float val);

  /**
   * \brief Returns the status of plateaux density pruning modality.
   */
  inline bool isDensityPruning () const { return density_pruning; }

  /**
   * \brief Switches on or off the plateaux density pruning modality.
   */
  inline void switchDensityPruning () { density_pruning = ! density_pruning; }

  /**
   * \brief Returns the minimal plateaux density percentage.
   */
  inline int minDensity () const { return min_density; }

  /**
   * \brief Increments the minimal plateaux density percentage.
   * @param Increment value.
   */
  void incMinDensity (int inc);

  /**
   * \brief Sets the minimal plateaux density percentage.
   * @param New undetection ratio value.
   */
  void setMinDensity (int val);

  /**
   * \brief Indicates whether points density is used for tracking.
   */
  inline bool isDensitySensitive () const { return (! density_insensitive); }

  /**
   * \brief Switches the points density use for tracking.
   */
  inline void switchDensitySensitivity () {
    density_insensitive = ! density_insensitive; }

  /**
   * \brief Returns the DTM cell size.
   */
  inline float getCellSize () const { return csize; }

inline int getOuts () const { return out_count; }

inline void resetOuts () { out_count = 0; }

  /**
   * \brief Labels cloud points used for a carriage track detection.
   * @param ct Detected carriage track.
   */
  void labelPoints (CarriageTrack *ct);


private :

  /** Maximal width of a track. */
  static const float MAX_TRACK_WIDTH;
  /** Default accepted count of successive plateau detection failures. */
  static const int DEFAULT_PLATEAU_LACK_TOLERANCE;
  /** Default tolered lack of both bounds detection. */
  static const int NOBOUNDS_TOLERANCE;
  /** Initial track extent on each side of the central plateau. */
  static const int INITIAL_TRACK_EXTENT;
  /** Default count of registered center positions and heights. */
  static const int DEFAULT_POS_AND_HEIGHT_REGISTER_SIZE;
  /** Default count of registered bound positions and consistencies. */
  static const int DEFAULT_UNSTABILITY_REGISTER_SIZE;
  /** Default plateau density percentage required to validate a track. */
  static const int DEFAULT_MIN_DENSITY;
  /** Default maximal value for accepted absolute shift length. */
  static const float DEFAULT_MAX_SHIFT_LENGTH;
  /** Default deviation offset for bounds unstability checking. */
  static const float LN_UNSTAB;
  /** Default unconsistency offset for bounds unstability checking. */
  static const int NB_UNSTAB;
  /** Position increment for test settings. */
  static const float POS_INCR;
  /** Amount of side trials in automatic mode. */
  static const int NB_SIDE_TRIALS;

  /** Points grid. */
  IPtTileSet *ptset;
  /** Cloud grid / Dtm grid ratio. */
  int subdiv;
  /** DTM cell size: pixel to cloud point (meter) ratio. */
  float csize;
  /** Automatic extraction modality. */
  bool auto_p;
  /** Profile registration status. */
  bool profileRecordOn;

  /** Directional scanner provider for detection purpose. */
  ScannerProvider scanp;
  /** Directional scanner provider for display purpose. */
  ScannerProvider discanp;
  /** Plateau detection features. */
  PlateauModel pfeat;
  /** Tolered successive failures of plateaux detection. */
  int plateau_lack_tolerance;
  /** Initial track extent on each side of the central plateau. */
  int initial_track_extent;
  /** Indicates if point density is considered for tracking. */
  bool density_insensitive;
  /** Track absolute shift length pruning modality. */
  bool shift_length_pruning;
  /** Maximal track absolute shift length accepted. */
  float max_shift_length;
  /** Plateaux density pruning modality. */
  bool density_pruning;
  /** Minimal plateaux density requested. */
  int min_density;

  /** Finally detected carriage track. */
  CarriageTrack *fct;
  /** Final detection result status. */
  int fstatus;
  /** Final stroke first input point in DTM pixels. */
  Pt2i fp1;
  /** Final stroke second input point in DTM pixels. */
  Pt2i fp2;
  /** Initially detected carriage track. */
  CarriageTrack *ict;
  /** Initial detection result status. */
  int istatus;
  /** Initial stroke first input point in DTM pixels. */
  Pt2i ip1;
  /** Initial stroke second input point in DTM pixels. */
  Pt2i ip2;

  /** Position and height register size. */
  int posht_nb;
  /** Last position reliabilities. */
  bool *lpok;
  /** Last position values. */
  float *lpos;
  /** Last height reliabilities. */
  bool *lhok;
  /** Last height values. */
  float *lht;
  /** Initially found reference start position. */
  float initial_refs;
  /** Initially found reference end position. */
  float initial_refe;
  /** Initially found reference height. */
  float initial_refh;
  /** Fine bounds detected at initial step. */
  bool initial_unbounded;
  /** Bounds stability register size. */
  int unstab_nb;
  /** Last start position values. */
  float *spos;
  /** Last end position values. */
  float *epos;
  /** Last start position consistency status. */
  bool *spok;
  /** Last end position consistency status. */
  bool *epok;

  int out_count;


  /**
   * \brief Detects a carriage track between input points.
   * @param exlimit Limit of plateaux extension.
   */
  void detect (int exlimit);

  /**
   * \brief Detects a carriage track between input points.
   */
  void detect ();

  /**
   * \brief Carries on carriage track detection.
   * @param onright Extension direction.
   * @param reversed Indicates whether scans are reversed.
   * @param exlimit Limit of plateaux extension.
   * @param ds Directional scanner used for detection.
   * @param disp Directional scanner used for display.
   * @param p1f Input reference point (in meters).
   * @param p12 Input stroke vector (in meters).
   * @param l12 Reference stroke length (in meters).
   * @param refs Template start position.
   * @param refe Template end position.
   * @param refh Template lower height.
   */
  void track (bool onright, bool reversed, int exlimit,
              DirectionalScanner *ds, DirectionalScanner *disp,
              Pt2f p1f, Vr2f p12, float l12,
              float refs, float refe, float refh);

  /**
   * \brief Resets bounds and center position and height registers.
   * @param ok Last position and height reliability.
   * @param pos Last position value.
   * @param ht Last height value.
   */
  void resetRegisters (bool ok = false, float pos = 0.0f, float ht = 0.0f);

  /**
   * \brief Sets the last position and returns estimated deviation.
   * @param ok Last position reliability.
   * @param pos Last position value if reliable.
   */
  float updatePosition (bool ok, float pos = 0.0f);

  /**
   * \brief Sets the last height and returns estimated slope.
   * @param ok Last height reliability.
   * @param ht Last height value if reliable.
   */
  float updateHeight (bool ok, float ht = 0.0f);

  /**
   * \brief Registers bounds positions and estimate bounds stability.
   * Returns 1 if start bound is much more stable than end bound,
   *         -1 if end bound is much more stable than start bound,
   *         0 in any other cases.
   * @param slat Last start bound position.
   * @param elat Last end bound position.
   * @param sok Last start bound consistency status.
   * @param eok Last end bound consistency status.
   * @param trw Track estimated width.
   */
  int boundsStability (float slast, float elast,
                       bool sok, bool eok, float trw);

  /**
   * \brief Compares points by distance to scan bound on integer basis.
   * @param p1 First point.
   * @param p2 Second point.
   */
  static bool compIFurther (Pt2f p1, Pt2f p2);

  /**
   * \brief Compares labelled points by distance to scan bound on integer basis.
   * The third dimension is used for the label.
   * @param p1 First point.
   * @param p2 Second point.
   */
  static bool compLFurther (Pt3f p1, Pt3f p2);

  /**
   * \brief Aligns input stroke on detected track points.
   * @param pts Central points of carriage track plateaux.
   */
  void alignInput (const std::vector<Pt2f> &pts);

  /**
   * \brief Tests shift computing between scans.
   */
  void testScanShiftExtraction () const;

  int scanShift (float pcenter);

};
#endif
