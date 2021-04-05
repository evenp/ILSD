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

#ifndef VR2I_H
#define VR2I_H


/** 
 * @class Vr2i vr2i.h
 * \brief Vector in the digital plane.
 */
class Vr2i
{
public:

  /**
   * \brief Creates a unit vector on X axis: (1,0).
   */
  Vr2i ();

  /**
   * \brief Creates a vector using coordinate values.
   * @param x X-coordinate value.
   * @param y Y-coordinate value.
   */
  Vr2i (int x, int y);

  /**
   * \brief Creates a copy from a given vector.
   * @param v Original vector to copy.
   */
  Vr2i (const Vr2i &v);

  /**
   * \brief Returns the X-coordinate value.
   */
  inline int x () const { return xv; }

  /**
   * \brief Returns the Y-coordinate value.
   */
  inline int y () const { return yv; }

  /**
   * \brief Sets the vector coordinate values.
   * @param x New value for X-coordinate.
   * @param y New value for Y-coordinate.
   */
  inline void set (int x, int y) { xv = x; yv = y; }

  /**
   * \brief Sets vector value on another vector.
   * @param vec Vector to copy.
   */
  inline void set (const Vr2i &vec) { xv = vec.xv; yv = vec.yv; }

  /**
   * \brief Returns the squared norm of the vector.
   * If intensity value holds on a byte, gradient holds on a short
   *   and gradient squared norm holds on an int.
   */
  inline int norm2 () const { return (xv * xv + yv * yv); }

  /**
   * \brief Returns the scalar product with given vector.
   * If intensity value holds on a byte, scalar product (SP) holds on a short
   *   and squared SP holds on a int.
   * @param vec Given vector.
   */
  inline int scalarProduct (Vr2i vec) const {
    return (xv * vec.xv + yv * vec.yv); }

  /**
   * \brief Returns the squared scalar product with given vector.
   * @param vec The given vector.
   */
  inline int squaredScalarProduct (Vr2i vec) const {
    return ((xv * vec.xv + yv * vec.yv)
            * (xv * vec.xv + yv * vec.yv)); }

  /**
   * \brief Returns if a given vector is on the left side of this vector.
   * @param vec Given vector.
   */
  inline bool leftside (Vr2i vec) const {
    return (xv * vec.yv > yv * vec.xv); }

  /**
   * \brief Returns the squared norm of the vector product with given vector.
   * @param vec Given vector.
   */
  inline int squaredVectorProduct (Vr2i vec) const {
    return ((xv * vec.yv - yv * vec.xv) * (xv * vec.yv - yv * vec.xv)); }

  /**
   * \brief Checks equivalence to the given vector.
   * @param v Given vector.
   */
  inline bool equals (Vr2i v) const {
    return (v.xv == xv && v.yv == yv); }

  /**
   * \brief Returns the manhattan length of the vector.
   */
  inline int manhattan () const {
    return ((xv > 0 ? xv : - xv) + (yv > 0 ? yv : - yv)); }

  /**
   * \brief Returns the chessboard length of the vector.
   */
  inline int chessboard () const {
    int x = (xv < 0 ? -xv : xv), y = (yv < 0 ? -yv : yv);
    return (x > y ? x : y); }

  /**
   * \brief Returns the CCW orthogonal vector.
   */
  Vr2i orthog () const;

  /**
   * \brief Sets the vector to the CCW orthogonal vector.
   */
  inline void setOrthog () { int tmp = xv; xv = - yv; yv = tmp; }

  /**
   * \brief Checks whether this vector has same direction as a reference vector.
   * @param ref Reference vector.
   */
  inline bool directedAs (const Vr2i &ref) const {
    return (xv * ref.xv + yv * ref.yv >= 0); }

  /**
   * \brief Checks whether this vector has the same orientation as a reference.
   * @param ref Reference vector.
   */
  bool orientedAs (const Vr2i &ref) const;

  /**
   * \brief Sets the vector to its opposite.
   */
  inline void invert () { xv = -xv; yv = -yv; }

  /**
   * \brief Returns the location of the steps between the vector ends.
   * DEPRECATED METHOD: DON'T USE ANY MORE. Use Pt2i::stepsTo (Pt2i) instead.
   * @param n Size of the returned array.
   */
  bool *steps (int *n) const;


private:

  /** Vector X-coordinate value. */
  int xv;
  /** Vector Y-coordinate value. */
  int yv;
};

#endif
