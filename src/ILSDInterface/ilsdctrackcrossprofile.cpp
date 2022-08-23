/*  Copyright 2021 Philippe Even, Phuc Ngo and Pierre Even,
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

#include <cstdlib>
#include <iostream>
#include "ilsdctrackcrossprofile.h"
#include "directionalscanner.h"
#include "asImage.h"
#include "imgui.h"
#include "math.h"
#include "asPainter.h"


ILSDCTrackCrossProfile::ILSDCTrackCrossProfile (CTrackDetector* detector,
                                                ILSDItemControl *item_ctrl)
                       : ILSDCrossProfileItem (item_ctrl)
{
  det = detector;
}


std::string ILSDCTrackCrossProfile::profileName () const
{
  return ("plateau");
}


void ILSDCTrackCrossProfile::paint (GLWindow* parentWindow)
{
  if (!structImage || size () != structImage->getImageResolution ())
  {
    if (structImage) delete structImage;
    structImage = new ASImage (size ());
    update ();
  }
  structImage->Draw (parentWindow);

  if (drawable)
  {
    if (det->getCarriageTrack (ctrl->isInitialDetection ()) != NULL)
      paintInfo ();
    else paintStatus ();
  }
}


std::vector<Pt2i> *ILSDCTrackCrossProfile::getDisplayScan (int num)
{
  CarriageTrack *ct = det->getCarriageTrack (ctrl->isInitialDetection ());
  if (ct != NULL) return (ct->getDisplayScan (num));
  else return NULL;
}


std::vector<Pt2f> *ILSDCTrackCrossProfile::getProfile (int num)
{
  CarriageTrack *ct = det->getCarriageTrack (ctrl->isInitialDetection ());
  if (ct != NULL) return (ct->getProfile (num));
  else return NULL;
}


void ILSDCTrackCrossProfile::updateDrawing ()
{
  // Sets display parameters
  det->getInputStroke (p1, p2, ctrl->isInitialDetection ());
  float scanx = (float) (p2.x () - p1.x ());
  float scany = (float) (p2.y () - p1.y ());
  p12.set (scanx, scany);
  l12 = (float) sqrt (scanx * scanx + scany * scany);
  if (scanx < 0) scanx = -scanx;
  if (scany < 0) scany = -scany;
  d12 = (scany > scanx ? scany : scanx) / l12;
  sratio = (alti_area_width - 2 * alti_area_margin) * iratio / l12;

  // Sets scan range
  CarriageTrack *ct = det->getCarriageTrack (ctrl->isInitialDetection ());
  if (ct != NULL)
  {
    ctrl->setMinScan (- ct->getRightScanCount ());
    ctrl->setMaxScan (ct->getLeftScanCount ());
  }
  else ctrl->resetScan ();

  if (ct != NULL)
  {
    // Sets scan direction
    reversed = ct->isScanReversed (0);

    // Sets height and side references
    Plateau *pl = ct->lastValidPlateau (ctrl->scan ());
    if (ctrl->scan () == 0
        && (pl == NULL || pl->getStatus () != Plateau::PLATEAU_RES_OK))
    {
      href = getProfile(0)->front().y ();
      profshift = 0.0f;
    }
    else
    {
      if (ctrl->isStaticHeight () || pl == NULL)
        href = ct->getHeightReference (0);
      else href = pl->getMinHeight ();
      if (pl == NULL) profshift = 0.0f;
      else profshift = l12 / (2 * iratio) - pl->estimatedCenter ();
    }
  }
}


void ILSDCTrackCrossProfile::update ()
{
  structImage->clear (ASColor::WHITE);
  if (drawable)
  {
    updateDrawing ();

    if (det->getCarriageTrack (ctrl->isInitialDetection ()) != NULL)
    {
      if (ctrl->isAligned ()) paintAlignedScans ();
      else paintScans ();
      paintProfile ();
    }
    else
    {
      if (ctrl->isAligned ()) ILSDCrossProfileItem::paintAlignedScans ();
      else ILSDCrossProfileItem::paintScans ();
      ILSDCrossProfileItem::paintProfile ();
    }
  }
}


void ILSDCTrackCrossProfile::paintScans ()
{
  ASPainter painter (structImage);

  // Draws detected area
  CarriageTrack *ct = det->getCarriageTrack (ctrl->isInitialDetection ());
  Plateau* pl = ct->lastValidPlateau (ctrl->scan ());
  if (pl != NULL)
  {
    int lx = alti_area_width + scan_area_width / 2;
    int lbb = alti_area_width + scan_area_margin - 1;
    int rbb = w_width + 1 - scan_area_margin;
    float start = pl->internalStart ();
    float end = pl->internalEnd ();
    float lstart = pl->externalStart ();
    float lend = pl->externalEnd ();

    std::vector<Pt2i>* discan = ct->getDisplayScan (ctrl->scan ());
    Pt2i scanstart = (reversed ? discan->back () : discan->front ());
    Pt2i scanend = (reversed ? discan->front () : discan->back ());
    Pt2f scancenter ((scanstart.x () + scanend.x ()) / 2.0f,
                     (scanstart.y () + scanend.y ()) / 2.0f);
    Pt2f p1f ((float) (p1.x ()), (float) p1.y ());
    Vr2f scanpos = p1f.vectorTo (scancenter);
    float delta = scanpos.scalarProduct (p12) / l12;

    float mes = (delta - start * iratio) * d12 * scan_res;
    int cx = lx - (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
    if (cx < lbb) cx = lbb;
    else
    {
      if (cx > rbb) cx = rbb;
      mes = (delta - lstart * iratio) * d12 * scan_res;
      int fx = lx - (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
      if (fx < rbb)
      {
        if (fx < lbb) fx = lbb;
        painter.fillRect (fx, w_height / 2 - scan_res - 1,
                          cx + 1 - fx, 2 * scan_res + 2, ASColor::GREEN);
      }
    }

    mes = (delta - end * iratio) * d12 * scan_res;
    int ex = lx - (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
    if (ex > rbb) ex = rbb;
    else
    {
      if (ex < lbb) ex = lbb;
      mes = (delta - lend * iratio) * d12 * scan_res;
      int fx = lx - (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
      if (fx > lbb)
      {
        if (fx > rbb) fx = rbb;
        painter.fillRect (ex, w_height / 2 - scan_res - 1,
                          fx + 1 - ex, 2 * scan_res + 2, ASColor::GREEN);
      }
    }

    if (cx < rbb && ex > lbb)
    {
      painter.setPen (ASPen (ASColor::GREEN, 2));
      painter.drawRect (cx, w_height / 2 - scan_res,
                        ex + 1 - cx, 2 * scan_res);
    }
  }

  // Draws frame and scans
  ILSDCrossProfileItem::paintScans ();
}


void ILSDCTrackCrossProfile::paintAlignedScans ()
{
  ASPainter painter (structImage);

  // Draws detected area
  CarriageTrack *ct = det->getCarriageTrack (ctrl->isInitialDetection ());
  Plateau* pl = ct->plateau (ctrl->scan ());
  if (pl != NULL)
  {
    std::vector<Pt2i>* discan = ct->getDisplayScan (0);
    int ssize = (int) discan->size ();
    int lx = alti_area_width + (scan_area_width / 2 - (ssize / 2) * scan_res)
                       + (ssize % 2 == 1 ? 0 : scan_res / 2);
    int lbb = alti_area_width + scan_area_margin - 1;
    int rbb = w_width + 1 - scan_area_margin;

    discan = ct->getDisplayScan (ctrl->scan ());
    Pt2i scanstart = (reversed ? discan->back () : discan->front ());
    Pt2i scanend = (reversed ? discan->front () : discan->back ());
    Vr2f scanpos ((float) (p1.x () - scanstart.x ()),
                  (float) (p1.y () - scanstart.y ()));
    float delta = scanpos.scalarProduct (p12) / l12;

    float start = pl->internalStart ();
    float mes = (delta + start * iratio) * d12 * scan_res;
    int cx = lx + (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
    if (cx < lbb) cx = lbb;
    else
    {
      if (cx > rbb) cx = rbb;
      start = pl->externalStart ();
      mes = (delta + start * iratio) * d12 * scan_res;
      int fx = lx + (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
      if (fx < rbb)
      {
        if (fx < lbb) fx = lbb;
        painter.fillRect (fx, w_height / 2 - scan_res - 1,
                          cx + 1 - fx, 2 * scan_res + 2, ASColor::GREEN);
      }
    }

    float end = pl->internalEnd ();
    mes = (delta + end * iratio) * d12 * scan_res;
    int ex = lx + (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
    if (ex > rbb) ex = rbb;
    else
    {
      if (ex < lbb) ex = lbb;
      end = pl->externalEnd ();
      mes = (delta + end * iratio) * d12 * scan_res;
      int fx = lx + (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
      if (fx > lbb)
      {
        if (fx > rbb) fx = rbb;
        painter.fillRect (ex, w_height / 2 - scan_res - 1,
                          fx + 1 - ex, 2 * scan_res + 2, ASColor::GREEN);
      }
    }

    if (cx < rbb && ex > lbb)
    {
      painter.setPen (ASPen (ASColor::GREEN, 2));
      painter.drawRect (cx, w_height / 2 - scan_res - 1,
                        ex + 1 - cx, 2 * scan_res + 2);
    }
  }

  // Draws frame and scans
  ILSDCrossProfileItem::paintAlignedScans ();
}


void ILSDCTrackCrossProfile::paintProfile ()
{
  ASPainter painter (structImage);
  int altiShift = alti_area_margin - ctrl->profileShift ();
  int refh = w_height / 2;
  int bbl = alti_area_margin - altiShift;
  int bbr = alti_area_width - altiShift - alti_area_margin;
  int bbd = refh + alti_area_margin - w_height;
  int bbu = refh - alti_area_margin;
  int abbl = altiShift + bbl;
  int abbr = altiShift + bbr;
  int abbd = refh - bbu;
  int abbu = refh - bbd;

  CarriageTrack *ct = det->getCarriageTrack (ctrl->isInitialDetection ());
  Plateau* pl = ct->plateau (ctrl->scan ());
  if (pl != NULL)
  {
    // Draws reference plateau
    int istart = 0, iend = 0, ilow = 0, ihigh = 0;
    float start = pl->referenceStart ();
    float end = pl->referenceEnd ();
    float height = pl->referenceHeight ();
    height -= href;
    ilow = (int) (height * sratio + (height < 0.0f ? -0.5f : 0.5f));
    height += det->model()->thicknessTolerance ();
    ihigh = (int) (height * sratio + (height < 0.0f ? -0.5f : 0.5f));
    istart = (int) ((start + profshift) * sratio + 0.5f);
    iend = (int) ((end + profshift) * sratio + 0.5f);
    if (ctrl->isRefDisplay () && ctrl->scan () != 0)
    {
      if (istart <= bbr && iend >= bbl && ihigh >= bbd && ilow <= bbu)
      {
        if (istart < bbl) istart = bbl;
        if (iend > bbr) iend = bbr;
        if (ihigh > bbu) ihigh = bbu;
        if (ilow < bbd) ilow = bbd;
        painter.fillRect (altiShift + istart, refh - ihigh,
                          iend - istart, ihigh - ilow, ASColor::RED);
      }
    }

    // Draws detected plateau
    if (ctrl->scan () != 0)
    {
      Pt2f dsss = pl->getDSSstart ();
      Pt2f dsse = pl->getDSSend ();
      float dssw = pl->getDSSwidth () / 2;
      int dss1 = (int) ((dsss.x() + profshift) * sratio + 0.5);
      int dss2 = (int) ((dsse.x() + profshift) * sratio + 0.5);
      int dssh1 = (int) ((dsss.y() + dssw - href) * sratio + 0.5);
      int dssh2 = (int) ((dsss.y() - dssw - href) * sratio + 0.5);
      int dssh3 = (int) ((dsse.y() - dssw - href) * sratio + 0.5);
      int dssh4 = (int) ((dsse.y() + dssw - href) * sratio + 0.5);
      painter.setPen (ASPen (ASColor::GRAY, 2));
      clipLine (painter, altiShift + dss1, refh - dssh1,
                altiShift + dss1, refh - dssh2, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + dss1, refh - dssh2,
                altiShift + dss2, refh - dssh3, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + dss2, refh - dssh3,
                altiShift + dss2, refh - dssh4, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + dss2, refh - dssh4,
                altiShift + dss1, refh - dssh1, abbl, abbd, abbr, abbu);
    }
    else
    {
      start = pl->internalStart ();
      end = pl->internalEnd ();
      height = pl->getMinHeight () - href;
      ilow = (int) (height * sratio + (height < 0.0f ? -0.5f : 0.5f));
      height += det->model()->thicknessTolerance ();
      ihigh = (int) (height * sratio + (height < 0.0f ? -0.5f : 0.5f));
      istart = (int) ((start + profshift) * sratio + 0.5);
      iend = (int) ((end + profshift) * sratio + 0.5);
      painter.setPen (ASPen (ASColor::GRAY, 2));
      if (istart <= bbr && iend >= bbl && ihigh >= bbd && ilow <= bbu)
      {
        if (istart < bbl) istart = bbl;
        if (iend > bbr) iend = bbr;
        if (ihigh > bbu) ihigh = bbu;
        if (ilow < bbd) ilow = bbd;
        painter.drawRect (altiShift + istart, refh - ihigh,
                          iend - istart, ihigh - ilow);
      }
    }

    // Draws external bounds
    start = pl->externalStart ();
    end = pl->externalEnd ();
    istart = (int) ((start + profshift) * sratio + 0.5);
    iend = (int) ((end + profshift) * sratio + 0.5);
    height += det->model()->thicknessTolerance ();
    ihigh = (int) (height * sratio + (height < 0.0f ? -0.5f : 0.5f));
    height -= 3 * det->model()->thicknessTolerance ();
    ilow = (int) (height * sratio + (height < 0.0f ? -0.5f : 0.5f));
    if (istart <= bbr && istart >= bbl && ihigh >= bbd && ilow <= bbu)
    {
      if (ihigh > bbu) ihigh = bbu;
      if (ilow < bbd) ilow = bbd;
      painter.drawLine (altiShift + istart, refh - ihigh,
                        altiShift + istart, refh - ilow);
    }
    if (iend <= bbr && iend >= bbl && ihigh >= bbd && ilow <= bbu)
    {
      if (ihigh > bbu) ihigh = bbu;
      if (ilow < bbd) ilow = bbd;
      painter.drawLine (altiShift + iend, refh - ihigh,
                        altiShift + iend, refh - ilow);
    }

    // Draws estimated plateau
    if (ctrl->isEstimDisplay ())
    {
      start = pl->estimatedStart ();
      end = pl->estimatedEnd ();
      istart = (int) ((start + profshift) * sratio + 0.5f);
      iend = (int) ((end + profshift) * sratio + 0.5f);
      int posy = refh - (ihigh + ilow) / 2
                 + (ctrl->scan () > 0 ? -POS_EST : POS_EST);
      painter.setPen (ASPen (ASColor::GREEN, 2));
      if (istart <= bbr && iend >= bbl
          && posy >= bbd + refh && posy <= bbu + refh)
      {
        if (istart < bbl) istart = bbl;
        if (iend > bbr) iend = bbr;
        painter.drawLine (altiShift + istart, posy, altiShift + iend, posy);
      }
      if (ctrl->scan () == 0)
      {
        posy -= 2 * POS_EST;
        if (istart <= bbr && iend >= bbl
            && posy >= bbd + refh && posy <= bbu + refh)
        {
          if (istart < bbl) istart = bbl;
          if (iend > bbr) iend = bbr;
          painter.drawLine (altiShift + istart, posy,
                            altiShift + iend, posy);
        }
      }
    }

    // Draws next predicted plateau
    if (ctrl->isPredDisplay ())
    {
      Plateau* pln = ct->plateau (ctrl->scan () + (ctrl->scan () < 0 ? -1 : 1));
      if (pln != NULL)
      {
        start = pln->referenceStart ();
        end = pln->referenceEnd ();
        istart = (int) ((start + profshift) * sratio + 0.5f);
        iend = (int) ((end + profshift) * sratio + 0.5f);
        int posy = refh - (ihigh + ilow) / 2
                   + (ctrl->scan () > 0 ? - POS_PRED : POS_PRED);
        painter.setPen (ASPen (ASColor::RED, 2));
        if (istart <= bbr && iend >= bbl
            && posy >= bbd + refh && posy <= bbu + refh)
        {
          if (istart < bbl) istart = bbl;
          if (iend > bbr) iend = bbr;
          painter.drawLine (altiShift + istart, posy, altiShift + iend, posy);
        }
        if (ctrl->scan () == 0)
        {
          posy -= 2 * POS_PRED;
          if (istart <= bbr && iend >= bbl
              && posy >= bbd + refh && posy <= bbu + refh)
          {
            if (istart < bbl) istart = bbl;
            if (iend > bbr) iend = bbr;
            painter.drawLine (altiShift + istart, posy,
                              altiShift + iend, posy);
          }
        }
      }
    }

    // Draws direction
    if (ctrl->isDirDisplay () && pl->reliable ())
    {
      float dev = pl->estimatedDeviation ();
      float cs = det->getCellSize ();
      int idev = (int) (LG_DIR * dev / cs + (dev < 0 ? -0.5f : 0.5f));
      if ((dev < 0.0f ? -dev : dev) > cs)
        painter.setPen (ASPen (ASColor::RED, 2));
      clipLine (painter, altiShift + alti_area_width / 2,
                refh - (ihigh + ilow) / 2,
                altiShift + alti_area_width / 2 + idev,
                refh + (ctrl->scan () > 0 ?
                  - LG_DIR : LG_DIR) - (ihigh + ilow) / 2,
                abbl, abbd, abbr, abbu);
    }
  }

  // Draws profile points
  ILSDCrossProfileItem::paintProfile ();
}


void ILSDCTrackCrossProfile::paintInfo ()
{
  CarriageTrack *ct = det->getCarriageTrack (ctrl->isInitialDetection ());
  Plateau* pl = ct->plateau (ctrl->scan ());
  if (pl != NULL)
  {
    // Draws metric reference values
    ILSDCrossProfileItem::paintInfo ();
    ASPainter painter (structImage);

    // Draws detection result values
    painter.setPen (ASPen (pl->isAccepted() ? ASColor::BLACK : ASColor::RED,
                           1));
    std::string valid = (ctrl->isInitialDetection () ?
                         std::string ("Init ") : std::string (""));
    valid += std::string ("Scan ") + to_string (ctrl->scan ())
             + std::string (" : ");
    painter.drawText (alti_area_margin, 1 * alti_area_margin, valid);

    if (pl->consistentStart ())
    {
      painter.setPen (ASPen (ASColor::BLACK, 1));
      valid = "Start OK";
    }
    else
    {
      painter.setPen (ASPen (ASColor::RED, 1));
      valid = "Start lost";
    }
    painter.drawText (alti_area_margin + alti_area_width / 4,
                      alti_area_margin, valid);
    if (pl->consistentWidth ())
    {
      painter.setPen (ASPen (ASColor::BLACK, 1));
      valid = "Width OK";
    }
    else
    {
      painter.setPen (ASPen (ASColor::RED, 1));
      valid = "Width lost";
    }
    painter.drawText (alti_area_margin + alti_area_width / 2,
                      alti_area_margin, valid);
    if (pl->consistentEnd ())
    {
      painter.setPen (ASPen (ASColor::BLACK, 1));
      valid = "End OK";
    }
    else
    {
      painter.setPen (ASPen (ASColor::RED, 1));
      valid = "End lost";
    }
    painter.drawText (alti_area_margin + (alti_area_width * 3) / 4,
                      alti_area_margin, valid);

    painter.setPen (ASPen (ASColor::BLACK, 1));
    valid = std::string ("Width estimation = ")
            + format (pl->estimatedWidth ());
    painter.drawText (alti_area_margin, 4 * alti_area_margin, valid);
    valid = std::string ("Angle deviation = ")
            + format (pl->estimatedDeviation ());
    painter.drawText (alti_area_margin, 7 * alti_area_margin, valid);

    // Draws user information
    if (ctrl->isLegendDisplay ())
    {
      int ypos = 8 * alti_area_margin;
      if (ctrl->scan () != 0 && ctrl->isRefDisplay ())
      {
        valid = std::string (
              "Plain red rectangle = predicted template from previous profile");
        painter.drawText (alti_area_margin, w_height - ypos, valid);
        ypos += 3 * alti_area_margin;
      }
      if (ctrl->isTemplateDisplay ())
      {
        valid = std::string (
              "Blue rectangle = detected plateau and its external bounds");
        painter.drawText (alti_area_margin, w_height - ypos, valid);
        ypos += 3 * alti_area_margin;
      }
      if (ctrl->isEstimDisplay ())
      {
        valid = std::string (
              "Over green line = estimated plateau position");
        painter.drawText (alti_area_margin, w_height - ypos, valid);
        ypos += 3 * alti_area_margin;
      }
      if (ctrl->isPredDisplay ())
      {
        valid = std::string (
              "Over red line = retained template for next detection");
        painter.drawText (alti_area_margin, w_height - ypos, valid);
      }
    }

    // Draws detection failure messages
    int status = pl->getStatus ();
    if (status != Plateau::PLATEAU_RES_OK)
    {
      if (status == Plateau::PLATEAU_RES_NOT_ENOUGH_INPUT_PTS)
        painter.drawText (alti_area_margin, 10 * alti_area_margin,
            std::string ("Not enough points in input area"));
      else if (status == Plateau::PLATEAU_RES_TOO_NARROW)
        painter.drawText (alti_area_margin, 10 * alti_area_margin,
            std::string ("Too narrow plateau"));
      else if (status == Plateau::PLATEAU_RES_NOT_ENOUGH_ALT_PTS)
        painter.drawText (alti_area_margin, 10 * alti_area_margin,
            std::string ("Not enough points at input height"));
      else if (status == Plateau::PLATEAU_RES_NOT_ENOUGH_CNX_PTS)
        painter.drawText (alti_area_margin, 10 * alti_area_margin,
            std::string ("Not enough successive points at candidate height"));
      else if (status == Plateau::PLATEAU_RES_NO_BOUND_POS)
        painter.drawText (alti_area_margin, 10 * alti_area_margin,
            std::string ("Both bounds undetected"));
      else if (status == Plateau::PLATEAU_RES_OPTIMAL_HEIGHT_UNDER_USED)
        painter.drawText (alti_area_margin, 10 * alti_area_margin,
            std::string ("Too few optimal height points used"));
      else if (status == Plateau::PLATEAU_RES_TOO_LARGE_NARROWING)
        painter.drawText (alti_area_margin, 10 * alti_area_margin,
            std::string ("Too large narrowing"));
      else if (status == Plateau::PLATEAU_RES_TOO_LARGE_WIDENING)
        painter.drawText (alti_area_margin, 10 * alti_area_margin,
            std::string ("Too large widening"));
      else if (status == Plateau::PLATEAU_RES_NO_BS)
        painter.drawText (alti_area_margin, 10 * alti_area_margin,
            std::string ("No blurred segment"));
      else if (status == Plateau::PLATEAU_RES_TOO_LARGE_BS_TILT)
        painter.drawText (alti_area_margin, 10 * alti_area_margin,
            std::string ("Too tilted blurred segment"));
      else if (status == Plateau::PLATEAU_RES_OUT_OF_HEIGHT_REF)
        painter.drawText (alti_area_margin, 10 * alti_area_margin,
            std::string ("Central point out of height reference"));
    }
  }
}


void ILSDCTrackCrossProfile::paintStatus ()
{
  ASPainter painter (structImage);

  painter.setPen (ASPen (ASColor::RED, 1));
  std::string valid = (ctrl->isInitialDetection () ?
                       std::string ("Initial ") : std::string ("Final "));
  valid += std::string ("failed");
  painter.drawText (alti_area_margin, 3 * alti_area_margin, valid);

  int status = det->getStatus (ctrl->isInitialDetection ());
  if (status != CTrackDetector::RESULT_OK)
  {
    if (status == CTrackDetector::RESULT_NONE)
      painter.drawText (alti_area_margin, 6 * alti_area_margin,
                        std::string ("No detection performed"));
    else if (status == CTrackDetector::RESULT_FAIL_TOO_NARROW_INPUT)
      painter.drawText (alti_area_margin, 6 * alti_area_margin,
                        std::string ("Input stroke too short"));
    else if (status == CTrackDetector::RESULT_FAIL_NO_AVAILABLE_SCAN)
      painter.drawText (alti_area_margin, 6 * alti_area_margin,
                        std::string ("No scan under the stroke"));
    else if (status == CTrackDetector::RESULT_FAIL_NO_CENTRAL_PLATEAU)
      painter.drawText (alti_area_margin, 6 * alti_area_margin,
                        std::string ("No plateau found"));
    else if (status == CTrackDetector::RESULT_FAIL_NO_CONSISTENT_SEQUENCE)
      painter.drawText (alti_area_margin, 6 * alti_area_margin,
                        std::string ("No long enough sequence of plateaux"));
    else if (status == CTrackDetector::RESULT_FAIL_NO_BOUNDS)
      painter.drawText (alti_area_margin, 6 * alti_area_margin,
                        std::string ("No plateau bounds found at start"));
    else if (status == CTrackDetector::RESULT_FAIL_TOO_HECTIC_PLATEAUX)
      painter.drawText (alti_area_margin, 6 * alti_area_margin,
                        std::string ("Too hectic plateaux sequence"));
    else if (status == CTrackDetector::RESULT_FAIL_TOO_SPARSE_PLATEAUX)
      painter.drawText (alti_area_margin, 6 * alti_area_margin,
                        std::string ("Too low plateaux density"));
  }
}


void ILSDCTrackCrossProfile::save (std::string fname)
{
  structImage->save (fname.c_str ());
}
