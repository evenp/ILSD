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

#include "ridgedetector.h"
#include <cmath>
#include <algorithm>

const int RidgeDetector::RESULT_NONE = 0;
const int RidgeDetector::RESULT_OK = 1;
const int RidgeDetector::RESULT_FAIL_TOO_NARROW_INPUT = -1;
const int RidgeDetector::RESULT_FAIL_NO_AVAILABLE_SCAN = -2;
const int RidgeDetector::RESULT_FAIL_NO_CENTRAL_BUMP = -3;

const int RidgeDetector::MIN_SELECTION_SIZE = 8;
const int RidgeDetector::DEFAULT_BUMP_LACK_TOLERANCE = 20;
const int RidgeDetector::INITIAL_RIDGE_EXTENT = 6;
const int RidgeDetector::DEFAULT_POS_AND_HEIGHT_REGISTER_SIZE = 8;


RidgeDetector::RidgeDetector ()
{
  ptset = NULL;
  profileRecordOn = false;
  bump_lack_tolerance = DEFAULT_BUMP_LACK_TOLERANCE;
  initial_ridge_extent = 0; // direction precalculation off
  fbg = NULL;
  fstatus = RESULT_NONE;
  ibg = NULL;
  istatus = RESULT_NONE;
  l12 = 1.0f;
  posht_nb = DEFAULT_POS_AND_HEIGHT_REGISTER_SIZE;
  lpok = new bool[posht_nb];
  lpos = new float[posht_nb];
  lhok = new bool[posht_nb];
  lht = new float[posht_nb];
  resetPositionsAndHeights (false, Pt2f (0.0f,0.0f));
}


RidgeDetector::~RidgeDetector ()
{
  clear ();
}


void RidgeDetector::clear ()
{
  if (fbg != NULL) delete fbg;
  fbg = NULL;
  fstatus = RESULT_NONE;
  if (ibg != NULL) delete ibg;
  ibg = NULL;
  istatus = RESULT_NONE;
}


void RidgeDetector::setPointsGrid (IPtTileSet *data, int width, int height,
                                   int subdiv, float cellsize)
{
  ptset = data;
  this->subdiv = subdiv;
  this->csize = cellsize;
  scanp.setSize (width * subdiv, height * subdiv);
  discanp.setSize (width, height);
}


Ridge *RidgeDetector::detect (const Pt2i &p1, const Pt2i &p2)
{
  // Cleans up former detection
  clear ();

  // Checks input stroke length
  ip1.set (p1);
  ip2.set (p2);
  fp1.set (p1);
  fp2.set (p2);
  Vr2f p12 (csize * (p2.x () - p1.x ()), csize * (p2.y () - p1.y ()));
  l12 = (float) sqrt (p12.x () * p12.x () + p12.y () * p12.y ());
  if (l12 < (float) MIN_SELECTION_SIZE)
  {
    fstatus = RESULT_FAIL_TOO_NARROW_INPUT;
    if (initial_ridge_extent != 0) istatus = RESULT_FAIL_TOO_NARROW_INPUT;
    return NULL;
  }

  // Initial detection (or final if no initial detection)
  detect (initial_ridge_extent);

  if (ibg != NULL && istatus != RESULT_FAIL_NO_CENTRAL_BUMP)
  {
    // Aligns input stroke orthogonally to detected ridge
    float fact = csize / (p12.x () * p12.x () + p12.y () * p12.y ());
    Pt2f p1f (csize * (p1.x () + 0.5f), csize * (p1.y () + 0.5f));
    Vr2f lshift;
    if (p12.x () > 0)
      if (p12.y () > 0)
        if (p12.x () > p12.y ())
          lshift.set (- (p12.x () * p12.y ()) * fact,
                      (p12.x () * p12.x ()) * fact);
        else
          lshift.set (- (p12.y () * p12.y ()) * fact,
                      (p12.x () * p12.y ()) * fact);
      else
        if (p12.x () > - p12.y ())
          lshift.set (- (p12.x () * p12.y ()) * fact,
                      (p12.x () * p12.x ()) * fact);
        else
          lshift.set ((p12.y () * p12.y ()) * fact,
                      - (p12.x () * p12.y ()) * fact);
    else
      if (p12.y () > 0)
        if (- p12.x () > p12.y ())
          lshift.set ((p12.x () * p12.y ()) * fact,
                      - (p12.x () * p12.x ()) * fact);
        else
          lshift.set (- (p12.y () * p12.y ()) * fact,
                      (p12.x () * p12.y ()) * fact);
      else
        if (p12.x () < p12.y ())
          lshift.set ((p12.x () * p12.y ()) * fact,
                      - (p12.x () * p12.x ()) * fact);
        else
          lshift.set ((p12.y () * p12.y ()) * fact,
                      - (p12.x () * p12.y ()) * fact);
    std::vector<Pt2f> pc;
    Bump *rbmp = ibg->bump (0);
    float rbmpc = rbmp->estimatedCenter().x () / l12;
    pc.push_back (Pt2f (p1f.x () + p12.x () * rbmpc,
                        p1f.y () + p12.y () * rbmpc));
    for (int i = 1; i < initial_ridge_extent ; i++)
    {
      rbmp = ibg->bump (i);
      if (rbmp != NULL && rbmp->getStatus () == Bump::RES_OK)
      {
        rbmpc = rbmp->estimatedCenter().x () / l12;
        pc.push_back (Pt2f (p1f.x () + i * lshift.x () + p12.x () * rbmpc,
                            p1f.y () + i * lshift.y () + p12.y () * rbmpc));
      }
      rbmp = ibg->bump (-i);
      if (rbmp != NULL && rbmp->getStatus () == Bump::RES_OK)
      {
        rbmpc = rbmp->estimatedCenter().x () / l12;
        pc.push_back (Pt2f (p1f.x () - i * lshift.x () + p12.x () * rbmpc,
                            p1f.y () - i * lshift.y () + p12.y () * rbmpc));
      }
    }
    // Realignment (position and orientation) and complete detection
    if ((int) (pc.size ()) > initial_ridge_extent)
    {
      alignInput (pc);
      detect (0);
    }
  }
  return (fbg);
}


void RidgeDetector::detect (int exlimit)
{
  Pt2i p1, p2;
  getInputStroke (p1, p2, exlimit != 0);
  Vr2f p12n ((p2.x () - p1.x ()) * csize / l12,
             (p2.y () - p1.y ()) * csize / l12);
  Pt2f p1f (csize * (p1.x () + 0.5f), csize * (p1.y () + 0.5f));

  // Creates adaptive directional scanners for point cloud and display
  DirectionalScanner *ds = scanp.getScanner (
    Pt2i (p1.x () * subdiv + subdiv / 2, p1.y () * subdiv + subdiv / 2),
    Pt2i (p2.x () * subdiv + subdiv / 2, p2.y () * subdiv + subdiv / 2),
    true);
  ds->releaseClearance ();
  DirectionalScanner *disp = discanp.getScanner (p1, p2, true);

  // Gets the central scan of the point cloud
  std::vector<Pt2i> pix;
  int nbp = ds->first (pix);
  for (int i = 0; nbp != 0 && i < subdiv / 2; i++)
    nbp = ds->nextOnRight (pix);
  nbp = 1;
  for (int i = 0; nbp != 0 && i < subdiv - 1 - subdiv / 2; i++)
    nbp = ds->nextOnLeft (pix);
  if (pix.empty ())
  {
    if (exlimit != 0)
      istatus = RESULT_FAIL_NO_AVAILABLE_SCAN;
    else fstatus = RESULT_FAIL_NO_AVAILABLE_SCAN;
    return;
  }

  // Gets the central scan for display
  std::vector<Pt2i> dispix;
  disp->first (dispix);

  // Gets and sorts scanned points by distance to first stroke point
  std::vector<Pt2f> cpts;
  std::vector<Pt2i>::iterator it = pix.begin ();
  while (it != pix.end ())
  {
    std::vector<Pt3f> ptcl;
    ptset->collectPoints (ptcl, it->x (), it->y ());
    std::vector<Pt3f>::iterator pit = ptcl.begin ();
    while (pit != ptcl.end ())
    {
      Vr2f pcl (pit->x () - p1f.x (), pit->y () - p1f.y ());
      cpts.push_back (Pt2f (pcl.scalarProduct (p12n), pit->z ()));
      pit ++;
    }
    it ++;
  }
  sort (cpts.begin (), cpts.end (), compFurther);

  // Detects the central bump
  Ridge *ridge = new Ridge ();
  if (exlimit != 0) ibg = ridge;
  else fbg = ridge;
  Bump *bmp = new Bump (&bfeat);
  bool success = bmp->detect (cpts, l12);
  if (profileRecordOn) ridge->start (bmp, dispix, cpts,
                                     scanp.isLastScanReversed ());
  else ridge->start (bmp, dispix, scanp.isLastScanReversed ());
  if (! success)
  {
    if (exlimit != 0)
      istatus = RESULT_FAIL_NO_CENTRAL_BUMP;
    else fstatus = RESULT_FAIL_NO_CENTRAL_BUMP;
    return;
  }

  // Sets template and detects next bumps on each side
  DirectionalScanner *ds2 = ds->getCopy ();
  DirectionalScanner *disp2 = disp->getCopy ();

  resetPositionsAndHeights (bmp->isAccepted (), bmp->estimatedCenter());
  track (true, scanp.isLastScanReversed (), exlimit,
         ds, disp, p1f, p12n, bmp);

  resetPositionsAndHeights (bmp->isAccepted (), bmp->estimatedCenter());
  track (false, scanp.isLastScanReversed (), exlimit,
         ds2, disp2, p1f, p12n, bmp);
}


void RidgeDetector::track (bool onright, bool reversed, int exlimit,
              DirectionalScanner *ds, DirectionalScanner *disp,
              Pt2f p1f, Vr2f p12n, Bump *refbmp)
{
  bool search = true;
  int nbfail = 0;
  int num = (onright ? -1 : 1);
  if (onright) exlimit = - exlimit;
  Ridge *ridge = (exlimit != 0 ? ibg : fbg);
  int confdist = 1;
  while (search && num != exlimit)
  {
    // Adaptive scan recentering on reference pattern
    Pt2i p1, p2;
    getInputStroke (p1, p2, exlimit != 0);
    int a = p2.x () - p1.x ();
    int b = p2.y () - p1.y ();
    if (a < 0.)
    {
      a = -a;
      b = -b;
    }
    float posx = p1.x () + ((p2.x () - p1.x ()) * csize / l12)
                           * refbmp->estimatedCenter().x () / csize;
    float posy = p1.y () + ((p2.y () - p1.y ()) * csize / l12)
                           * refbmp->estimatedCenter().x () / csize;
    float valc = a * posx + b * posy;
    int c = (int) (valc < 0.0f ? valc - 0.5f : valc + 0.5f);

    disp->bindTo (a, b, c);
    ds->bindTo (a, b, c * subdiv + subdiv / 2);

    // Collects next scan points and sorts them by distance
    std::vector<Pt2i> pix;
    std::vector<Pt2i> dispix;
    if ((onright && ! reversed) || (reversed && ! onright))
      disp->nextOnRight (dispix);
    else disp->nextOnLeft (dispix);
    if (dispix.empty ()) search = false;
    else
      for (int i = 0; search && i < subdiv; i++)
        if ((onright && ! reversed) || (reversed && ! onright))
        {
          if (ds->nextOnRight (pix) == 0) search = false;
        }
        else if (ds->nextOnLeft (pix) == 0) search = false;
    if (pix.empty ()) search = false;
    else
    {
      std::vector<Pt2f> pts;
      std::vector<Pt2i>::iterator it = pix.begin ();
      while (it != pix.end ())
      {
        std::vector<Pt3f> ptcl;
        ptset->collectPoints (ptcl, it->x (), it->y ());
        std::vector<Pt3f>::iterator pit = ptcl.begin ();
        while (pit != ptcl.end ())
        {
          Vr2f pcl (pit->x () - p1f.x (), pit->y () - p1f.y ());
          pts.push_back (Pt2f (pcl.scalarProduct (p12n), pit->z ()));
          pit ++;
        }
        it ++;
      }
      sort (pts.begin (), pts.end (), compFurther);

      // Detects the bump and updates the ridge section
      Bump *bump = new Bump (&bfeat);
      bump->track (pts, l12, refbmp, confdist);
      if (profileRecordOn) ridge->add (onright, bump, dispix, pts);
      else ridge->add (onright, bump, dispix);
      if (bump->isAccepted ()) nbfail = 0;
      else if (++nbfail >= bump_lack_tolerance) search = false;

      if (search)
      {
        // Estimates deviation and slope
        bump->setDeviation (updatePosition (bump->isFound (),
                                            bump->estimatedCenter().x ()));
        bump->setSlope (updateHeight (bump->isFound (),
                                      bump->estimatedCenter().y ()));

        // Updates reference pattern for next bump detection
        if (bump->isFound ())
        {
          refbmp = bump;
        }
        if (bump->isAccepted ())
        {
          float c1 = bump->estimatedCenter().x ();
          Bump *lbmp = ridge->bump (num < 0 ? num + confdist : num - confdist);
          float dc = (lbmp->estimatedCenter().x () - c1) / confdist;
          for (int i = 1; i < confdist; i ++)
          {
            lbmp = ridge->bump (num < 0 ? num + i : num - i);
            if (lbmp->contains (c1 + dc * i)) lbmp->accept ();
          }
        }
        confdist = 0;
      }
      confdist ++;
    }
    num += (onright ? -1 : 1);
  }
}


void RidgeDetector::alignInput (const std::vector<Pt2f> &pts)
{
  // Get coordinates expectation
  std::vector<Pt2f>::const_iterator it = pts.begin ();
  float xmin = it->x ();
  float xmax = xmin;
  float ymin = it->y ();
  float ymax = ymin;
  float xm = xmin;
  float ym = ymin;
  it ++;
  while (it != pts.end ())
  {
    xm += it->x ();
    ym += it->y ();
    if (it->x () < xmin) xmin = it->x ();
    else if (it->x () > xmax) xmax = it->x ();
    if (it->y () < ymin) ymin = it->y ();
    else if (it->y () > ymax) ymax = it->y ();
    it ++;
  }
  xm /= (int) (pts.size ());
  ym /= (int) (pts.size ());

  // Get coordiantes variance
  float xv = 0.0f, yv = 0.0f, xyv = 0.0f;
  do
  {
    it --;
    xv += (it->x () - xm) * (it->x () - xm);
    yv += (it->y () - ym) * (it->y () - ym);
    xyv += (it->x () - xm) * (it->y () - ym);
  }
  while (it != pts.begin ());
  xv /= (int) (pts.size ());
  yv /= (int) (pts.size ());
  xyv /= (int) (pts.size ());

  // Get linear approx ax - by = c
  float a = 1.0f, b = 1.0f, c = 0.0f;
  if (xmax - xmin > ymax - ymin)
  {
    a = xyv / xv;
    c = a * xm - ym;
  }
  else
  {
    b = xyv / yv;
    c = xm - b * ym;
  }

  // Get new stroke
  float d = b * it->x () + a * it->y ();
  float den = a * a + b * b;
  float x = (a * c + b * d) / den;
  float y = (a * d - b * c) / den;
  den = sqrt (den);
  int dir = (a * (ip2.x () - ip1.x ()) - b * (ip2.y () - ip1.y ()) < 0 ?
             -1 : 1);
  float length = Ridge::MAX_WIDTH * dir;
  fp1.set ((int) ((x - a * length) / csize),
           (int) ((y + b * length) / csize));
  fp2.set ((int) ((x + a * length) / csize),
           (int) ((y - b * length) / csize));
  Vr2f p12 (csize * (fp2.x () - fp1.x ()), csize * (fp2.y () - fp1.y ()));
  l12 = (float) sqrt (p12.x () * p12.x () + p12.y () * p12.y ());
}


void RidgeDetector::resetPositionsAndHeights (bool ok, Pt2f center)
{
  for (int i = 1; i < posht_nb; i++)
  {
    lpok[i] = false;
    lpos[i] = 0.0f;
    lhok[i] = false;
    lht[i] = 0.0f;
  }
  lpok[0] = ok;
  lpos[0] = center.x ();
  lhok[0] = ok;
  lht[0] = center.y ();
}


float RidgeDetector::updatePosition (bool ok, float pos)
{
  int nbok = 0, last = -1, first = -1;
  for (int i = posht_nb - 1; i > 0; i--)
  {
    lpok[i] = lpok[i-1];
    lpos[i] = lpos[i-1];
    if (lpok[i])
    {
      if (nbok != 0) last = i;
      else first = i;
      nbok ++;
    }
  }
  lpok[0] = ok;
  lpos[0] = pos;
  if (ok)
  {
    if (nbok != 0) last = 0;
    else first = 0;
    nbok ++;
  }

  if (nbok <= 1) return 0.0f;
  float dtrend = 0.0f, trend = (lpos[last] - lpos[first]) / (first - last);
  if (nbok == 2) return (trend);
  int last2 = -1;
  for (int i = first - 1; i > last; i --)
  {
    if (lpok[i])
    {
      if (dtrend == 0.0f)
      {
        dtrend = (lpos[last] - lpos[i]) / (i - last) - trend;
        last2 = i;
      }
      else if (((lpos[last] - lpos[i]) / (i - last) - trend) * dtrend < 0.0f)
        return (trend);
      else last2 = i;
    }
  }
  return ((lpos[last] - lpos[last2]) / (last2 - last));
}


float RidgeDetector::updateHeight (bool ok, float ht)
{
  int nbok = 0, last = -1, first = -1;
  for (int i = posht_nb - 1; i > 0; i--)
  {
    lhok[i] = lhok[i-1];
    lht[i] = lht[i-1];
    if (lhok[i])
    {
      if (nbok != 0) last = i;
      else first = i;
      nbok ++;
    }
  }
  lhok[0] = ok;
  lht[0] = ht;
  if (ok)
  {
    if (nbok != 0) last = 0;
    else first = 0;
    nbok ++;
  }

  if (nbok <= 1) return 0.0f;
  float dtrend = 0.0f, trend = (lht[last] - lht[first]) / (first - last);
  if (nbok == 2) return (trend);
  int last2 = -1;
  for (int i = first - 1; i > last; i --)
  {
    if (lhok[i])
    {
      if (dtrend == 0.0f)
      {
        dtrend = (lht[last] - lht[i]) / (i - last) - trend;
        last2 = i;
      }
      else if (((lht[last] - lht[i]) / (i - last) - trend) * dtrend < 0.0f)
        return (trend);
      else last2 = i;
    }
  }
  return (lht[last] - lht[last2]) / (last2 - last);
}


bool RidgeDetector::compFurther (Pt2f p1, Pt2f p2)
{
  return (p2.x () > p1.x ());
}


bool RidgeDetector::isOver () const
{
  return (bfeat.isOver ());
}


void RidgeDetector::switchOver ()
{
  bfeat.switchOver ();
}


void RidgeDetector::setOver (bool status)
{
  bfeat.setOver (status);
}


void RidgeDetector::incBumpLackTolerance (int dir)
{
  setBumpLackTolerance (bump_lack_tolerance + dir);
}


void RidgeDetector::setBumpLackTolerance (int val)
{
  bump_lack_tolerance = val;
  if (bump_lack_tolerance < 0) bump_lack_tolerance = 0;
}
