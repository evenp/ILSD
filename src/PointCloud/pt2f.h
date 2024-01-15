/*  Copyright 2021 Philippe Even, Phuc Ngo and Pierre Even,
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

#ifndef PT2F_H
#define PT2F_H

#include "vr2f.h"


/** 
 * @class Pt2f pt2f.h
 * \brief Point in euclidean plane.
 */
class Pt2f
{
public:

  /**
   * \brief Creates a point at origin.
   */
  Pt2f ();

  /**
   * \brief Creates a point from coordinate values.
   * @param x X-coordinate value.
   * @param y Y-coordinate value.
   */
  Pt2f (float x, float y);

  /**
   * \brief Creates a copy of a given point.
   * @param p Original point.
   */
  Pt2f (const Pt2f &p);

  /**
   * \brief Deletes the point.
   */
  ~Pt2f () { }

  /**
   * \brief Returns the X-coordinate value.
   */
  inline float x () const
  {
    return xp;
  }

  /**
   * \brief Returns the Y-coordinate value.
   */
  inline float y () const
  {
    return yp;
  }

  /**
   * \brief Returns the nth coordinate.
   * @param n Coordinate index.
   */
  inline float get (int n) const
  {
    return (n ? yp : xp);
  }

  /**
   * \brief Sets the point coordinates.
   * @param x New X-coordinate value.
   * @param y New Y-coordinate value.
   */
  inline void set (float x, float y)
  {
    xp = x;
    yp = y;
  }

  /**
   * \brief Sets the point coordinates.
   * @param p Point to copy.
   */
  inline void set (const Pt2f &p)
  {
    xp = p.xp;
    yp = p.yp;
  }

  /**
   * \brief Sets the point Y coordinate.
   * @param y New Y-coordinate value.
   */
  inline void setY (float y) { yp = y; }

  /**
   * \brief Checks equivalence to given point up to epsilon.
   * @param p Given point.
   * @param eps Test value.
   */
  inline bool equals (Pt2f p, float eps = 0.0001) const
  {
    return (p.xp - xp < eps && xp - p.xp < eps
            && p.yp - yp < eps && yp - p.yp < eps);
  }

  /**
   * \brief Normalizes the point.
   */
  void normalize ();

  /**
   * \brief Returns distance to another point.
   * @param p Given point.
   */
  float distance (const Pt2f &p) const;

  /**
   * \brief Returns the discrete vector to given point.
   * @param p Given point.
   */
  Vr2f vectorTo (Pt2f p) const;


protected:

  /** Point X-coordinate. */
  float xp;
  /** Point Y-coordinate. */
  float yp;
};

#endif
