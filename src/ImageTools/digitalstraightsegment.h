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

#ifndef DIGITAL_STRAIGHT_SEGMENT_H
#define DIGITAL_STRAIGHT_SEGMENT_H

#include "digitalstraightline.h"
#include "absrat.h"


/** 
 * @class DigitalStraightSegment digitalstraightsegment.h
 * \brief Digital straight segment is a bounded digital straight line.
 * The digital straightline is defined by equation : c <= ax + by < c + nu
 *   where a >= 0 and nu > 0. Seemingly b is unconstrained when a = 0.
 * The bounds are horizontal lines (min <= y <= max) if the segment is vertical,
 *   vertical lines (min <= x <= max) if the segment is horizontal.
 * The bound points belong to the digital straight segment.
 */
class DigitalStraightSegment : public DigitalStraightLine
{
public:

  /**
   * \brief Creates a default digital straight segment.
   */
  DigitalStraightSegment ();

  /**
   * \brief Creates a null-thick segment joining two points.
   * @param p1 First point on the line.
   * @param p2 Second point on the line.
   * @param type Digital line type : DSL_THIN, DSL_NAIVE or DSL_STANDARD.
   * @param xmin Segment min X-coordinate.
   * @param ymin Segment min Y-coordinate.
   * @param xmax Segment max X-coordinate.
   * @param ymax Segment max Y-coordinate.
   */
  DigitalStraightSegment (Pt2i p1, Pt2i p2, int type,
                          int xmin, int ymin, int xmax, int ymax);

  /**
   * \brief Creates a digital straight segment from an antipodal pair.
   * @param p1 First segment end of the antipodal pair.
   * @param p1 Second segment end of the antipodal pair.
   * @param p3 Opposite point of the antipodal pair.
   * @param xmin Segment min X coordinate.
   * @param ymin Segment min Y coordinate.
   * @param xmax Segment max X coordinate.
   * @param ymax Segment max Y coordinate.
   */
  DigitalStraightSegment (Pt2i p1, Pt2i p2, Pt2i p3,
                          int xmin, int ymin, int xmax, int ymax);

  /**
   * \brief Creates a digital straight segment from end points and width.
   * @param p1 First end point of the segment.
   * @param p2 Second end point of the segment.
   * @param width Width value.
   */
  DigitalStraightSegment (Pt2i p1, Pt2i p2, int width);

  /**
   * \brief Sets identical to given digital straight segment.
   * @param dss Original straight segment.
   */
  inline void set (const DigitalStraightSegment &dss) {
    a = dss.a; b = dss.b; c = dss.c; nu = dss.nu;
    min = dss.min; max = dss.max; }

  /**
   * \brief Creates a digital straight segment from another one.
   * @param dss Pointer to the digital straight segment to copy.
   */
  DigitalStraightSegment (DigitalStraightSegment *dss);

  /**
   * \brief Returns a bounding point of the digital line.
   * @param upper True for an upper bounding point, false for a lower one.
   */
  Pt2i getABoundingPoint (bool upper) const;

  /**
   * \brief Adds segment points to the provided vector.
   * @param pts Vector of points to complete.
   */
  void getPoints (std::vector<Pt2i> &pts) const;

  /**
   * \brief Provides the naive central segment end points coordinates.
   * @param x1 Start point X-coordinate to set.
   * @param x2 Start point Y-coordinate to set.
   * @param x2 End point X-coordinate to set.
   * @param y2 End point Y-coordinate to set.
   */
  void naiveLine (AbsRat &x1, AbsRat &y1, AbsRat &x2, AbsRat &y2) const;

  /**
   * \brief Returns an erosion of the segment.
   * @param num Erosion value numerator.
   * @param den Erosion value denominator.
   */
  DigitalStraightSegment *erosion (int num, int den) const;

  /**
   * \brief Returns a dilation of the segment.
   * @param num Dilation value numerator.
   * @param den Dilation value denominator.
   */
  DigitalStraightSegment *dilation (int num, int den) const;

  /**
   * \brief Returns a dilated segment of given radius.
   * @param radius Dilation radius.
   */
  DigitalStraightSegment *dilation (int radius) const;

  /**
   * \brief Dilates the segment of given radius.
   * @param radius Dilation radius.
   */
  void dilate (int radius);

  /**
   * \brief Inquires if given point belongs to a dilation of the segment.
   * @param p Tested point.
   * @param tol Dilation value (some tolerence).
   */
  bool contains (Pt2i p, int tol) const;

  /**
   * \brief Returns the squared length of the digital straight segment.
   */
  int length2 () const;


protected:

  /**
   * \brief Bounding line lower coordinate.
   * If the segment is stictly horizontal, it enforces x >= xmin.
   * If the segment is vertical or diagonal, it enforces y >= ymin.
   * The bound belongs to the segment.
   */
  int min;
  /**
   * \brief Bounding line upper coordinate.
   * If the segment is stictly horizontal, it enforces x <= xmax.
   * If the segment is vertical or diagonal, it enforces y <= ymax.
   * The bound belongs to the segment.
   */
  int max;


  /**
   * \brief Creates a segment from parameter values.
   * @param va Slope X coordinate.
   * @param vb Slope Y coordinate.
   * @param vc Shift to origin.
   * @param vnu Arithmetical width.
   * @param vmin Bounding line lower coordinate.
   * @param vmax Bounding line upper coordinate.
   */
  DigitalStraightSegment (int va, int vb, int vc, int vnu, int vmin, int vmax);

  /**
   * \brief Sets the provided area on the segment limits.
   * @param xmin Left border of the area to set.
   * @param ymin Bottom border of the area to set.
   * @param width Width of the area to set.
   * @param height Height of the area to set.
   */
  void adjustWorkArea (int &xmin, int &ymin, int &width, int &height) const;

};
#endif
