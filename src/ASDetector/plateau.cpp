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

#include "plateau.h"
#include "bsproto.h"
#include <cmath>
#include <algorithm>

const int Plateau::PLATEAU_RES_NONE = 0;
const int Plateau::PLATEAU_RES_OK = 1;
const int Plateau::PLATEAU_RES_NOT_ENOUGH_INPUT_PTS = -1;
const int Plateau::PLATEAU_RES_NO_BOUND_POS = -2;
const int Plateau::PLATEAU_RES_NO_BS = -3;
const int Plateau::PLATEAU_RES_NOT_ENOUGH_ALT_PTS = -4;
const int Plateau::PLATEAU_RES_NOT_ENOUGH_CNX_PTS = -5;
const int Plateau::PLATEAU_RES_TOO_LARGE_WIDENING = -6;
const int Plateau::PLATEAU_RES_OPTIMAL_HEIGHT_UNDER_USED = -7;
const int Plateau::PLATEAU_RES_IMPASSABLE_EVENT = -10;
const int Plateau::PLATEAU_RES_TOO_LARGE_BS_TILT = -11;
const int Plateau::PLATEAU_RES_TOO_LARGE_NARROWING = -12;
const int Plateau::PLATEAU_RES_TOO_NARROW = -13;
const int Plateau::PLATEAU_RES_OUT_OF_HEIGHT_REF = -14;


Plateau::Plateau (PlateauModel *pmod, int ct_shift)
{
  this->pmod = pmod;
  scan_shift = ct_shift;
  status = PLATEAU_RES_NONE;
  accepted = false;
  s_ref = -1.0f;
  e_ref = -1.0f;
  h_ref = -1.0f;
  s_est = 0.0f;
  e_est = 0.0f;
  h_min = 0.0f;
  sdist = pmod->maxLength ();
  edist = pmod->maxLength ();
  s_num = 0;
  e_num = 0;
  s_int = 0.0f;
  e_int = 0.0f;
  s_ext = 0.0f;
  e_ext = 0.0f;
  s_ok = false;
  e_ok = false;
  w_ok = false;
  slope_est = 0.0f;
  dev_est = 0.0f;
  width_change = 0;
  dss = NULL;
  locheight = 0.0f;
}


Plateau::~Plateau ()
{
  if (dss != NULL) delete dss;
}


bool Plateau::detect (const std::vector<Pt2f> &ptsh, bool all, float exh)
{
  // Checks input point vector size
  if ((int) (ptsh.size ()) < pmod->minCountOfPoints ())
  {
    status = PLATEAU_RES_NOT_ENOUGH_INPUT_PTS;
    return false;
  }

  // Detects height interval with the highest number of impacts
  std::vector<Pt2f> ptsv (ptsh);
  sort (ptsv.begin (), ptsv.end (), compHigher);

  std::vector<Pt2f>::iterator it = ptsv.begin ();
  int nbhmax = 1;
  int nbh = 1;
  float meanh = it->y ();
  float exhh = exh + 2 * pmod->thicknessTolerance ();
  std::vector<Pt2f>::iterator itmin = it;
  while (it != ptsv.end ())
  {
    if (all || it->y () < exh || it->y () >= exhh) nbh ++;
    if (it->y () - itmin->y () > pmod->thicknessTolerance ())
    {
      do
      {
        itmin ++;
        if (all || itmin->y () < exh || itmin->y () >= exhh) nbh --;
      }
      while (itmin != it
             && it->y () - itmin->y () > pmod->thicknessTolerance ());
    }
    else
    {
      if (nbh > nbhmax)
      {
        nbhmax = nbh;
        meanh = itmin->y ();
      }
    }
    it ++;
  }
  h_min = meanh;

  // Checks if this height interval has enough points
  if (nbhmax < pmod->minCountOfPoints ()) 
  {
    status = PLATEAU_RES_NOT_ENOUGH_ALT_PTS;
    return false;
  }

  // Detects the longest sequence without interruption in this height interval
  int indi = -1, indj = -1;
  int imin = 0, jmax = 0;
  bool in = false;
  std::vector<Pt2f>::const_iterator pit = ptsh.begin ();
  while (pit != ptsh.end ())
  {
    indj ++;
    if (pit->y () >= meanh && pit->y () < meanh + pmod->thicknessTolerance ())
    {
      if (! in)
      {
        in = true;
        indi = indj;
      }
    }
    else
    {
      if (in)
      {
        in = false;
        if (indj - indi > jmax - imin)
        {
          imin = indi;
          jmax = indj;
        }
      }
    }
    pit ++;
  }
  if (in)
  {
    indj ++;
    if (indj - indi > jmax - imin)
    {
      imin = indi;
      jmax = indj;
    }
  }

  // Updates plateau structure with found values
  s_num = imin;
  e_num = (jmax < 0 ? 0 : jmax - 1);
  s_int = ptsh[s_num].x ();
  e_int = ptsh[e_num].x ();
  h_min = meanh;

  // Checks sequence size and length
  if (ptsh[e_num].x () - ptsh[s_num].x () < pmod->minLength ()) 
  {
    status = PLATEAU_RES_TOO_NARROW;
    return false;
  }
  if (jmax - imin < pmod->minCountOfPoints ())
  {
    status = PLATEAU_RES_NOT_ENOUGH_CNX_PTS;
    return false;
  }
  if (jmax - imin < pmod->optHeightMinUse () * nbhmax)
  {
    status = PLATEAU_RES_OPTIMAL_HEIGHT_UNDER_USED;
    return false;
  }

  // Checks the plateau bounds sharpness
  if (s_num == 0)
  {
    s_ext = s_int - pmod->maxLength ();
    s_ok = false;
  }
  else
  {
    s_ext = ptsh[s_num - 1].x ();
    sdist = s_int - s_ext;
    s_ok = (sdist <= pmod->boundAccuracy ());
  }
  if (e_num == (int) (ptsh.size ()) - 1)
  {
    e_ext = e_int + pmod->maxLength ();
    e_ok = false;
  }
  else
  {
    e_ext = ptsh[e_num + 1].x ();
    edist = e_ext - e_int;
    e_ok = (edist <= pmod->boundAccuracy ());
  }
  if (s_ok)
    if (e_ok)
    {
      s_est = s_int - sdist / 2;
      e_est = e_int + edist / 2;
      status = PLATEAU_RES_OK;
    }
    else
    {
      s_est = s_int - sdist / 2;
      e_est = s_int + pmod->startLength ();
      status = PLATEAU_RES_OK;
    }
  else
    if (e_ok)
    {
      s_est = e_int - pmod->startLength ();
      e_est = e_int + edist / 2;
      status = PLATEAU_RES_OK;
    }
    else status = PLATEAU_RES_NO_BOUND_POS;
  accepted = true;
  return (status == PLATEAU_RES_OK);
}


bool Plateau::track (const std::vector<Pt2f> &ptsh,
                     float lstart, float lend, float lheight,
                     float cshift, int confdist)
{
  // Updates assigned reference pattern
  s_ref = lstart;
  e_ref = lend;
  s_est = s_ref;
  e_est = e_ref;
  if (confdist == 0) // initial detection only
  {
    if (ptsh.empty ())
    {
      s_int = s_ref;
      e_int = e_ref;
      s_ext = s_ref - pmod->maxLength ();
      e_ext = e_ref + pmod->maxLength ();
      h_min = 0.0f;
      h_ref = 0.0f;
    }
    else h_ref = (ptsh.front().y () + ptsh.back().y ()) / 2
                - pmod->thicknessTolerance () / 2;
  }
  else h_ref = lheight;
  sdist = 0.0f;
  edist = 0.0f;

  // Checks input point vector size
  if ((int) (ptsh.size ()) < pmod->minCountOfPoints ())
  {
    status = PLATEAU_RES_NOT_ENOUGH_INPUT_PTS;
    return false;
  }
  int lpt = (int) (ptsh.size ()) - 1;

  // Translates to int and finds start point
  float lcenter = (lstart + lend) / 2 + cshift;
  int icenter = (int) (lcenter * 1000 + (lcenter < 0 ? - 0.5f : 0.5f));
  int ifirst = 0;
  int i = 0;
  bool searching = true;
  std::vector<Pt2i> ptsi;
  std::vector<Pt2f>::const_iterator it = ptsh.begin ();
  locheight = it->y ();
  while (it != ptsh.end ())
  {
    int x = floor (it->x () * 1000);
    if (searching && x > icenter)
    {
      searching = false;
      if (i == 0) ifirst = 0;
      else if (x - icenter > icenter - ptsi.back().x ()) ifirst = i - 1;
      else ifirst = i;
    }
    ptsi.push_back (Pt2i (x, floor ((it->y () - locheight) * 1000)));
    i ++;
    it ++;
  }
  int myend = i;

  // Checks the reference height
  if ((confdist != 0)
      && (ptsh[ifirst].y () < h_ref - confdist * pmod->slopeTolerance ()
          || ptsh[ifirst].y () > h_ref + pmod->thicknessTolerance ()
                                 + confdist * pmod->slopeTolerance ()))
  {
    s_int = ptsh[ifirst].x ();
    e_int = ptsh[ifirst].x ();
    s_ext = (ifirst == 0 ? s_int - pmod->maxLength ()
                         : ptsh[ifirst - 1].x ());
    e_ext = (ifirst == lpt ? e_int + pmod->maxLength ()
                           : ptsh[ifirst + 1].x ());
    s_est = (s_int + s_ext) / 2;
    e_est = (e_int + e_ext) / 2;
    status = PLATEAU_RES_OUT_OF_HEIGHT_REF;
    return false;
  }

  // Creates a putative blurred segment
  int stol = (int) (pmod->thicknessTolerance () * 1000 + 0.5f);
  BSProto *bsp = new BSProto (stol, ptsi[ifirst]);

  // Extends the blurred segment
  e_num = ifirst + 1;
  s_num = ifirst - 1;
  int isLarge = true;
  int pinch_l = (int) (pmod->minLength () * 1000 + 0.5f);
  int lstop = 0, rstop = 0;
  bool scanningRight = (s_num >= 0);
  bool scanningLeft = (e_num < myend);
  int lextent = 0, rextent = 0;
  std::vector<int> bsadds;
  while (scanningRight || scanningLeft)
  {
    while (scanningRight && (rextent <= lextent || ! scanningLeft))
    {
      bool added = bsp->addRightSorted (ptsi[s_num]);
      rextent = ptsi[ifirst].x () - ptsi[s_num].x ();
      if (isLarge && rextent + lextent > pinch_l)
      {
        EDist pinch_th (bsp->digitalThickness());
        int nth = pmod->bsPinchMargin ()
                  + (int) (((float) pinch_th.num ()) / pinch_th.den ());
        if (nth < stol) bsp->setMaxWidth (EDist (nth, 1));
        isLarge = false;
      }
      if (added)
      {
        rstop = 0;
        bsadds.push_back (s_num);
      }
      else if (++rstop > pmod->maxInterruption ()) scanningRight = false;
      if (--s_num < 0) scanningRight = false;
    }
    while (scanningLeft && (lextent <= rextent || ! scanningRight))
    {
      bool added = bsp->addLeftSorted (ptsi[e_num]);
      lextent = ptsi[e_num].x () - ptsi[ifirst].x ();
      if (isLarge && rextent + lextent > pinch_l)
      {
        EDist pinch_th (bsp->digitalThickness());
        int nth = pmod->bsPinchMargin ()
                  + (int) (((float) pinch_th.num ()) / pinch_th.den ());
        if (nth < stol) bsp->setMaxWidth (EDist (nth, 1));
        isLarge = false;
      }
      if (added)
      {
        lstop = 0;
        bsadds.push_back (e_num);
      }
      else if (++lstop > pmod->maxInterruption ()) scanningLeft = false;
      if (++e_num >= myend) scanningLeft = false;
    }
  }
  if (rstop) bsp->removeRight (rstop);
  if (lstop) bsp->removeLeft (lstop);
  s_num += rstop + 1;
  e_num -= lstop + 1;

  // Tests
  s_ext = (s_num == 0 ? ptsh[0].x () - pmod->maxLength ()
                      : ptsh[s_num - 1].x ());
  e_ext = (e_num == lpt ? ptsh[lpt].x () + pmod->maxLength ()
                        : ptsh[e_num + 1].x ());
  if (e_ext - s_ext < pmod->minLength ())
  {
    s_int = ptsh[s_num].x ();
    e_int = ptsh[e_num].x ();
    s_est = (s_int + s_ext) / 2;
    e_est = (e_int + e_ext) / 2;
    status = PLATEAU_RES_TOO_NARROW;
    delete bsp;
    return false;
  }
  if (1 + e_num - s_num < pmod->minCountOfPoints ())
  {
    s_int = ptsh[s_num].x ();
    e_int = ptsh[e_num].x ();
    s_est = (s_int + s_ext) / 2;
    e_est = (e_int + e_ext) / 2;
    status = PLATEAU_RES_NOT_ENOUGH_ALT_PTS;
    delete bsp;
    return false;
  }

  // Checks and possibly shortens blurred segment
  if (bsp->isNotFlat ())
  {
    Pt2i bslastr = bsp->getLastRight ();
    Pt2i bslastl = bsp->getLastLeft ();
    bool antir = bsp->isAntipodal (bslastr);
    bool antil = bsp->isAntipodal (bslastl);
    if (antir || antil)
    {
      BSProto *bsp2 = new BSProto (stol, ptsi[ifirst]);
      bsp2->setMaxWidth (bsp->getMaxWidth ());
      std::vector<int>::iterator it = bsadds.begin ();
      while (it != bsadds.end ())
      {
        if (*it < ifirst)
        {
          if (! (antir && ptsi[*it].equals (bslastr)))
            bsp2->addRightSorted (ptsi[*it]);
        }
        else
        {
          if (! (antil && ptsi[*it].equals (bslastl)))
            bsp2->addLeftSorted (ptsi[*it]);
        }
        it ++;
      }
      delete bsp;
      bsp = bsp2;
    }
  }

  // Analyses the BS
  BlurredSegment *bs = bsp->endOfBirth ();
  if (bs != NULL)
  {
    dss = bs->holdSegment ();
    delete bs;
    Vr2i dssvec = dss->supportVector ();
    int dssvx = (dssvec.x () < 0 ? - dssvec.x () : dssvec.x ());
    int dssvy = (dssvec.y () < 0 ? - dssvec.y () : dssvec.y ());
    if (dssvx * pmod->bsMaxTilt () < dssvy * 100)
    {
      status = PLATEAU_RES_TOO_LARGE_BS_TILT;
      s_int = ptsh[s_num].x ();
      e_int = ptsh[e_num].x ();
      s_ext = (s_num == 0 ? s_int - pmod->maxLength ()
                          : ptsh[s_num - 1].x ());
      e_ext = (e_num == lpt ? e_int + pmod->maxLength ()
                            : ptsh[e_num + 1].x ());
      s_est = (s_int + s_ext) / 2;
      e_est = (e_int + e_ext) / 2;
      delete bsp;
      return false;
    }
    int a, b, c;
    dss->getCentralLine (a, b, c);
    int ihmin = (c - a * icenter) / b - stol / 2;
    h_min = locheight + ihmin * 0.001f;
  }
  else
  {
    status = PLATEAU_RES_NO_BS;
    s_int = ptsh[s_num].x ();
    e_int = ptsh[e_num].x ();
    s_ext = (s_num == 0 ? s_int - pmod->maxLength () : ptsh[s_num - 1].x ());
    e_ext = (e_num == lpt ? e_int + pmod->maxLength () : ptsh[e_num + 1].x ());
    s_est = (s_int + s_ext) / 2;
    e_est = (e_int + e_ext) / 2;
    delete bsp;
    return false;
  }

  status = PLATEAU_RES_OK;
  if (confdist == 0) setFirstBounds (ptsh);
  else
  {
    setBounds (ptsh);
    setPosition (confdist * pmod->widthMoveTolerance ());
  }
  delete bsp;
  return (status == PLATEAU_RES_OK);
}


// AMRELnet version
bool Plateau::track (const std::vector<Pt2f> &ptsh, Plateau *refp,
                     int confdist, float cshift, float l12)
{
  if (confdist == 0)
  {
    // Initializes plateau position
    s_ref = 0.0f;
    e_ref = l12;
    // s_est = ptsh.front().x (); // potentially restrictive
    // e_est = ptsh.back().x ();
    s_est = s_ref;
    e_est = e_ref;
    if (ptsh.empty ())
    {
      s_int = s_ref;
      e_int = e_ref;
      s_ext = s_ref - pmod->maxLength ();
      e_ext = e_ref + pmod->maxLength ();
      h_min = 0.0f;
      h_ref = 0.0f;
      status = PLATEAU_RES_NOT_ENOUGH_INPUT_PTS;
      return false;
    }
    h_ref = (ptsh.front().y () + ptsh.back().y ()) / 2
            - pmod->thicknessTolerance () / 2;
  }
  else
  {
    // Updates assigned reference pattern
    if (refp->possible ())
    {
      s_ref = refp->s_est;
      e_ref = refp->e_est;
    }
    else
    {
      s_ref = refp->s_ref;
      e_ref = refp->e_ref;
    }
    if (pmod->isDeviationPredictionOn () || ! refp->possible ())
    {
      s_ref += refp->dev_est;
      e_ref += refp->dev_est;
    }
    s_est = s_ref;
    e_est = e_ref;
    h_ref = (refp->status == PLATEAU_RES_OK ? refp->h_min : refp->h_ref);
    if (pmod->isSlopePredictionOn () || refp->status != PLATEAU_RES_OK)
      h_ref += refp->slope_est;
    if (ptsh.empty ())
    {
      s_int = s_ref;
      e_int = e_ref;
      s_ext = s_ref - pmod->maxLength ();
      e_ext = e_ref + pmod->maxLength ();
      h_min = h_ref;
      status = PLATEAU_RES_NOT_ENOUGH_INPUT_PTS;
      return false;
    }
  }
  sdist = 0.0f;
  edist = 0.0f;

  // Handles too short sequences
  int lpt = (int) (ptsh.size ()) - 1;
  if ((int) (ptsh.size ()) < pmod->minCountOfPoints ())
  {
    float center = (s_ref + e_ref) / 2;
    int pnum = 0;
    if (confdist == 0) // get horizontal sequence nearer from center
    {
      int ict = 0;
      std::vector<Pt2f>::const_iterator it = ptsh.begin ();
      float dct = center - it->x ();
      bool search = (dct >= 0.0f);
      it ++;
      while (search && it != ptsh.end ()) // get point nearer from center
      {
        float d2 = center - it->x ();
        if (d2 < 0.0f)
        {
          d2 = - d2;
          search = false;
        }
        if (d2 < dct)
        {
          ict = pnum;
          dct = d2;
        }
        pnum ++;
        it ++;
      }
      pnum = ict;
      e_num = ict;
      it = ptsh.begin () + ict;
      h_min = it->y ();
      h_ref = h_min;
      search = true;
      it ++;
      pnum ++;
      while (search && it != ptsh.end ()) // get end points at same altitude
      {
        if (it->y () < h_ref - confdist * pmod->slopeTolerance ()
            || it->y () > h_ref + pmod->thicknessTolerance ()
                          + confdist * pmod->slopeTolerance ())
          search = false;
        else e_num = pnum;
        pnum ++;
        it ++;
      }
      pnum = ict;
      s_num = ict;
      if (s_num != 0)
      {
        it = ptsh.begin () + ict;
        search = true;
        do // get start points at same altitude
        {
          it --;
          pnum --;
          if (it->y () < h_ref - confdist * pmod->slopeTolerance ()
              || it->y () > h_ref + pmod->thicknessTolerance ()
                            + confdist * pmod->slopeTolerance ())
            search = false;
          else s_num = pnum;
        }
        while (search && it != ptsh.begin ());
      }
    }
    else // get ref height sequence almost nearer from center
    {
      bool in = false, out = false;
      std::vector<Pt2f>::const_iterator it = ptsh.begin ();
      while (it != ptsh.end ())
      {
        if (it->y () < h_ref - confdist * pmod->slopeTolerance ()
            || it->y () > h_ref + pmod->thicknessTolerance ()
                          + confdist * pmod->slopeTolerance ())
        {
          if (in && ! out)
          {
            out = true;
            e_num = pnum - 1;
          }
        }
        else
        {
          if (! in || (out && it->x () < center))
          {
            in = true;
            out = false;
            s_num = pnum;
          }
        }
        if (in && ! out)
        {
          out = true;
          e_num = lpt;
        }
        pnum ++;
        it ++;
      }
      s_int = ptsh[s_num].x ();
      e_int = ptsh[e_num].x ();
    }
    s_ext = (s_num == 0 ? s_int - pmod->maxLength () : ptsh[s_num - 1].x ());
    e_ext = (e_num == lpt ? e_int + pmod->maxLength () : ptsh[e_num + 1].x ());
    s_est = (s_int + s_ext) / 2;
    e_est = (e_int + e_ext) / 2;
    if (e_ext - s_ext < pmod->minLength ())
    {
      status = PLATEAU_RES_TOO_NARROW;
      return false;
    }
    status = PLATEAU_RES_NOT_ENOUGH_INPUT_PTS;
    return false;
  }

  // Translates to int and finds start point
  float lcenter = (s_est + e_est) / 2 + cshift;
  int icenter = (int) (lcenter * 1000 + 0.5f);
  int ifirst = 0;
  int i = 0;
  bool searching = true;
  std::vector<Pt2i> ptsi;
  std::vector<Pt2f>::const_iterator it = ptsh.begin ();
  locheight = it->y ();
  while (it != ptsh.end ())
  {
    int x = floor (it->x () * 1000);
    if (searching && x > icenter)
    {
      searching = false;
      if (i == 0) ifirst = 0;
      else if (x - icenter > icenter - ptsi.back().x ()) ifirst = i - 1;
      else ifirst = i;
    }
    ptsi.push_back (Pt2i (x, floor ((it->y () - locheight) * 1000)));
    i ++;
    it ++;
  }
  int myend = i;

  // Checks the reference height
  if ((confdist != 0)
      && (ptsh[ifirst].y () < h_ref - confdist * pmod->slopeTolerance ()
          || ptsh[ifirst].y () > h_ref + pmod->thicknessTolerance ()
                                 + confdist * pmod->slopeTolerance ()))
  {
    s_int = ptsh[ifirst].y ();
    e_int = ptsh[ifirst].y ();
    s_ext = (ifirst == 0 ? s_int - pmod->maxLength ()
                         : ptsh[ifirst - 1].x ());
    e_ext = (ifirst == lpt ? e_int + pmod->maxLength ()
                           : ptsh[ifirst + 1].x ());
    s_est = (s_int + s_ext) / 2;
    e_est = (e_int + e_ext) / 2;
    status = PLATEAU_RES_OUT_OF_HEIGHT_REF;
    return false;
  }

  // Creates a putative blurred segment
  int stol = (int) (pmod->thicknessTolerance () * 1000 + 0.5f);
  BSProto *bsp = new BSProto (stol, ptsi[ifirst]);

  // Extends the blurred segment
  e_num = ifirst + 1;
  s_num = ifirst - 1;
  int isLarge = true;
  int pinch_l = (int) (pmod->minLength () * 1000 + 0.5f);
  int lstop = 0, rstop = 0;
  bool scanningRight = (s_num >= 0);
  bool scanningLeft = (e_num < myend);
  int lextent = 0, rextent = 0;
  std::vector<int> bsadds;
  while (scanningRight || scanningLeft)
  {
    while (scanningRight && (rextent <= lextent || ! scanningLeft))
    {
      bool added = bsp->addRightSorted (ptsi[s_num]);
      rextent = ptsi[ifirst].x () - ptsi[s_num].x ();
      if (isLarge && rextent + lextent > pinch_l)
      {
        EDist pinch_th (bsp->digitalThickness());
        int nth = (int) (((float) pinch_th.num ()) / pinch_th.den ());
        bsp->setMaxWidth (EDist (nth + pmod->bsPinchMargin (), 1));
        isLarge = false;
      }
      if (added)
      {
        rstop = 0;
        bsadds.push_back (s_num);
      }
      else if (++rstop > pmod->maxInterruption ()) scanningRight = false;
      if (--s_num < 0) scanningRight = false;
    }
    while (scanningLeft && (lextent <= rextent || ! scanningRight))
    {
      bool added = bsp->addLeftSorted (ptsi[e_num]);
      lextent = ptsi[e_num].x () - ptsi[ifirst].x ();
      if (isLarge && rextent + lextent > pinch_l)
      {
        EDist pinch_th (bsp->digitalThickness());
        int nth = (int) (((float) pinch_th.num ()) / pinch_th.den ());
        bsp->setMaxWidth (EDist (nth + pmod->bsPinchMargin (), 1));
        isLarge = false;
      }
      if (added)
      {
        lstop = 0;
        bsadds.push_back (e_num);
      }
      else if (++lstop > pmod->maxInterruption ()) scanningLeft = false;
      if (++e_num >= myend) scanningLeft = false;
    }
  }
  if (rstop) bsp->removeRight (rstop);
  if (lstop) bsp->removeLeft (lstop);
  s_num += rstop + 1;
  e_num -= lstop + 1;

  // Tests
  s_ext = (s_num == 0 ? ptsh[0].x () - pmod->maxLength ()
                      : ptsh[s_num - 1].x ());
  e_ext = (e_num == lpt ? ptsh[lpt].x () + pmod->maxLength ()
                        : ptsh[e_num + 1].x ());
  if (e_ext - s_ext < pmod->minLength ())
  {
    s_int = ptsh[s_num].x ();
    e_int = ptsh[e_num].x ();
    s_est = (s_int + s_ext) / 2;
    e_est = (e_int + e_ext) / 2;
    status = PLATEAU_RES_TOO_NARROW;
    delete bsp;
    return false;
  }
  if (1 + e_num - s_num < pmod->minCountOfPoints ())
  {
    s_int = ptsh[s_num].x ();
    e_int = ptsh[e_num].x ();
    s_est = (s_int + s_ext) / 2;
    e_est = (e_int + e_ext) / 2;
    status = PLATEAU_RES_NOT_ENOUGH_ALT_PTS;
    delete bsp;
    return false;
  }

  // Checks and possibly shortens blurred segment
  if (bsp->isNotFlat ())
  {
    Pt2i bslastr = bsp->getLastRight ();
    Pt2i bslastl = bsp->getLastLeft ();
    bool antir = bsp->isAntipodal (bslastr);
    bool antil = bsp->isAntipodal (bslastl);
    if (antir || antil)
    {
      BSProto *bsp2 = new BSProto (stol, ptsi[ifirst]);
      bsp2->setMaxWidth (bsp->getMaxWidth ());
      std::vector<int>::iterator it = bsadds.begin ();
      while (it != bsadds.end ())
      {
        if (*it < ifirst)
        {
          if (! (antir && ptsi[*it].equals (bslastr)))
            bsp2->addRightSorted (ptsi[*it]);
        }
        else
        {
          if (! (antil && ptsi[*it].equals (bslastl)))
            bsp2->addLeftSorted (ptsi[*it]);
        }
        it ++;
      }
      delete bsp;
      bsp = bsp2;
    }
  }

  // Analyses the BS
  BlurredSegment *bs = bsp->endOfBirth ();
  if (bs != NULL)
  {
    dss = bs->holdSegment ();
    delete bs;
    Vr2i dssvec = dss->supportVector ();
    int dssvx = (dssvec.x () < 0 ? - dssvec.x () : dssvec.x ());
    int dssvy = (dssvec.y () < 0 ? - dssvec.y () : dssvec.y ());
    if (dssvx * pmod->bsMaxTilt () < dssvy * 100)
    {
      status = PLATEAU_RES_TOO_LARGE_BS_TILT;
      s_int = ptsh[s_num].y ();
      e_int = ptsh[e_num].y ();
      s_ext = (s_num == 0 ? s_int - pmod->maxLength ()
                          : ptsh[s_num - 1].x ());
      e_ext = (e_num == lpt ? e_int + pmod->maxLength ()
                            : ptsh[e_num + 1].x ());
      s_est = (s_int + s_ext) / 2;
      e_est = (e_int + e_ext) / 2;
      delete bsp;
      return false;
    }
    int a, b, c;
    dss->getCentralLine (a, b, c);
    int ihmin = (c - a * icenter) / b - stol / 2;
    h_min = locheight + ihmin * 0.001f;
  }
  else
  {
    status = PLATEAU_RES_NO_BS;
    s_int = ptsh[s_num].y ();
    e_int = ptsh[e_num].y ();
    s_ext = (s_num == 0 ? s_int - pmod->maxLength () : ptsh[s_num - 1].x ());
    e_ext = (e_num == lpt ? e_int + pmod->maxLength () : ptsh[e_num + 1].x ());
    s_est = (s_int + s_ext) / 2;
    e_est = (e_int + e_ext) / 2;
    delete bsp;
    return false;
  }

  status = PLATEAU_RES_OK;
  if (confdist == 0) setFirstBounds (ptsh);
  else
  {
    setBounds (ptsh);
    setPosition (confdist * pmod->widthMoveTolerance ());
  }
  delete bsp;
  return (status == PLATEAU_RES_OK);
}


void Plateau::setFirstBounds (const std::vector<Pt2f> &ptsh)
{
  s_int = ptsh[s_num].x ();
  e_int = ptsh[e_num].x ();
  float w = e_int - s_int;
  w_ok = (w < pmod->maxLength () && w > pmod->minLength ());

  // Compares start bounds with reference pattern
  if (s_num == 0)
  {
    s_ext = s_int - pmod->maxLength ();
    s_ok = false;
    w_ok = false;
  }
  else
  {
    s_ext = ptsh[s_num - 1].x ();
    s_ok = true;
  }

  // Compares end bounds with reference pattern
  if (e_num == (int) (ptsh.size ()) - 1)
  {
    e_ext = e_int + pmod->maxLength ();
    e_ok = false;
    w_ok = false;
  }
  else
  {
    e_ext = ptsh[e_num + 1].x ();
    e_ok = true;
  }
  s_est = s_int;
  e_est = e_int;
}


void Plateau::setBounds (const std::vector<Pt2f> &ptsh)
{
  s_int = ptsh[s_num].x ();
  e_int = ptsh[e_num].x ();

  // Compares start bounds with reference pattern
  if (s_num == 0)
  {
    s_ext = s_int - pmod->maxLength ();
    s_ok = (s_ref <= s_int + pmod->sideShiftTolerance ());
  }
  else
  {
    s_ext = ptsh[s_num - 1].x ();
    s_ok = (s_ref >= s_ext - pmod->sideShiftTolerance ()
            && s_ref <= s_int + pmod->sideShiftTolerance ());
  }

  // Compares end bounds with reference pattern
  if (e_num == (int) (ptsh.size ()) - 1)
  {
    e_ext = e_int + pmod->maxLength ();
    e_ok = (e_ref >= e_int - pmod->sideShiftTolerance ());
  }
  else
  {
    e_ext = ptsh[e_num + 1].x ();
    e_ok = (e_ref >= e_int - pmod->sideShiftTolerance ()
            && e_ref <= e_ext + pmod->sideShiftTolerance ());
  }

  // Checks width tolerance
  w_ok = (e_ref - s_ref <= e_ext - s_ext + pmod->widthMoveTolerance ()
          && e_ref - s_ref >= e_int - s_int - pmod->widthMoveTolerance ());
}


void Plateau::setPosition (float wmt)
{
  // Rule 1 : cannot be outside of the detected plateau
  // Rule 2 : minimal width modification
  // Rule 3 : minimal side shift

  if (status == PLATEAU_RES_OK)
  {
    sdist = 0.0f;
    if (s_ref < s_ext) sdist = s_ref - s_ext;
    else if (s_ref > s_int) sdist = s_ref - s_int;
    edist = 0.0f;
    if (e_ref < e_int) edist = e_ref - e_int;
    else if (e_ref > e_ext) edist = e_ref - e_ext;

    if (sdist < 0.0f)
    {
      if (edist > 0.0f)
      {
        // Case 1 (both refs on exterior): minimal narrowing
        s_est = s_ext;
        e_est = e_ext;
        if (e_ext - s_ext < pmod->minLength ())
          status = PLATEAU_RES_TOO_LARGE_NARROWING;
      }

      else // (edist <= 0)
      {
        // Case 2 : Only start ref on exterior: set s_est on s_ext
        //   moves e_est only if e_ref is on interior or to avoid minLength
        s_est = s_ext;
        e_est = (e_ref < e_int ? e_int : e_ref);
        if (e_est - s_est < pmod->minLength ())
        {
          if (e_ext - s_est < pmod->minLength ())
            status = PLATEAU_RES_TOO_LARGE_NARROWING;
          else e_est = s_est + pmod->minLength ();
        }
      }
    }

    else if (edist > 0.0f) // (sdist >= 0)
    {
      // Case 2 : Only end ref on exterior : set e_est on e_ext
      //   moves s_est only if s_ref is on interior or to avoid minLength
      e_est = e_ext;
      s_est = (s_ref > s_int ? s_int : s_ref);
      if (e_est - s_est < pmod->minLength ())
      {
        if (e_est - s_ext < pmod->minLength ())
          status = PLATEAU_RES_TOO_LARGE_NARROWING;
        else s_est = e_est - pmod->minLength ();
      }
    }

    else if (sdist > 0.0f)
    {
      if (edist < 0.0f)
      {
        // Case 3 : Both refs on interior
        s_est = s_int;
        e_est = e_int;
        if (e_est - s_est > pmod->maxLength ())
          status = PLATEAU_RES_TOO_LARGE_WIDENING;
      }

      else
      {
        // Case 4 : Only start ref on interior : sets s_est on s_int
        //   moves e_est only to avoid minLength
        s_est = s_int;
        e_est = e_ref;
        if (e_est - s_est < pmod->minLength ())
        {
          if (e_int - s_est < pmod->minLength ())
            status = PLATEAU_RES_TOO_LARGE_NARROWING;
          else e_est = s_est + pmod->minLength ();
        }
      }
    }

    else if (edist < 0.0f) // sdist = 0
    {
      // Case 4 : Only end ref on interior : sets e_est on e_int
      //   moves s_est only to avoid minLength
      e_est = e_int;
      s_est = s_ref;
      if (e_est - s_est < pmod->minLength ())
      {
        if (e_est - s_int < pmod->minLength ())
          status = PLATEAU_RES_TOO_LARGE_NARROWING;
        else s_est = e_est - pmod->minLength ();
      }
    }

    else
    {
      // Case 0 : Both in interval
      s_est = s_ref;
      e_est = e_ref;
    }
  }

  // Checks too large narrowing or widening
  if (e_est - s_est < e_ref - s_ref - 2 * wmt)
  {
    // Case of a narrowing but still greater than pmod->minLength ()
    // Should be warned
    width_change = -1;
  }
  else if (e_est - s_est > 2 * wmt + e_ref - s_ref)
  {
    width_change = 1;
  }
}


bool Plateau::isConnectedTo (Plateau *next)
{
  float ct = (s_int + e_int) / 2;
  if (ct < next->s_est || ct > next->e_est) return false;
  ct = (next->s_int + next->e_int) / 2;
  return (ct >= s_est && ct <= e_est);
}


bool Plateau::fit (Plateau *next, Plateau *orig, int cdist)
{
  s_est = next->s_int + (orig->s_int - next->s_int) / cdist;
  e_est = next->e_int + (orig->e_int - next->e_int) / cdist;
  if (s_est < s_ext)
  {
    s_est = s_ext;
    if (e_est - s_est < pmod->minLength ()) e_est = s_est + pmod->minLength ();
  }
  if (e_est > e_ext)
  {
    e_est = e_ext;
    if (e_est - s_est < pmod->minLength ()) s_est = e_est - pmod->minLength ();
  }
  // Enforces connexity constraint
  float cte = (next->s_int + next->e_int) / 2;
  if (cte < s_est || cte > e_est) return false;
  if (cdist == 2)
  {
    cte = (orig->s_int + orig->e_int) / 2;
    if (cte < s_est || cte > e_est) return false;
  }
/*
  if (next->e_int < s_est || next->s_int > e_est) return false;
  if (cdist == 2
      && (orig->e_int < s_est || orig->s_int > e_est)) return false;
*/
  s_int = s_est;
  e_int = e_est;
  return true;
}


bool Plateau::sticksTo (float sts, float ste, float cte1, float cte2)
{
  s_est = sts;
  e_est = ste;
  if (s_est < s_ext)
  {
    s_est = s_ext;
    if (e_est - s_est < pmod->minLength ()) e_est = s_est + pmod->minLength ();
  }
  if (e_est > e_ext)
  {
    e_est = e_ext;
    if (e_est - s_est < pmod->minLength ()) s_est = e_est - pmod->minLength ();
  }
  if (cte1 < s_est || cte1 > e_est) return false;
  if (cte2 < s_est || cte2 > e_est) return false;
  s_int = s_est;
  e_int = e_est;
  return true;
}


float Plateau::getDSSwidth () const
{
  return (dss == NULL ? 0.0f : dss->width () * 0.001f / dss->period ());
}


Pt2f Plateau::getDSSstart () const
{
 if (dss == NULL) return (Pt2f (0.0f, 0.0f));
  AbsRat x1, y1, x2, y2;
  dss->naiveLine (x1, y1, x2, y2);
  return Pt2f (x1.num () * 0.001f / x1.den (),
               locheight + y1.num () * 0.001f / y1.den ());
}


Pt2f Plateau::getDSSend () const
{
  if (dss == NULL) return (Pt2f (0.0f, 0.0f));
  AbsRat x1, y1, x2, y2;
  dss->naiveLine (x1, y1, x2, y2);
  return Pt2f (x2.num () * 0.001f / x2.den (),
               locheight + y2.num () * 0.001f / y2.den ());
}


bool Plateau::compHigher (Pt2f p1, Pt2f p2)
{
  return (p2.y () > p1.y ());
}


bool Plateau::recompIFurther (Pt2i p1, Pt2i p2)
{
  return (p2.x () > p1.x () || (p2.x () == p1.x () && p2.y () > p1.y ()));
}


bool Plateau::lessTiltedThan (Plateau *pl) const
{
  if (dss == NULL) return false;
  Vr2i dir = dss->supportVector ();
  int dirx = (dir.x () < 0 ? - dir.x () : dir.x ());
  int diry = (dir.y () < 0 ? - dir.y () : dir.y ());
  if (dirx * pmod->bsMaxTilt () < diry * 100) return false;

  if (pl->dss == NULL) return true;
  Vr2i dir2 = pl->dss->supportVector ();
  int dir2x = (dir2.x () < 0 ? - dir2.x () : dir2.x ());
  int dir2y = (dir2.y () < 0 ? - dir2.y () : dir2.y ());
  return (dirx * dir2y > dir2x * diry);
}


bool Plateau::thinerThan (Plateau *pl) const
{
  if (dss == NULL) return false;
  if (pl->dss == NULL) return true;
  return (dss->width () * pl->dss->period ()
          < pl->dss->width () * dss->period ());
}


/*
void Plateau::trace () const
{
  std::cout << "H_REF = " << h_ref << " et H_MIN = " << h_min << std::endl;
  std::cout << "S_REF = " << s_ref << " et E_REF = " << e_ref << std::endl;
  std::cout << "S_INT = " << s_int << " et E_INT = " << e_int << std::endl;
  std::cout << "S_EXT = " << s_ext << " et E_EXT = " << e_ext << std::endl;
  std::cout << "S_NUM = " << s_num << " et E_NUM = " << e_num << std::endl;
  std::cout << "S_EST = " << s_est << " et E_EST = " << e_est << std::endl;
  std::cout << "S_PR = " << (s_int - s_ext)
            << " et E_PR = " << (e_ext - e_int) << std::endl;
  std::cout << "S_OK = " << s_ok << ", E_OK = " << e_ok
            << " et W_OK = " << w_ok << std::endl;
  std::cout << "SDIST = " << sdist << " et EDIST = " << edist << std::endl;
  std::cout << "WIDTH_CHANGE = " << width_change << std::endl;
}

void Plateau::traceExt (const std::vector<Pt2f> &ptsh) const
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
