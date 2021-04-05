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

#ifndef DIGITAL_STRAIGHT_LINE_H
#define DIGITAL_STRAIGHT_LINE_H

#include "pt2i.h"
#include "edist.h"


/** 
 * @class DigitalStraightLine digitalstraightline.h
 * \brief Digital straight line.
 * The digital straightline is defined by equation : c <= ax + by < c + nu
 *   where a >= 0, b > 0 if a = 0, and nu > 0.
 */
class DigitalStraightLine
{
public:

  /** Digital line type : Thin line (width = 1). */
  static const int DSL_THIN;
  /** Digital line type : Naive line (width = max (|a|,|b|)). */
  static const int DSL_NAIVE;
  /** Digital line type : Standard line (width = |a|+|b|). */
  static const int DSL_STANDARD;


  /**
   * \brief Creates a digital straightline from its equation parameters.
   * @param a X value slope parameter of equation : c <= ax + by < c + nu
   * @param b Y value slope parameter of equation : c <= ax + by < c + nu
   * @param c Shift parameter of equation : c <= ax + by < c + nu
   * @param nu Width parameter of equation : c <= ax + by < c + nu
   */
  DigitalStraightLine (int a, int b, int c, int nu);

  /**
   * \brief Creates a digital straight line centered on two points.
   * @param p1 First leaning point.
   * @param p2 Second leaning point.
   * @param type Digital line type : DSL_THIN, DSL_NAIVE or DSL_STANDARD.
   */
  DigitalStraightLine (Pt2i p1, Pt2i p2, int type);

  /**
   * \brief Creates a digital straight line from an antipodal pair.
   * @param p1 First segment end point of the antipodal pair.
   * @param p1 Second segment end point of the antipodal pair.
   * @param p3 Opposite point of the antipodal pair.
   */
  DigitalStraightLine (Pt2i p1, Pt2i p2, Pt2i p3);

  /**
   * \brief Creates a digital straight line from two points and side shift.
   * It creates a parallel line to the line centered on given points,
   *   with side shift.
   * Unknown use, care the side correctness in case of.
   * @param p1 First point.
   * @param p2 Second point.
   * @param type Digital line type : DSL_THIN, DSL_NAIVE or DSL_STANDARD
   * @param atRight Rightwards orthogonal shift to p1p2 line.
   */
  DigitalStraightLine (Pt2i p1, Pt2i p2, int type, int atRight);

  /**
   * Creates a similar digital straight line to an other one.
   * @param l The original line.
   */
  DigitalStraightLine (const DigitalStraightLine &l);

  /**
   * Deletes the digital straight line.
   */
  virtual ~DigitalStraightLine () { }

  /**
   * \brief Returns the parameters of the digital straight line equations.
   * @param a X Slope parameter to provide.
   * @param b Y Slope parameter to provide.
   * @param c Shift parameter to provide.
   * @param nu Width parameter to provide.
   */
  inline void equation (int &a, int &b, int &c, int &nu) const {
    a = this->a; b = this->b; c = this->c; nu = this->nu; }

  /**
   * \brief Sets given values with the three central naive line parameters.
   * @param a X Slope parameter to provide.
   * @param b Y Slope parameter to provide.
   * @param c0 Shift parameter of the central naive line.
   */
  inline void getCentralLine (int &a, int &b, int &c0) const {
    a = this->a; b = this->b; c0 = c + nu / 2; }

  /**
   * \brief Returns the period of the digital straight line : max (|a|,|b|).
   */
  inline int period () const {
    int absb = b < 0 ? -b : b;
    return (a < absb ? absb : a); }

  /**
   * \brief Returns the lower icoefficient of the digital straight line.
   * Returns min (|a|,|b|).
   */
  inline int antiperiod () const {
    int absb = b < 0 ? -b : b;
    return (a < absb ? a : absb); }

  /**
   * \brief Returns the standard width : |a| + |b|.
   */
  inline int standard () const { return (a + (b < 0 ? -b : b)); }

  /**
   * \brief Returns the manhattan width of the digital straight line.
   */
  inline int width () const { return (nu); }

  /**
   * \brief Returns a support vector of the digital straight line.
   */
  inline Vr2i supportVector () const { return (Vr2i (-b, a)); }

  /**
   * \brief Returns the signed manhattan distance to the given point.
   * Returns the distance to line bounds
   * Right values are provided for points on the right side (yp > line (yp)).
   */
  int manhattan (Pt2i pix) const;

  /**
   * \brief Returns a bounding point of the digital straight line.
   * @param upper Set to true for upper bounding point, to false for lower one.
   */
  virtual Pt2i getABoundingPoint (bool upper) const;

  /**
   * \brief Adds points of one of the bounding lines to given vector.
   *   The bounding line is naive if (nu < period).
   *   Restricts the points in the specified area.
   * @param bound Vector of points to complete.
   * @param opposite True to get the opposite bounding line (ax+by=c+nu-1).
   * @param xmin Left border of the viewport.
   * @param ymin Bottom border of the viewport.
   * @param width Width of the viewport.
   * @param height Height of the viewport.
   */
  void getBoundPoints (std::vector<Pt2i> &bound, bool opposite,
                       int xmin, int ymin, int width, int height) const;

  /**
   * \brief Adds points of both bounding lines to given vector.
   * @param bound Vector of points to complete.
   * @param xmin Left border of the viewport.
   * @param ymin Bootom border of the viewport.
   * @param width Width of the viewport.
   * @param height Height of the viewport.
   */
  void getBounds (std::vector<Pt2i> &bound,
                  int xmin, int ymin, int width, int height) const;

  /**
   * \brief Checks if given point belongs to the digital line.
   * @param p Given point.
   */
  bool owns (const Pt2i &p) const;

  /**
   * \brief Checks if given segment entirely belongs to the digital line.
   * @param p1 Given segment start point.
   * @param p2 Given segment end point.
   */
  bool owns (const Pt2i &p1, const Pt2i &p2) const;

  /**
   * \brief Checks if given segment belongs even partially to the digital line.
   * @param p1 Given segment start point.
   * @param p2 Given segment end point.
   */
  bool crosses (const Pt2i &p1, const Pt2i &p2) const;

  /**
   * \brief Returns the center of the intersection with another digital line.
   * @param l The other digital line.
   */
  const Pt2i centerOfIntersection (DigitalStraightLine *l) const;

  /**
   * \brief Returns the center of the intersection with line P1-P2.
   *   Care : returns (0,0) if p1 == p2 or if P1P2 is parallel to the line.
   * @param p1 Start point of the crossed line.
   * @param p2 End point of the crossed line.
   */
  const Pt2i centerOfIntersection (Pt2i p1, Pt2i p2) const;

  /**
   * \brief Returns the squared Euclidean thickness of the digital line.
   */
  const EDist squaredEuclideanThickness () const {
    return (EDist (nu * nu, a * a + b * b)); }

  /**
   * \brief Returns the plane paving index of given point.
   *    Plane paving index is the index of the line containing the point
   * when paving the Euclidean plane by the digital straight line.
   * @param pt Given point.
   */
  int pavingIndex (Pt2i pt) const;
  int rmd (Pt2i pt) const;



protected:

  /**
   * \brief X value of the slope parameter in equation : c <= ax + by < c + nu.
   *  a is greater or equal to 0.
   */
  int a;

  /**
   * \brief Y value of the slope parameter in equation : c <= ax + by < c + nu.
   * b is a positive value when the line is horizontal (a = 0).
   */
  int b;

  /**
   * \brief Shift parameter in equation : c <= ax + by < c + nu.
   */
  int c;

  /**
   * \brief Width parameter in equation : c <= ax + by < c + nu.
   * nu is a positive value.
   */
  int nu;


  /**
   * \brief Adjusts the provided area on the straight line limits.
   * @param xmin Left coordinate of the area.
   * @param ymin Bottom coordinate of the area.
   * @param width Width of the area.
   * @param height Height of the area.
   */
  virtual void adjustWorkArea (int &xmin, int &ymin,
                               int &width, int &height) const;

  /**
   * \brief Returns the greater common divisor between two integer values.
   * @param a First integer.
   * @param b Second integer.
   */
  static int pgcd (int a, int b);
};
#endif
