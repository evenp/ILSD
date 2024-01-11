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
 *   with possible orthographic or adaption modalities.
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
   * \brief Sets the scanned area.
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
   * @param adaptive Directional scanner adaption modality.
   */
  DirectionalScanner *getScanner (Pt2i p1, Pt2i p2, bool adaptive = false);
  
  /**
   * \brief Returns a directional scanner from scan center, vector and length.
   * The scan strip is composed of parallel scans (line segments),
   *   the first one defined by its center, its direct vector, and its length.
   * @param centre Initial scan center.
   * @param normal Initial scan director vector.
   * @param length Initial scan length.
   * @param adaptive Directional scanner adaption modality.
   */
  DirectionalScanner *getScanner (Pt2i centre, Vr2i normal,
                                  int length, bool adaptive = false);

  /**
   * \brief Returns whether the input vector (P1P2 or normal) has been reversed.
   */
  inline bool isLastScanReversed () const { return last_scan_reversed; }

  /**
   * \brief Sets the orthographic scanner modality (aligned to main directions).
   * @param status New status for the orthographic scanner modality.
   */
  inline void setOrtho (bool status) { isOrtho = status; }

  /**
   * \brief Returns whether input vector (P1P2 or normal) is reversed.
   * @param vec Vector used for directional scanner initialization (getScanner).
   */
  static bool isReversed (const Vr2i &vec);


private:

  /** Orthographic scanner modality (scans aligned to main directions). */
  bool isOrtho;
  /** Input vector permutation status. */
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
