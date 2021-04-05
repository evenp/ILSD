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

#include "blurredsegment.h"


BlurredSegment::BlurredSegment ()
{
  plist = NULL;
  dss = NULL;
  scan = NULL;
}


BlurredSegment::BlurredSegment (BiPtList *ptlist, DigitalStraightSegment *seg,
                                const Pt2i &aps, const Pt2i &ape,
                                const Pt2i &apv)
{
  plist = ptlist;
  dss = seg;
  scan = NULL;
  laps.set (aps);
  lape.set (ape);
  lapv.set (apv);
}


BlurredSegment::~BlurredSegment ()
{
  if (plist != NULL) delete plist;
  if (dss != NULL) delete dss;
  if (scan != NULL) delete scan;
}


void BlurredSegment::setScan (const Pt2i &pt1, const Pt2i &pt2)
{
  scan = new DigitalStraightLine (pt1, pt2, DigitalStraightLine::DSL_NAIVE);
}


void BlurredSegment::setScan (const Pt2i &center, const Vr2i &dir)
{
  Pt2i p2 (center.x () + dir.x (), center.y () + dir.y ());
  scan = new DigitalStraightLine (center, p2, DigitalStraightLine::DSL_NAIVE);
}


EDist BlurredSegment::minimalWidth () const
{
  return (EDist (dss->width (), dss->period ()));
}


std::vector<Pt2i> BlurredSegment::getAllPoints () const
{
  return (plist->frontToBackPoints ());
}


std::vector<Pt2i> *BlurredSegment::getAllRight () const
{
  return (plist->backPoints ());
}


std::vector<Pt2i> *BlurredSegment::getAllLeft () const
{
  return (plist->frontPoints ());
}


DigitalStraightSegment *BlurredSegment::holdSegment ()
{
  DigitalStraightSegment *tmp = dss;
  dss = NULL;
  return tmp;
}


int BlurredSegment::size () const
{
  return (plist->size ());
}


int BlurredSegment::extent () const
{
  int l = 0;
  if (scan != NULL)
  {
    l = scan->pavingIndex (plist->backPoint ())
        + scan->pavingIndex (plist->frontPoint ());
  }
  else
  {
    Pt2i pr = plist->backPoint ();
    Pt2i pl = plist->frontPoint ();
    Pt2i c ((pr.x () + pl.x ()) / 2, (pr.y () + pl.y ()) / 2);
    Pt2i d (c.x () + pl.y () - pr.y (), c.y () + pr.x () - pl.x ());
    DigitalStraightLine line (c, d, DigitalStraightLine::DSL_NAIVE);
    l = line.pavingIndex (plist->backPoint ())
        + line.pavingIndex (plist->frontPoint ());
  }
  return (l < 0 ? 1 - l : 1 + l);
}


std::vector<Pt2i> BlurredSegment::getStartPt () const
{
  std::vector<Pt2i> res;
  res.push_back (plist->initialPoint ());
  return res;
}


const Pt2i BlurredSegment::getLastRight () const
{
  return (plist->backPoint ());
}


const Pt2i BlurredSegment::getLastLeft () const
{
  return (plist->frontPoint ());
}


int BlurredSegment::getSquarredLength () const
{
  Pt2i rp = plist->backPoint ();
  Pt2i lp = plist->frontPoint ();
  return ((rp.x () - lp.x ()) * (rp.x () - lp.x ())
          + (rp.y () - lp.y ()) * (rp.y () - lp.y ()));
}


const Pt2i BlurredSegment::getMiddle () const
{
  Pt2i rp = plist->backPoint ();
  Pt2i lp = plist->frontPoint ();
  return (Pt2i ((rp.x () + lp.x ()) / 2, (rp.y () + lp.y ()) / 2));
}


Vr2i BlurredSegment::getSupportVector ()
{
  return (dss->supportVector ());
}


Vr2i BlurredSegment::boundingBoxSize () const
{
  int xmin, ymin, xmax, ymax;
  plist->findExtrema (xmin, ymin, xmax, ymax);
  return (Vr2i (xmax - xmin, ymax - ymin));
}


std::vector <std::vector <Pt2i> > BlurredSegment::connectedComponents () const
{
  std::vector <std::vector <Pt2i> > ccs;
  std::vector <Pt2i> pts = getAllPoints ();
  if (pts.size () > 1)
  {
    std::vector <Pt2i> cc;
    bool started = false;
    std::vector <Pt2i>::const_iterator it = pts.begin ();
    Pt2i pix (*it++);
    while (it != pts.end ())
    {
      if (it->isConnectedTo (pix))
      {
        if (! started)
        {
          cc.push_back (pix);
          started = true;
        }
        cc.push_back (*it);
      }
      else
      {
        if (started)
        {
          ccs.push_back (cc);
          cc.clear ();
          started = false;;
        }
      }
      pix.set (*it++);
    }
  }
  return ccs;
}


int BlurredSegment::countOfConnectedPoints () const
{
  int count = 0;
  std::vector <Pt2i> pts = getAllPoints ();
  if (pts.size () > 1)
  {
    bool started = false;
    std::vector <Pt2i>::const_iterator it = pts.begin ();
    Pt2i pix (*it++);
    while (it != pts.end ())
    {
      if (it->isConnectedTo (pix))
      {
        if (started) count ++;
        else
        {
          count += 2;
          started = true;
        }
      }
      else
      {
        if (started) started = false;;
      }
      pix.set (*it++);
    }
  }
  return count;
}


int BlurredSegment::countOfConnectedComponents () const
{
  int count = 0;
  std::vector <Pt2i> pts = getAllPoints ();
  if (pts.size () > 1)
  {
    bool started = false;
    std::vector <Pt2i>::const_iterator it = pts.begin ();
    Pt2i pix (*it++);
    while (it != pts.end ())
    {
      if (it->isConnectedTo (pix))
      {
        if (! started)
        {
          count ++;
          started = true;
        }
      }
      else
      {
        if (started) started = false;;
      }
      pix.set (*it++);
    }
  }
  return count;
}


int BlurredSegment::countOfConnectedPoints (int min) const
{
  int count = 0;
  std::vector <Pt2i> pts = getAllPoints ();
  if (pts.size () > 1)
  {
    int cpt = 1;
    std::vector <Pt2i>::const_iterator it = pts.begin ();
    Pt2i pix (*it++);
    while (it != pts.end ())
    {
      if (it->isConnectedTo (pix))
      {
        if (++cpt == min) count += min;
        else if (cpt > min) count ++;
      }
      else cpt = 1;
      pix.set (*it++);
    }
  }
  return count;
}


int BlurredSegment::countOfConnectedComponents (int min) const
{
  int count = 0;
  std::vector <Pt2i> pts = getAllPoints ();
  if (pts.size () > 1)
  {
    int cpt = 1;
    std::vector <Pt2i>::const_iterator it = pts.begin ();
    Pt2i pix (*it++);
    while (it != pts.end ())
    {
      if (it->isConnectedTo (pix))
      {
        if (++cpt == min) count ++;
      }
      else cpt = 1;;
      pix.set (*it++);
    }
  }
  return count;
}


std::vector <std::vector <Pt2i> >
BlurredSegment::getConnectedComponents () const
{
  std::vector <std::vector <Pt2i> > res;
  std::vector <Pt2i> pts = getAllPoints ();
  if (pts.size () > 1)
  {
    std::vector <Pt2i>::const_iterator bit = pts.begin ();
    std::vector <Pt2i>::const_iterator eit = pts.end ();
    while (bit != eit)
    {
      std::vector <Pt2i> lres;
      Pt2i pix = *bit++;
      bool compose = true;
      do
      {
        lres.push_back (pix);
        compose = bit->isConnectedTo (pix);
        if (compose) pix.set (*bit++);
      }
      while (compose && bit != eit);
      if (compose) lres.push_back (pix);
      res.push_back (lres);
    }
  }
  return res;
}
