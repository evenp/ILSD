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

#ifndef BIPT_LIST_H
#define BIPT_LIST_H

#include "pt2i.h"
#include "edist.h"
#include <deque>


/** 
 * @class BiPtList biptlist.h
 * \brief Bi-directional list of points.
 */
class BiPtList
{
public:

  /**
   * \brief Creates a extendable bi-directional list with one point.
   * @param pt Initial point of the list.
   */
  BiPtList (Pt2i pt);

  /**
   * \brief Deletes the bi-directional list.
   */
  ~BiPtList ();

  /**
   * \brief Returns the count of point in the bi-directional list.
   */
  inline int size () const { return (cpt); }

  /**
   * \brief Returns the count of point on back part of bi-directional list.
   */
  inline int backSize () const { return (cpt - start - 1); }

  /**
   * \brief Returns the count of point on front part of bi-directional list.
   */
  inline int frontSize () const { return (start); }

  /**
   * \brief Returns the initial point of the bi-directional list.
   */
  inline Pt2i initialPoint () const { return (pts[start]); }

  /**
   * \brief Returns the back end point of the bi-directional list.
   */
  inline Pt2i backPoint () const { return (pts.back ()); }

  /**
   * \brief Returns the front end point of the bi-directional list.
   */
  inline Pt2i frontPoint () const { return (pts.front ()); }

  /**
   * \brief Returns a point Manhattan height to the line between end points.
   * Manhattan height is the lowest value of horizontal or vertical height.
   * @param pt Tested point.
   */
  EDist heightToEnds (const Pt2i &pt) const;

  /**
   * \brief Adds a point on front side.
   * @param pt Point to add.
   */
  void addFront (Pt2i pt);

  /**
   * \brief Adds a point on back side.
   * @param pt Point to add.
   */
  void addBack (Pt2i pt);

  /**
   * \brief Removes points on front side.
   * @param n The amount of points to remove.
   */
  void removeFront (int n);

  /**
   * \brief Removes points on back side.
   * @param n The amount of points to remove.
   */
  void removeBack (int n);

  /**
   * \brief Sets given parameters with point min and max coordinates.
   * @param xmin Minimum X value to set.
   * @param ymin Minimum Y value to set.
   * @param xmax Maximum X value to set.
   * @param ymax Maximum Y value to set.
   */
  void findExtrema (int &xmin, int &ymin, int &xmax, int &ymax) const;

  /**
   * \brief Returns front to back points in a vector.
   */
  std::vector<Pt2i> frontToBackPoints () const;

  /**
   * \brief Returns a pointer to a empty vector of points.
   */
  std::vector<Pt2i> *emptyVector () const;

  /**
   * \brief Returns a pointer to a vector containing front points.
   * Front points are entered from segment edge to the initial point excluded.
   */
  std::vector<Pt2i> *frontPoints () const;

  /**
   * \brief Returns a pointer to a vector containing back points.
   * Back points are entered from initial point excluded to segment edge.
   */
  std::vector<Pt2i> *backPoints () const;


private:

  /** List of points. */
  std::deque<Pt2i> pts;
  /** Index of the initial point. */
  int start;
  /** Length of the point list. */
  int cpt;


  /**
   * \brief Returns a point X-height to the line between list end points.
   * X-height is the horizontal distance.
   * @param pt Tested point.
   */
  EDist xHeightToEnds (const Pt2i &pt) const;

  /**
   * \brief Returns a point Y-height to the line between list end points.
   * Y-height is the vertical distance.
   * @param pt Tested point.
   */
  EDist yHeightToEnds (const Pt2i &pt) const;

};
#endif
