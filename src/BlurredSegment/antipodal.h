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

#ifndef ANTIPODAL_H
#define ANTIPODAL_H

#include "chvertex.h"
#include "edist.h"


/** 
 * @class Antipodal antipodal.h
 * \brief Horizontal or vertical antipodal pair of a polyline convex hull.
 */
class Antipodal
{
public:

  /**
   * \brief Builds an empty (undetermined) horizontal antipodal pair.
   */
  Antipodal ();

  /**
   * \brief Aligns the antipodal pair on vertical direction.
   */
  inline void setVertical () { ix = 1; iy = 0; }

  /**
   * \brief Initializes the vertex/edge pair from three unordered vertices.
   * @param v1 First vertex. 
   * @param v2 Second vertex.
   * @param v3 Third vertex.
   */
  void init (CHVertex *v1, CHVertex *v2, CHVertex *v3);

  /**
   * \brief Sets both vertex and edge of the antipodal pair.
   * @param pt New vertex.
   * @param es Start vertex of new edge.
   * @param ee End vertex of new edge.
   */
  inline void setVertexAndEdge (CHVertex *pt, CHVertex *es, CHVertex *ee) {
    vpt = pt; ept1 = es; ept2 = ee; }

  /**
   * \brief Sets the vertex of the antipodal pair.
   * @param pt New vertex.
   */
  inline void setVertex (CHVertex *pt) { vpt = pt; }

  /**
   * \brief Sets the edge of the antipodal pair.
   * @param es Start vertex of new edge.
   * @param ee End vertex of new edge.
   */
  inline void setEdge (CHVertex *es, CHVertex *ee) {
    ept1 = es; ept2 = ee; }

  /**
   * \brief Returns the vertex of the antipodal pair.
   */
  inline CHVertex *vertex () const { return vpt; }

  /**
   * \brief Returns the leaning edge start vertex of the antipodal pair.
   */
  inline CHVertex *edgeStart () const { return ept1; }

  /**
   * \brief Returns the leaning edge end vertex of the antipodal pair.
   */
  inline CHVertex *edgeEnd () const { return ept2; }

  /**
   * \brief Returns the antipodal pair horizontal thickness.
   * It is computed as the vertex horizontal distance to the edge.
   */
  EDist thickness () const;

  /**
   * \brief Computes the antipodal pair horizontal thickness.
   * It is the vertex horizontal distance to the edge.
   * @param num Numerator of the thickness rational value.
   * @param den Denominator of the thickness rational value.
   */
  //void thickness (int &num, int &den) const;

  /**
   * \brief Returns the remainder of the edge line equation for given vertex.
   * @param v Given vertex.
   */
  int remainder (CHVertex *v) const;

  /**
   * \brief Checks if the antipodal edge lies in first quadrant.
   * More formally, checks if sign(Ex) = sign(Ey).
   */
  bool edgeInFirstQuadrant () const;

  /**
   * \brief Returns the edge vector Y coordinate.
   */
  int getA () const;

  /**
   * \brief Returns the edge vector X coordinate.
   */
  int getB () const;

  /**
   * \brief Updates the antipodal pair after the insertion of a new vertex.
   * @param pt Pointer to inserted vertex.
   */
  void update (CHVertex *pt);

  /**
   * \brief Returns a string that represents the antipodal pair.
   */
  // friend ostream& operator<< (ostream &os, const Antipodal &ap);


protected:

  /** First coordinate (X for horizonal pair, Y for vertical pair). */
  int ix;
  /** Second coordinate (Y for horizonal pair, X for vertical pair). */
  int iy;

  /** Leaning vertex. */
  CHVertex *vpt;
  /** Start vertex of leaning edge. */
  CHVertex *ept1;
  /** End vertex of leaning edge. */
  CHVertex *ept2;

};
#endif
