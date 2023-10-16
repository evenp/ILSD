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

#ifndef PT3I_H
#define PT3I_H


/** 
 * @class Pt pt3i.h
 * \brief 3D integer point.
 */
class Pt3i
{
public:

  /**
   * \brief Creates a point at origin.
   */
  Pt3i ();

  /**
   * \brief Creates a point from coordinates values.
   * @param x X-coordinate value.
   * @param y Y-coordinate value.
   * @param z Z-coordinate value.
   */
  Pt3i (int x, int y, int z);

  /**
   * \brief Creates a copy of a given point.
   * @param p Original point.
   */
  Pt3i (const Pt3i &p);

  /**
   * \brief Deletes the point.
   */
  ~Pt3i () { }

  /**
   * \brief Returns the X-coordinate value.
   */
  inline int x () const { return xp; }

  /**
   * \brief Returns the Y-coordinate value.
   */
  inline int y () const { return yp; }

  /**
   * \brief Returns the Z-coordinate value.
   */
  inline int z () const { return zp; }

  /**
   * \brief Returns true if the point is greater than a given point.
   * Greater means greater X value, then greater Y value (if same X values),
   *   and then greater Z value (if same X and Y values).
   * @param p Given point.
   */
  bool greaterThan (const Pt3i &p) const;

  /**
   * \brief Returns true if the point if further (only X/Y) than a given point.
   * Further means greater Y value, then greater X value (if same Y values).
   * @param p Given point.
   */
  inline bool furtherThan (const Pt3i &p) const {
    return (yp > p.yp || (yp == p.yp && xp > p.xp)); }

  /**
   * \brief Returns the found flag. ???
   */
  inline bool found () const { return (nb != 0); }

  /**
   * \brief Returns the found flag. ???
   */
  inline bool refound () const { return (nb > 1); }

  /**
   * \brief Returns the point nth coordinate value.
   * @param n Coordinate index.
   */
  inline int get (int n) const {
    return (n == 2 ? zp : (n == 1 ? yp : xp)); }

  /**
   * \brief Sets the point coordinates.
   * @param x New X-coordinate value.
   * @param y New Y-coordinate value.
   * @param z New Z-coordinate value.
   */
  inline void set (int x, int y, int z) { xp = x; yp = y; zp = z; }

  /**
   * \brief Sets the point coordinates from another point.
   * @param p Point to recopy.
   */
  inline void set (const Pt3i &p) { xp = p.xp; yp = p.yp; zp = p.zp; }

  /**
   * \brief Checks equivalence to a given point.
   * @param p Given point.
   */
  inline bool equals (const Pt3i &p) const {
    return (p.xp == xp && p.yp == yp && p.zp == zp); }

  /**
   * \brief Checks if the point lies above a given point.
   * @param p Given point.
   */
  inline bool vertical (const Pt3i &p) const {
    return (p.xp == xp && p.yp == yp); }

  /**
   * \brief Checks if the point lies at same height than a given point.
   * @param p Given point.
   */
  inline bool horizontal (const Pt3i &p) const { return (p.zp == zp); }

  /**
   * \brief Marks as found if x-y equivalent to a given point.
   * @param p Given point.
   */
  bool find (Pt3i p);


protected:

  /** Point X-coordinate value. */
  int xp;
  /** Point Y-coordinate value. */
  int yp;
  /** Point Z-coordinate value. */
  int zp;
  /** Number of clones. */
  int nb;

};
#endif
