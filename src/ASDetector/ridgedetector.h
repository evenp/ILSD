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

#ifndef RIDGE_DETECTOR_H
#define RIDGE_DETECTOR_H

#include "ridge.h"
#include "ipttileset.h"
#include "scannerprovider.h"


/** 
 * @class RidgeDetector ridgedetector.h
 * \brief Ridge structure detector.
 * All calculations in meters, but not input ip1, ip2, fp1, fp2.
 */
class RidgeDetector
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
  /** Extraction failure : no central bump detected. */
  static const int RESULT_FAIL_NO_CENTRAL_BUMP;


  /**
   * \brief Creates a new ridge structure detector.
   */
  RidgeDetector ();

  /**
   * \brief Deletes the ridge detector.
   */
  ~RidgeDetector ();

  /**
   * \brief Clears stored detected feature.
   */
  void clear ();

  /**
   * \brief Returns the profile model used.
   */
  inline BumpModel *model () { return &bfeat; }

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
   * \brief Detects a ridge between input points.
   * Returns the detected ridge.
   * @param p1 First input point.
   * @param p2 Second input point.
   */
  Ridge *detect (const Pt2i &p1, const Pt2i &p2);

  /**
   * \brief Returns the detection status.
   * @param initial Initial detection if true, final otherwise.
   */
  inline int getStatus (bool initial = false) {
    return (initial ? istatus : fstatus); }

  /**
   * \brief Returns the detected ridge.
   */
  inline Ridge *getRidge (bool initial = false) {
    return (initial ? ibg : fbg); }

  /**
   * \brief Returns the final detection input stroke points.
   */
  inline void getInputStroke (Pt2i &p1, Pt2i &p2, bool initial = false) const {
    p1.set (initial ? ip1 : fp1); p2.set (initial ? ip2 : fp2); }

  /**
   * \brief Returns the profile registration status.
   */
  inline bool isProfileRecorded () { return profileRecordOn; }

  /**
   * \brief Sets the profile registration status on or off.
   * @param status New status for profile registration modality.
   */
  inline void recordProfile (bool status) { profileRecordOn = status; }

  /**
   * \brief Checks whether no successful final detection is stored.
   */
  inline bool noFinalDetection () const { return (fbg == NULL); }

  /**
   * \brief Checks whether no successful initial detection is stored.
   */
  inline bool noInitialDetection () const { return (ibg == NULL); }

  /**
   * \brief Returns accepted count of successive bump detections failures.
   */
  inline int getBumpLackTolerance () const {
    return bump_lack_tolerance; }

  /**
   * \brief Increments accepted count of successive bump detection failures.
   * @param dir Increment direction: 1 if increment, -1 if decrement.
   */
  void incBumpLackTolerance (int dir);

  /**
   * \brief Sets accepted count of successive bump detection failures.
   * @param val New count value.
   */
  void setBumpLackTolerance (int val);

  /**
   * \brief Returns the ridge orientation (true for ridge, false for hollow).
   */
  bool isOver () const;

  /**
   * \brief Inverts the ridge orientation (ridge or hollow).
   */
  void switchOver ();

  /**
   * \brief Sets the ridge orientation.
   * @param status Orientation status : to ridge of true, to hollow otherwise.
   */
  void setOver (bool status);

  /**
   * \brief Switches on or off the initialization step.
   */
  inline void switchInitialization () {
    if (initial_ridge_extent != 0) initial_ridge_extent = 0;
    else initial_ridge_extent = INITIAL_RIDGE_EXTENT; }

  /**
   * \brief Indicates whether initialization step is on.
   */
  inline bool isInitializationOn () const {
    return (initial_ridge_extent != 0); }

  /**
   * \brief Returns the DTM cell size.
   */
  inline float getCellSize () const { return csize; };

  /**
   * \brief Returns the status of measures estimation modality.
   */
  bool isMeasured () const { return bfeat.isMeasured (); }

  /**
   * \brief Switches measures estimation modality.
   */
  void switchMeasured () { bfeat.switchMeasured (); }


private :

  /** Minimal input stroke size to run a ridge detection. */
  static const int MIN_SELECTION_SIZE;
  /** Default accepted count of successive bump detection failures. */
  static const int DEFAULT_BUMP_LACK_TOLERANCE;
  /** Initial ridge extent on each side of the central bump. */
  static const int INITIAL_RIDGE_EXTENT;
  /** Default count of registered positions and heights. */
  static const int DEFAULT_POS_AND_HEIGHT_REGISTER_SIZE;

  /** Points grid. */
  IPtTileSet *ptset;
  /** Cloud grid / Dtm grid ratio. */
  int subdiv;
  /** DTM cell size: pixel to cloud point (meter) ratio. */
  float csize;
  /** Profile registration status. */
  bool profileRecordOn;

  /** Directional scanner provider for detection purspose. */
  ScannerProvider scanp;
  /** Directional scanner provider for display purspose. */
  ScannerProvider discanp;
  /** Bump detection features. */
  BumpModel bfeat;
  /** Accepted count of successive bump detection failures. */
  int bump_lack_tolerance;
  /** Initial ridge extent on each side of the central bump. */
  int initial_ridge_extent;

  /** Final detected buried wall. */
  Ridge *fbg;
  /** Final detection result status. */
  int fstatus;
  /** Final stroke first input point in DTM pixels. */
  Pt2i fp1;
  /** Final stroke second input point in DTM pixels. */
  Pt2i fp2;
  /** Initial detected buried wall. */
  Ridge *ibg;
  /** Initial detection result status. */
  int istatus;
  /** Initial stroke first input point in DTM pixels. */
  Pt2i ip1;
  /** Initial stroke second input point in DTM pixels. */
  Pt2i ip2;
  /** Current distance in meters between input points. */
  float l12;

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


  /**
   * \brief Detects a ridge between input points.
   * @param exlimit Limit of bumps extension.
   */
  void detect (int exlimit);

  /**
   * \brief Carries on ridge detection.
   * @param onright Extension direction.
   * @param reversed Indicates whether scans are reversed.
   * @param exlimit Limit of bumps extension.
   * @param ds Directional scanner used for detection.
   * @param disp Directional scanner used for display.
   * @param p1f Input reference point (in meters).
   * @param p12n Normalized input stroke vector.
   * @param refbmp Reference bump.
   */
  void track (bool onright, bool reversed, int exlimit,
              DirectionalScanner *ds, DirectionalScanner *disp,
              Pt2f p1f, Vr2f p12n, Bump *refbmp);

  /**
   * \brief Aligns input stroke on detected ridge points.
   * @param pts Central points of ridge bumps.
   */
  void alignInput (const std::vector<Pt2f> &pts);

  /**
   * \brief Resets position and height registers.
   * @param ok Last position and height reliability.
   * @param center Last center.
   */
  void resetPositionsAndHeights (bool ok, Pt2f center);

  /**
   * \brief Sets the last position and returns estimated deviation.
   * @param ok Last position reliability.
   * @param pos Last position value if reliable.
   */
  float updatePosition (bool ok, float pos = 0.0f);

  /**
   * \brief Sets the last height and returns estimated slope.
   * @param ok Last position reliability.
   * @param ht Last height value if reliable.
   */
  float updateHeight (bool ok, float ht = 0.0f);

  /**
   * \brief Compares points by distance to scan bound.
   * @param p1 First point.
   * @param p2 Second point.
   */
  static bool compFurther (Pt2f p1, Pt2f p2);

};
#endif
