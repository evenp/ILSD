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

#ifndef VR2F_H
#define VR2F_H


/** 
 * @class Vr2f vr2f.h
 * \brief Vector in the euclidean plane.
 */
class Vr2f
{
public:

  /**
   * \brief Creates a unit vector on X axis.
   */
  Vr2f ();

  /**
   * \brief Creates a vector from coordinate values.
   * @param x X-coordinate value.
   * @param y Y-coordinate value.
   */
  Vr2f (float x, float y);

  /**
   * \brief Creates a copy of a given vector.
   * @param v Original vector.
   */
  Vr2f (const Vr2f &v);

  /**
   * \brief Returns the X-coordinate value.
   */
  inline float x () const { return xv; }

  /**
   * \brief Returns the X-coordinate value.
   */
  inline float y () const { return yv; }

  /**
   * \brief Sets the vector coordinates.
   * @param x New X-coordinate value.
   * @param y New Y-coordinate value.
   */
  inline void set (float x, float y) { xv = x; yv = y; }

  /**
   * \brief Returns the squared norm of the vector.
   */
  inline float norm2 () const { return (xv * xv + yv * yv); }

  /**
   * \brief Returns the scalar product with given vector.
   * @param vec Given vector.
   */
  inline float scalarProduct (Vr2f vec) const {
    return (xv * vec.xv + yv * vec.yv); }

  /**
   * \brief Checks equivalence to given vector.
   * @param v Given vector.
   * @param eps Test value.
   */
  inline bool equals (Vr2f p, float eps = 0.0001) const {
    return (p.xv - xv < eps && xv - p.xv < eps
            && p.yv - yv < eps && yv - p.yv < eps); }

  /**
   * \brief Sets the vector to the CCW orthogonal vector.
   */
  inline void setOrthog () { int tmp = xv; xv = - yv; yv = tmp; }


private:

  /** Vector X-coordinate value. */
  float xv;
  /** Vector Y-coordinate value. */
  float yv;

};
#endif
