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

#ifndef PT3F_H
#define PT3F_H


/** 
 * @class Pt3f pt3f.h
 * \brief Point in euclidean space.
 */
class Pt3f
{
public:

  /**
   * \brief Creates a point at origin.
   */
  Pt3f ();

  /**
   * \brief Creates a point from coordinates values.
   * @param x X-coordinate value.
   * @param y Y-coordinate value.
   * @param z Z-coordinate value.
   */
  Pt3f (float x, float y, float z);

  /**
   * \brief Creates a copy of given point.
   * @param p Original point.
   */
  Pt3f (const Pt3f &p);

  /**
   * \brief Deletes the point.
   */
  ~Pt3f () { }

  /**
   * \brief Returns the X-coordinate value.
   */
  inline float x () const { return xp; }

  /**
   * \brief Returns the Y_coordinate value.
   */
  inline float y () const { return yp; }

  /**
   * \brief Returns the Z-coordinate value.
   */
  inline float z () const { return zp; }

  /**
   * \brief Returns the nth coordinate value.
   * @param n Coordinate index.
   */
  inline float get (int n) const {
    return (n ? (n == 1 ? yp : zp) : xp); }

  /**
   * \brief Sets the point coordinates.
   * @param x New X-coordinate value.
   * @param y New Y-coordinate value.
   * @param z New Z-coordinate value.
   */
  inline void set (float x, float y, float z) {
    xp = x; yp = y; zp = z; }

  /**
   * \brief Sets the point coordinates from another point.
   * @param p Point to recopy.
   */
  inline void set (const Pt3f &p) {
    xp = p.xp; yp = p.yp; zp = p.zp; }

  /**
   * \brief Checks equivalence to given point up to epsilon.
   * @param p Given point.
   * @param eps Test value.
   */
  inline bool equals (const Pt3f &p, float eps = 0.0001) const {
    return (p.xp - xp < eps && xp - p.xp < eps
            && p.yp - yp < eps && yp - p.yp < eps
            && p.zp - zp < eps && zp - p.zp < eps); }

  /**
   * \brief Returns distance to another point.
   * @param p Given point.
   */
  float distance (const Pt3f &p) const;

  /**
   * \brief Checks if the point lies above a given point.
   * @param p Given point.
   * @param eps Test value.
   */
  inline bool vertical (const Pt3f &p, float eps = 0.0001) const {
    return (p.xp - xp < eps && xp - p.xp < eps
            && p.yp - yp < eps && yp - p.yp < eps); }

  /**
   * @fn bool horizontal (const Pt3f &p)
   * \brief Checks if the point lies at same height than a given point.
   * @param p Given point.
   * @param eps Test value.
   */
  inline bool horizontal (const Pt3f &p, float eps = 0.0001) const {
    return (p.zp - zp < eps && zp - p.zp < eps); }

  /**
   * \brief Returns true if the point is greater than the given point.
   * Greater means greater X value, then greater Y value (if same X values),
   *   and then greater Z value (if same X and Y values).
   * @param p Given point.
   */
  bool greaterThan (const Pt3f &p) const;

  /**
   * \brief Returns true if the point if further (only X/Y) than given point.
   * Further means greater Y value, then greater X value (if same Y values).
   * @param p Given point.
   */
  inline bool furtherThan (const Pt3f &p) const {
    return (yp > p.yp || (yp == p.yp && xp > p.xp)); }

  /**
   * \brief Normalizes the point. ???
   */
  void normalize ();

  /**
   * \brief Returns the scalar product with a given point.
   * @param p Given point.
   */
  inline float scalar (const Pt3f &p) const {
    return (p.xp * xp + p.yp * yp + p.zp * zp); }


protected:

  /** Point X-coordinate value. */
  float xp;
  /** Point Y-coordinate value. */
  float yp;
  /** Point Z-coordinate value. */
  float zp;
};

#endif
