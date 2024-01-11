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

#ifndef CARRIAGE_TRACK_H
#define CARRIAGE_TRACK_H

#include "ctracksection.h"

// Display modes
#define CTRACK_DISP_SCANS 0
#define CTRACK_DISP_CONNECT 1
#define CTRACK_DISP_BOUNDS 2
#define CTRACK_DISP_SPINE 3
#define CTRACK_DISP_CENTER 4
#define CTRACK_DISP_MAX 1


/** 
 * @class CarriageTrack carriagetrack.h
 * \brief Carriage track detected in LiDAR data.
 */
class CarriageTrack
{
public:

  /** Minimal value of carriage tracks width. */
  static const float MIN_WIDTH;
  /** Maximal value of carriage tracks width. */
  static const float MAX_WIDTH;


  /**
    \brief Creates a new carriage track.
   */
  CarriageTrack ();

  /**
   * \brief Deletes the carriage track.
   */
  ~CarriageTrack ();

  /**
   * \brief Clears right or left sections.
   */
  void clear (bool onright);

  /**
   * \brief Returns whether the carriage track was successfully detected.
   */
  inline bool isValid () const { return (status == 1); }

  /**
   * \brief Returns the carriage track detection status.
   */
  inline int getStatus () const { return status; }

  /**
   * \brief Sets the carriage track detection status.
   */
  inline void setStatus (int val) { status = val; }

  /**
   * \brief Sets the carriage track detection seed.
   * @param p1 Seed start point.
   * @param p2 Seed end point.
   * @param cs DTM grid cell size.
   */
  void setDetectionSeed (Pt2i p1, Pt2i p2, float cs);

  /**
   * \brief Gets the seed start point.
   */
  inline Pt2i getSeedStart () const { return seed_p1; }

  /**
   * \brief Gets the seed end point.
   */
  inline Pt2i getSeedEnd () const { return seed_p2; }

  /**
   * \brief Starts the carriage track with a central plateau and displayed scan.
   * @param pl Central plateau to be added.
   * @param dispix Scan used for display.
   * @param reversed Reversed scan modality (P2 -> P1).
   */
  void start (Plateau *pl, const std::vector<Pt2i> &dispix, bool reversed);

  /**
   * \brief Starts the carriage track with a central plateau, scan and profile.
   * @param pl Central plateau to be added.
   * @param dispix Scan used for display.
   * @param pts Height profile extracted from detection scan.
   * @param reversed Reversed scan modality (P2 -> P1).
   */
  void start (Plateau *pl, const std::vector<Pt2i> &dispix,
              const std::vector<Pt2f> &pts, bool reversed);

  /**
   * \brief Adds a plateau to the track section and displayed scan.
   * @param onright Indicates if adding deals with right section.
   * @param pl Plateau to be added.
   * @param dispix Scan used for display.
   */
  void add (bool onright, Plateau *pl, const std::vector<Pt2i> &dispix);

  /**
   * \brief Adds a plateau to the track section, scan and profile.
   * @param onright Indicates if adding deals with right section.
   * @param pl Plateau to be added.
   * @param dispix Scan used for display.
   * @param pts Height profile extracted from detection scan.
   */
  void add (bool onright, Plateau *pl, const std::vector<Pt2i> &dispix,
                                       const std::vector<Pt2f> &pts);

  /**
   * \brief Returns the number of accepted plateaux.
   */
  int getAcceptedCount () const;

  /**
   * \brief Returns the number of plateaux on right side.
   */
  int getRightScanCount () const;

  /**
   * \brief Returns the number of plateaux on left side.
   */
  int getLeftScanCount () const;

  /**
   * \brief Indicates whether the scans of a plateau are reversed.
   * @param num plateau number.
   */
  bool isScanReversed (int num) const;

  /**
   * \brief Returns a pointer to a scan to be displayed.
   * @param num plateau number.
   */
  std::vector<Pt2i> *getDisplayScan (int num);

  /**
   * \brief Returns one of the carriage track plateaux.
   * Returns NULL if the plateau is not instanciated.
   * @param num plateau number.
   */
  Plateau *plateau (int num) const;

  /**
   * \brief Sets a plateau as accepted.
   * @param num plateau number.
   */
  void accept (int num);

  /**
   * \brief Returns the height reference of a plateau.
   * This reference is the lower altitude of the plateau.
   * Returns 0 if the plateau is not successfully detected.
   * @param num plateau number.
   */
  float getHeightReference (int num) const;

  /**
   * \brief Provides the last valid plateau from given position.
   * The last valid plateau is searched towards the central plateau.
   * @param num Relative index of the plateau (negative = right).
   */
  Plateau *lastValidPlateau (int num) const;

  /**
   * \brief Returns a point profile to be displayed.
   * @param num Relative index of the plateau (negative = right).
   */
  std::vector<Pt2f> *getProfile (int num);

  /**
   * \brief Fills a vector with all the carriage track points.
   * @param pts Pointer to a vector to fill with carriage track points.
   * @param acc Plateau acceptation modality.
   * @param imw Detection image width.
   * @param imh Detection image height.
   * @param iratio Image to meter ratio : inverse of cell size.
   */
  void getPoints (std::vector<Pt2i> *pts, bool acc,
                  int imw, int imh, float iratio);

  /**
   * \brief Fills a vector with all the carriage track points for each plateau.
   * @param pts Pointer to a vector to fill with carriage track point lines.
   * @param acc Plateau acceptation modality.
   * @param imw Detection image width.
   * @param imh Detection image height.
   * @param iratio Image to meter ratio : inverse of cell size.
   */
  void getPoints (std::vector<std::vector<Pt2i> > *pts, bool acc,
                  int imw, int imh, float iratio);

  /**
   * \brief Returns the connected carriage track points.
   * @param pts Pointer to a vector to fill with carriage track points in.
   * @param acc Plateau acceptation modality.
   * @param imw Detection image width.
   * @param imh Detection image height.
   * @param iratio Image to meter ratio : inverse of cell size.
   */
  void getConnectedPoints (std::vector<Pt2i> *pts, bool acc,
                           int imw, int imh, float iratio);

  /**
   * \brief Returns the connected carriage track points for each plateau.
   * @param pts Pointer to a vector to fill with carriage track point lines.
   * @param acc Plateau acceptation modality.
   * @param imw Detection image width.
   * @param imh Detection image height.
   * @param iratio Image to meter ratio : inverse of cell size.
   */
  void getConnectedPoints (std::vector<std::vector<Pt2i> > *pts,
                           bool acc, int imw, int imh, float iratio);

  /**
   * \brief Prunes track tails plateaux.
   * The last sequence of successive plateaux should have a minimal size.
   * Returns whether all plateaux have been refused.
   * @param tailMinSize Minimal size of last sequence of successive plateaux.
   */
  bool prune (int tailMinSize);

  /**
   * \brief Returns the last plateau number on left side.
   */
  int leftEnd ();

  /**
   * \brief Returns the last plateau POSITIVE number on right side.
   */
  int rightEnd ();

  /**
   * \brief Returns the spread between last accepted plateaux on each side.
   */
  int spread ();

  /**
   * \brief Returns the count of refused plateaux before last ones.
   */
  int nbHoles ();

  /**
   * \brief Returns the relative length of absolute shifts.
   */
  float relativeShiftLength () const;

  /**
   * \brief Adds plateau positions to given vectors.
   * The plateau position is an estimate of summit or center point.
   * The second vector is used for the end positions of intervals (start / end).
   * @param pts Vector to complete with plateau positions (or start positions).
   * @param pts2 Vector to complete with plateau interval end positions.
   * @param disp Display style.
   * @param iratio Meter to DTM pixel ratio.
   * @param smoothed Plateau accept modality.
   */
  void getPosition (std::vector<Pt2i> &pts, std::vector<Pt2i> &pts2,
                    int disp, float iratio, bool smoothed);

  /**
   * \brief Returns scan shift for given side position of the center.
   * @param pcenter Side position of the center.
   */
  int scanShift (float pcenter);


private :

  /** Detection status. */
  int status;
  /** Central section. */
  CTrackSection startsec;
  /** Track sections on right side. */
  std::vector<CTrackSection *> rights;
  /** Track sections on left side. */
  std::vector<CTrackSection *> lefts;
  /** Current right section. */
  CTrackSection *curright;
  /** Current left section. */
  CTrackSection *curleft;

  /** Detection seed first input point in DTM pixels. */
  Pt2i seed_p1;
  /** Detection seed second input point in DTM pixels. */
  Pt2i seed_p2;
  /** Detection seed length in DTM pixels (not to recompute it regularly). */
  float seed_length;
  /** DTM cell size: pixel to cloud point (meter) ratio. */
  float cell_size;


  /**
   * \brief Adds a plateau center to the given vector of points.
   * @param pt Vector of point to complete with the center point.
   * @param num Scan index.
   * @param rev Status of the scan reversion modality.
   * @param pp1 Central scan start position (in DTM reference).
   * @param p12 Central scan director vector (in DTM reference).
   * @param l12 Central scan length (in DTM reference).
   * @param iratio Meter to DTM pixel ratio.
   * @param smoothed Plateau accept modality.
   */
  void addPlateauCenter (std::vector<Pt2i> &pt, int num, bool rev,
                         Pt2i pp1, Vr2i p12, float l12,
                         float iratio, bool smoothed);

  /**
   * \brief Adds a plateau center to the given vector of points.
   * @param spt Vector of point to complete with the start point.
   * @param ept Vector of point to complete with the end point.
   * @param num Scan index.
   * @param rev Status of the scan reversion modality.
   * @param pp1 Central scan start position (in DTM reference).
   * @param p12 Central scan director vector (in DTM reference).
   * @param l12 Central scan length (in DTM reference).
   * @param iratio Meter to DTM pixel ratio.
   * @param smoothed Plateau accept modality.
   */
  void addPlateauBounds (std::vector<Pt2i> &spt, std::vector<Pt2i> &ept2,
                         int num, bool rev, Pt2i pp1, Vr2i p12, float l12,
                         float iratio, bool smoothed);

};
#endif
