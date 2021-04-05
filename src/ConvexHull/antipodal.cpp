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

#include "antipodal.h"


Antipodal::Antipodal ()
{
  ix = 0;
  iy = 1;
  vpt = NULL;
  ept1 = NULL;
  ept2 = NULL;
}


void Antipodal::init (CHVertex *v1, CHVertex *v2, CHVertex *v3)
{
  if (v1->get (iy) < v2->get (iy))
  {
    if (v2->get (iy) < v3->get (iy))
    {
      vpt = v2;
      ept1 = v1;
      ept2 = v3;
    }
    else
    {
      if (v1->get (iy) < v3->get (iy))
      {
        vpt = v3;
        ept1 = v1;
        ept2 = v2;
      }
      else
      {
        vpt = v1;
        ept1 = v2;
        ept2 = v3;
      }
    }
  }
  else
  {
    if (v1->get (iy) < v3->get (iy))
    {
      vpt = v1;
      ept1 = v2;
      ept2 = v3;
    }
    else
    {
      if (v2->get (iy) <= v3->get (iy))     // EQUIV : rather than "<" !!!
      {
        vpt = v3;
        ept1 = v1;
        ept2 = v2;
      }
      else
      {
        vpt = v2;
        ept1 = v1;
        ept2 = v3;
      }
    }
  }
}


EDist Antipodal::thickness () const
{
  int den = ept2->get (iy) - ept1->get (iy);
  return (EDist (((vpt->get (ix) - ept1->get (ix)) * den
                  - (vpt->get (iy) - ept1->get (iy))
                    * (ept2->get (ix) - ept1->get (ix))), den));
}


int Antipodal::remainder (CHVertex *v) const
{
  int a = ept2->y () - ept1->y ();
  int b = ept2->x () - ept1->x ();
  if (a == 0) return ((b > 0 ? -b : b) * v->y ());
  if (a < 0)
  {
    a = -a;
    b = -b;
  }
  return (a * v->x () - b * v->y ());
}


bool Antipodal::edgeInFirstQuadrant () const
{
  if (iy) return true;
  int a = ept2->y () - ept1->y ();
  if (a == 0) return true;
  return (a > 0 ? (ept1->x () < ept2->x ()) : (ept2->x () < ept1->x ()));
}


int Antipodal::getA () const
{
  int a = ept2->y () - ept1->y ();
  return (a < 0 ? -a : a);
}


int Antipodal::getB () const
{
  int a = ept2->y () - ept1->y ();
  int b = ept2->x () - ept1->x ();
  if (a < 0) b = -b;
  else if (a == 0 && b < 0) b = -b;
  return (b);
}


/*
ostream& operator<< (ostream &os, const Antipodal &ap)
{
  os << (ap.ix ? "AV [" : "AH [") << *(ap.vpt) << " + ("
     << *(ap.ept1) << " - " << *(ap.ept2) << ")]";
  if (ap.remainder (ap.vpt) == ap.remainder (ap.ept1)) os << "--HS--";
  return os;
}
*/


void Antipodal::update (CHVertex *pt)
{
  CHVertex *rpt = pt->right ();
  CHVertex *lpt = pt->left ();

  int rmp = remainder (pt);
  int rmv = remainder (vpt);
  int rme = remainder (ept1);
  int zpt = pt->get (iy);     // vertical AP : Z -> X -- horizontal AP : Z -> Y
  int zav = vpt->get (iy);    // coord of antipodal vertex
  int zas = ept1->get (iy);   // coord of antipodal edge start
  int zae = ept2->get (iy);   // coord of antipodal edge end

  CHVertex *pvertex;
  if (remainder (rpt) == rmv) pvertex = rpt;
  else if (remainder (lpt) == rmv) pvertex = lpt;
  else pvertex = vpt;

  CHVertex *pedge;
  if (remainder (rpt) == rme) pedge = rpt;
  else if (remainder (lpt) == rme) pedge = lpt;
  else pedge = ept1;


  // P on the line supported by the Edge
  if (rmp == rme)
  {
    // P between start end end of antipodal Edge : no change (IMPOSSIBLE)
    if ((zpt == zas) || (zpt == zae) || ((zpt < zas) != (zpt < zae))) return;
    // -> prolongation of antipodal Edge up to P
    setEdge (pt, pedge);
    return;
  }

  // P on the line (parallel to Edge) supported by the Vertex
  if (rmp == rmv)
  {
    // P at the height of Edge -> P is the new Vertex
    if ((zpt == zas) || (zpt == zae) || ((zpt < zas) != (zpt < zae)))
      setVertex (pt);
    else
    {
      // P beyond Edge Start : -> the Edge Start is the new Vertex
      if ((zas == zae) || ((zas < zpt) != (zas < zae))) setVertex (ept1);
      // P beyond Edge End : -> the Edge End is the new Vertex
      if ((zae < zpt) != (zae < zas)) setVertex (ept2);
      // -> the new Edge joins P to the former Vertex
      setEdge (pt, pvertex);
    }
    return;
  }
 
  // P strictly between antipodal Edge and Vertex -> no change
  if ((rmp < rmv) != (rmp < rme)) return;


  // P at the height of the antipodal Vertex
  if (zpt == zav)
  {
    // P beyond the antipodal Vertex
    if ((rmv < rmp) != (rmv < rme))
    {				
      // -> P is the new Vertex
      setVertex (pt);
      return;
    }

    CHVertex *oldvpt = vpt;
    if (zav != lpt->get (iy))
    {
      if (oldvpt->vprod (oldvpt->left (), lpt, pt) > 0)
      {
        setVertex (oldvpt);
        setEdge (lpt, pt);
      }
      else
      {
        setVertex (pt);
        setEdge (oldvpt, oldvpt->left ());
      }
    }
    else
    {
      if (oldvpt->vprod (oldvpt->right (), rpt, pt) < 0)
      {
        setVertex (oldvpt);
        setEdge (rpt, pt);
      }
      else
      {
        setVertex (pt);
        setEdge (oldvpt, oldvpt->right ());
      }
    }
    return;
  }


  // Main case
  //==============================================================
  CHVertex *cvx = NULL;   // candidate rotation vertex
  CHVertex *lvx, *rvx;    // left and right vertices of candidate
  int zvx;                // coord of candidate

  bool firstQuad = true;
  if (edgeInFirstQuadrant ())
  {
    if (((rmp > rme) && (rmp > rmv) && (zpt > zav))
        || ((rmp < rme) && (rmp < rmv) && (zpt < zav))) firstQuad = false;
  }
  else
    if (((rmp > rme) && (rmp > rmv) && (zpt < zav))
        || ((rmp < rme) && (rmp < rmv) && (zpt > zav))) firstQuad = false;

  if (firstQuad)
  {
    if ((rme < rmp) != (rme < rmv)) cvx = pvertex;
    if ((rmv < rme) != (rmv < rmp))
      cvx = (ept1->right () == ept2 ? ept1 : ept2);
    zvx = cvx->get (iy);
    lvx = cvx->left ();
    rvx = cvx->right ();

    while (cvx->vprod (rvx, rpt, pt) > 0)
    {
      cvx = rvx;
      lvx = cvx->left ();
      rvx = cvx->right ();
      zvx = cvx->get (iy);
      int zpn = lvx->get (iy);
      if ((zpt == zvx) || (zpt == zpn) || ((zpt < zvx) != (zpt < zpn))) break;
    }

    if (zvx == zpt)
    {
      if (cvx->vprod (rvx, rpt, pt) <= 0)   // Au lieu de < chez Phuong
      {
        setVertex (cvx);
        setEdge (rpt, pt);
      }
      else
      {
        setVertex (pt);
        setEdge (cvx, rvx);
      }
    }
    else
    {
      int zpn = rpt->get (iy);
      if ((zvx == zpn) || ((zvx < zpt) != (zvx < zpn)))
      {
        setVertex (cvx);
        setEdge (rpt, pt);
      }
      else
      {
        setVertex (pt);
        setEdge (lvx, cvx);
      }
    }
  }

  else // second quadrant
  {
    if ((rme < rmp) != (rme < rmv)) cvx = pvertex;
    if ((rmv < rme) != (rmv < rmp))
      cvx = (ept1->left () == ept2 ? ept1 : ept2);
    zvx = cvx->get (iy);
    rvx = cvx->right ();
    lvx = cvx->left ();

    while (cvx->vprod (lvx, lpt, pt) < 0)
    {
      cvx = lvx;
      rvx = cvx->right ();
      lvx = cvx->left ();
      zvx = cvx->get (iy);
      int zvn = rvx->get (iy);
      if ((zpt == zvx) || (zpt == zvn) || ((zpt < zvx) != (zpt < zvn))) break;
    }
    if (zvx == zpt)
    {
      if (cvx->vprod (lvx, lpt, pt) >= 0)
      {
        setVertex (cvx);
        setEdge (lpt, pt);
      }
      else
      {
        setVertex (pt);
        setEdge (cvx, lvx);
      }
    }
    else
    {
      int zvn = lpt->get (iy);
      if ((zvx == zvn) || ((zvx < zvn) != (zvx < zpt)))
      {
        setVertex (cvx);
        setEdge (lpt, pt);
      }
      else
      {
        setVertex (pt);
        setEdge (rvx, cvx);
      }
    }
  }
}
