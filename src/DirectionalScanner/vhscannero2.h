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

#ifndef VH_SCANNER_O2_H
#define VH_SCANNER_O2_H

#include "adaptivescannero2.h"


/** 
 * @class VHScannerO2 vhscannero2.h
 * \brief Vertical / horizontal adpative DS for the 2nd octant.
 */
class VHScannerO2 : public AdaptiveScannerO2
{
public:
  
  /**
   * \brief Creates a vh scanner from pattern, start and upper bound.
   * The scan strip is composed of parallel scan lines, the first one being
   *   defined by a start point, a line pattern, and an upper bound.
   * @param xmin Left border of the scan area.
   * @param ymin Bottom border of the scan area.
   * @param xmax Right border of the scan area.
   * @param ymax Top border of the scan area.
   * @param a Value of parameter 'a' of the discrete support line.
   * @param b Value of parameter 'b' of the discrete support line.
   * @param c Value of parameter 'c' of the upper bounding line.
   * @param nbs Size of the support line pattern.
   * @param steps Support line pattern.
   * @param sx X-coordinate of the central scan start point.
   * @param sy Y-coordinate of the central scan start point.
   */
  VHScannerO2 (int xmin, int ymin, int xmax, int ymax,
               int a, int b, int c,
               int nbs, bool *steps, int sx, int sy);

  /**
   * \brief Creates a vh scanner from pattern, center and bounds.
   * The scan strip is composed of parallel scan lines, the first one being
   *   defined by a center, a line pattern, upper and lower bounds.
   * @param xmin Left border of the scan area.
   * @param ymin Bottom border of the scan area.
   * @param xmax Right border of the scan area.
   * @param ymax Top border of the scan area.
   * @param a Value of parameter 'a' of the discrete support line.
   * @param b Value of parameter 'b' of the discrete support line.
   * @param c1 Value of parameter 'c' of one of the support lines.
   * @param c2 Value of parameter 'c' of the other support line.
   * @param nbs Size of the support line pattern.
   * @param steps Support line pattern.
   * @param cx X-coordinate of the central scan center.
   * @param cy Y-coordinate of the central scan center.
   */
  VHScannerO2 (int xmin, int ymin, int xmax, int ymax,
               int a, int b, int c1, int c2,
               int nbs, bool *steps, int cx, int cy);

  /**
   * \brief Creates a vh scanner from pattern, center and length.
   * The scan strip is composed of parallel scan lines, the first one being
   *   defined by a center, a line pattern, and a length value.
   * @param xmin Left border of the scan area.
   * @param ymin Bottom border of the scan area.
   * @param xmax Right border of the scan area.
   * @param ymax Top border of the scan area.
   * @param a Value of parameter 'a' of the discrete support line.
   * @param b Value of parameter 'b' of the discrete support line.
   * @param nbs Size of the support line pattern.
   * @param steps Support line pattern.
   * @param cx X-coordinate of the central scan center.
   * @param cy Y-coordinate of the central scan center.
   * @param length Length of a scan strip.
   */
  VHScannerO2 (int xmin, int ymin, int xmax, int ymax,
               int a, int b, int nbs, bool *steps,
               int cx, int cy, int length);

  /**
   * \brief Returns a copy of the directional scanner.
   */
  DirectionalScanner *getCopy ();

  /**
   * \brief Gets the central scan in a vector.
   * Adds central scan points to given vector and returns its new size.
   * @param scan Vector of points to be completed.
   */
  int first (std::vector<Pt2i> &scan) const;

  /**
   * \brief Gets the next scan on the left in a vector.
   * Adds points of next left scan to given vector and returns its new size.
   * @param scan Vector of points to be completed.
   */
  int nextOnLeft (std::vector<Pt2i> &scan);

  /**
   * \brief Gets the next scan on the right in a vector.
   * Adds points of next right scan to given vector and returns its new size.
   * @param scan Vector of points to be completed.
   */
  int nextOnRight (std::vector<Pt2i> &scan);

  /**
   * \brief Gets next skipped scan to the left in a vector.
   * Adds points of next left scan to given vector and returns its new size.
   * @param scan Vector of points to be completed.
   * @param skip Skip length.
   */
  int skipLeft (std::vector<Pt2i> &scan, int skip);

  /**
   * \brief Gets next skipped scan to the right in a vector.
   * Adds points of next right scan to given vector and returns its new size.
   * @param scan Vector of points to be completed.
   * @param skip Skip length.
   */
  int skipRight (std::vector<Pt2i> &scan, int skip);


private :

  /**
   * \brief Creates a copy of given directional scanner.
   * @param ds Source directional scanner.
   */
  VHScannerO2 (VHScannerO2 *ds);

};

#endif
