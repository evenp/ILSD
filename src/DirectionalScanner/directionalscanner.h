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

#ifndef DIRECTIONAL_SCANNER_H
#define DIRECTIONAL_SCANNER_H

#include "pt2i.h"


/** 
 * @class DirectionalScanner directionalscanner.h
 * \brief Incremental directional scanner.
 * This scanner iterately provides parallel scan lines.
 */
class DirectionalScanner
{
public:
  
  /**
   * \brief Deletes the directional scanner.
   */
  virtual ~DirectionalScanner ();

  /**
   * \brief Returns a copy of the directional scanner.
   */
  virtual DirectionalScanner *getCopy () = 0;

  /**
   * \brief Gets the central scan in a vector.
   * Adds central scan points to given vector and returns its new size.
   * @param scan Vector of points to be completed.
   */
  virtual int first (std::vector<Pt2i> &scan) const = 0;

  /**
   * \brief Gets next scan on the left in a vector.
   * Adds points of next left scan to given vector and returns its new size.
   * @param scan Vector of points to be completed.
   */
  virtual int nextOnLeft (std::vector<Pt2i> &scan) = 0;

  /**
   * \brief Gets next scan on the right in a vector.
   * Adds points of next right scan to given vector and returns its new size.
   * @param scan Vector of points to be completed.
   */
  virtual int nextOnRight (std::vector<Pt2i> &scan) = 0;

  /**
   * \brief Gets next skipped scan to the left in a vector.
   * Adds points of next left scan to given vector and returns its new size.
   * @param scan Vector of points to be completed.
   * @param skip Skip length.
   */
  virtual int skipLeft (std::vector<Pt2i> &scan, int skip) = 0;

  /**
   * \brief Gets next skipped scan to the right in a vector.
   * Adds points of next right scan to given vector and returns its new size.
   * @param scan Vector of points to be completed.
   * @param skip Skip length.
   */
  virtual int skipRight (std::vector<Pt2i> &scan, int skip) = 0;

  /**
   * \brief Binds the scan strip to wrap the given digital line.
   * Resets bounding lines parameters to center the scan strip on given line.
   * @param a New value for the 'a' parameter of current scan strip.
   * @param b New value for the 'b' parameter of current scan strip.
   * @param c New value for the 'c' parameter of current scan strip.
   */
  virtual void bindTo (int a, int b, int c);

  /**
   * \brief Returns the scanner coordinates of given point.
   * Scanner coordinates are the scan index and the position in the scan.
   * @param pt Image coordinates of the point.
   */
  virtual Pt2i locate (const Pt2i &pt) const;

  /**
   * \brief Releases clearance status of output vector before filling.
   */
  inline void releaseClearance () { clearance = false; }


protected:

  /** Scanable area left border. */
  int xmin;
  /** Scanable area bottom border. */
  int ymin;
  /** Scanable area right border. */
  int xmax;
  /** Scanable area top border. */
  int ymax;

  /** Parameter 'a' of the upper support discrete line. */
  int dla;
  /** Parameter 'b' of the upper support discrete line. */
  int dlb;
  /** Parameter 'c' of the upper support discrete line. */
  int dlc2;

  /** Size of the discrete line pattern. */
  int nbs;

  /** Discrete line pattern. */
  bool *steps;
  /** Pointer to the end of discrete line pattern. */
  bool *fs;

  /** X-start position of central scan (still used in locate (Pt2i)). */
  int ccx;
  /** Y-start position of central scan (still used in locate (Pt2i)). */
  int ccy;
  /** X-start position of last scan to the left. */
  int lcx;
  /** Y-start position of last scan to the left. */
  int lcy;
  /** X-start position of last scan to the right. */
  int rcx;
  /** Y-start position of last scan to the right. */
  int rcy;

  /** Current pattern step in scan line direction for left scans. */
  bool *lst2;
  /** Current pattern step in scan line direction for right scans. */
  bool *rst2;

  /** Flag indicating if the output vector should be cleared before filling.
      Set to true by default. */
  bool clearance;


  /**
   * \brief Creates an empty directional scanner.
   */
  DirectionalScanner () { }

  /**
   * \brief Creates an incremental directional scanner.
   * Creates a directional scanner from pattern and start.
   * The scan strip is composed of parallel scan lines, the first one being
   *   defined by a start point and a line pattern.
   * @param xmini Left border of the scan area.
   * @param ymini Bottom border of the scan area.
   * @param xmaxi Right border of the scan area.
   * @param ymaxi Top border of the scan area.
   * @param nb Size of the support line pattern.
   * @param st Support line pattern.
   * @param sx X-coordinate of the central scan start point.
   * @param sy Y-coordinate of the central scan start point.
   */
  DirectionalScanner (int xmini, int ymini, int xmaxi, int ymaxi,
                      int nb, bool* st, int sx, int sy)
             : xmin (xmini), ymin (ymini), xmax (xmaxi), ymax (ymaxi),
               nbs (nb), steps (st),
               ccx (sx), ccy (sy), lcx (sx), lcy (sy), rcx (sx), rcy (sy),
               clearance (true) { }

  /**
   * \brief Creates a copy of given directional scanner.
   * @param ds Source directional scanner.
   */
  DirectionalScanner (DirectionalScanner *ds)
         : xmin (ds->xmin), ymin (ds->ymin), xmax (ds->xmax), ymax (ds->ymax),
           dla (ds->dla), dlb (ds->dlb), dlc2 (ds->dlc2),
           nbs (ds->nbs), steps (ds->steps), fs (ds->fs),
           ccx (ds->ccx), ccy (ds->ccy),
           lcx (ds->lcx), lcy (ds->lcy), rcx (ds->rcx), rcy (ds->rcy),
           lst2 (ds->lst2), rst2 (ds->rst2), clearance (ds->clearance) { }

};
#endif
