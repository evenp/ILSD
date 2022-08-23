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

#include "carriagetrack.h"
#include <cmath>


const float CarriageTrack::MIN_WIDTH = 2.0f;
const float CarriageTrack::MAX_WIDTH = 6.0f;


CarriageTrack::CarriageTrack ()
{
  status = 1;   // OK
  curright = NULL;
  curleft = NULL;
}


CarriageTrack::~CarriageTrack ()
{
  std::vector<CTrackSection *>::iterator it = lefts.begin ();
  while (it != lefts.end ())
  {
    if (*it != NULL) delete *it;
    it ++;
  }
  it = rights.begin ();
  while (it != rights.end ())
  {
    if (*it != NULL) delete *it;
    it ++;
  }
}


void CarriageTrack::clear (bool onright)
{
  if (onright)
  {
    if (rights.front()->getScanCount () != 0)
    {
      bool rev = curright->isReversed ();
      std::vector<CTrackSection *>::iterator it = rights.begin ();
      while (it != rights.end ()) delete *it++;
      rights.clear ();
      curright = new CTrackSection ();
      curright->setReversed (rev);
      rights.push_back (curright);
    }
  }
  else
  {
    if (lefts.front()->getScanCount () != 0)
    {
      bool rev = curleft->isReversed ();
      std::vector<CTrackSection *>::iterator it = lefts.begin ();
      while (it != lefts.end ()) delete *it++;
      lefts.clear ();
      curleft = new CTrackSection ();
      curleft->setReversed (rev);
      lefts.push_back (curleft);
    }
  }
}


void CarriageTrack::start (Plateau *pl, const std::vector<Pt2i> &dispix,
                           bool reversed)
{
  startsec.setReversed (reversed);
  startsec.add (pl, dispix);
  curright = new CTrackSection ();
  curright->setReversed (reversed);
  rights.push_back (curright);
  curleft = new CTrackSection ();
  curleft->setReversed (reversed);
  lefts.push_back (curleft);
}


void CarriageTrack::start (Plateau *pl, const std::vector<Pt2i> &dispix,
                           const std::vector<Pt2f> &pts, bool reversed)
{
  startsec.setReversed (reversed);
  startsec.add (pl, dispix, pts);
  curright = new CTrackSection ();
  curright->setReversed (reversed);
  rights.push_back (curright);
  curleft = new CTrackSection ();
  curleft->setReversed (reversed);
  lefts.push_back (curleft);
}


void CarriageTrack::add (bool onright, Plateau *pl,
                         const std::vector<Pt2i> &dispix)
{
  if (onright) curright->add (pl, dispix);
  else curleft->add (pl, dispix);
}


void CarriageTrack::add (bool onright, Plateau *pl,
                         const std::vector<Pt2i> &dispix,
                         const std::vector<Pt2f> &pts)
{
  if (onright) curright->add (pl, dispix, pts);
  else curleft->add (pl, dispix, pts);
}


int CarriageTrack::getAcceptedCount () const
{
  int count = startsec.getAcceptedCount ();
  std::vector<CTrackSection *>::const_iterator it = rights.begin ();
  while (it != rights.end ()) count += (*it++)->getAcceptedCount ();
  it = lefts.begin ();
  while (it != lefts.end ()) count += (*it++)->getAcceptedCount ();
  return count;
}


int CarriageTrack::getRightScanCount () const
{
  int count = 0;
  std::vector<CTrackSection *>::const_iterator it = rights.begin ();
  while (it != rights.end ()) count += (*it++)->getScanCount ();
  return count;
}


int CarriageTrack::getLeftScanCount () const
{
  int count = 0;
  std::vector<CTrackSection *>::const_iterator it = lefts.begin ();
  while (it != lefts.end ()) count += (*it++)->getScanCount ();
  return count;
}


Plateau *CarriageTrack::plateau (int num) const
{
  if (num < 0)
  { 
    num = - num - 1;
    int scan = 0; 
    while (num >= rights[scan]->getScanCount ())
    { 
      num -= rights[scan]->getScanCount ();
      scan ++;
      if (scan >= (int) (rights.size ())) return NULL;
    }
    return (rights[scan]->plateau (num));
  }
  else if (num > 0)
  {
    num --;
    int scan = 0; 
    while (num >= lefts[scan]->getScanCount ())
    {
      num -= lefts[scan]->getScanCount ();
      scan ++;
      if (scan >= (int) (lefts.size ())) return NULL;
    }
    return (lefts[scan]->plateau (num));
  }
  else return (startsec.plateau (0));
}


void CarriageTrack::accept (int num)
{
  if (num < 0)
  { 
    num = - num - 1;
    int scan = 0; 
    while (num >= rights[scan]->getScanCount ())
    { 
      num -= rights[scan]->getScanCount ();
      scan ++;
      if (scan >= (int) (rights.size ())) return;
    }
    rights[scan]->accept (num);
  }
  else if (num > 0)
  {
    num --;
    int scan = 0; 
    while (num >= lefts[scan]->getScanCount ())
    {
      num -= lefts[scan]->getScanCount ();
      scan ++;
      if (scan >= (int) (lefts.size ())) return;
    }
    lefts[scan]->accept (num);
  }
  else startsec.accept (0);
}


float CarriageTrack::getHeightReference (int num) const
{
  if (num < 0)
  { 
    num = - num - 1;
    int scan = 0; 
    while (num >= rights[scan]->getScanCount ())
    { 
      num -= rights[scan]->getScanCount ();
      scan ++;
    }
    return (rights[scan]->getHeightReference (num));
  }
  else if (num > 0)
  {
    num --;
    int scan = 0; 
    while (num >= lefts[scan]->getScanCount ())
    {
      num -= lefts[scan]->getScanCount ();
      scan ++;
    }
    return (lefts[scan]->getHeightReference (num));
  }
  else return (startsec.getHeightReference (0));
}


bool CarriageTrack::isScanReversed (int num) const
{
  if (num < 0)
  {
    num = - num - 1;
    int scan = 0;
    while (num >= rights[scan]->getScanCount ())
    {
      num -= rights[scan]->getScanCount ();
      scan ++;
    }
    return (rights[scan]->isReversed ());
  }
  else if (num > 0)
  {
    num --;
    int scan = 0;
    while (num >= lefts[scan]->getScanCount ())
    {
      num -= lefts[scan]->getScanCount ();
      scan ++;
    }
    return (lefts[scan]->isReversed ());
  }
  else return (startsec.isReversed ());
}


std::vector<Pt2i> *CarriageTrack::getDisplayScan (int num)
{
  if (num < 0)
  {
    num = - num - 1;
    int scan = 0;
    while (num >= rights[scan]->getScanCount ())
    {
      num -= rights[scan]->getScanCount ();
      scan ++;
    }
    return (rights[scan]->getDisplayScan (num));
  }
  else if (num > 0)
  {
    num --;
    int scan = 0;
    while (num >= lefts[scan]->getScanCount ())
    {
      num -= lefts[scan]->getScanCount ();
      scan ++;
    }
    return (lefts[scan]->getDisplayScan (num));
  }
  else return (startsec.getDisplayScan (0));
}


Plateau *CarriageTrack::lastValidPlateau (int num) const
{
  bool searching = true;
  Plateau *ret = NULL;
  do
  {
    ret = plateau (num);
    if (ret != NULL || num == 0) searching = false;
    else (num -= (num < 0 ? -1 : 1));
  }
  while (searching);
  return ret;
}


std::vector<Pt2f> *CarriageTrack::getProfile (int num)
{
  if (num < 0)
  {
    num = - num - 1;
    int scan = 0;
    while (num >= rights[scan]->getScanCount ())
    {
      num -= rights[scan]->getScanCount ();
      scan ++;
    }
    return (rights[scan]->getProfile (num));
  }
  else if (num > 0)
  {
    num --;
    int scan = 0;
    while (num >= lefts[scan]->getScanCount ())
    {
      num -= lefts[scan]->getScanCount ();
      scan ++;
    }
    return (lefts[scan]->getProfile (num));
  }
  else return (startsec.getProfile (0));
}


void CarriageTrack::getPoints (std::vector<Pt2i> *pts, bool acc, float iratio,
                               const Pt2i &pp1, const Pt2i &pp2)
{
  Plateau *pl = plateau (0);
  if (pl != NULL)
  {
    Vr2i p12 = pp1.vectorTo (pp2);
    float l12 = (float) (sqrt (p12.norm2 ()));
    int mini = - getRightScanCount ();
    int maxi = getLeftScanCount ();
    for (int i = mini; i <= maxi; i++)
    {
      pl = plateau (i);
      if (pl != NULL && (acc ? pl->isAccepted ()
                             : pl->getStatus () == Plateau::PLATEAU_RES_OK))
      {
        float sint = pl->internalStart () * iratio;
        float eint = pl-> internalEnd () * iratio;
        std::vector<Pt2i> *scan = getDisplayScan (i);
        std::vector<Pt2i>::iterator it = scan->begin ();
        while (it != scan->end ())
        {
          Vr2i p1x = pp1.vectorTo (*it);
          float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
          if (dist >= sint && dist < eint) pts->push_back (*it);
          it ++;
        }
      }
    }
  }
}


void CarriageTrack::getConnectedPoints (std::vector<Pt2i> *pts, bool acc,
                              const Pt2i &pp1, const Pt2i &pp2, float iratio)
{
  Plateau *pl = plateau (0);
  if (pl != NULL)
  {
    Vr2i p12 = pp1.vectorTo (pp2);
    float l12 = (float) (sqrt (p12.norm2 ()));
    bool free_path = true;
    int lacks = 0;
    float slast = 0.0f, elast = 0.0f;
    bool rev = isScanReversed (0);
    int res = getConnectedPlateau (pts, acc, 0, lacks, &slast, &elast,
                                   rev, iratio, pp1, p12, l12);
    if (res == 1) return;

    free_path = true;
    slast = 0.0f;
    elast = 0.0f;
    lacks = 0;
    int maxi = getLeftScanCount ();
    for (int num = 1; free_path && num <= maxi; num++)
    {
      res = getConnectedPlateau (pts, acc, num, lacks, &slast, &elast,
                                 rev, iratio, pp1, p12, l12);
      if (res == 1) free_path = false;
      else if (res != 0) lacks ++;
      else lacks = 0;
    }
    free_path = true;
    slast = 0.0f;
    elast = 0.0f;
    lacks = 0;
    int mini = - getRightScanCount ();
    for (int num = -1; free_path && num >= mini; num--)
    {
      res = getConnectedPlateau (pts, acc, num, lacks, &slast, &elast,
                                 rev, iratio, pp1, p12, l12);
      if (res == 1) free_path = false;
      else if (res != 0) lacks ++;
      else lacks = 0;
    }
  }
}


int CarriageTrack::getConnectedPlateau (std::vector<Pt2i> *pts, bool acc,
                              int num, int lacks, float *slast, float *elast,
                              bool rev, float iratio,
                              Pt2i pp1, Vr2i p12, float l12)
{
  Plateau *pl = plateau (num);
  if (pl == NULL || (acc ? ! pl->isAccepted ()
                         : pl->getStatus () != Plateau::PLATEAU_RES_OK))
    return (-1);

  float sint = pl->internalStart () * iratio;
  float eint = pl->internalEnd () * iratio;
  bool started = false, searching = true;
  float sdif = 0.f, edif = 0.0f;

  if (lacks ++)
  {
    sdif = (*slast - sint) / lacks;
    edif = (*elast - eint) / lacks;
  }
  while (lacks --)
  {
    started = false;
    searching = true;
    float sval = sint + sdif * lacks;
    float eval = eint + edif * lacks;
    std::vector<Pt2i> *scan = getDisplayScan (num + (num > 0 ? -lacks : lacks));
    std::vector<Pt2i>::iterator it = scan->begin ();
    while (searching && it != scan->end ())
    {
      Vr2i p1x = pp1.vectorTo (*it);
      float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
      if (rev)
      {
        if (started)
        {
          if (dist < sval) searching = false;
          else pts->push_back (*it);
        }
        else if (dist <= eval)
        {
          started = true;
          pts->push_back (*it);
        }
      }
      else
      {
        if (started)
        {
          if (dist > eval) searching = false;
          else pts->push_back (*it);
        }
        else if (dist >= sval)
        {
          started = true;
          pts->push_back (*it);
        }
      }
      it ++;
    }
  }
  
  *slast = sint;
  *elast = eint;
  return 0;
}


bool CarriageTrack::prune (int tailMinSize)
{
  int nb = (int) (rights.size ()) - 1;
  while (nb >= 0 && rights[nb]->pruneDoubtfulTail (tailMinSize)) nb --;
  nb = (int) (lefts.size ()) - 1;
  while (nb >= 0 && lefts[nb]->pruneDoubtfulTail (tailMinSize)) nb --;
  return (spread () < tailMinSize);
}


int CarriageTrack::leftEnd ()
{
  int num = 0;
  for (int i = 0; i < (int) (lefts.size ()); i++)
    num += lefts[i]->lastPlateau () + 1;
  return num;
}


int CarriageTrack::rightEnd ()
{
  int num = 0;
  for (int i = 0; i < (int) (rights.size ()); i++)
    num += rights[i]->lastPlateau () + 1;
  return num;
}


int CarriageTrack::spread ()
{
  return (1 + leftEnd () + rightEnd ());
}


int CarriageTrack::nbHoles ()
{
  int nb = 0;
  for (int i = 0; i < (int) (rights.size ()); i++)
    nb += rights[i]->nbHoles ();
  for (int i = 0; i < (int) (lefts.size ()); i++)
    nb += lefts[i]->nbHoles ();
  return nb;
}


float CarriageTrack::relativeShiftLength () const
{
  float shift = 1.0f;
  int cumlength = 1;
  for (int i = 0; i < (int) (rights.size ()); i++)
    shift += rights[i]->shiftLength (cumlength);
  for (int i = 0; i < (int) (lefts.size ()); i++)
    shift += lefts[i]->shiftLength (cumlength);
  return (shift / cumlength);
}


void CarriageTrack::getPosition (
                         std::vector<Pt2i> &pts, std::vector<Pt2i> &pts2,
                         int disp, float iratio, bool smoothed)
{
  if (startsec.plateau (0) != NULL)
  {
    Pt2i pp1, pp2;
    if (startsec.getScanBounds (0, pp1, pp2))
    {
      bool rev = startsec.isReversed ();
      if (rev) { Pt2i tmp (pp1); pp1.set (pp2); pp2.set (tmp); }
      std::vector<Pt2i> pline;
      Vr2i p12 = pp1.vectorTo (pp2);
      float l12 = (float) (sqrt (p12.norm2 ()));
      int mini = - getRightScanCount ();
      int maxi = getLeftScanCount ();
      if (disp > CTRACK_DISP_BOUNDS)
        for (int num = mini; num <= maxi; num++)
          addPlateauCenter (pts, num, rev,
                            pp1, p12, l12, iratio, smoothed);
      else
        for (int num = mini; num <= maxi; num++)
          addPlateauBounds (pts, pts2, num, rev,
                            pp1, p12, l12, iratio, smoothed);
    }
  }
}

void CarriageTrack::addPlateauCenter (std::vector<Pt2i> &pt, int num, bool rev,
                                      Pt2i pp1, Vr2i p12, float l12,
                                      float iratio, bool smoothed)
{
  Plateau* pl = plateau (num);
  if (pl != NULL && pl->inserted (smoothed))
  {
    int sdraw = -1, snum = 0;
    float sint = (pl->internalStart () + pl->internalEnd ()) * iratio / 2;
    std::vector<Pt2i>* scan = getDisplayScan (num);
    std::vector<Pt2i>::const_iterator it = scan->begin ();
    while (it != scan->end ())
    {
      Vr2i p1x = pp1.vectorTo (*it);
      float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
      if (rev)
      {
        if (sdraw == -1 && dist < sint) sdraw = (snum == 0 ? 0 : snum - 1);
      }
      else
      {
        if (sdraw == -1 && dist >= sint) sdraw = snum;
      }
      snum ++;
      it ++;
    }
//    if (rev)
//    {
    if (sdraw == -1) sdraw = (int) (scan->size ()) - 1;
//    }
    pt.push_back ((*scan)[sdraw]);
  }
}

void CarriageTrack::addPlateauBounds (
                              std::vector<Pt2i> &spt, std::vector<Pt2i> &ept,
                              int num, bool rev, Pt2i pp1, Vr2i p12, float l12,
                              float iratio, bool smoothed)
{
  Plateau* pl = plateau (num);
  if (pl != NULL && pl->inserted (smoothed))
  {
    int sdraw = -1, edraw = -1, snum = 0;
    float sint = pl->internalStart () * iratio;
    float eint = pl->internalEnd () * iratio;
    std::vector<Pt2i>* scan = getDisplayScan (num);
    std::vector<Pt2i>::const_iterator it = scan->begin ();
    while (it != scan->end ())
    {
      Vr2i p1x = pp1.vectorTo (*it);
      float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
      if (rev)
      {
        if (edraw == -1 && dist <= eint) edraw = snum;
        if (sdraw == -1 && dist < sint) sdraw = (snum == 0 ? 0 : snum - 1);
      }
      else
      {
        if (sdraw == -1 && dist >= sint) sdraw = snum;
        if (edraw == -1 && dist > eint) edraw = (snum == 0 ? 0 : snum - 1);
      }
      snum ++;
      it ++;
    }
//    if (rev)
//    {
    if (sdraw == -1) sdraw = (int) (scan->size ()) - 1;
//    }
//    else
//    {
    if (edraw == -1) edraw = (int) (scan->size ()) - 1;
//    }
    spt.push_back ((*scan)[sdraw]);
    ept.push_back ((*scan)[edraw]);
  }
}
