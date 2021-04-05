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

#ifndef CARRIAGE_TRACK_SECTION_H
#define CARRIAGE_TRACK_SECTION_H

#include "plateau.h"
#include "pt2i.h"
#include "pt2f.h"


/** 
 * @class CTrackSection ctracksection.h
 * \brief Carriage track section.
 */
class CTrackSection
{
public:

  /**
    \brief Creates a new carriage track section.
   */
  CTrackSection ();

  /**
   * \brief Deletes the carriage track section.
   */
  ~CTrackSection ();

  /**
   * \brief Adds a plateau to the track section with displayed scan.
   * @param pl Plateau to be added.
   * @param dispix Scan used for display.
   */
  void add (Plateau *pl, const std::vector<Pt2i> &dispix);

  /**
   * \brief Adds a plateau to the track section.
   * @param pl Plateau to be added.
   * @param dispix Scan used for display.
   * @param pts Height profile extracted from detection scan.
   */
  void add (Plateau *pl, const std::vector<Pt2i> &dispix,
                         const std::vector<Pt2f> &pts);

  /**
   * \brief Returns the number of tracked plateaux (successful or not).
   */
  inline int getScanCount () const { return ((int) (plateaux.size ())); }

  /**
   * \brief Return one of the carriage track plateaux.
   * Returns NULL if the plateau is not instanciated.
   * @param num Index of the plateau.
   */
  Plateau *plateau (int num) const;

  /**
   * \brief Return a plateau height reference.
   * This reference is the lower altitude of the plateau.
   * Returns 0 if the first plateau is not successfully detected.
   * @param num Index of the plateau.
   */
  float getHeightReference (int num) const;

  /**
   * \brief Returns a pointer to a scan to be displayed.
   */
  inline std::vector<Pt2i> *getDisplayScan (int num) { return &(discans[num]); }

  /**
   * \brief Returns a point profile to be displayed.
   */
  inline std::vector<Pt2f> *getProfile (int num) {
    return (num < (int) (points.size ()) ? &(points[num]) : NULL); }

  /**
   * \brief Sets stored scans direction.
   * @param rev Indicates whether stored scans are reversed.
   */
  inline void setReversed (bool status) { reversed = status; }

  /**
   * \brief Indicates whether stored scans are reversed.
   */
  inline bool isReversed () const { return reversed; }

  /**
   * \brief Sets a plateau as accepted.
   */
  void accept (int num);

  /**
   * \brief Returns the count of accepted plateaux.
   */
  int getAcceptedCount () const;

  /**
   * \brief Prunes too small groups of successive plateaux on last part.
   * @param tailMinSize Minimal group size.
   */
  bool pruneDoubtfulTail (int tailMinSize);

  /**
   * \brief Returns the number of the last accepted plateau in the section.
   */
  inline int lastPlateau () const { return last; }

  /**
   * \brief Returns the number of the refused plateaux before the last one.
   */
  inline int nbHoles () const { return holes; }

  /**
   * \brief Return the length of absolute shifts.
   * @param cumlength Cumulated section length.
   */
  float shiftLength (int &cumlength) const;

  /**
   * \brief Gets the bounds of a stored DTM scan.
   * Returns false if scans are not stored or index is invalid.
   * @param ind Scan index.
   * @param p1 Point to set to scan start bound position.
   * @param p2 Point to set to scan end bound position.
   */
  bool getScanBounds (int ind, Pt2i &p1, Pt2i &p2);


private :

  /** Impacts ordered by distance. */
  std::vector<std::vector <Pt2f> > points;
  /** Detected plateaux. */
  std::vector<Plateau *> plateaux;

  /** Image scans for display. */
  std::vector<std::vector <Pt2i> > discans;
  /** Image scans inversion status. */
  bool reversed;
  /** Number of last accepted plateau in the section. */
  int last;
  /** Number of refused plateau before the last one in the section. */
  int holes;

  /**
   * \brief Clears data structures related to detection.
   */
  void clearDetectionData ();

  /**
   * \brief Clears data structures related to display.
   */
  void clearDisplayData ();

};
#endif
