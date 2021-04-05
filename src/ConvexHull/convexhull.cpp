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

#include "convexhull.h"


ConvexHull::ConvexHull (const Pt2i &lpt, const Pt2i &cpt, const Pt2i &rpt)
{
  CHVertex *cvert = new CHVertex (cpt);
  leftVertex = new CHVertex (lpt);
  rightVertex = new CHVertex (rpt);
  lastToLeft = false;

  if (lpt.toLeft (cpt, rpt))
  {
    leftVertex->setRight (cvert);
    cvert->setLeft (leftVertex);
    cvert->setRight (rightVertex);
    rightVertex->setLeft (cvert);
    rightVertex->setRight (leftVertex);
    leftVertex->setLeft (rightVertex);
  }
  else
  {
    leftVertex->setRight (rightVertex);
    rightVertex->setLeft (leftVertex);
    rightVertex->setRight (cvert);
    cvert->setLeft (rightVertex);
    cvert->setRight (leftVertex);
    leftVertex->setLeft (cvert);
  }

  aph.init (leftVertex, cvert, rightVertex);
  apv.setVertical ();
  apv.init (leftVertex, cvert, rightVertex);

  gbg.push_back (leftVertex);
  gbg.push_back (cvert);
  gbg.push_back (rightVertex);

  old_left = leftVertex;
  old_right = rightVertex;
  old_aph_vertex = aph.vertex ();
  old_aph_edge_start = aph.edgeStart ();
  old_aph_edge_end = aph.edgeEnd ();
  old_apv_vertex = apv.vertex ();
  old_apv_edge_start = apv.edgeStart ();
  old_apv_edge_end = apv.edgeEnd ();
}


ConvexHull::~ConvexHull ()
{
  for (int i = 0; i < (int) (gbg.size ()); ++i) delete gbg [i];
}


void ConvexHull::preserve ()
{
  old_aph_vertex = aph.vertex ();
  old_aph_edge_start = aph.edgeStart ();
  old_aph_edge_end = aph.edgeEnd ();
  old_apv_vertex = apv.vertex ();
  old_apv_edge_start = apv.edgeStart ();
  old_apv_edge_end = apv.edgeEnd ();
  old_left = leftVertex;
  old_right = rightVertex;
}


void ConvexHull::restore ()
{
  rconnect->setLeft (rdisconnect);
  lconnect->setRight (ldisconnect);
  leftVertex = old_left;
  rightVertex = old_right;
  aph.setVertexAndEdge (old_aph_vertex, old_aph_edge_start, old_aph_edge_end);
  apv.setVertexAndEdge (old_apv_vertex, old_apv_edge_start, old_apv_edge_end);
}


bool ConvexHull::addPoint (const Pt2i &pt, bool toleft)
{
  if (inHull (pt, toleft)) return false;
  CHVertex *vx = new CHVertex (pt);
  lastToLeft = toleft;
  gbg.push_back (vx);
  preserve ();
  insert (vx, toleft);
  aph.update (vx);
  apv.update (vx);
  return true;
}


bool ConvexHull::addPointDS (const Pt2i &pt, bool toleft)
{
  CHVertex *vx = new CHVertex (pt);
  lastToLeft = toleft;
  gbg.push_back (vx);
  preserve ();
  insertDS (vx, toleft);
  aph.update (vx);
  apv.update (vx);
  return true;
}


bool ConvexHull::moveLastPoint (const Pt2i &pos)
{
  restore ();
  if (inHull (pos, lastToLeft)) return false;
  gbg.pop_back ();
  preserve ();
  addPoint (pos, lastToLeft);
  return true;
}


EDist ConvexHull::thickness () const
{
  EDist aphw = aph.thickness ();
  EDist apvw = apv.thickness ();
  return (apvw.lessThan (aphw) ? apvw : aphw);
}


void ConvexHull::antipodalEdgeAndVertex (Pt2i &s, Pt2i &e, Pt2i &v) const
{
  EDist aphw = aph.thickness ();
  EDist apvw = apv.thickness ();
  const Antipodal *ap = (apvw.lessThan (aphw) ? &apv : &aph);
  s.set (*(ap->edgeStart ()));
  e.set (*(ap->edgeEnd ()));
  v.set (*(ap->vertex ()));
}


bool ConvexHull::inHull (const Pt2i &pt, bool toleft) const
{
  CHVertex *ext = (toleft ? leftVertex : rightVertex);
  return (pt.toLeftOrOn (*ext, *(ext->right ()))
          && pt.toLeftOrOn (*(ext->left ()), *ext));
}


void ConvexHull::insert (CHVertex *pt, bool toleft)
{
  bool opIn = false; // Opposite polyline top in the new convex hull
  CHVertex *opVertex = NULL; // Opposite vertex

  if (toleft)
  {
    lconnect = leftVertex;
    rconnect = leftVertex;
    leftVertex = pt;
    opVertex = rightVertex;
  }
  else
  {
    lconnect = rightVertex;
    rconnect = rightVertex;
    rightVertex = pt;
    opVertex = leftVertex;
  }

  ldisconnect = lconnect->right ();
  while (pt->toLeftOrOn (*lconnect, *(lconnect->left ())))
  {
    if (lconnect == opVertex) opIn = true;
    ldisconnect = lconnect;
    lconnect = lconnect->left ();
  }
  if (opIn)
  {
    if (toleft) rightVertex = lconnect;
    else leftVertex = lconnect;
  }

  opIn = false;
  rdisconnect = rconnect->left ();
  while (! pt->toLeft (*rconnect, *(rconnect->right ())))
  {
    if (rconnect == opVertex) opIn = true;
    rdisconnect = rconnect;
    rconnect = rconnect->right ();
  }
  if (opIn)
  {
    if (toleft) rightVertex = rconnect;
    else leftVertex = rconnect;
  }
  
  lconnect->setRight (pt);
  pt->setLeft (lconnect);
  rconnect->setLeft (pt);
  pt->setRight (rconnect);
}


void ConvexHull::insertDS (CHVertex *pt, bool toleft)
{
  if (toleft)
  {
    lconnect = leftVertex;
    rconnect = leftVertex;
    leftVertex = pt;
  }
  else
  {
    lconnect = rightVertex;
    rconnect = rightVertex;
    rightVertex = pt;
  }

  ldisconnect = lconnect->right ();
  while (pt->toLeftOrOn (*lconnect, *(lconnect->left ())))
  {
    ldisconnect = lconnect;
    lconnect = lconnect->left ();
  }

  rdisconnect = rconnect->left ();
  while (! pt->toLeft (*rconnect, *(rconnect->right ())))
  {
    rdisconnect = rconnect;
    rconnect = rconnect->right ();
  }
  
  lconnect->setRight (pt);
  pt->setLeft (lconnect);
  rconnect->setLeft (pt);
  pt->setRight (rconnect);
}


/*
ostream& operator<< (ostream &os, const ConvexHull &ch)
{
  os << "APH = " << ch.aph << endl;
  os << "APV = " << ch.apv << endl;
  os << "FIRST " << *(ch.leftVertex);
  CHVertex *next = ch.leftVertex->right ();
  int i = 0;
  while (i++ < 20 && next != ch.leftVertex)
  {
    os << " - " << *next;
    next = next->right ();
  }
  if (i >= 20) os << " ---";
  os << endl;
  os << "LAST " << *(ch.rightVertex);
  next = ch.rightVertex->left ();
  i = 0;
  while (i++ < 20 && next != ch.rightVertex)
  {
    os << " - " << *next;
    next = next->left ();
  }
  if (i >= 20) os << " ---";

  return os;
}
*/
