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

#include "ridge.h"
#include "pt3f.h"
#include <cmath>

#define EPSILON 0.0001f

const float Ridge::MIN_HEIGHT = 0.2f;
const float Ridge::MAX_WIDTH = 8.0f;


Ridge::Ridge ()
{
  curright = NULL;
  curleft = NULL;
}


Ridge::~Ridge ()
{
  std::vector<RidgeSection *>::iterator it = lefts.begin ();
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


void Ridge::start (Bump *bump, const std::vector<Pt2i> &dispix, bool reversed)
{
  startsec.setReversed (reversed);
  startsec.add (bump, dispix);
  curright = new RidgeSection ();
  curright->setReversed (reversed);
  rights.push_back (curright);
  curleft = new RidgeSection ();
  curleft->setReversed (reversed);
  lefts.push_back (curleft);
}


void Ridge::start (Bump *bump, const std::vector<Pt2i> &dispix,
                   const std::vector<Pt2f> &pts, bool reversed)
{
  startsec.setReversed (reversed);
  startsec.add (bump, dispix, pts);
  curright = new RidgeSection ();
  curright->setReversed (reversed);
  rights.push_back (curright);
  curleft = new RidgeSection ();
  curleft->setReversed (reversed);
  lefts.push_back (curleft);
}


void Ridge::add (bool onright, Bump *bump, const std::vector<Pt2i> &dispix,
                                           const std::vector<Pt2f> &pts)
{
  if (onright) curright->add (bump, dispix, pts);
  else curleft->add (bump, dispix, pts);
}


void Ridge::add (bool onright, Bump *bump, const std::vector<Pt2i> &dispix)
{
  if (onright) curright->add (bump, dispix);
  else curleft->add (bump, dispix);
}


int Ridge::getRightScanCount () const
{
  int count = 0;
  std::vector<RidgeSection *>::const_iterator it = rights.begin ();
  while (it != rights.end ()) count += (*it++)->getScanCount ();
  return count;
}


int Ridge::getLeftScanCount () const
{
  int count = 0;
  std::vector<RidgeSection *>::const_iterator it = lefts.begin ();
  while (it != lefts.end ()) count += (*it++)->getScanCount ();
  return count;
}


Bump *Ridge::bump (int num) const
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
    return (rights[scan]->bump (num));
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
    return (lefts[scan]->bump (num));
  }
  else return (startsec.bump (0));
}


float Ridge::getHeightReference (int num) const
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


bool Ridge::isScanReversed (int num) const
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


std::vector<Pt2i> *Ridge::getDisplayScan (int num)
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


Bump *Ridge::lastValidBump (int num) const
{
  bool searching = true;
  Bump *ret = NULL;
  do
  {
    ret = bump (num);
    if ((ret != NULL && ret->isFound ()) || num == 0) searching = false;
    else (num -= (num < 0 ? -1 : 1));
  }
  while (searching);
  return ret;
}


std::vector<Pt2f> *Ridge::getProfile (int num)
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


void Ridge::getPosition (std::vector<Pt2i> &pts, std::vector<Pt2i> &pts2,
                         int disp, float iratio, bool smoothed) 
{
  if (startsec.bump (0) != NULL)
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
      if (disp > RIDGE_DISP_BOUNDS)
        for (int num = mini; num <= maxi; num++)
          addBumpCenter (pts, num, rev, disp, pp1, p12, l12, iratio, smoothed);
      else
        for (int num = mini; num <= maxi; num++)
          addBumpBounds (pts, pts2, num, rev, pp1, p12, l12, iratio, smoothed);
    }
  }
}

void Ridge::addBumpCenter (std::vector<Pt2i> &pt, int num, bool rev,
                           int disp, Pt2i pp1, Vr2i p12, float l12,
                           float iratio, bool smoothed)
{
  Bump* bmp = bump (num);
  if (bmp != NULL && bmp->inserted (smoothed))
  {
    int sdraw = -1, snum = 0;
    float sint = (disp == RIDGE_DISP_SPINE ?
                  bmp->estimatedSummit ().x () : bmp->estimatedCenter().x ())
                 * iratio;
    std::vector<Pt2i>* scan = getDisplayScan (num);
    std::vector<Pt2i>::const_iterator it = scan->begin ();
    while (it != scan->end ())
    {
      Vr2i p1x = pp1.vectorTo (*it);
      float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
      if (rev)
      {
        if (sdraw == -1 && dist < sint) sdraw = snum - 1;
      }
      else
      {
        if (sdraw == -1 && dist >= sint) sdraw = snum;
      }
      snum ++;
      it ++;
    }
    if (rev)
    {
      if (sdraw == -1) sdraw = (int) (scan->size ()) - 1;
    }
    pt.push_back ((*scan)[sdraw]);
  }
}

void Ridge::addBumpBounds (std::vector<Pt2i> &spt, std::vector<Pt2i> &ept,
                           int num, bool rev, Pt2i pp1, Vr2i p12, float l12,
                           float iratio, bool smoothed)
{
  Bump* bmp = bump (num);
  if (bmp != NULL && bmp->inserted (smoothed))
  {
    int sdraw = -1, edraw = -1, snum = 0;
    float sint = bmp->internalStart () * iratio;
    float eint = bmp->internalEnd () * iratio;
    std::vector<Pt2i>* scan = getDisplayScan (num);
    std::vector<Pt2i>::const_iterator it = scan->begin ();
    while (it != scan->end ())
    {
      Vr2i p1x = pp1.vectorTo (*it);
      float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
      if (rev)
      {
        if (edraw == -1 && dist <= eint) edraw = snum;
        if (sdraw == -1 && dist < sint) sdraw = snum - 1;
      }
      else
      {
        if (sdraw == -1 && dist >= sint) sdraw = snum;
        if (edraw == -1 && dist > eint) edraw = snum - 1;
      }
      snum ++;
      it ++;
    }
    if (rev)
    {
      if (sdraw == -1) sdraw = (int) (scan->size ()) - 1;
    }
    else
    {
      if (edraw == -1) edraw = (int) (scan->size ()) - 1;
    }
    spt.push_back ((*scan)[sdraw]);
    ept.push_back ((*scan)[edraw]);
  }
}


float Ridge::scanPeriod (float iratio)
{
  std::vector<Pt2i> *scan = getDisplayScan (0);
  Pt2i spt (scan->front ());
  Pt2i ept (scan->back ());
  int b = (ept.x () > spt.x () ? ept.x () - spt.x () : spt.x () - ept.x ());
  int a = (ept.y () > spt.y () ? ept.y () - spt.y () : spt.y () - ept.y ());
  float dist = (float) (a > b ? a : b);
  dist /= (float) sqrt (a * a + b * b);
  return (dist / iratio);
}


Pt2f Ridge::localize (int num, float pos, float irat)
{
  std::vector<Pt2i> *scan = getDisplayScan (0);
  bool rev = isScanReversed (0);
  Pt2i p1 (scan->front ());
  Pt2i p2 (scan->back ());
  float p1px = 0.f, p1py = 0.f;
  int p12x = p2.x () - p1.x ();
  int p12y = p2.y () - p1.y ();
  float l12 = (float) sqrt (p12x * p12x + p12y * p12y);
  float q = 0.0f;
  if (p12y > (p12x < 0 ? - p12x : p12x))
  {
    q = num * p12x / (irat * l12);
    p1px = p12x * (pos + q) / l12 - num / irat;
    p1py = p12y * (pos + q) / l12;
  }
  else
  {
    if (p12x < 0) num = - num;
    q = num * p12y / (irat * l12);
    p1px = p12x * (pos - q) / l12;
    p1py = p12y * (pos - q) / l12 + num /irat;
  }
  return (Pt2f (rev ? p2.x () - p1px : p1.x () + p1px,
                rev ? p2.y () - p1py : p1.y () + p1py));
}


int Ridge::countOfMeasureLines () const
{
  int nb = 0;
  int m1 = - getRightScanCount ();
  int m2 = getLeftScanCount ();
  for (int i = m1; i <= m2; i++)
    if (bump(i)->hasMeasureLine ()) nb ++;
  return nb;
}


void Ridge::getMeasureLines (std::vector<float> &measures)
{
  int m1 = - getRightScanCount ();
  int m2 = getLeftScanCount ();
  for (int i = m1; i <= m2; i++)
  {
    Bump *bmp = bump (i);
    measures.push_back ((float) (i));
    measures.push_back (bmp->getMeasureLineTranslationRatio ());
    measures.push_back (bmp->getMeasureLineRotationRatio ());
  }
}


void Ridge::setMeasureLines (std::vector<float> &measures)
{
  int m1 = - getRightScanCount ();
  int m2 = getLeftScanCount ();
  std::vector<float>::iterator it = measures.begin ();
  while (it != measures.end ())
  {
    float val = *it++;
    int num = (int) (val + (val < 0.0f ? -0.5f : 0.5f));
    float trsl = *it++;
    float rot = *it++;
    if (num >= m1 && num <= m2)
    {
      Bump *bmp = bump (num);
      bmp->setMeasureLineTranslationRatio (trsl);
      bmp->setMeasureLineRotationRatio (rot);
    }
  }
  updateMeasure ();
}


void Ridge::updateMeasure ()
{
  int m1 = - getRightScanCount ();
  int m2 = getLeftScanCount ();
  for (int i = m1; i <= m2; i++)
  {
    Bump *bmp = bump (i);
    if (bmp->isFound ()) bmp->updateMeasure (getProfile (i));
  }
}


float Ridge::estimateVolume (int m1, int m2, float iratio,
                             float &meas_low, float &meas_up)
{
  float meas_est = 0.0f;
  meas_low = 0.0f;
  meas_up = 0.0f;
  bool start_found = false;
  float mest, mlow, mup, pest, plow, pup;
  int step = 0, lg1 = 0, lg2 = 0;
  if (m1 > m2) { int tmp = m1; m1 = m2; m2 = tmp; }
  if (m1 < - getRightScanCount ()) m1 = - getRightScanCount ();
  if (m2 > getLeftScanCount ()) m2 = getLeftScanCount ();
  for (int i = m1; i <= m2; i++)
  {
    step ++;
    Bump* bmp = bump (i);
    if (bmp != NULL && bmp->isFound ())
    {
      mest = bmp->estimatedArea ();
      mlow = bmp->estimatedAreaLowerBound ();
      mup = bmp->estimatedAreaUpperBound ();
      if (! start_found)
      {
        pest = mest;
        plow = mlow;
        pup = mup;
        lg1 = i;
        lg2 = i;
        start_found = true;
        step = 0;
      }
      else
      {
        meas_est += (mest + pest) * step / 2;
        meas_low += (mlow + plow) * step / 2;
        meas_up += (mup + pup) * step / 2;
        step = 0;
        lg2 = i;
      }
    }
  }
  if (start_found && lg2 != lg1)
  {
    float isd = scanPeriod (iratio);
    meas_est *= isd;
    meas_low *= isd;
    meas_up *= isd;
  }
  else
  {
    meas_est = 0.0f;
    meas_low = 0.0f;
    meas_up = 0.0f;
  }
  return meas_est;
}


float Ridge::estimateSlope (int m1, int m2, float irat, float &lg2, float &lg3,
                            float &zmin, float &zmax)
{
  bool nopt = true;
  lg2 = 0.0f;
  lg3 = 0.0f;
  Pt2f cen2, oldcen2;
  Pt3f cen3, oldcen3;
  if (m1 > m2) { int tmp = m1; m1 = m2; m2 = tmp; }
  if (m1 < - getRightScanCount ()) m1 = - getRightScanCount ();
  if (m2 > getLeftScanCount ()) m2 = getLeftScanCount ();
  for (int i = m1; i <= m2; i++)
  {
    if (bump(i)->isAccepted ())
    {
      Pt2f pt = bump(i)->estimatedCenter ();
      cen2.set (localize (i, pt.x (), irat));
      cen3.set (cen2.x (), cen2.y (), pt.y ());
      if (nopt)
      {
        zmin = pt.y ();
        zmax = zmin;
        nopt = false;
      }
      else
      {
        if (pt.y () < zmin) zmin = pt.y ();
        else if (pt.y () > zmax) zmax = pt.y ();
        lg2 += oldcen2.distance (cen2);
        lg3 += oldcen3.distance (cen3);
      }
      oldcen2.set (cen2); 
      oldcen3.set (cen3); 
    }
  }
  if (lg2 < EPSILON) return (0.0f);
  return (100 * (zmax - zmin) / lg2);
}


int Ridge::meanWidth (int m1, int m2, float mhratio,
                      float &mwidth, float &sigma)
{
  mwidth = 0.0f;
  sigma = 0.0f;
  std::vector<float> meas;
  if (m1 > m2) { int tmp = m1; m1 = m2; m2 = tmp; }
  if (m1 < - getRightScanCount ()) m1 = - getRightScanCount ();
  if (m2 > getLeftScanCount ()) m2 = getLeftScanCount ();
  for (int i = m1; i <= m2; i++)
  {
    if (bump(i)->isAccepted ())
    {
      float w = bump(i)->estimatedWidth (getProfile (i), mhratio);
      meas.push_back (w);
      mwidth += w;
    }
  }
  if (meas.empty ()) return 0;
  mwidth /= ((int) meas.size ());
  std::vector<float>::iterator it = meas.begin ();
  while (it != meas.end ())
  {
    sigma += (*it - mwidth) * (*it - mwidth);
    it ++;
  }
  sigma = (float) sqrt (sigma / (int) (meas.size ()));
  return ((int) meas.size ());
}


int Ridge::meanHeight (int m1, int m2, float &mheight, float &sigma)
{
  mheight = 0.0f;
  sigma = 0.0f;
  std::vector<float> meas;
  if (m1 > m2) { int tmp = m1; m1 = m2; m2 = tmp; }
  if (m1 < - getRightScanCount ()) m1 = - getRightScanCount ();
  if (m2 > getLeftScanCount ()) m2 = getLeftScanCount ();
  for (int i = m1; i <= m2; i++)
  {
    if (bump(i)->isAccepted ())
    {
      float h = bump(i)->estimatedHeight ();
      if (h < 0.0f) h = - h;
      meas.push_back (h);
      mheight += h;
    }
  }
  if (meas.empty ()) return 0;
  mheight /= ((int) meas.size ());
  std::vector<float>::iterator it = meas.begin ();
			float min = 10000.0f;
			float max = -10000.0f;
  while (it != meas.end ())
  {
			float val = *it - mheight;
			if (val < min) min = val;
			if (val > max) max = val;
    sigma += (*it - mheight) * (*it - mheight);
    it ++;
  }
  sigma = (float) sqrt (sigma / (int) (meas.size ()));
  return ((int) (meas.size ()));
}
