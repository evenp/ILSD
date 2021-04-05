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

#ifndef RIDGE_SECTION_H
#define RIDGE_SECTION_H

#include "bump.h"
#include "pt2i.h"
#include "pt2f.h"


/** 
 * @class RidgeSection ridgesection.h
 * \brief Ridge structure section.
 */
class RidgeSection
{
public:

  /**
    \brief Creates a new ridge structure section.
   */
  RidgeSection ();

  /**
   * \brief Deletes the ridge section.
   */
  ~RidgeSection ();

  /**
   * \brief Adds a bump to the ridge section with displayed scan.
   * @param bump Bump to be added.
   * @param dispix Scan used for display.
   */
  void add (Bump *bump, const std::vector<Pt2i> &dispix);

  /**
   * \brief Adds a bump to the ridge section with scans and profiles.
   * @param bump Bump to be added.
   * @param dispix Scan used for display.
   * @param pts Height profile extracted from detection scan.
   */
  void add (Bump *bump, const std::vector<Pt2i> &dispix,
                        const std::vector<Pt2f> &pts);

  /**
   * \brief Returns the number of tracked bumps.
   */
  inline int getScanCount () const { return ((int) (bumps.size ())); }

  /**
   * \brief Return one of the ridge structure bumps.
   * Returns NULL if the bump is not instanciated.
   * @param num Index of the bump.
   */
  Bump *bump (int num) const;

  /**
   * \brief Return a bump height reference.
   * This reference is the lower altitude of the bump.
   * Returns 0 if the first bump is not successfully detected.
   * @param num Index of the bump.
   */
  float getHeightReference (int num) const;

  /**
   * \brief Returns a pointer to a scan to be displayed.
   * Returns NULL if not registered.
   * @num Index of the bump.
   */
  inline std::vector<Pt2i> *getDisplayScan (int num) {
    return &(discans[num]); }

  /**
   * \brief Returns a point profile to be displayed.
   * Returns NULL if not registered.
   * @num Index of the bump.
   */
  inline std::vector<Pt2f> *getProfile (int num) {
    return (num < (int) (points.size ()) ? &(points[num]) : NULL); }

  /**
   * \brief Sets stored scans direction.
   * @param status Indicates whether stored scans are reversed.
   */
  inline void setReversed (bool status) { reversed = status; }

  /**
   * \brief Indicates whether stored scans are reversed.
   */
  inline bool isReversed () const { return reversed; }

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
  std::vector<Bump *> bumps;

  /** Image scans for display. */
  std::vector<std::vector <Pt2i> > discans;
  /** Image scans inversion status. */
  bool reversed;


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
