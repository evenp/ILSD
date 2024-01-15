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
#include "scannerprovider.h"
#include <cmath>
#include <algorithm>


const float CarriageTrack::MIN_WIDTH = 2.0f;
const float CarriageTrack::MAX_WIDTH = 6.0f;


CarriageTrack::CarriageTrack ()
{
  status = 1;   // OK
  curright = NULL;
  curleft = NULL;
  seed_length = 1.0f;
  cell_size = 1.0f;
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


void CarriageTrack::setDetectionSeed (Pt2i p1, Pt2i p2, float cs)
{
  seed_p1.set (p1);
  seed_p2.set (p2);
  seed_length = (float) sqrt ((p2.x () - p1.x ()) * (p2.x () - p1.x ())
                               + (p2.y () - p1.y ()) * (p2.y () - p1.y ()));
  cell_size = cs;
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


void CarriageTrack::getPoints (std::vector<Pt2i> *pts, bool acc,
                               int imw, int imh, float iratio)
{
  ScannerProvider sp;
  sp.setSize (imw, imh);
  DirectionalScanner *ds = sp.getScanner (seed_p1, seed_p2, true);
  bool rev = sp.isLastScanReversed ();
  Vr2i seed (seed_p1.vectorTo (seed_p2));
  int a = seed.x (), b = seed.y ();
  if (a < 0.)
  {
    a = -a;
    b = -b;
  }
  float l12 = (float) (sqrt (seed.norm2 ()));
  Vr2f p12n (seed.x () / l12, seed.y () / l12);
  int mini = - getRightScanCount ();
  int maxi = getLeftScanCount ();

  std::vector<Pt2i> pix;
  std::vector<Pt2i>::iterator pit;
  ds->first (pix);
  int i = 0;
  do
  {
    Plateau *pl = plateau (i);
    if (i != 0)
    {
      ds->bindTo (a, b, pl->scanShift ());
      if ((i > 0 && rev) || (i < 0 && ! rev)) ds->nextOnRight (pix);
      else ds->nextOnLeft (pix);
    }
    if (pl != NULL && pl->inserted (acc))
    {
      float sint = pl->internalStart () * iratio;
      float eint = pl->internalEnd () * iratio;
      bool cherche = (sint < eint), in = false;
      for (pit = pix.begin (); cherche && pit != pix.end (); pit ++)
      {
        Vr2i p1x (seed_p1.vectorTo (*pit));
        float dist = p12n.x () * p1x.x () + p12n.y () * p1x.y ();
        if (! in)
        {
          if ((rev && dist < eint) || ((! rev) && dist >= sint)) in = true;
        }
        if (in)
        {
          if ((rev && dist < sint) || ((! rev) && dist >= eint))
            cherche = false;
          else pts->push_back (*pit);
        }
      }
    }
    if (i == maxi) i = -1;
    else i += (i < 0 ? -1 : 1);
  }
  while (i >= mini);
}


void CarriageTrack::getPoints (std::vector<std::vector<Pt2i> > *pts, bool acc,
                               int imw, int imh, float iratio)
{
  ScannerProvider sp;
  sp.setSize (imw, imh);
  DirectionalScanner *ds = sp.getScanner (seed_p1, seed_p2, true);
  bool rev = sp.isLastScanReversed ();
  Vr2i seed (seed_p1.vectorTo (seed_p2));
  Vr2i ssdir (seed);
  if (ssdir.x () < 0) ssdir.invert ();
  float l12 = (float) (sqrt (seed.norm2 ()));
  Vr2f p12n (seed.x () / l12, seed.y () / l12);
  int mini = - getRightScanCount ();
  int maxi = getLeftScanCount ();

  std::vector<Pt2i> pix;
  std::vector<Pt2i>::iterator pit;
  ds->first (pix);
  int i = 0;
  do
  {
    Plateau *pl = plateau (i);
    if (i != 0)
    {
      ds->bindTo (ssdir.x (), ssdir.y (), pl->scanShift ());
      if ((i > 0 && rev) || (i < 0 && ! rev)) ds->nextOnRight (pix);
      else ds->nextOnLeft (pix);
    }
    if (pl != NULL && pl->inserted (acc))
    {
      std::vector<Pt2i> line;
      float sint = pl->internalStart () * iratio;
      float eint = pl->internalEnd () * iratio;
      bool cherche = (sint < eint), in = false;
      for (pit = pix.begin (); cherche && pit != pix.end (); pit ++)
      {
        Vr2i p1x (seed_p1.vectorTo (*pit));
        float dist = p12n.x () * p1x.x () + p12n.y () * p1x.y ();
        if (! in)
        {
          if ((rev && dist < eint) || ((! rev) && dist >= sint)) in = true;
        }
        if (in)
        {
          if ((rev && dist < sint) || ((! rev) && dist >= eint))
            cherche = false;
          else line.push_back (*pit);
        }
      }
      pts->push_back (line);
    }
    if (i == maxi) i = -1;
    else i += (i < 0 ? -1 : 1);
  }
  while (i >= mini);
}


void CarriageTrack::getConnectedPoints (std::vector<Pt2i> *pts, bool acc,
                                        int imw, int imh, float iratio)
{
  ScannerProvider sp;
  sp.setSize (imw, imh);
  DirectionalScanner *ds = sp.getScanner (seed_p1, seed_p2, true);
  bool rev = sp.isLastScanReversed ();
  Vr2i seed (seed_p1.vectorTo (seed_p2));
  int a = seed.x (), b = seed.y ();
  if (a < 0.)
  {
    a = -a;
    b = -b;
  }
  float l12 = (float) (sqrt (seed.norm2 ()));
  Vr2f p12n (seed.x () / l12, seed.y () / l12);
  int mini = - getRightScanCount ();
  int maxi = getLeftScanCount ();

  int lacks = 0;
  float sint = 0.0f, eint = 0.0f, slast = 0.0f, elast = 0.0f;
  float sini = 0.0, eini = 0.0f, sdif = 0.0f, edif = 0.0f;
  std::vector<Pt2i> pix;
  std::vector<Pt2i>::iterator pit;
  ds->first (pix);
  int i = 0;
  do
  {
    Plateau *pl = plateau (i);
    if (pl == NULL || ! pl->inserted (acc)) lacks ++;
    else
    {
      sint = pl->internalStart () * iratio;
      eint = pl->internalEnd () * iratio;
      if (lacks ++)
      {
        sdif = (slast - sint) / lacks;
        edif = (elast - eint) / lacks;
      }
      while (lacks)
      {
        lacks --;
        bool cherche = true, in = false;
        float sval = sint + sdif * lacks;
        float eval = eint + edif * lacks;
        if (i != 0)
        {
          Plateau *spl = plateau (i + (i < 0 ? lacks : - lacks));
          ds->bindTo (a, b, spl->scanShift ());
          if ((i > 0 && rev) || (i < 0 && ! rev)) ds->nextOnRight (pix);
          else ds->nextOnLeft (pix);
        }
        for (pit = pix.begin (); cherche && pit != pix.end (); pit ++)
        {
          Vr2i p1x (seed_p1.vectorTo (*pit));
          float dist = p12n.x () * p1x.x () + p12n.y () * p1x.y ();
          if (! in)
          {
//            if ((rev && dist < eval) || ((! rev) && dist >= sval)) in = true;
if ((rev && dist <= eval) || ((! rev) && dist >= sval)) in = true;
          }
          if (in)
          {
//            if ((rev && dist < sval) || ((! rev) && dist >= eval))
if ((rev && dist < sval) || ((! rev) && dist > eval))
              cherche = false;
            else pts->push_back (*pit);
          }
        }
      }
      slast = sint;
      elast = eint;
    }
    // Updates the scanner traversal
    if (i == 0)
    {
      sini = sint;
      eini = eint;
      lacks = 0;  // for safety, if ever the initial scan is not accepted
    }
    if (i == maxi)
    {
      i = -1;
      slast = sini;
      elast = eini;
      lacks = 0;
    }
    else i += (i < 0 ? -1 : 1);
  }
  while (i >= mini);
}


void CarriageTrack::getConnectedPoints (std::vector<std::vector<Pt2i> > *pts,
                                     bool acc, int imw, int imh, float iratio)
{
  int i = 0, lacks = 0, flacks = -1, blacks = -1;
  float slast = 0.0f, elast = 0.0f;
  float sint = 0.0f, eint = 0.0f, sdif = 0.0f, edif = 0.0f;
  int mini = - getRightScanCount ();
  int maxi = getLeftScanCount ();
  for (int j = 0; flacks < 0 && j <= maxi; j++)
    if (plateau(j)->inserted (acc)) flacks = j;
  for (int j = 0; blacks < 0 && j >= mini; j--)
    if (plateau(j)->inserted (acc)) blacks = -j;
  if (blacks == -1)
    if (flacks == -1) return;
    else i = flacks;
  else if (blacks > 0)
  {
    i = - blacks;
    if (flacks > 0)
    {
      lacks = flacks + blacks - 1;
      slast = plateau(flacks)->internalStart () * iratio;
      elast = plateau(flacks)->internalEnd () * iratio;
    }
  }

  ScannerProvider sp;
  sp.setSize (imw, imh);
  DirectionalScanner *ds = sp.getScanner (seed_p1, seed_p2, true);
  bool rev = sp.isLastScanReversed ();
  if (blacks == -1)
  {
    if (rev) ds->skipRight (flacks);
    else ds->skipLeft (flacks);
  }
  else if (flacks == -1)
  {
    if (rev) ds->skipLeft (blacks);
    else ds->skipRight (blacks);
  }
  Vr2i seed (seed_p1.vectorTo (seed_p2));
  int a = seed.x (), b = seed.y ();
  if (a < 0.)
  {
    a = -a;
    b = -b;
  }
  float l12 = (float) (sqrt (seed.norm2 ()));
  Vr2f p12n (seed.x () / l12, seed.y () / l12);

  std::vector<Pt2i> pix;
  std::vector<Pt2i>::iterator pit;
  do
  {
    Plateau *pl = plateau (i);
    if (pl->inserted (acc))
    {
      sint = pl->internalStart () * iratio;
      eint = pl->internalEnd () * iratio;
      if (lacks ++)
      {
        sdif = (slast - sint) / lacks;
        edif = (elast - eint) / lacks;
        if (i < 0 && i + lacks > 1) lacks = 1 - i;
        if (i > 0 && i - lacks < 0) lacks = i;
      }
      while (lacks)
      {
        lacks --;
        std::vector<Pt2i> line;
        bool cherche = true, in = false;
        float sval = sint + sdif * lacks;
        float eval = eint + edif * lacks;
        int cur = i + (i < 0 ? lacks : - lacks);
        if (cur == 0) ds->first (pix);
        else
        {
          ds->bindTo (a, b, plateau (cur)->scanShift ());
          if ((i > 0 && rev) || (i < 0 && ! rev)) ds->nextOnRight (pix);
          else ds->nextOnLeft (pix);
        }
        for (pit = pix.begin (); cherche && pit != pix.end (); pit ++)
        {
          Vr2i p1x (seed_p1.vectorTo (*pit));
          float dist = p12n.x () * p1x.x () + p12n.y () * p1x.y ();
          if (! in)
          {
//            if ((rev && dist < eval) || ((! rev) && dist >= sval)) in = true;
if ((rev && dist <= eval) || ((! rev) && dist >= sval)) in = true;
          }
          if (in)
          {
//            if ((rev && dist < sval) || ((! rev) && dist >= eval))
if ((rev && dist < sval) || ((! rev) && dist > eval))
              cherche = false;
            else line.push_back (*pit);
          }
        }
        pts->push_back (line);
      }
      slast = sint;
      elast = eint;
    }
    else lacks ++;

    // Updates the scanner traversal
    if (i == mini)
    {
      i = 1;
      lacks = (flacks > 0 ? blacks + flacks - 1 : 0);
      slast = plateau(-blacks)->internalStart () * iratio;
      elast = plateau(-blacks)->internalEnd () * iratio;
      std::reverse (pts->begin (), pts->end ());
    }
    else i += (i <= 0 ? -1 : 1);
  }
  while (i <= maxi);
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
    if (sdraw == -1) sdraw = (int) (scan->size ()) - 1;
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
    if (sdraw == -1) sdraw = (int) (scan->size ()) - 1;
    if (edraw == -1) edraw = (int) (scan->size ()) - 1;
    spt.push_back ((*scan)[sdraw]);
    ept.push_back ((*scan)[edraw]);
  }
}
