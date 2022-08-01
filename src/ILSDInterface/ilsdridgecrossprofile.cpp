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
#include "ilsdridgecrossprofile.h"
#include "directionalscanner.h"
#include "asImage.h"
#include "math.h"
#include "asPainter.h"

#define EPSILON 0.0001f


ILSDRidgeCrossProfile::ILSDRidgeCrossProfile (RidgeDetector* detector,
                                              ILSDItemControl *item_ctrl)
                      : ILSDCrossProfileItem (item_ctrl)
{
  det = detector;
  meas_est = 0.0f;
  meas_low = 0.0f;
  meas_up = 0.0f;
}


std::string ILSDRidgeCrossProfile::profileName () const
{
  return ("bump");
}


void ILSDRidgeCrossProfile::paint (GLWindow* context)
{
  if (! structImage || size () != structImage->getImageResolution ())
  {
    if (structImage) delete structImage;
    structImage = new ASImage (size ());
    update ();
  }
  structImage->Draw (context);

  if (drawable)
  {
    if (det->getRidge (ctrl->isInitialDetection ()) != NULL) paintInfo ();
    else paintStatus ();
  }
}


void ILSDRidgeCrossProfile::update ()
{
  structImage->clear (ASColor::WHITE);
  if (drawable)
  {
    updateDrawing ();
    if (det->getRidge (ctrl->isInitialDetection ()) != NULL)
    {
      if (ctrl->isAligned ()) paintAlignedScans ();
      else paintScans ();
      paintProfile ();
      if (ctrl->isCurrentScanMeasured ()) updateMeasure ();
      // paintInfo (painter);
    }
    else
    {
      if (ctrl->isAligned ()) ILSDCrossProfileItem::paintAlignedScans ();
      else ILSDCrossProfileItem::paintScans ();
      ILSDCrossProfileItem::paintProfile ();
      // else paintStatus (painter);
    }
  }
}


void ILSDRidgeCrossProfile::updateMeasure ()
{
  if (ctrl->isMeasuring ())
  {
    Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
    meas_est = 0.0f;
    meas_low = 0.0f;
    meas_up = 0.0f;
    if (ctrl->isSetMeasure ())
    {
      int m1 = ctrl->measureIntervalStart ();
      int m2 = ctrl->measureIntervalStop ();
      if (m1 == m2)
      {
        Bump* bmp = rdg->bump (m1);
        if (bmp != NULL && bmp->isFound ())
        {
          meas_est = bmp->estimatedArea ();
          meas_low = bmp->estimatedAreaLowerBound ();
          meas_up = bmp->estimatedAreaUpperBound ();
        }
      }
      else meas_est = rdg->estimateVolume (m1, m2, iratio, meas_low, meas_up);
    }
  }
}


std::vector<Pt2i> *ILSDRidgeCrossProfile::getDisplayScan (int num)
{
  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  if (rdg != NULL) return (rdg->getDisplayScan (num));
  else return NULL;
}


std::vector<Pt2f> *ILSDRidgeCrossProfile::getProfile (int num)
{
  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  if (rdg != NULL) return (rdg->getProfile (num));
  else return NULL;
}


void ILSDRidgeCrossProfile::updateDrawing ()
{
  // Sets display parameters
  det->getInputStroke (p1, p2, false);
  float scanx = (float) (p2.x () - p1.x ());
  float scany = (float) (p2.y () - p1.y ());
  p12.set (scanx, scany);
  l12 = (float) sqrt (scanx * scanx + scany * scany);
  if (scanx < 0) scanx = -scanx;
  if (scany < 0) scany = -scany;
  d12 = (scany > scanx ? scany : scanx) / l12;
  sratio = (alti_area_width - 2 * alti_area_margin) * iratio / l12;

  // Sets scan range
  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  if (rdg != NULL)
  {
    ctrl->setMinScan (- rdg->getRightScanCount ());
    ctrl->setMaxScan (rdg->getLeftScanCount ());
  }
  else ctrl->resetScan ();

  if (rdg != NULL)
  {
    // Sets scan direction
    reversed = rdg->isScanReversed (0);

    // Sets height and side references
    Bump *bmp = rdg->lastValidBump (ctrl->scan ());
    if (ctrl->scan () == 0 && (bmp == NULL || ! bmp->isFound ()))
    {
      href = getProfile(0)->front().y ();
      profshift = 0.0f;
    }
    else
    {
      if (ctrl->isStaticHeight () || bmp == NULL)
        href = rdg->bump(0)->estimatedMassCenter().y ();
      else href = bmp->estimatedMassCenter().y ();
      if (bmp == NULL) profshift = 0.0f;
      else profshift = l12 / (2 * iratio) - bmp->estimatedCenter().x ();
    }
  }
}


float ILSDRidgeCrossProfile::getMeasureLineTranslationRatio () const
{
  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  if (rdg != NULL)
  {
    Bump* bmp = rdg->bump (ctrl->scan ());
    if (bmp->getStatus () == Bump::RES_OK)
      return (bmp->getMeasureLineTranslationRatio ());
  }
  return 0.0f;
}

void ILSDRidgeCrossProfile::incMeasureLineTranslationRatio (int inc)
{
  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  if (rdg != NULL)
  {
    Bump* bmp = rdg->bump (ctrl->scan ());
    if (bmp->getStatus () == Bump::RES_OK)
    {
      bmp->incMeasureLineTranslationRatio (inc,
                                           rdg->getProfile (ctrl->scan ()));
      if (ctrl->isCurrentScanMeasured ()) updateMeasure ();
    }
  }
}

void ILSDRidgeCrossProfile::setMeasureLineTranslationRatio (float val)
{
  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  if (rdg != NULL)
  {
    Bump* bmp = rdg->bump (ctrl->scan ());
    if (bmp->getStatus () == Bump::RES_OK)
    {
      bmp->setMeasureLineTranslationRatio (val,
                                           rdg->getProfile (ctrl->scan ()));
      if (ctrl->isCurrentScanMeasured ()) updateMeasure ();
    }
  }
}

float ILSDRidgeCrossProfile::getMeasureLineRotationRatio () const
{
  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  if (rdg != NULL)
  {
    Bump* bmp = rdg->bump (ctrl->scan ());
    if (bmp->getStatus () == Bump::RES_OK)
      return bmp->getMeasureLineRotationRatio ();
  }
  return 0.0f;
}

void ILSDRidgeCrossProfile::incMeasureLineRotationRatio (int inc)
{
  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  if (rdg != NULL)
  {
    Bump* bmp = rdg->bump (ctrl->scan ());
    if (bmp->getStatus () == Bump::RES_OK)
    {
      bmp->incMeasureLineRotationRatio (inc, rdg->getProfile (ctrl->scan ()));
      if (ctrl->isCurrentScanMeasured ()) updateMeasure ();
    }
  }
}

void ILSDRidgeCrossProfile::setMeasureLineRotationRatio (float val)
{
  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  if (rdg != NULL)
  {
    Bump* bmp = rdg->bump (ctrl->scan ());
    if (bmp->getStatus () == Bump::RES_OK)
    {
      bmp->setMeasureLineRotationRatio (val, rdg->getProfile (ctrl->scan ()));
      if (ctrl->isCurrentScanMeasured ()) updateMeasure ();
    }
  }
}


void ILSDRidgeCrossProfile::paintScans ()
{
  ASPainter painter (structImage);

  // Draws detected area
  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  Bump* bmp = rdg->lastValidBump (ctrl->scan ());
  if (bmp != NULL)
  {
    int lx = alti_area_width + scan_area_width / 2;
    int lbb = alti_area_width + scan_area_margin - 1;
    int rbb = w_width + 1 - scan_area_margin;
    float start = bmp->estimatedStart ();
    float end = bmp->estimatedEnd ();
    float pos = bmp->estimatedCenter().x ();

    std::vector<Pt2i> *discan = rdg->getDisplayScan (ctrl->scan ());
    Pt2i scanstart = (reversed ? discan->back () : discan->front ());
    Pt2i scanend = (reversed ? discan->front () : discan->back ());
    Pt2f scancenter ((scanstart.x () + scanend.x ()) / 2.0f,
                     (scanstart.y () + scanend.y ()) / 2.0f);
    Pt2f p1f ((float) p1.x (), (float) p1.y ());
    Vr2f scanpos = p1f.vectorTo (scancenter);
    float delta = scanpos.scalarProduct (p12) / l12;

    float mes = (delta - start * iratio) * d12 * scan_res;
    int cx = lx - (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
    if (cx < rbb)
    {
      if (cx < lbb) cx = lbb;
      mes = (delta - end * iratio) * d12 * scan_res;
      int ex = lx - (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
      if (ex > lbb)
      {
        if (ex > rbb) ex = rbb;
        painter.setPen(ASPen (ASColor::GREEN, 2));
        painter.drawRect (cx, w_height / 2 - scan_res,
                          ex + 1 - cx, 2 * scan_res);
      }
    }

    mes = (delta - pos * iratio) * d12 * scan_res;
    int fx = lx - (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
    if (fx > lbb && fx < rbb)
    {
      painter.setPen (ASPen (ASColor::BLUE, 2));
      painter.drawLine (fx, w_height / 2 - 2 * scan_res - 1,
                        fx, w_height / 2 + 2 * scan_res + 1);
    }
  }

  // Draws frame and scans
  ILSDCrossProfileItem::paintScans ();
}


void ILSDRidgeCrossProfile::paintAlignedScans ()
{
  ASPainter painter (structImage);

  // Draws detected area
  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  Bump* bmp = rdg->bump (ctrl->scan ());
  if (bmp != NULL)
  {
    std::vector<Pt2i> *discan = rdg->getDisplayScan (0);
    int ssize = (int) discan->size ();
    int lx = alti_area_width + (scan_area_width / 2 - (ssize / 2) * scan_res)
             + (ssize % 2 == 1 ? 0 : scan_res / 2);
    int lbb = alti_area_width + scan_area_margin - 1;
    int rbb = w_width + 1 - scan_area_margin;

    discan = rdg->getDisplayScan (ctrl->scan ());
    Pt2i scanstart = (reversed ? discan->back () : discan->front ());
    Pt2i scanend = (reversed ? discan->front () : discan->back ());
    Vr2f scanpos ((float) (p1.x () - scanstart.x ()),
                  (float) (p1.y () - scanstart.y ()));
    float delta = scanpos.scalarProduct (p12) / l12;

    float summit = bmp->estimatedCenter().x ();
    float start = bmp->estimatedStart ();
    float end = bmp->estimatedEnd ();

    float mes = (delta + start * iratio) * d12 * scan_res;
    int cx = lx + (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
    if (cx < rbb)
    {
      if (cx < lbb) cx = lbb;
      mes = (delta + end * iratio) * d12 * scan_res;
      int ex = lx + (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
      if (ex > lbb)
      {
        if (ex > rbb) ex = rbb;
        painter.setPen (ASPen (ASColor::GREEN, 2));
        painter.drawRect (cx, w_height / 2 - scan_res,
                          ex + 1 - cx, 2 * scan_res);
      }
    }

    mes = (delta + summit * iratio) * d12 * scan_res;
    int fx = lx + (int) (mes < 0.0f ? mes - 0.5f : mes + 0.5f);
    if (fx > lbb && fx < rbb)
    {
      painter.setPen (ASPen (ASColor::BLUE, 2));
      painter.drawLine (fx, w_height / 2 - 2 * scan_res,
                        fx, w_height / 2 + 2 * scan_res);
    }
  }

  // Draws frame and scans
  ILSDCrossProfileItem::paintAlignedScans ();
}


void ILSDRidgeCrossProfile::paintProfile ()
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

  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  Bump *bmp = rdg->bump (ctrl->scan ());
  if (bmp != NULL)
  {
    int isum = 0, ilow = 0, ihigh = 0, iright = 0;

    // Draws reference bump
    float summit = bmp->referenceMassCenter().x ();
    float alti = bmp->referenceMassCenter().y () - href;
    float height = bmp->referenceHeight ();
    float width = bmp->referenceWidth ();
    ilow = (int) (height * sratio + (height < 0.0f ? -0.5f : 0.5f));
    iright = (int) (width * sratio + (height < 0.0f ? -0.5f : 0.5f));
    ihigh = (int) (alti * sratio + (height < 0.0f ? -0.5f : 0.5f));
    isum = (int) ((summit + profshift) * sratio + 0.5f);
    if (ctrl->isRefDisplay ())
    {
      int i1 = isum - iright / 2;
      int i2 = isum + iright - iright / 2;
      int i3 = ihigh + 2;
      int i4 = ihigh - 2;
      if (i1 <= bbr && i2 >= bbl && i3 >= bbd && i4 <= bbu)
      {
        if (i1 < bbl) i1 = bbl;
        if (i2 > bbr) i2 = bbr;
        if (i3 > bbu) i3 = bbu;
        if (i4 < bbd) i4 = bbd;
        painter.fillRect (altiShift + i1,
                          refh - i3, i2 - i1, i3 - i4, ASColor::RED);
      }
      i1 = isum - 2;
      i2 = isum + 2;
      i3 = ihigh + ilow / 2;
      i4 = ihigh - ilow  + ilow / 2;
      if (i1 <= bbr && i2 >= bbl && i3 >= bbd && i4 <= bbu)
      {
        if (i1 < bbl) i1 = bbl;
        if (i2 > bbr) i2 = bbr;
        if (i3 > bbu) i3 = bbu;
        if (i4 < bbd) i4 = bbd;
        painter.fillRect (altiShift + i2,
                          refh - i3, i2 - i1, i3 - i4, ASColor::RED);
      }
    }

    // Draws start trend
    if (ctrl->isTemplateDisplay () && bmp->hasStartTrend ())
    {
      Pt2f st_start (bmp->startTrendStart ());
      Pt2f st_end (bmp->startTrendEnd ());
      float st_width = bmp->startTrendThickness () / 2;
      int st_sx = (int) ((st_start.x () + profshift) * sratio + 0.5);
      int st_ex = (int) ((st_end.x () + profshift) * sratio + 0.5); 
      int st_sd = (int) ((st_start.y () + st_width - href) * sratio + 0.5);
      int st_su = (int) ((st_start.y () - st_width - href) * sratio + 0.5);
      int st_ed = (int) ((st_end.y () + st_width - href) * sratio + 0.5);
      int st_eu = (int) ((st_end.y () - st_width - href) * sratio + 0.5);
      painter.setPen (ASPen (ASColor::RED, 2));
      clipLine (painter, altiShift + st_sx, refh - st_sd,
                altiShift + st_sx, refh - st_su, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + st_sx, refh - st_su,
                altiShift + st_ex, refh - st_eu, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + st_ex, refh - st_eu,
                altiShift + st_ex, refh - st_ed, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + st_ex, refh - st_ed,
                altiShift + st_sx, refh - st_sd, abbl, abbd, abbr, abbu);
    }

    // Draws end trend
    if (ctrl->isTemplateDisplay () && bmp->hasEndTrend ())
    {
      Pt2f et_start (bmp->endTrendStart ());
      Pt2f et_end (bmp->endTrendEnd ());
      float et_width = bmp->endTrendThickness () / 2;
      int et_sx = (int) ((et_start.x () + profshift) * sratio + 0.5);
      int et_ex = (int) ((et_end.x () + profshift) * sratio + 0.5);
      int et_sd = (int) ((et_start.y () + et_width - href) * sratio + 0.5);
      int et_su = (int) ((et_start.y () - et_width - href) * sratio + 0.5);
      int et_ed = (int) ((et_end.y () + et_width - href) * sratio + 0.5);
      int et_eu = (int) ((et_end.y () - et_width - href) * sratio + 0.5);
      painter.setPen (ASPen (ASColor::RED, 2));
      clipLine (painter, altiShift + et_sx, refh - et_sd,
                altiShift + et_sx, refh - et_su, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + et_sx, refh - et_su,
                altiShift + et_ex, refh - et_eu, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + et_ex, refh - et_eu,
                altiShift + et_ex, refh - et_ed, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + et_ex, refh - et_ed,
                altiShift + et_sx, refh - et_sd, abbl, abbd, abbr, abbu);
    }

    // Draws bump
    if (ctrl->isTemplateDisplay () && bmp->getStatus () == Bump::RES_OK)
    {
      Pt2f start (bmp->start ());
      Pt2f end (bmp->end ());
      Pt2f center (bmp->estimatedSurfCenter ());
      Pt2f mcenter (bmp->estimatedCenter ());
      int imx = (int) ((mcenter.x () + profshift) * sratio + 0.5f);
      int imy = (int) ((mcenter.y () - href) * sratio + 0.5f);
      int icx = (int) ((center.x () + profshift) * sratio + 0.5f);
      int icy = (int) ((center.y () - href) * sratio + 0.5f);
      int isx = (int) ((start.x () + profshift) * sratio + 0.5f);
      int isy = (int) ((start.y () - href) * sratio + 0.5f);
      int iex = (int) ((end.x () + profshift) * sratio + 0.5f);
      int iey = (int) ((end.y () - href) * sratio + 0.5f);
      painter.setPen (ASPen (ASColor::BLUE, 2));
//      clipLine (painter, altiShift + isx, refh - isy,
//                altiShift + iex, refh - iey, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + isx, refh - isy,
                altiShift + icx, refh - icy, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + icx, refh - icy,
                altiShift + iex, refh - iey, abbl, abbd, abbr, abbu);
      clipLine (painter, altiShift + icx, refh - icy,
                altiShift + imx, refh - imy, abbl, abbd, abbr, abbu);

      // Draws baseline
      start.set (bmp->lineStart ());
      end.set (bmp->lineEnd ());
      isx = (int) ((start.x () + profshift) * sratio + 0.5f);
      isy = (int) ((start.y () - href) * sratio + 0.5f);
      iex = (int) ((end.x () + profshift) * sratio + 0.5f);
      iey = (int) ((end.y () - href) * sratio + 0.5f);
      painter.setPen (ASPen (ASColor::BLUE, 2));
      clipLine (painter, altiShift + isx, refh - isy,
                altiShift + iex, refh - iey, abbl, abbd, abbr, abbu);

      // Draws direction
      if (ctrl->isDirDisplay () && bmp->isAccepted ())
      {
        float dev = bmp->estimatedDeviation ();
        float cs = det->getCellSize ();
        int idev = (int) (LG_DIR * dev / cs + (dev < 0 ? -0.5f : 0.5f));
        if ((dev < 0.0f ? -dev : dev) > cs)
          painter.setPen (ASPen (ASColor::RED, 2));
        else painter.setPen (ASPen (ASColor::GREEN, 2));
        clipLine (painter, altiShift + isum, refh - ihigh, 
                  altiShift + isum + idev, refh - ihigh - LG_DIR,
                  abbl, abbd, abbr, abbu);
      }

      // Draws measure lower line
      if (ctrl->isMeasuring ())
      {
        int lisx = (int) ((bmp->measureLineStart().x () + profshift)
                          * sratio + 0.5f);
        int lisy = (int) ((bmp->measureLineStart().y () - href)
                          * sratio + 0.5f);
        int liex = (int) ((bmp->measureLineEnd().x () + profshift)
                          * sratio + 0.5f);
        int liey = (int) ((bmp->measureLineEnd().y () - href)
                          * sratio + 0.5f);
        painter.setPen (ASPen (ASColor::GREEN, 2));
        clipLine (painter, altiShift + lisx, refh - lisy,
                  altiShift + liex, refh - liey, abbl, abbd, abbr, abbu);
      }
    }
  }

  // Draws profile points
  ILSDCrossProfileItem::paintProfile ();
}


void ILSDRidgeCrossProfile::paintInfo ()
{
  // Draws metric reference values
  ILSDCrossProfileItem::paintInfo ();

  ASPainter painter (structImage);

  Ridge *rdg = det->getRidge (ctrl->isInitialDetection ());
  if (rdg == NULL) return;
  Bump* bmp = rdg->bump (ctrl->scan ());
  if (bmp != NULL)
  {
    // Draws detection result values
    int jpos = 1;
    painter.setPen (ASPen (bmp->isAccepted () ? ASColor::BLACK : ASColor::RED,
                           1));
    std::string valid = std::string ("S") + to_string (ctrl->scan ())
                        + std::string (" : ");
    if (bmp->isAccepted ()) valid += std::string ("  Bump OK  ");
    else if (bmp->isFound ()) valid += std::string ("  Bump lost");
    else valid += std::string ("  Bump undetected");
    painter.drawText (alti_area_margin, jpos * alti_area_margin, valid);
    jpos += 3;

    if (bmp->isFound ())
    {
      painter.setPen (ASPen (ASColor::BLACK, 1));
      valid = std::string ("Height estimation = ")
              + format (bmp->estimatedHeight ()) + std::string (" m");
      painter.drawText (alti_area_margin, jpos * alti_area_margin, valid);
      jpos += 3;
      valid = std::string ("Width estimation = ")
              + format (bmp->estimatedWidth ()) + std::string (" m");
      painter.drawText (alti_area_margin, jpos * alti_area_margin, valid);
      jpos += 3;
      if (ctrl->isMeasuring ())
      {
        valid = std::string ("Area: ") + format (bmp->estimatedArea ())
            + std::string (" m2 [") + format (bmp->estimatedAreaLowerBound ())
            + std::string (", ") + format (bmp->estimatedAreaUpperBound ())
            + std::string ("]");
        painter.drawText (alti_area_margin, jpos * alti_area_margin, valid);
        jpos += 3;
      }
    }

    // Draws geometrical measures estimation
    int ypos = 8 * alti_area_margin;
    if (ctrl->isMeasuring () && ctrl->isSetMeasure ())
    {
      painter.setPen (ASPen (ASColor::BLACK, 1));
      int m1 = ctrl->measureIntervalStart ();
      int m2 = ctrl->measureIntervalStop ();
      if (m1 == m2)
        valid = std::string ("Area = ")
                + format (meas_est) + std::string (" m2 [")
                + format (meas_low) + std::string (", ")
                + format (meas_up) + std::string ("]");
      else
        valid = std::string ("Vol = ")
                + format (meas_est) + std::string (" m3 [")
                + format (meas_low) + std::string (", ")
                + format (meas_up) + std::string ("]");
      painter.drawText (alti_area_margin, w_height - ypos, valid);
      ypos += 3 * alti_area_margin;
    }

    // Draws shifts to reference
    if (ctrl->scan () != 0)
    {
      int validx = alti_area_margin + (alti_area_width * 3) / 5;
      valid = std::string ("dY = ") + format (bmp->positionShift ());
      painter.drawText (validx, 1 * alti_area_margin, valid);
      valid = std::string ("dZ = ") + format (bmp->altitudeShift ());
      painter.drawText (validx, 4 * alti_area_margin, valid);
      valid = std::string ("dW = ") + format (bmp->widthShift ());
      painter.drawText (validx, 7 * alti_area_margin, valid);
      valid = std::string ("dH = ") + format (bmp->heightShift ());
      painter.drawText (validx, 10 * alti_area_margin, valid);
    }

    // Draws user information
    if (ctrl->isLegendDisplay ())
    {
      if (ctrl->scan () != 0 && ctrl->isDirDisplay ())
      {
        valid = std::string ("Slanted green stroke = estimated deviation");
        painter.drawText (alti_area_margin, w_height - ypos, valid);
        ypos += 3 * alti_area_margin;
      }
      if (ctrl->scan () != 0 && ctrl->isRefDisplay ())
      {
        valid = std::string (
          "Red cross = predicted template from previous bump");
        painter.drawText (alti_area_margin, w_height - ypos, valid);
        ypos += 3 * alti_area_margin;
      }
      if (ctrl->isTemplateDisplay ())
      {
        valid = std::string (
                   "Blue triangle = detected bump with center and bounds");
        painter.drawText (alti_area_margin, w_height - ypos, valid);
        ypos += 3 * alti_area_margin;
        if (ctrl->isMeasuring ())
          valid = std::string ("Green line = measure baseline");
        else valid = std::string ("Green line = detected baseline");
        painter.drawText (alti_area_margin, w_height - ypos, valid);
      }
    }

    // Draws failure messages
    int status = bmp->getStatus ();
    if (status != Bump::RES_OK)
    {
      if (status == Bump::RES_NOT_ENOUGH_INPUT_PTS)
        painter.drawText (alti_area_margin, 4 * alti_area_margin,
                          std::string ("Not enough points in input area"));
      if (status == Bump::RES_HOLE_IN_INPUT_PTS)
        painter.drawText (alti_area_margin, 4 * alti_area_margin,
                          std::string ("Presence of hole in input points"));
      else if (status == Bump::RES_TOO_LOW)
        painter.drawText (alti_area_margin, 4 * alti_area_margin,
                          std::string ("Too low bump"));
      else if (status == Bump::RES_TOO_NARROW)
        painter.drawText (alti_area_margin, 4 * alti_area_margin,
                          std::string ("Too narrow bump"));
      else if (status == Bump::RES_EMPTY_SCAN)
        painter.drawText (alti_area_margin, 4 * alti_area_margin,
                          std::string ("Empty scan"));
      else if (status == Bump::RES_NO_BUMP_LINE)
        painter.drawText (alti_area_margin, 4 * alti_area_margin,
                          std::string ("No bump found"));
      else if (status == Bump::RES_LINEAR)
        painter.drawText (alti_area_margin, 4 * alti_area_margin,
                          std::string ("Linear profile"));
      else if (status == Bump::RES_ANGULAR)
        painter.drawText (alti_area_margin, 4 * alti_area_margin,
                          std::string ("Pure break-in-slope profile"));
    }
    else
    {
      int def = bmp->getDefault ();
      if (def != Bump::DEF_NONE)
      {
        valid = std::string ("Def:");
        if (def & Bump::DEF_POSITION) valid += std::string (" X");
        if (def & Bump::DEF_ALTITUDE) valid += std::string (" Y");
        if (def & Bump::DEF_WIDTH) valid += std::string (" W");
        if (def & Bump::DEF_HEIGHT) valid += std::string (" H");
        painter.drawText (alti_area_margin, jpos * alti_area_margin, valid);
      }
    }
  }
}


void ILSDRidgeCrossProfile::paintStatus ()
{
  ASPainter painter (structImage);

  painter.setPen (ASPen (ASColor::RED, 1));
  std::string valid = std::string ("Detection failed");
  painter.drawText (alti_area_margin, 1 * alti_area_margin, valid);

  int status = det->getStatus (false);
  // int status = det->getStatus (ctrl->isInitialDetection ());
  if (status != RidgeDetector::RESULT_OK)
  {
    if (status == RidgeDetector::RESULT_NONE)
      painter.drawText (alti_area_margin, 4 * alti_area_margin,
                        std::string ("No detection performed"));
    else if (status == RidgeDetector::RESULT_FAIL_TOO_NARROW_INPUT)
      painter.drawText (alti_area_margin, 4 * alti_area_margin,
                        std::string ("Input stroke too short"));
    else if (status == RidgeDetector::RESULT_FAIL_NO_AVAILABLE_SCAN)
      painter.drawText (alti_area_margin, 4 * alti_area_margin,
                        std::string ("No scan under the stroke"));
    else if (status == RidgeDetector::RESULT_FAIL_NO_CENTRAL_BUMP)
      painter.drawText (alti_area_margin, 4 * alti_area_margin,
                        std::string ("No bump found"));
  }
}


void ILSDRidgeCrossProfile::save (std::string fname)
{
  structImage->save (fname.c_str ());
}
