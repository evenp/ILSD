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

#ifndef RIDGE_H
#define RIDGE_H

#include "ridgesection.h"

// Display styles
#define RIDGE_DISP_SCANS 0
#define RIDGE_DISP_CONNECT 1
#define RIDGE_DISP_BOUNDS 2
#define RIDGE_DISP_SPINE 3
#define RIDGE_DISP_CENTER 4
#define RIDGE_DISP_MAX 4


/** 
 * @class Ridge ridge.h
 * \brief Ridge or hollow stucture detected in LiDAR data.
 */
class Ridge
{
public:

  /** Minimal value of ridge structure height. */
  static const float MIN_HEIGHT;
  /** Maximal value of ridge structure width. */
  static const float MAX_WIDTH;


  /**
    \brief Creates a new ridge structure.
   */
  Ridge ();

  /**
   * \brief Deletes the ridge.
   */
  ~Ridge ();

  /**
   * \brief Starts the ridge structure with a central bump and displayed scan.
   * @param pl Central bump to be added.
   * @param dispix Scan used for display.
   * @param reversed Reversed scan modality (P2 -> P1).
   */
  void start (Bump *bump, const std::vector<Pt2i> &dispix, bool reversed);

  /**
   * \brief Starts the ridge structure with a central bump, scan and profile.
   * @param pl Central bump to be added.
   * @param dispix Scan used for display.
   * @param pts Height profile extracted from detection scan.
   * @param reversed Reversed scan modality (P2 -> P1).
   */
  void start (Bump *bump, const std::vector<Pt2i> &dispix,
              const std::vector<Pt2f> &pts, bool reversed);

  /**
   * \brief Adds a bump to the ridge with displayed scan.
   * @param onright Indicates if adding deals with right section.
   * @param bump Bump to be added.
   * @param dispix Scan used for display.
   */
  void add (bool onright, Bump *bump, const std::vector<Pt2i> &dispix);

  /**
   * \brief Adds a bump to the ridge with scan and profile.
   * @param onright Indicates if adding deals with right section.
   * @param bump Bump to be added.
   * @param dispix Scan used for display.
   * @param pts Height profile extracted from detection scan.
   */
  void add (bool onright, Bump *bump,
            const std::vector<Pt2i> &dispix, const std::vector<Pt2f> &pts);

  /**
   * \brief Returns the number of bumps on right side, start bump excluded.
   */
  int getRightScanCount () const;

  /**
   * \brief Returns the number of bumps on left side, start bump excluded.
   */
  int getLeftScanCount () const;

  /**
   * \brief Indicates whether the scans of a bump are reversed.
   * @param num bump number.
   */
  bool isScanReversed (int num) const;

  /**
   * \brief Returns a pointer to a scan to be displayed.
   * @param num bump number.
   */
  std::vector<Pt2i> *getDisplayScan (int num);

  /**
   * \brief Returns one of the buried wall bumps.
   * Returns NULL if the bump is not instanciated.
   * @param num bump number.
   */
  Bump *bump (int num) const;

  /**
   * \brief Returns the height reference of a bump.
   * This reference is the mean altitude of the bump.
   * Returns 0 if the bump is not successfully detected.
   * @param num bump number.
   */
  float getHeightReference (int num) const;

  /**
   * \brief Provides the last valid bump from given position.
   * The last valid bump is searched towards the central bump.
   * @param num Relative index of the bump (negative = right).
   */
  Bump *lastValidBump (int num) const;

  /**
   * \brief Returns a point profile to be displayed.
   * @param num Relative index of the bump (negative = right).
   */
  std::vector<Pt2f> *getProfile (int num);

  /**
   * \brief Adds bump positions to given vectors.
   * The bump position is an estimate of summit or center point.
   * The second vector is used for the end positions of intervals (start / end).
   * @param pts Vector to complete with bump positions (or start positions).
   * @param pts2 Vector to complete with bump interval end positions.
   * @param disp Display style.
   * @param iratio Meter to DTM pixel ratio.
   * @param smoothed Bump accept modality.
   */
  void getPosition (std::vector<Pt2i> &pts, std::vector<Pt2i> &pts2,
                    int disp, float iratio, bool smoothed);

  /**
   * \brief Returns the number of lines with a measure line defined.
   */
  int countOfMeasureLines () const;

  /**
   * \brief Gets bump measure lines in provided vector.
   * @param measures Provided vector.
   */
  void getMeasureLines (std::vector<float> &measures);

  /**
   * \brief Sets bump measure lines from provided vector.
   * @param measures Provided vector.
   */
  void setMeasureLines (std::vector<float> &measures);

  /**
   * \brief Updates bumps area measure.
   */
  void updateMeasure ();

  /**
   * \brief Estimates and returns the ridge volume between two scans.
   * @param m1 First scan index.
   * @param m2 Second scan index.
   * @param iratio World to scan unit ratio.
   * @param meas_low Volume estimate lower bound.
   * @param meas_up Volume estimate upper bound.
   */
  float estimateVolume (int m1, int m2, float iratio,
                        float &meas_low, float &meas_up);

  /**
   * \brief Computes and returns the mean slope between two scans.
   * @param m1 First scan index.
   * @param m2 Second scan index.
   * @param iratio World to scan unit ratio.
   * @param lg2 Horizontal length.
   * @param lg3 Three-dimensional length.
   * @param zmin Minimal height.
   * @param zmax Maximal height.
   */
  float estimateSlope (int m1, int m2, float irat, float &lg2, float &lg3,
                       float &zmin, float &zmax);

  /**
   * \brief Returns ridge mean width at given height on given section.
   *  Returns the count of measured bumps.
   * @param m1 First scan index of section.
   * @param m2 Second scan index of section.
   * @param mhratio Ratio to specify height at which width is measured.
   * @param mwidth Measured width.
   * @param sigma Width standard deviation.
   */
  int meanWidth (int m1, int m2, float mhratio, float &mwidth, float &sigma);

  /**
   * \brief Gets ridge mean height on a given section.
   *  Returns the count of measured bumps.
   * @param m1 First scan index of section.
   * @param m2 Second scan index of section.
   * @param mheight Measured height.
   * @param sigma Height standard deviation.
   */
  int meanHeight (int m1, int m2, float &mheight, float &sigma);


private :

  /** Input reference point (start) */
  Pt2i ip1;
  /** Input reference vector (start -> end) */
  Vr2i ip1p2;
  /** Central section. */
  RidgeSection startsec;
  /** Ridge sections on right side. */
  std::vector<RidgeSection *> rights;
  /** Ridge sections on left side. */
  std::vector<RidgeSection *> lefts;
  /** Current right section. */
  RidgeSection *curright;
  /** Current left section. */
  RidgeSection *curleft;


  /**
   * \brief Adds a bump center to the given vector of points.
   * @param pt Vector of point to complete with the center point.
   * @param num Scan index.
   * @param rev Status of the scan reversion modality.
   * @param disp Display style.
   * @param pp1 Central scan start position (in DTM reference).
   * @param p12 Central scan director vector (in DTM reference).
   * @param l12 Central scan length (in DTM reference).
   * @param iratio Meter to DTM pixel ratio.
   * @param smoothed Bump accept modality.
   */
  void addBumpCenter (std::vector<Pt2i> &pt, int num, bool rev,
                      int disp, Pt2i pp1, Vr2i p12, float l12,
                      float iratio, bool smoothed);

  /**
   * \brief Adds a bump center to the given vector of points.
   * @param spt Vector of point to complete with the start point.
   * @param ept Vector of point to complete with the end point.
   * @param num Scan index.
   * @param rev Status of the scan reversion modality.
   * @param pp1 Central scan start position (in DTM reference).
   * @param p12 Central scan director vector (in DTM reference).
   * @param l12 Central scan length (in DTM reference).
   * @param iratio Meter to DTM pixel ratio.
   * @param smoothed Bump accept modality.
   */
  void addBumpBounds (std::vector<Pt2i> &spt, std::vector<Pt2i> &ept2,
                      int num, bool rev, Pt2i pp1, Vr2i p12, float l12,
                      float iratio, bool smoothed);

  /**
   * \brief Returns the distance between successive scans in world unit.
   * @param iratio World to scan unit ratio.
   */
  float scanPeriod (float iratio);

  /**
   * \brief Gets ridge point at distance pos on scan num.
   * @param num Scan index.
   * @param pos Point position coordinate on the scan.
   * @param irat World to scan unit ratio.
   */
  Pt2f localize (int num, float pos, float irat);

};
#endif
