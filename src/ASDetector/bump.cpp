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

#include "bump.h"
#include <cmath>
#include "bsproto.h"

#define EPSILON 0.0001f

const int Bump::RES_NONE = 0;
const int Bump::RES_OK = 1;
const int Bump::RES_NOT_ENOUGH_INPUT_PTS = -1;
const int Bump::RES_HOLE_IN_INPUT_PTS = -2;
const int Bump::RES_TOO_LOW = -3;
const int Bump::RES_TOO_NARROW = -4;
const int Bump::RES_EMPTY_SCAN = -5;
const int Bump::RES_NO_BUMP_LINE = -6;
const int Bump::RES_LINEAR = -7;
const int Bump::RES_ANGULAR = -8;

const int Bump::DEF_NONE = 0;
const int Bump::DEF_POSITION = 1;
const int Bump::DEF_ALTITUDE = 2;
const int Bump::DEF_HEIGHT = 4;
const int Bump::DEF_WIDTH = 8;

const float Bump::REL_ASSIGNED_THICKNESS = 0.05f;
const float Bump::REL_PINCH_LENGTH = 0.1f;
const int Bump::PINCH_COUNT = 12;
const float Bump::REL_PINCH_MARGIN = 0.6f;
const int Bump::MIN_TREND_SIZE = 10;
const int Bump::MAX_TREND_INTERRUPT = 4;

const float Bump::MAX_LINE_TRANSLATION_RATIO = 0.8f;
const float Bump::MAX_LINE_ROTATION_RATIO = 0.95f;
const float Bump::RATIO_INC = 0.01f;


Bump::Bump (BumpModel *bmod)
{
  this->bmod = bmod;
  status = RES_NONE;
  def = DEF_NONE;
  accepted = false;
  over = bmod->isOver ();

  ref = NULL;
  s_est = 0.0f;
  e_est = 0.0f;
  h_est = 0.0f;
  w_est = 0.0f;
  a_num = 0;
  s_num = 0;
  e_num = 0;
  s_int = 0.0f;
  e_int = 0.0f;
  s_ext = 0.0f;
  e_ext = 0.0f;
  slope_est = 0.0f;
  dev_est = 0.0f;

  start_trend = NULL;
  end_trend = NULL;
  trends_height = 0.0f;

  mline_p = false;
  mline_trsl = 0.0f;
  mline_rot = 0.0f;
  mline_start.set (start_est);
  mline_end.set (end_est);
  mline_sind = s_num;
  mline_eind = e_num;
  mline_tind = a_num;

  area_est = 0.0f;
  area_up = 0.0f;
  area_low = 0.0f;
}


Bump::~Bump ()
{
  if (start_trend != NULL) delete start_trend;
  if (end_trend != NULL) delete end_trend;
}


bool Bump::detect (const std::vector<Pt2f> &ptsh, float l12)
{
  // Checks input point vector size
  if ((int) ptsh.size () < bmod->minCountOfPoints ())
  {
    status = RES_NOT_ENOUGH_INPUT_PTS;
    return false;
  }

  // Checks input points distribution
  if (hasHole (ptsh, l12))
  {
    status = RES_HOLE_IN_INPUT_PTS;
    return false;
  }

  // Initializes bounds
  s_num = 0;
  e_num = (int) (ptsh.size ()) - 1;

  // Searches trends on profile sides
  if (bmod->isDetectingTrend ())
  {
    trends_height = ptsh.front().y ();
    s_num = setTrend (ptsh, 0, l12, false) - 1;
    if (s_num == e_num)
    {
      status = RES_LINEAR;
      return false;
    }
    e_num = (int) (ptsh.size ()) - setTrend (ptsh, s_num, l12, true);
    if (e_num == s_num)
    {
      status = RES_ANGULAR;
      return false;
    }
  }

  bool ok = getBump (ptsh);

  if (ok) updateMeasure (&ptsh);
  return (ok);
}


bool Bump::track (const std::vector<Pt2f> &ptsh, float l12,
                  Bump *ref, int refdist)
{
  // Updates assigned reference pattern
  this->ref = ref;
  float refx = ref->estimatedCenter ().x ();
  if (bmod->isDeviationPredictionOn () || refdist > 1)
    refx += ref->estimatedDeviation () * (refdist - 1);
  float refy = ref->estimatedCenter ().y ();
  if (bmod->isSlopePredictionOn () || refdist > 1)
    refy += ref->estimatedSlope () * (refdist - 1);
  Pt2f center_ref (refx, refy);

  // Checks input point vector size
  if ((int) ptsh.size () < bmod->minCountOfPoints ())
  {
    status = RES_NOT_ENOUGH_INPUT_PTS;
    return false;
  }

  // Checks input points distribution (also when tracking ???)
  if (hasHole (ptsh, l12))
  {
    status = RES_HOLE_IN_INPUT_PTS;
    return false;
  }

  // Initializes bounds
  s_num = 0;
  e_num = (int) (ptsh.size ()) - 1;

  // Searches trends on profile sides
  if (bmod->isDetectingTrend ())
  {
    trends_height = ptsh.front().y ();
    AbsRat x1, y1, x2, y2;
    float tx, tw;
    float smaxx = ref->estimatedCenter().x ();
    float emaxx = ref->estimatedSurfCenter().x ();
    if (smaxx > emaxx)
    {
      smaxx = emaxx;
      emaxx = ref->estimatedCenter().x ();
    }
    DigitalStraightSegment *ltrend
      = ref->lastStartTrend (bmod->trendPersistence ());
    if (ltrend != NULL)
    {
      ltrend->naiveLine (x1, y1, x2, y2);
      tx = x1.num () * 0.0005f / x1.den () + x2.num () * 0.0005f / x2.den ();
      tw = x2.num () * 0.001f / x2.den () - x1.num () * 0.001f / x1.den ();
      if (ptsh.front().x () < tx)
        s_num = trackTrend (ptsh, tx, tw, smaxx, false);
      if (start_trend == NULL) // second trial
      {
        s_num = 0;
        s_num = setTrend (ptsh, 0, l12, false) - 1;
      }
      if (s_num == e_num)
      {
        status = RES_LINEAR;
        return false;
      }
    }
    else
    {
      s_num = setTrend (ptsh, 0, l12, false) - 1;
      if (s_num == e_num)
      {
        status = RES_LINEAR;
        return false;
      }
    }
    ltrend = ref->lastEndTrend (bmod->trendPersistence ());
    if (ltrend != NULL)
    {
      ltrend->naiveLine (x1, y1, x2, y2);
      tx = x1.num () * 0.0005f / x1.den () + x2.num () * 0.0005f / x2.den ();
      tw = x2.num () * 0.001f / x2.den () - x1.num () * 0.001f / x1.den ();
      if (ptsh.back().x () > tx)
        e_num = (int) (ptsh.size ()) - 1
                - trackTrend (ptsh, tx, tw, emaxx, true);
      if (end_trend == NULL) // second trial
      {
        e_num = (int) (ptsh.size ()) - 1;
        e_num = (int) (ptsh.size ()) - setTrend (ptsh, s_num, l12, true);
      }
      if (e_num == s_num)

      {
        status = RES_ANGULAR;
        return false;
      }
    }
    else
    {
      e_num = (int) (ptsh.size ()) - setTrend (ptsh, s_num, l12, true);
      if (e_num == s_num)
      {
        status = RES_ANGULAR;
        return false;
      }
    }
  }

  // Detects the bump
  if (! getBump (ptsh)) return false;

  // Checks consistency to input pattern
  float pshift = (bmod->massReferenced () ? mcenter_est.x () : scenter_est.x ())
                 - center_ref.x ();
  float ashift = (bmod->massReferenced () ? mcenter_est.y () : scenter_est.y ())
                 - center_ref.y ();
  float absrefw = (ref->w_est < 0 ? - ref->w_est : ref->w_est);
  float absrefh = (ref->h_est < 0 ? - ref->h_est : ref->h_est);
  if (bmod->positionControl () == 1)
  {
    if (bmod->outOfPositionTolerance (pshift)) def |= DEF_POSITION;
  }
  else if (bmod->positionControl () == 2)
  {
    if (bmod->outOfPositionRelTolerance (pshift, absrefw))
      def |= DEF_POSITION;
  }
  if (bmod->altitudeControl () == 1)
  {
    if (bmod->outOfAltitudeTolerance (ashift)) def |= DEF_ALTITUDE;
  }
  else if (bmod->altitudeControl () == 2)
  {
    if (bmod->outOfAltitudeRelTolerance (ashift, absrefh))
      def |= DEF_ALTITUDE;
  }
  if (bmod->widthControl () == 1)
  {
    if (bmod->outOfWidthTolerance (w_est - ref->w_est)) def |= DEF_WIDTH;
  }
  else if (bmod->widthControl () == 2)
  {
    if (bmod->outOfWidthRelTolerance (w_est - ref->w_est, absrefw))
      def |= DEF_WIDTH;
  }
  if (bmod->heightControl () == 1)
  {
    if (bmod->outOfHeightTolerance (h_est - ref->h_est)) def |= DEF_HEIGHT;
  }
  else if (bmod->heightControl () == 2)
  {
    if (bmod->outOfHeightRelTolerance (h_est - ref->h_est, absrefh))
      def |= DEF_HEIGHT;
  }
  accepted = (def == DEF_NONE);

  updateMeasure (&ptsh);
  return (accepted);
}


bool Bump::getBump (const std::vector<Pt2f> &ptsh)
{

  // Constructs the baseline (ax + by = c)
  if (! setBaseLine (ptsh, s_num, e_num))
  {
    if (start_trend == NULL || end_trend == NULL)
    {
      status = RES_NO_BUMP_LINE;
      return false;
    }
  }
  // base of the convex hull if one trend missing
  line_start.set (ptsh[s_num]);
  line_end.set (ptsh[e_num]);
  float a = line_end.y () - line_start.y ();
  float b = line_start.x () - line_end.x ();
  float c = a * line_start.x () + b * line_start.y ();
  float den2 = a * a + b * b;
  float den = (float) sqrt (den2);

  // Checks bump size and width
  if (e_num - s_num < bmod->minCountOfPoints ())
  {
    status = RES_NOT_ENOUGH_INPUT_PTS;
    return false;
  }
  if (line_end.x () - line_start.x () < bmod->minWidth ())
  {
    status = RES_TOO_NARROW;
    return false;
  }

  // Sets bump summit position (since baseline is determined)
  h_est = getSummit (ptsh, a, b, c) / den;

  // Checks bump height
  if ((over && h_est < bmod->minHeight ())
      || ((! over) && h_est > - bmod->minHeight ()))
  {
    status = RES_TOO_LOW;
    return false;
  }

  // Sets reference bounds
  s_int = ptsh[s_num].x ();
  if (s_num == 0) s_ext = s_int - bmod->undetectedBoundDistance ();
  else s_ext = ptsh[s_num - 1].x ();
  e_int = ptsh[e_num].x ();
  if (e_num == ((int) ptsh.size ()) - 1)
    s_ext = e_int + bmod->undetectedBoundDistance ();
  else e_ext = ptsh[e_num + 1].x ();
  start_est.set (ptsh[s_num]);
  end_est.set (ptsh[e_num]);
  s_est = s_int;
  e_est = e_int;
  w_est = end_est.x () - start_est.x ();

  // Checks amount of points on bump reference area
  if (e_num - s_num < bmod->minCountOfPoints ())
  {
    status = RES_NOT_ENOUGH_INPUT_PTS;
    return false;
  }

  // Checks bump width
  if (e_est - s_est < bmod->minWidth ())
  {
    status = RES_TOO_NARROW;
    return false;
  }

  // Finds orthogonal split line
  float area = verticalSplit (ptsh, a, b, c, den2);

  // Finds center of mass
    setMassCenter (ptsh, a, b, c, den, area / 2);

  status = RES_OK;
  accepted = true;
  return (true);
}


bool Bump::hasHole (const std::vector<Pt2f> &ptsh, float l12) const
{
  float dhole = 0.;
  std::vector<Pt2f>::const_iterator it = ptsh.begin ();
  float px = it->x ();
  while (++it != ptsh.end ())
  {
    if (it->x () - px > dhole) dhole = it->x () - px;
    px = it->x ();
  }
  return (dhole > l12 * bmod->holeMaxRelativeLength ());
}


int Bump::setTrend (const std::vector<Pt2f> &ptsh,
                    int limit, float l12, bool rev)
{
  limit = (int) (ptsh.size ()) - limit;
  int incr = (rev ? -1 : 1);
  std::vector<Pt2f>::const_iterator it
                = (rev ? (ptsh.end () - 1) : ptsh.begin ());
  Pt2i pt ((int) (it->x () * 1000 + 0.5),
           (int) ((it->y () - trends_height) * 1000 + 0.5f));
  int xstart = pt.x ();
  int ass_thick = (int) (l12 * REL_ASSIGNED_THICKNESS * 1000);
  if (ass_thick < bmod->trendMinPinch ()) ass_thick = bmod->trendMinPinch ();
  int pinch_len = (int) (l12 * REL_PINCH_LENGTH * 1000);
  BSProto *bsp = new BSProto (ass_thick, pt);
  bool scanning = true;
  bool unpinched = true;
  int length = 1;
  while (scanning && length < limit)
  {
    it += incr;
    pt.set ((int) (it->x () * 1000 + 0.5),
            (int) ((it->y () - trends_height) * 1000 + 0.5f));
    bool added = bsp->addRightSorted (pt);
    if (added)
    {
      int xcur = pt.x ();
      length ++;
      if (unpinched && (length > PINCH_COUNT || xcur - xstart > pinch_len))
      {
        EDist pinch_th (bsp->digitalThickness());
        int nth = (int) ((1 + REL_PINCH_MARGIN) * pinch_th.num ());
        if (nth < ass_thick * pinch_th.den ())
        {
          if (nth < bmod->trendMinPinch () * pinch_th.den ())
          {
            bsp->setMaxWidth (EDist (bmod->trendMinPinch (), 1));
          }
          else
          {
            bsp->setMaxWidth (EDist (nth, pinch_th.den ()));
          }
        }
        unpinched = false;
      }
    }
    else scanning = false;
  }
  if (unpinched || length < MIN_TREND_SIZE) length = 1;
  else if (rev) end_trend = bsp->endOfBirth()->holdSegment ();
  else start_trend = bsp->endOfBirth()->holdSegment ();
  return (length);
}


int Bump::trackTrend (const std::vector<Pt2f> &ptsh, float tx, float tw,
                      float maxx, bool rev)
{
  int incr = (rev ? -1 : 1);
  std::vector<Pt2f>::const_iterator its
                = (rev ? (ptsh.end () - 1) : ptsh.begin ());
  std::vector<Pt2f>::const_iterator ite
                = (rev ? ptsh.begin () : (ptsh.end () - 1));
  Pt2i pt ((int) (its->x () * 1000 + 0.5),
           (int) ((its->y () - trends_height) * 1000 + 0.5f));

  int ict = (int) (tx * 1000 + (tx < 0 ? -0.5f : 0.5f));
  int imax = (int) (maxx * 1000 + (maxx < 0 ? -0.5f : 0.5f));
  int ifirst = 0, ilast = 0;
  int i = 0;
  bool searchf = true, searchl = true;
  std::vector<Pt2i> ptsi;
  do
  {
    int ix = (int) (its->x () * 1000 + (its->x () < 0 ? - 0.5f : 0.5f));
    if (searchf && ix > ict)
    {
      searchf = false;
      ifirst = i;
      if (i != 0 && (ix - ict) * incr > (ict - ptsi.back().x ()) * incr)
        ifirst --;
      // don't test imax here, because next test already valid even further
    }
    else if (searchl && ix > imax)
    {
      searchl = false;
      ilast = i;
      if (ilast != 0 && (ix - imax) * incr > (imax - ptsi.back().x ()) * incr)
        ilast --;
    }
    ptsi.push_back (
      Pt2i (ix, (int) ((its->y () - trends_height) * 1000 + 0.5f)));
    i ++;
    its += incr;
  }
  while (searchl && its != ite);

  // Creates a putative blurred segment
  int asst = rev ? (int) (endTrendsMaxThickness (bmod->trendPersistence ())
                          * 500.0f + 0.5f)
                 : (int) (startTrendsMaxThickness (bmod->trendPersistence ())
                          * 500.0f + 0.5f);
  if (asst < bmod->trendMinPinch ()) asst = bmod->trendMinPinch ();
  int pinch_len = (int) (tw * 250 + 0.5f);
  BSProto *bsp = new BSProto (asst, ptsi[ifirst]);

  // Extends the blurred segment
  int ie = ifirst + 1;
  int is = ifirst - 1;
  int isLarge = true;
  int lstop = 0, rstop = 0;
  bool rscan = (is >= 0);
  bool lscan = (ie < (int) (ptsi.size ()));
  int lextent = 0, rextent = 0;
  std::vector<int> bsadds;
  while (rscan || lscan)
  {
    while (rscan && (rextent <= lextent || ! lscan))
    {
      bool added = bsp->addRightSorted (ptsi[is]);
      rextent = (ptsi[ifirst].x () - ptsi[is].x ()) * incr;
      if (isLarge && rextent + lextent > pinch_len)
      {
        EDist pinch_th (bsp->digitalThickness ());
        int nth = (int) ((1 + REL_PINCH_MARGIN) * pinch_th.num ());
        if (nth < asst * pinch_th.den ())
        {
          if (nth < bmod->trendMinPinch () * pinch_th.den ())
            bsp->setMaxWidth (EDist (bmod->trendMinPinch (), 1));
          else bsp->setMaxWidth (EDist (nth, pinch_th.den ()));
        }
        isLarge = false;
      }
      if (added)
      {
        rstop = 0;
        bsadds.push_back (is);
      }
      else if (++rstop > MAX_TREND_INTERRUPT) rscan = false;
      if (--is < 0) rscan = false;
    }
    while (lscan && (lextent <= rextent || ! rscan))
    {
      bool added = bsp->addLeftSorted (ptsi[ie]);
      lextent = (ptsi[ie].x () - ptsi[ifirst].x ()) * incr;
      if (isLarge && rextent + lextent > pinch_len)
      {
        EDist pinch_th (bsp->digitalThickness ());
        int nth = (int) ((1 + REL_PINCH_MARGIN) * pinch_th.num ());
        if (nth < asst * pinch_th.den ())
        {
          if (nth < bmod->trendMinPinch () * pinch_th.den ())
            bsp->setMaxWidth (EDist (bmod->trendMinPinch (), 1));
          else bsp->setMaxWidth (EDist (nth, pinch_th.den ()));
        }
        isLarge = false;
      }
      if (added)
      {
        lstop = 0;
        bsadds.push_back (ie);
      }
      else if (++lstop > MAX_TREND_INTERRUPT) lscan = false;
      if (++ie >= ilast) lscan = false;
    }
  }
  is += rstop + 1;
  ie -= lstop + 1;

  // Analyses the BS
  if (isLarge || ie + 1 - is < MIN_TREND_SIZE) return (0);
  if (rev) end_trend = bsp->endOfBirth()->holdSegment ();
  else start_trend = bsp->endOfBirth()->holdSegment ();
  return (rev ? is : ie);
}


bool Bump::setBaseLine (const std::vector<Pt2f> &ptsh, int &s_num, int &e_num)
{
  // Computes the line L(a,b,c) that joins the sequence ends
  Pt2f start = ptsh[s_num];
  Pt2f end = ptsh[e_num];
  float a = end.y () - start.y ();
  float b = start.x () - end.x ();
  float c = a * start.x () + b * start.y ();
  float a2, b2, c2;
  float absa = (a < 0.0f ? - a : a);
  float absb = (b < 0.0f ? - b : b);
  float eps = EPSILON * (absa < absb ? absb : absa);


  // Find highest point over L (lowest for hollow structures)
  double h = 0.f;
  int s = -1;
  for (int i = s_num + 1; i < e_num; i++)
  {
    double hloc = c - a * ptsh[i].x () - b * ptsh[i].y ();
    if ((over && hloc > h) || ((! over) && hloc < h))
    {
      h = hloc;
      s = i;
    }
  }
  if (s == -1) return false;  // not a bump line

  // Builds sub-sequences
  std::vector<int> seq[2];
  std::vector<bool> ok[2];
  for (int i = s_num; i < s; i++)
  {
    seq[0].push_back (i);
    ok[0].push_back (true);
  }
  for (int i = e_num; i > s; i--)
  {
    seq[1].push_back (i);
    ok[1].push_back (true);
  }

  int f[] = {0, 0};
  bool searching = true;
  do
  {
    // Finds deepest points (highest for hollow structures)
    searching = false;
    h = 0.0f;
    for (int j = 0; j < 2; j++)
    {
      for (int i = 1; i < (int) (seq[j].size ()); i++)
      {
        if (ok[j][i])
        {
          double hloc = c - a * ptsh[seq[j][i]].x () - b * ptsh[seq[j][i]].y ();
          if (i != f[j] && ((over && hloc < h - eps)
                            || ((! over) && hloc > h + eps)))
          {
            h = hloc;
            f[j] = i;
            searching = true;
          }
        }
      }
    }
    if (searching)
    {
      // Sets L on deepest points (highest for hollow structures)
      a = ptsh[seq[1][f[1]]].y () - ptsh[seq[0][f[0]]].y ();
      b = ptsh[seq[0][f[0]]].x () - ptsh[seq[1][f[1]]].x ();
      c = a * ptsh[seq[0][f[0]]].x () + b * ptsh[seq[0][f[0]]].y ();

      for (int j = 0; j < 2; j++)
      {
        // Withdraw inner points
        for (int i = f[j] + 1; i < (int) (seq[j].size()); i ++)
          if (ok[j][i]
              && ((over && c + eps >= a * ptsh[seq[j][i]].x ()
                                      + b * ptsh[seq[j][i]].y ())
                  || ((! over) && c - eps <= a * ptsh[seq[j][i]].x ()
                                             + b * ptsh[seq[j][i]].y ())))
            ok[j][i] = false;

        // Withdraw outer points
        a2 = ptsh[seq[j][f[j]]].y () - ptsh[seq[j][0]].y ();
        b2 = ptsh[seq[j][0]].x () - ptsh[seq[j][f[j]]].x ();
        c2 = a2 * ptsh[seq[j][0]].x () + b2 * ptsh[seq[j][0]].y ();
        for (int i = 1; i < f[j]; i++)
          if (ok[j][i]
              && ((over && c2 + eps > a2 * ptsh[seq[j][i]].x ()
                                      + b2 * ptsh[seq[j][i]].y ())
                  || ((! over) && c2 - eps < a2 * ptsh[seq[j][i]].x ()
                                             + b2 * ptsh[seq[j][i]].y ())))
            ok[j][i] = false;
      }
    }
  }
  while (searching);
  s_num = seq[0][f[0]];
  e_num = seq[1][f[1]];
  return (true);
}


float Bump::getSummit (const std::vector<Pt2f> &ptsh,
                      float a, float b, float c)
{
  float smax = 0.0f;
  int cindex = s_num;
  std::vector<Pt2f>::const_iterator it = ptsh.begin () + s_num;
  std::vector<Pt2f>::const_iterator lastit = ptsh.begin () + e_num;
  while (++it != lastit)
  {
    cindex ++;
    float s = (c - a * it->x () - b * it->y ());
    if ((over && s > smax) || ((! over) && s < smax))
    {
      summit_est.set (it->x (), it->y ());
      smax = s;
      a_num = cindex;
    }
  }
  return (smax);
}


float Bump::getBiasedAreaAndSurfaceCenter (const std::vector<Pt2f> &ptsh,
                           float a, float b, float c, float den)
{
  std::vector<Pt2f>::const_iterator it = ptsh.begin () + s_num;

  // Finds bump section area
  float h = (c - a * it->x () - b * it->y ());
  float area = (it->x () - start_est.x ()) * h / 2; 
  float hint = h;
  float xval = s_int;
  float vend = 0.0f, vloc = 0.0f;
  int index = s_num;
  do  // from start to end
  {
    it ++;
    index ++;
    h = (c - a * it->x () - b * it->y ());
    area += (it->x () - xval) * (h + hint) / 2;
    xval = it->x ();
    hint = h;
  }
  while (index < e_num);
  area += (end_est.x () - e_int) * hint / 2;
  if (! over) area = - area;

  // Finds split line and surface point above center of mass
  while (vend < area / 2)
  {
    it --;  // from end to split line
    h = (c - a * it->x () - b * it->y ());
    vloc = (xval - it->x ()) * (h + hint) / 2;
    if (! over) vloc = - vloc;
    vend += vloc;
    xval = it->x ();
    hint = h;
  }
  float yint = it->y ();
  it ++;
  float ratio = ((vloc < 0.01f && vloc > -0.01f) ?
                 0.0f : (vend - area / 2) / vloc);
  scenter_est.set (xval + ratio * (it->x () - xval),
                  yint + ratio * (it->y () - yint));

  return (area / den);
}


float Bump::verticalSplit (const std::vector<Pt2f> &ptsh,
                           float a, float b, float c, float den2)
{
  std::vector<Pt2f>::const_iterator it1 = ptsh.begin () + s_num;
  std::vector<Pt2f>::const_iterator it2 = ptsh.begin () + e_num;
  float absa = (a < 0.0f ? - a : a);
  float absb = (b < 0.0f ? - b : b);
  float eps = EPSILON * (absa < absb ? absb : absa);
  float cs = a * start_est.y () - b * start_est.x ();
  float ce = a * end_est.y () - b * end_est.x ();
  int sig = ce < cs ? -1 : 1;
  Pt2f sold (start_est);
  Pt2f eold (end_est);
  Pt2f saold (start_est);
  Pt2f eaold (end_est);
  it1 ++;
  it2 --;
  float sh = 0.0f, eh = 0.0f;
  float svol = 0.0f, evol = 0.0f, savol = 0.0f, eavol = 0.0f;
  float hnext = 0.0f, cnext = 0.0f, deltac = 0.0f;
  int nbsrat = 0, nberat = 0;
  while (it1 != it2)
  {
    if (evol > svol)
    {
      hnext = (c - a * it1->x () - b * it1->y ());
      cnext = a * it1->y () - b * it1->x ();
      deltac = (cnext - cs) * sig;
      if (deltac > 0.0f)
      {
        savol = svol;
        if (over) svol += deltac * (sh + hnext) / 2;
        else svol -= deltac * (sh + hnext) / 2;
        sh = hnext;
        cs = cnext;
        saold.set (sold);
        sold.set (*it1);
      }
      else nbsrat ++;
      it1 ++;
    }
    else
    {
      hnext = (c - a * it2->x () - b * it2->y ());
      cnext = a * it2->y () - b * it2->x ();
      deltac = (ce - cnext) * sig;
      if (deltac > 0.0f)
      {
        eavol = evol;
        if (over) evol += deltac * (eh + hnext) / 2;
        else evol -= deltac * (eh + hnext) / 2;
        eh = hnext;
        ce = cnext;
        eaold.set (eold);
        eold.set (*it2);
      }
      else nberat ++;
      it2 --;
    }
  }
  if (evol > svol)
  {
    float vrat = (evol - eavol > eps || evol - eavol < - eps ?
                  ((evol + svol) / 2 - eavol) / (evol - eavol) : 0.0f);
    scenter_est.set (eaold.x () + vrat * (eold.x () - eaold.x ()),
                     eaold.y () + vrat * (eold.y () - eaold.y ()));
  }
  else if (svol > evol)
  {
    float vrat = (svol - savol > eps || svol - savol < - eps ?
                  ((evol + svol) / 2 - savol) / (svol - savol) : 0.0f);
    scenter_est.set (saold.x () + vrat * (sold.x () - saold.x ()),
                     saold.y () + vrat * (sold.y () - saold.y ()));
  }
  else scenter_est.set (sold);
  return ((evol + svol) / den2);
}


void Bump::setMassCenter (const std::vector<Pt2f> &ptsh,
                          float a, float b, float c, float den, float area_2)
{
  std::vector<Pt2f>::const_iterator it1 = ptsh.begin () + s_num;
  std::vector<Pt2f>::const_iterator it2 = ptsh.begin () + e_num;

  float vloc = 0.0f;
  float h = 0.0f;
  float absa = (a < 0.0f ? - a : a);
  float absb = (b < 0.0f ? - b : b);
  float eps = EPSILON * (absa < absb ? absb : absa);
  float oldv = vloc, oldh = h;
  Pt2f spt (start_est);
  Pt2f ept (end_est);
  float sh = 0.0f, eh = 0.0f;
  float dist = (float) sqrt ((spt.vectorTo (ept)).norm2 ());
  float curd = dist;
  float hrat = 0.0f;
  while (it1 < it2 && sh <= h - eps)
  {
    it1 ++;
    sh = (c - a * it1->x () - b * it1->y ());
    if (! over) sh = -sh;
  }
  while (it2 > it1 && eh <= h - eps)
  {
    it2 --;
    eh = (c - a * it2->x () - b * it2->y ());
    if (! over) eh = -eh;
  }
  while (it1 != it2 && vloc < area_2)
  {
    oldh = h;
    oldv = vloc;
    if (sh < eh)
    {
      hrat = (eh - h > eps || eh - h < - eps ? (sh - h) / (eh - h) : 0.0f);
      ept.set (ept.x () + (it2->x () - ept.x ()) * hrat,
               ept.y () + (it2->y () - ept.y ()) * hrat);
      curd = (float) sqrt ((ept.vectorTo (*it1)).norm2 ());
      vloc += (curd + dist) * (sh - h) / (2 * den);
      h = sh;
      do
      {
        it1 ++;
        sh = (c - a * it1->x () - b * it1->y ());
        if (! over) sh = -sh;
      }
      while (it1 != it2 && sh <= h);
    }
    else
    {
      hrat = (sh - h > eps || sh - h < - eps ? (eh - h) / (sh - h) : 0.0f);
      spt.set (spt.x () + (it1->x () - spt.x ()) * hrat,
               spt.y () + (it1->y () - spt.y ()) * hrat);
      curd = (float) sqrt ((spt.vectorTo (*it2)).norm2 ());
      vloc += (curd + dist) * (eh - h) / (2 * den);
      h = eh;
      do
      {
        it2 --;
        eh = (c - a * it2->x () - b * it2->y ());
        if (! over) eh = -eh;
      }
      while (it1 != it2 && eh <= h);
    }
    dist = curd;
  }
  h = (vloc - oldv > eps || vloc - oldv < - eps ?
       oldh + (h - oldh) * (area_2 - oldv) / (vloc - oldv) : oldh);
  if (! over) h = -h;
  float cc = b * scenter_est.x () - a * scenter_est.y ();
  mcenter_est.set ((a * (c - h) + b * cc) / (a * a + b * b),
                   (b * (c - h) - a * cc) / (a * a + b * b));
}


float Bump::startTrendThickness () const
{
  return (start_trend == NULL ?
          0.0f : start_trend->width () * 0.001f / start_trend->period ());
}


float Bump::startTrendsMaxThickness (int nb) const
{
  float th1 = (start_trend == NULL ?
               0.0f : start_trend->width () * 0.001f / start_trend->period ());
  float th2 = (nb == 1 || ref == NULL ?
               0.0f : ref->startTrendsMaxThickness (nb - 1));
  return (th1 > th2 ? th1 : th2);
}


Pt2f Bump::startTrendStart () const
{
  if (start_trend == NULL) return (Pt2f (0.0f, 0.0f));
  AbsRat x1, y1, x2, y2;
  start_trend->naiveLine (x1, y1, x2, y2);
  return Pt2f (x1.num () * 0.001f / x1.den (),
               trends_height + y1.num () * 0.001f / y1.den ());
}

Pt2f Bump::startTrendEnd () const
{
  if (start_trend == NULL) return (Pt2f (0.0f, 0.0f));
  AbsRat x1, y1, x2, y2;
  start_trend->naiveLine (x1, y1, x2, y2);
  return Pt2f (x2.num () * 0.001f / x2.den (),
               trends_height + y2.num () * 0.001f / y2.den ());
}

DigitalStraightSegment *Bump::lastStartTrend (int nb) const
{
  if (start_trend != NULL) return start_trend;
  if (nb == 1 || ref == NULL) return NULL;
  return (ref->lastStartTrend (nb - 1));
}


float Bump::endTrendThickness () const
{
  return (end_trend == NULL ?
          0.0f : end_trend->width () * 0.001f / end_trend->period ());
}

float Bump::endTrendsMaxThickness (int nb) const
{
  float th1 = (end_trend == NULL ?
               0.0f : end_trend->width () * 0.001f / end_trend->period ());
  float th2 = (nb == 1 || ref == NULL ?
               0.0f : ref->endTrendsMaxThickness (nb - 1));
  return (th1 > th2 ? th1 : th2);
}

Pt2f Bump::endTrendStart () const
{
  if (end_trend == NULL) return (Pt2f (0.0f, 0.0f));
  AbsRat x1, y1, x2, y2;
  end_trend->naiveLine (x1, y1, x2, y2);
  return Pt2f (x1.num () * 0.001f / x1.den (),
               trends_height + y1.num () * 0.001f / y1.den ());
}

Pt2f Bump::endTrendEnd () const
{
  if (end_trend == NULL) return (Pt2f (0.0f, 0.0f));
  AbsRat x1, y1, x2, y2;
  end_trend->naiveLine (x1, y1, x2, y2);
  return Pt2f (x2.num () * 0.001f / x2.den (),
               trends_height + y2.num () * 0.001f / y2.den ());
}

DigitalStraightSegment *Bump::lastEndTrend (int nb) const
{
  if (end_trend != NULL) return end_trend;
  if (nb == 1 || ref == NULL) return NULL;
  return (ref->lastEndTrend (nb - 1));
}


void Bump::updateMeasure (const std::vector<Pt2f> *ptsh)
{
  if (bmod->isMeasured () && status == RES_OK) setMeasureLine (ptsh);
}


void Bump::incMeasureLineTranslationRatio (int inc,
                                           const std::vector<Pt2f> *ptsh)
{
  setMeasureLineTranslationRatio (mline_trsl + inc * RATIO_INC, ptsh);
}

void Bump::setMeasureLineTranslationRatio (float val,
                                           const std::vector<Pt2f> *ptsh)
{
  if (val < 0.0) val = 0.0f;
  else if (val > MAX_LINE_TRANSLATION_RATIO)
    val = MAX_LINE_TRANSLATION_RATIO;
  bool increase = (val > mline_trsl);
  mline_trsl = val;
  if (increase) setMeasureLineRotationRatio (mline_rot, ptsh);
  else
  {
    mline_p = (mline_trsl > RATIO_INC / 2 || mline_rot < - RATIO_INC / 2
                                          || mline_rot > RATIO_INC / 2);
    if (ptsh != NULL) updateMeasure (ptsh);
  }
}

void Bump::incMeasureLineRotationRatio (int inc,
                                        const std::vector<Pt2f> *ptsh)
{
  setMeasureLineRotationRatio (mline_rot + inc * RATIO_INC, ptsh);
}

void Bump::setMeasureLineRotationRatio (float val,
                                        const std::vector<Pt2f> *ptsh)
{
  mline_rot = val;
  if (mline_rot - mline_trsl < - MAX_LINE_ROTATION_RATIO)
    mline_rot = mline_trsl - MAX_LINE_ROTATION_RATIO;
  else if (mline_rot + mline_trsl > MAX_LINE_ROTATION_RATIO)
    mline_rot = MAX_LINE_ROTATION_RATIO - mline_trsl;
  mline_p = (mline_trsl > RATIO_INC / 2 || mline_rot < - RATIO_INC / 2
                                        || mline_rot > RATIO_INC / 2);
  if (ptsh != NULL) updateMeasure (ptsh);
}


void Bump::setMeasureLine (const std::vector<Pt2f> *ptsh)
{
  if (! mline_p)
  {
    mline_start.set (start_est);
    mline_end.set (end_est);
    mline_sind = s_num + 1;
    mline_eind = e_num - 1;
    mline_eind = a_num;
    estimateArea (ptsh);
    setAreaUpperBound (ptsh);
    setAreaLowerBound (ptsh);
    return;
  }

  // Sets baseline equation
  float a = end_est.y () - start_est.y ();
  float b = start_est.x () - end_est.x ();
  float c = a * start_est.x () + b * start_est.y ();
  float absa = (a < 0.0f ? - a : a);
  float absb = (b < 0.0f ? - b : b);
  float eps = EPSILON * (absa < absb ? absb : absa);
  float hini = a * summit_est.x () + b * summit_est.y () - c;
  int sig = (hini < 0 ? -1 : 1);
  hini *= sig;

  // Sets height levels
  float hs = hini * mline_trsl;
  float he = hs;
  float hold;
  float incl = mline_rot;
  if (incl < 0.0f)
  {
    hs -= hini * incl;
    if (hs < 0.0f) hs = 0.0f;
  }
  else if (incl > 0.0f)
  {
    he += hini * incl;
    if (he > 0.95f * hini) he = 0.95f * hini;
  }

  // Searches start point
  float h = hini;
  mline_eind = a_num;
  mline_sind = a_num;
  std::vector<Pt2f>::const_iterator fit = ptsh->begin () + s_num;
  std::vector<Pt2f>::const_iterator it = ptsh->begin () + mline_sind;
  do
  {
    hold = h;
    it --;
    mline_sind --;
    h = (a * it->x () + b * it->y () - c) * sig;
  }
  while (h > hs && it != fit);
  Pt2f pt (*it++);
  Pt2f npt (*it);
  float sx = pt.x ();
  float sy = pt.y ();
  if (hold - h > eps || hold - h < - eps)
  {
    sx += (npt.x () - pt.x ()) * (hs - h) / (hold - h);
    sy += (npt.y () - pt.y ()) * (hs - h) / (hold - h);
  }
  mline_start.set (sx, sy);
  mline_sind ++;

  // Searches end point
  h = hini;
  fit = ptsh->begin () + e_num;
  it = ptsh->begin () + mline_eind;
  do
  {
    hold = h;
    it ++;
    mline_eind ++;
    h = (a * it->x () + b * it->y () - c) * sig;
  }
  while (h > he && it != fit);
  pt.set (*it--);
  npt.set (*it);
  sx = pt.x ();
  sy = pt.y ();
  if (hold - h > eps || hold - h < - eps)
  {
    sx += (npt.x () - pt.x ()) * (he - h) / (hold - h);
    sy += (npt.y () - pt.y ()) * (he - h) / (hold - h);
  }
  mline_end.set (sx, sy);
  mline_eind --;

  // Searches summit
  a = mline_end.y () - mline_start.y ();
  b = mline_start.x () - mline_end.x ();
  c = a * mline_start.x () + b * mline_start.y ();
  hold = (a * it->x () + b * it->y () - c) * sig;
  mline_tind = mline_eind;
  int ipt = mline_eind;
  while (ipt != mline_sind)
  {
    it --;
    ipt --;
    h = (a * it->x () + b * it->y () - c) * sig;
    if (h > hold)
    {
      hold = h;
      mline_tind = ipt;
    }
  }
  estimateArea (ptsh);
  setAreaUpperBound (ptsh);
  setAreaLowerBound (ptsh);
}


void Bump::estimateArea (const std::vector<Pt2f> *ptsh)
{
  Pt2f mept (mline_p ? mline_end : end_est);
  Pt2f mspt (mline_p ? mline_start : start_est);
  int sind = (mline_p ? mline_sind : s_num);
  int eind = (mline_p ? mline_eind : e_num);

  // Base line parameters
  float a = mept.y () - mspt.y ();
  float b = mspt.x () - mept.x ();
  float c = a * mspt.x () + b * mspt.y ();

  std::vector<Pt2f>::const_iterator it1 = ptsh->begin () + sind;
  std::vector<Pt2f>::const_iterator it2 = ptsh->begin () + eind;

  float oldx = mspt.x (), oldy = 0.0f;
  float posx = it1->x (), dy = it1->y () - (c - a * it1->x ()) / b;
  area_est = (posx - oldx) * dy / 2;

  // Area integration
  while (it1 != it2)
  {
    it1 ++;
    oldx = posx;
    oldy = dy;
    posx = it1->x ();
    dy = it1->y () - (c - a * it1->x ()) / b;
    area_est += (posx - oldx) * (dy + oldy) / 2;
  }

  area_est += (mept.x () - posx) * dy / 2;
  if (area_est < 0.0f) area_est = - area_est;
}


void Bump::setAreaUpperBound (const std::vector<Pt2f> *ptsh)
{
  area_up = 0.0f;
  int sig = (over ? 1 : -1);
  Pt2f mept (mline_p ? mline_end : end_est);
  Pt2f mspt (mline_p ? mline_start : start_est);
  int sind = (mline_p ? mline_sind : s_num + 1);
  int eind = (mline_p ? mline_eind : e_num - 1);

  // Baseline parameters
  float a = mept.y () - mspt.y ();
  float b = mspt.x () - mept.x ();
  float c = a * mspt.x () + b * mspt.y ();
  float den = (float) sqrt (a * a + b * b);
  float absa = (a < 0.0f ? - a : a);
  float absb = (b < 0.0f ? - b : b);
  float eps = EPSILON * (absa < absb ? absb : absa);
  std::vector<Pt2f>::const_iterator it1 = ptsh->begin () + sind;
  std::vector<Pt2f>::const_iterator it2 = ptsh->begin () + eind;
  float sh = (c - a * it1->x () - b * it1->y ()) * sig;
  float eh = (c - a * it2->x () - b * it2->y ()) * sig;

  float h = 0.0f;
  Pt2f spt (mspt);
  Pt2f ept (mept);
  float dist = (float) sqrt ((spt.vectorTo(ept)).norm2 ());
  float curd = 0.0f;
  float hrat = 0.0f;
  do
  {
    if (sh < eh)
    {
      hrat = (eh - h > eps || eh - h < - eps ? (sh - h) / (eh - h) : 0.0f);
      ept.set (ept.x () + (it2->x () - ept.x ()) * hrat,
               ept.y () + (it2->y () - ept.y ()) * hrat);
      curd = (float) sqrt ((ept.vectorTo(*it1)).norm2 ());
      area_up += (curd + dist) * (sh - h) / 2;
      h = sh;
      do
      {
        it1 ++;
        sh = (c - a * it1->x () - b * it1->y ()) * sig;
      }
      while (sh <= h && it1 != it2);
    }
    else
    {
      hrat = (sh - h > eps || sh -h < - eps ? (eh - h) / (sh - h) : 0.0f);
      spt.set (spt.x () + (it1->x () - spt.x ()) * hrat,
               spt.y () + (it1->y () - spt.y ()) * hrat);
      curd = (float) sqrt ((spt.vectorTo(*it2)).norm2 ());
      area_up += (curd + dist) * (eh - h) / 2;
      h = eh;
      do
      {
        it2 --;
        eh = (c - a * it2->x () - b * it2->y ()) * sig;
      }
      while (eh <= h && it1 != it2);
    }
    dist = curd;
  }
  while (it1 != it2);
  area_up += (0.0f + dist) * (eh - h) / 2;
  area_up /= den;
}


void Bump::setAreaLowerBound (const std::vector<Pt2f> *ptsh)
{
  area_low = 0.0f;
  int sig = (over ? 1 : -1);
  Pt2f mept (mline_p ? mline_end : end_est);
  Pt2f mspt (mline_p ? mline_start : start_est);
  int sind = (mline_p ? mline_sind : s_num);
  int eind = (mline_p ? mline_eind : e_num);
  int tind = (mline_p ? mline_tind : a_num);

  // Baseline parameters
  float a = mept.y () - mspt.y ();
  float b = mspt.x () - mept.x ();
  float c = a * mspt.x () + b * mspt.y ();
  float den = (float) sqrt (a * a + b * b);
  float absa = (a < 0.0f ? - a : a);
  float absb = (b < 0.0f ? - b : b);
  float eps = EPSILON * (absa < absb ? absb : absa);

  std::vector<Pt2f>::const_iterator it1 = ptsh->begin () + tind;
  float h = (c - a * it1->x () - b * it1->y ()) * sig;
  Pt2f spt (*it1);
  Pt2f ept (*it1);
  std::vector<Pt2f>::const_iterator it2 = it1-- + 1;
  float sh = (c - a * it1->x () - b * it1->y ()) * sig;
  float eh = (c - a * it2->x () - b * it2->y ()) * sig;

  int inds = tind - 1, inde = tind + 1;
  float dist = 0.0f;
  float curd = dist;
  float hrat = 0.0f;

  bool sdown = (inds >= sind), edown = (inde <= eind);
  while (sdown || edown)
  {
    if (sh > eh)
    {
      hrat = (h - eh > eps || h - eh < - eps ? (h - sh) / (h - eh) : 0.0f);
      ept.set (ept.x () + (it2->x () - ept.x ()) * hrat,
               ept.y () + (it2->y () - ept.y ()) * hrat);
      curd = (float) sqrt ((ept.vectorTo(*it1)).norm2 ());
      area_low += (curd + dist) * (h - sh) / 2;
      h = sh;
      do
      {
        it1 --;
        inds --;
        sh = (c - a * it1->x () - b * it1->y ()) * sig;
        if (sh < eps || inds < sind || it1 == ptsh->begin ())
        {
          sdown = false;
          sh = -100.0f; // to force end side down
        }
      }
      while (sdown && sh >= h);
    }
    else
    {
      hrat = (h - sh > eps || h - sh < - eps ? (h - eh) / (h - sh) : 0.0f);
      spt.set (spt.x () + (it1->x () - spt.x ()) * hrat,
               spt.y () + (it1->y () - spt.y ()) * hrat);
      curd = (float) sqrt ((spt.vectorTo(*it2)).norm2 ());
      area_low += (curd + dist) * (h - eh) / 2;
      h = eh;
      do
      {
        it2 ++;
        inde ++;
        eh = (c - a * it2->x () - b * it2->y ()) * sig;
        if (eh < eps || inde > eind || it2 == ptsh->end () - 1)
        {
          edown = false;
          eh = -100.0f; // to force start side down
        }
      }
      while (edown && eh >= h);
    }
    dist = curd;
  }
  curd = (float) sqrt ((mspt.vectorTo(mept)).norm2 ());
  area_low += (curd + dist) * h / 2;
  area_low /= den;
}


float Bump::estimatedWidth (std::vector<Pt2f> *ptsh, float hrat)
{
  float a = end_est.y () - start_est.y ();
  float b = start_est.x () - end_est.x ();
  float c = a * start_est.x () + b * start_est.y ();
  float absa = (a < 0.0f ? - a : a);
  float absb = (b < 0.0f ? - b : b);
  float eps = EPSILON * (absa < absb ? absb : absa);
  std::vector<Pt2f>::iterator lit = ptsh->begin ();
  std::vector<Pt2f>::iterator it = lit + a_num;
  float h = a * it->x () + b * it->y () - c;
  int sig = (h < 0 ? - 1 : 1);
  h *= sig;
  float hprev = h, hnext = 0.0f;

  bool searching = true;
  while (it != lit && searching)
  {
    hnext = (a * it->x () + b * it->y () - c) * sig;
    if (hnext < h * hrat)
      searching = false;
    else
    {
      hprev = hnext;
      it --;
    }
  }
  float rat = (hprev - hnext < eps ?
               0.0f : (hprev - h * hrat) / (hprev - hnext));
  Pt2f spos ((it - 1)->x () + (it->x () - (it - 1)->x ()) * rat,
             (it - 1)->y () + (it->y () - (it - 1)->y ()) * rat);

  it = lit + a_num;
  lit = ptsh->end () - 1;
  hprev = h;
  searching = true;
  while (it != lit && searching)
  {
    hnext = (a * it->x () + b * it->y () - c) * sig;
    if (hnext < h * hrat)
      searching = false;
    else
    {
      hprev = hnext;
      it ++;
    }
  }
  rat = (hprev - hnext < eps ?
               0.0f : (hprev - h * hrat) / (hprev - hnext));
  Pt2f epos ((it - 1)->x () + (it->x () - (it - 1)->x ()) * rat,
             (it - 1)->y () + (it->y () - (it - 1)->y ()) * rat);
 
  return ((float) sqrt ((spos.x () - epos.x ()) * (spos.x () - epos.x ())
                        + (spos.y () - epos.y ()) * (spos.y () - epos.y ())));
}


/*
void Bump::trace () const
{
  std::cout << "S_INT = " << s_int << " et E_INT = " << e_int << std::endl;
  std::cout << "S_PR = " << (s_int - s_ext)
            << " et E_PR = " << (e_ext - e_int) << std::endl;
}

void Bump::traceExt (const std::vector<Pt2f> &ptsh) const
{
  std::vector<Pt2f>::const_iterator it = ptsh.begin (); 
  int i = 0;
  while (i < e_num + 2 && it != ptsh.end ())
  {
    if (i >= s_num - 1) std::cout << "TRACE (" << it->x ()
                                  << ", " << it->y () << ")" << std::endl;
    i ++;
    it ++;
  }
}
*/
