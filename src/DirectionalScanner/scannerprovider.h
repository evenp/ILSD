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

#ifndef SCANNER_PROVIDER_H
#define SCANNER_PROVIDER_H

#include "directionalscanner.h"


/** 
 * @class ScannerProvider scannerprovider.h
 * \brief Directional scanner provider.
 * Provides ad-hoc directional scanners in the relevant octant
 *   and according to static or dynamical control.
 */
class ScannerProvider
{
public:

  /**
   * \brief Builds a directional scanner provider.
   */
  ScannerProvider () : isOrtho (false), last_scan_reversed (false),
                       xmin (0), ymin (0), xmax (100), ymax (100) { }
  
  /**
   * \brief Sets the scanned area size.
   * @param sizex Scan area width.
   * @param sizey Scan area height.
   */
   void setSize (int sizex, int sizey) {
     xmax = xmin + sizex; ymax = ymin + sizey; }

  /**
   * \brief Sets the scanned area size.
   * @param x0 Left column coordinate of the scan area.
   * @param y0 Lower line coordinate of the scan area.
   * @param sizex Scan area width.
   * @param sizey Scan area height.
   */
   void setArea (int x0, int y0, int sizex, int sizey) {
     xmin = x0, ymin = y0, xmax = x0 + sizex; ymax = y0 + sizey; }

  /**
   * \brief Returns a directional scanner from initial scan end points.
   * Returns a directional scanner from two control points.
   * The scan strip is composed of parallel scans (line segments),
   *   the initial one being defined by control points p1 and p2.
   * @param p1 Initial scan start point.
   * @param p2 Initial scan end point.
   * @param controlable Control modality (true for an adaptive scanner).
   */
  DirectionalScanner *getScanner (Pt2i p1, Pt2i p2, bool controlable = false);
  
  /**
   * \brief Returns a directional scanner from scan center, vector and length.
   * The scan strip is composed of parallel scans (line segments),
   *   the first one defined by its center, its direct vector, and its length.
   * @param centre Initial scan center.
   * @param normal Initial scan director vector.
   * @param length Initial scan length.
   * @param controlable Control modality (true for an adaptive scanner).
   */
  DirectionalScanner *getScanner (Pt2i centre, Vr2i normal,
                                  int length, bool controlable = false);

  /**
   * \brief Returns whether the currently used scan end points were permutated.
   */
  inline bool isLastScanReversed () const { return last_scan_reversed; }

  /**
   * \brief Sets the orthogonal scanner modality.
   * @param status New status for the orthogonal scanner modality.
   */
  inline void setOrtho (bool status) { isOrtho = status; }


private:

  /** Orthogonal scanner modality. */
  bool isOrtho;
  /** Last scan end points permutation modality. */
  bool last_scan_reversed;

  /** Scan area lowest x coordinate. */
  int xmin;
  /** Scan area lowest y coordinate. */
  int ymin;
  /** Scan area highest x coordinate. */
  int xmax;
  /** Scan area highest y coordinate. */
  int ymax;

};
#endif
