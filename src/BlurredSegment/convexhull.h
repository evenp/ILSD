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

#ifndef CONVEXHULL
#define CONVEXHULL

#include "antipodal.h"


/** 
 * @class ConvexHull convexhull.h
 * \brief Convex hull of a polyline.
 */
class ConvexHull
{
public:

  /**
   * \brief Creates a convex hull from a triangle.
   * Be very careful with the points ordering : lpt, cpt, then rpt.
   * Ensure that the points are NOT COLINEAR (not tested here).
   * @param lpt : left end vertex of the polyline.
   * @param cpt : center vertex of the polyline.
   * @param rpt : right end vertex of the polyline.
   */
  ConvexHull (const Pt2i &lpt, const Pt2i &cpt, const Pt2i &rpt);

  /**
   * \brief Deletes the convex hull.
   * Removes all registered vertices.
   */
  ~ConvexHull ();

  /**
   * \brief Restores the convexhull features after a modification.
   */
  void restore ();

  
  /**
   * \brief Appends a new point at one side of the convex hull.
   * @param pt Reference to the point to add.
   * @param Side at which the point is added.
   */
  bool addPoint (const Pt2i &pt, bool toleft);

  /**
   * \brief Appends a new point at one side of the convex hull.
   * To be used with directional scans:
   *   in that case, added point can not be inside the hull.
   * @param pt Reference to the point to add.
   * @param toleft Add the point at left side if true, right side otherwise.
   */
  bool addPointDS (const Pt2i &pt, bool toleft);

  /**
   * \brief Moves the last inserted point and returns the success.
   * @param pos New position for the last point.
   */
  bool moveLastPoint (const Pt2i &pos);

  /**
   * \brief Checks whether the line to given point crosses the hull.
   * @param pt Given point.
   * @param toleft Set to true if the point was added on left side.
   */
  bool inHull (const Pt2i &pt, bool toleft) const;


  /**
   * \brief Returns the antipodal edge and vertex.
   * @param s Edge start vertex.
   * @param e Edge end vertex.
   * @param v Vertex.
   */
  void antipodalEdgeAndVertex (Pt2i &s, Pt2i &e, Pt2i &v) const;

  /**
   * \brief Returns the convex hull thickness.
   * The thickness is the minimal vertical or horizontal thickness.
   * It is computed as the minimal value of both antipodal pairs.
   */
  EDist thickness () const;

  /**
   * \brief Returns a string that represents the convex hull.
   */
  // friend ostream& operator<< (ostream &os, const ConvexHull &ch);

  /**
   * \brief Returns the first (left) vertex of the convex hull.
   */
  inline CHVertex *getFirstVertex () const { return (leftVertex); }

  /**
   * \brief Returns the last (right) vertex of the convex hull.
   */
  inline CHVertex *getLastVertex () const { return (rightVertex); }

  /**
   * \brief Returns the horizontal antipodal vertex.
   */
  inline CHVertex *getAphVertex () const { return (aph.vertex ()); }

  /**
   * \brief Returns the horizontal antipodal edge start vertex.
   */
  inline CHVertex *getAphEdgeStart () const { return (aph.edgeStart ()); }

  /**
   * \brief Returns the horizontal antipodal edge end vertex.
   */
  inline CHVertex *getAphEdgeEnd () const { return (aph.edgeEnd ()); }

  /**
   * \brief Returns the vertical antipodal vertex.
   */
  inline CHVertex *getApvVertex () const { return (apv.vertex ()); }

  /**
   * \brief Returns the vertical antipodal edge start vertex.
   */
  inline CHVertex *getApvEdgeStart () const { return (apv.edgeStart ()); }

  /**
   * \brief Returns the vertical antipodal edge end vertex.
   */
  inline CHVertex *getApvEdgeEnd () const { return (apv.edgeEnd ()); }


protected:

  /** Polyline left end point. */
  CHVertex *leftVertex;
  /** Polyline right end point. */
  CHVertex *rightVertex;
  /** Indicates if the last vertex was entered to the left. */
  bool lastToLeft;

  /** Antipodal pair in horizontal direction. */
  Antipodal aph;
  /** Antipodal pair in vertical direction. */
  Antipodal apv;

  /** Registered vertex of previous horizontal antipodal pair. */
  CHVertex *old_aph_vertex;
  /** Registered edge start of previous horizontal antipodal pair. */
  CHVertex *old_aph_edge_start;
  /** Registered edge end of previous horizontal antipodal pair. */
  CHVertex *old_aph_edge_end;
  /** Registered vertex of previous vertical antipodal pair. */
  CHVertex *old_apv_vertex;
  /** Registered edge start of previous vertical antipodal pair. */
  CHVertex *old_apv_edge_start;
  /** Registered edge end of previous vertical antipodal pair. */
  CHVertex *old_apv_edge_end;
  /** Registered left end point of previous polyline. */
  CHVertex *old_left;
  /** Registered right end point of previous polyline. */
  CHVertex *old_right;
  /** Registered connected point to the left of previous polyline. */
  CHVertex *lconnect;
  /** Registered disconnected point to the left of previous polyline. */
  CHVertex *ldisconnect;
  /** Registered connected point to the right of previous polyline. */
  CHVertex *rconnect;
  /** Registered disconnected point to the right of previous polyline. */
  CHVertex *rdisconnect;

  /** Collection of released vertices for clearance. */
  std::vector<CHVertex*> gbg;


private:

  /**
   * \brief Stores convex hull features before a modification.
   */
  void preserve ();

  /**
   * \brief Inserts a new point into the convex hull.
   * @param pt Pointer to the point to add.
   * @param toleft Adds to left if true, to right otherwise.
   */
  void insert (CHVertex *pt, bool toleft);

  /**
   * \brief Inserts a new point into the convex hull.
   * To be used with directional scans :
   *   In that case, opposite ends of the polyline can never pass each other.
   * @param pt Pointer to the point to add.
   * @param toleft Adds to left if true, to right otherwise.
   */
  void insertDS (CHVertex *pt, bool toleft);

};
#endif
