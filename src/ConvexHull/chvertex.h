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

#ifndef CHVERTEX_H
#define CHVERTEX_H

#include "pt2i.h"


/** 
 * @class CHVertex chvertex.h
 * \brief Chained vertex with two adjacent points, on left and right.
 */
class CHVertex : public Pt2i
{
public:

  /**
   * \brief Builds a default vertex.
   */
  CHVertex ();

  /**
   * \brief Builds a vertex on given coordinates.
   * @param x First ccordinate value.
   * @param y Second ccordinate value.
   */
  CHVertex (int x, int y);

  /**
   * \brief Builds a vertex at position of given point.
   * @param p Reference to given point.
   */
  CHVertex (const Pt2i &p);

  /**
   * \brief Deletes the vertex.
   */
  ~CHVertex ();

  /**
   * \brief Returns a pointer to adjacent vertex on left side.
   */
  inline CHVertex *left () const { return lv; }

  /**
   * \brief Returns a pointer to adjacent vertex on right side.
   */
  inline CHVertex *right () const { return rv; }

  /**
   * \brief Sets adjacent vertex on left side.
   * @param v Pointer to new adjacent vertex.
   */
  inline void setLeft (CHVertex *v) { lv = v; }

  /**
   * \brief Sets adjacent vertex on right side.
   * @param v Pointer to new adjacent vertex.
   */
  inline void setRight (CHVertex *v) { rv = v; }

  /**
   * \brief Returns the cross product of vector (pt - this) and vector (vx, vy).
   * The cross product of U and V is defined by: CP = Ux * Vy - Vx * Uy.
   * @param pt Given end point.
   * @param vx First coordinate of given vector.
   * @param vy Second coordinate of given vector.
   */
  inline int vprod (CHVertex *pt, int vx, int vy) const {
    return ((pt->xp - xp) * vy - vx * (pt->yp - yp)); }

  /**
   * \brief Returns the cross product of vector (p2 - this) and vector (p4 - p3)
   * The cross product of U and V is defined by: CP = Ux * Vy - Vx * Uy.
   * @param p2 First end point.
   * @param p3 Second start point.
   * @param p4 Second end point.
   */
  inline int vprod (CHVertex *p2, CHVertex *p3, CHVertex *p4) const {
    return ((p2->xp - xp) * (p4->yp - p3->yp)
            - (p4->xp - p3->xp) * (p2->yp - yp)); }

  // friend ostream& operator<< (ostream &os, const CHVertex &v);


protected:

  /** Adjacent vertex on left side. */
  CHVertex *lv;
  /** Adjacent vertex on right side. */
  CHVertex *rv;

};
#endif
