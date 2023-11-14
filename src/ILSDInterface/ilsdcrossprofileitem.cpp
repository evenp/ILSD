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
#include "ilsdcrossprofileitem.h"
#include "directionalscanner.h"
#include "asImage.h"
#include "asPainter.h"

const int ILSDCrossProfileItem::POS_PRED = 30;
const int ILSDCrossProfileItem::POS_EST = 25;
const int ILSDCrossProfileItem::LG_DIR = 20;
const int ILSDCrossProfileItem::MIN_SCAN = 8;


ILSDCrossProfileItem::ILSDCrossProfileItem (ILSDItemControl *item_ctrl)
{
  ctrl = item_ctrl;
  drawable = false;

  alti_area_margin = 5;
  scan_area_margin = 5;
  resetControls ();

  image = NULL;
  imageWidth = 0;
  imageHeight = 0;
  ptset = NULL;

  iratio = 1.0f; // (1/csize)
  href = 0.0f;
  profshift = 0.0f;
  sratio = 1.0f;
  scale = 1;
  zcale = 1;
  scalength = 0;
  zcalength = 0;
  reversed = false;
  ctrl->resetMeasure ();

  structImage = new ASImage (size ());
}


ILSDCrossProfileItem::~ILSDCrossProfileItem ()
{
  delete structImage;
}


ASCanvasPos ILSDCrossProfileItem::size () const
{
  return ASCanvasPos (w_width + 6, w_height + 6);
}


void ILSDCrossProfileItem::buildScans (Pt2i pt1, Pt2i pt2)
{
  p1.set (pt1);
  p2.set (pt2);
  int scanx = p2.x () - p1.x ();
  int scany = p2.y () - p1.y ();
  drawable = (scanx * scanx + scany * scany >= MIN_SCAN * MIN_SCAN);
  ctrl->resetMeasure ();
}

void ILSDCrossProfileItem::rebuildScans () { }


std::vector<Pt2i> *ILSDCrossProfileItem::getCurrentScan ()
{
  return (getDisplayScan (ctrl->scan ()));
}


void ILSDCrossProfileItem::setData (ASImage* image, IPtTileSet* pdata)
{
  this->image = image;
  imageWidth = image->width ();
  imageHeight = image->height ();
  ptset = pdata;
  iratio = imageWidth / ptset->xmSpread ();  // (1/csize)
  ctrl->resetScan ();
  ctrl->resetMeasure ();
}


bool ILSDCrossProfileItem::isStripAdaptable () const
{
  return false;
}


void ILSDCrossProfileItem::resetControls ()
{
  alti_area_width = ctrl->profileWidth ();
  scan_area_width = ctrl->scanWidth ();
  w_height = ctrl->viewHeight ();
  w_width = alti_area_width + scan_area_width;
  scan_res = ctrl->scanResolution ();
}


void ILSDCrossProfileItem::updateProfile () { }


void ILSDCrossProfileItem::paintScans ()
{
  ASPainter painter (structImage);

  // Draws frame
  painter.setPen (ASPen (ASColor::RED, 2));
  painter.drawRect (alti_area_width + scan_area_margin - 1,
                    scan_area_margin - 1,
                    scan_area_width + 2 - 2 * scan_area_margin,
                    w_height + 2 - 2 * scan_area_margin);

  std::vector<Pt2i>* discan = getDisplayScan (ctrl->scan ());
  if (discan == NULL) return;

  int lx = alti_area_width + scan_area_width / 2;
  int ly = w_height / 2;
  Pt2i scanstart = (reversed ? discan->back () : discan->front ());
  Pt2i scanend = (reversed ? discan->front () : discan->back ());
  Pt2f scancenter ((scanstart.x () + scanend.x ()) / 2.0f,
                   (scanstart.y () + scanend.y ()) / 2.0f);

  // Current scan
  int ssize = (int) discan->size ();
  int cx = lx - (ssize / 2) * scan_res
              - (ssize % 2 == 1 ? scan_res / 2 : 0);
  int cy = ly - scan_res / 2;
  std::vector<Pt2i>::iterator it;
  if (reversed)
  {
    if (cx < w_width - scan_area_margin - scan_res)
    {
      it = discan->end ();
      do
      {
        it --;
        if (cx >= alti_area_width + scan_area_margin)
          painter.fillRect (cx, cy, scan_res, scan_res,
                ASBrush (image->GetPixelColor ((*it).x (),
                                               imageHeight - 1 - (*it).y ())));
          cx += scan_res;
      }
      while (cx < w_width - scan_area_margin - scan_res
             && it != discan->begin ());
    }
  }
  else
  {
    it = discan->begin ();
    while (cx < w_width - scan_area_margin - scan_res
           && it != discan->end ())
    {
      if (cx >= alti_area_width + scan_area_margin)
        painter.fillRect (cx, cy, scan_res, scan_res,
              ASBrush (image->GetPixelColor ((*it).x (),
                                             imageHeight - 1 - (*it).y ())));
      it ++;
      cx += scan_res;
    }
  }

  // Lower scans (right side)
  cy = ly + 2 * scan_res - scan_res / 2;
  int lowbound = w_height - scan_area_margin - scan_res;
  int num = ctrl->scan () - 1;
  int minscan = ctrl->minScan ();
  while (num >= minscan && cy < lowbound)
  {
    discan = getDisplayScan (num);
    if (discan != NULL)
    {
      Pt2i p1n = (reversed ? discan->back () : discan->front ());
      Vr2f p1c (scancenter.x () - p1n.x (), scancenter.y () - p1n.y ());
      float prod = p1c.scalarProduct (p12) / l12;
      cx = lx - (int) (prod * d12 * scan_res + scan_res / 2.0f
                       + (prod < 0 ? -0.5f : 0.5f));
      if (reversed)
      {
        if (cx < w_width - scan_area_margin - scan_res)
        {
          it = discan->end();
          do
          {
            it --;
            if (cx >= alti_area_width + scan_area_margin)
            {
              if (num == 0) painter.fillRect (cx, cy, scan_res, scan_res,
                   ASBrush (ASColor (image->GetPixelColor ((*it).x (),
                                        imageHeight - 1 - (*it).y ())).asInt ()
                            & (ASColor::alphaMask | ASColor::greenMask)));
              else painter.fillRect (cx, cy, scan_res, scan_res,
                   ASBrush (image->GetPixelColor ((*it).x (),
                                        imageHeight - 1 - (*it).y ())));
            }
            cx += scan_res;
          }
          while (cx < w_width - scan_area_margin - scan_res
                 && it != discan->begin());
        }
      }
      else
      {
        it = discan->begin ();
        while (cx < w_width - scan_area_margin - scan_res
               && it != discan->end ())
        {
          if (cx >= alti_area_width + scan_area_margin)
          {
            if (num == 0) painter.fillRect (cx, cy, scan_res, scan_res,
                 ASBrush (ASColor (image->GetPixelColor ((*it).x (),
                                      imageHeight - 1 - (*it).y ())).asInt ()
                          & (ASColor::alphaMask | ASColor::greenMask)));
            else painter.fillRect (cx, cy, scan_res, scan_res,
                 ASBrush (image->GetPixelColor ((*it).x (),
                                      imageHeight - 1 - (*it).y ())));
          }
          it ++;
          cx += scan_res;
        }
      }
    }
    cy += scan_res;
    num --;
  }

  // Upper scans (left side)
  cy = ly - 2 * scan_res - scan_res / 2;
  num = ctrl->scan () + 1;
  int maxscan = ctrl->maxScan ();
  while (num <= maxscan && cy > scan_res)
  {
    if (ctrl->scan () == num) cy -= scan_res;
    discan = getDisplayScan (num);
    if (discan != NULL)
    {
      Pt2i p1n = (reversed ? discan->back () : discan->front ());
      Vr2f p1c (scancenter.x () - p1n.x (), scancenter.y () - p1n.y ());
      float prod = p1c.scalarProduct (p12) / l12;
      cx = lx - (int) (prod * d12 * scan_res + scan_res / 2.0f
                       + (prod < 0 ? -0.5f : 0.5f));
      if (reversed)
      {
        if (cx < w_width - scan_area_margin - scan_res)
        {
          it = discan->end ();
          do
          {
            it --;
            if (cx >= alti_area_width + scan_area_margin)
            {
              if (num == 0) painter.fillRect (cx, cy, scan_res, scan_res,
                   ASBrush (ASColor (image->GetPixelColor ((*it).x (),
                                        imageHeight - 1 - (*it).y ())).asInt ()
                            & (ASColor::alphaMask | ASColor::greenMask)));
              else painter.fillRect (cx, cy, scan_res, scan_res,
                   ASBrush (image->GetPixelColor ((*it).x (),
                                        imageHeight - 1 - (*it).y ())));
            }
            cx += scan_res;
          }
          while (cx < w_width - scan_area_margin - scan_res
                 && it != discan->begin());
        }
      }
      else
      {
        it = discan->begin ();
        while (cx < w_width - scan_area_margin - scan_res
               && it != discan->end ())
        {
          if (cx >= alti_area_width + scan_area_margin)
          {
            if (num == 0) painter.fillRect (cx, cy, scan_res, scan_res,
                 ASBrush (ASColor (image->GetPixelColor ((*it).x (),
                                      imageHeight - 1 - (*it).y ())).asInt ()
                          & (ASColor::alphaMask | ASColor::greenMask)));
            else painter.fillRect (cx, cy, scan_res, scan_res,
                 ASBrush (image->GetPixelColor ((*it).x (),
                                      imageHeight - 1 - (*it).y ())));
          }
          it ++;
          cx += scan_res;
        }
      }
    }
    cy -= scan_res;
    num ++;
  }
  paintMeasureSection ();
}


void ILSDCrossProfileItem::paintAlignedScans ()
{
  ASPainter painter (structImage);

  // Draws frame
  painter.setPen (ASPen (ASColor::RED, 2));
  painter.drawRect (alti_area_width + scan_area_margin - 1,
                    scan_area_margin - 1,
                    scan_area_width + 2 - 2 * scan_area_margin,
                    w_height + 2 - 2 * scan_area_margin);

  // Draws central scan
  std::vector<Pt2i>* discan = getDisplayScan (0);
  int ssize = (int) discan->size ();
  int lx = alti_area_width + scan_area_width / 2 - (ssize / 2) * scan_res
           - (ssize % 2 == 1 ? scan_res / 2 : 0);
  int ly = w_height / 2 - scan_res / 2;
  int cx = lx, cy = ly;

  if (ctrl->scan () > 0) cy += (1 + ctrl->scan ()) * scan_res;
  if (ctrl->scan () < 0) cy -= (1 - ctrl->scan ()) * scan_res;
  std::vector<Pt2i>::iterator it;
  if (reversed)
  {
    it = discan->end();
    do
    {
      it --;
      if (cx >= alti_area_width + scan_area_margin)
      {
        if (ctrl->scan () == 0)
          painter.fillRect (cx, cy, scan_res, scan_res,
               ASBrush (image->GetPixelColor ((*it).x (),
                                    imageHeight - 1 - (*it).y ())));
        else painter.fillRect (cx, cy, scan_res, scan_res,
               ASBrush (ASColor (image->GetPixelColor ((*it).x (),
                                    imageHeight - 1 - (*it).y ())).asInt ()
                        & (ASColor::alphaMask | ASColor::greenMask)));
      }
      cx += scan_res;
    }
    while (cx < w_width - scan_area_margin - scan_res
           && it != discan->begin ());
  }
  else
  {
    it = discan->begin ();
    while (cx < w_width - scan_area_margin - scan_res
           && it != discan->end ())
    {
      if (cx >= alti_area_width + scan_area_margin)
      {
        if (ctrl->scan () == 0)
          painter.fillRect (cx, cy, scan_res, scan_res,
               ASBrush (image->GetPixelColor ((*it).x (),
                                    imageHeight - 1 - (*it).y ())));
        else painter.fillRect (cx, cy, scan_res, scan_res,
               ASBrush (ASColor (image->GetPixelColor ((*it).x (),
                                    imageHeight - 1 - (*it).y ())).asInt ()
                        & (ASColor::alphaMask | ASColor::greenMask)));
      }
      it ++;
      cx += scan_res;
    }
  }
  int ccy = cy;

  // Draws lower part (right side)
  cy += (ctrl->scan () == 0 ? 2 * scan_res : scan_res);
  int lowbound = w_height - scan_area_margin - scan_res;
  int num = -1;
  if (cy < scan_area_margin)
  {
    int offset = ((scan_res - 1) + scan_area_margin - cy) / scan_res;
    num -= offset;
    cy += offset * scan_res;
  }
  int minscan = ctrl->minScan ();
  while (num >= minscan && cy < lowbound)
  {
    if (ctrl->scan () == num) cy += scan_res;
    discan = getDisplayScan (num);
    cx = lx;
    if (reversed)
    {
      it = discan->end ();
      do
      {
        it --;
        if (cx >= alti_area_width + scan_area_margin)
          painter.fillRect (cx, cy, scan_res, scan_res,
                            ASBrush (image->GetPixelColor ((*it).x (),
                                        imageHeight - 1 - (*it).y ())));
        cx += scan_res;
      }
      while (cx < w_width - scan_area_margin - scan_res
             && it != discan->begin ());
    }
    else
    {
      it = discan->begin();
      while (cx < w_width - scan_area_margin - scan_res
             && it != discan->end ())
      {
        if (cx >= alti_area_width + scan_area_margin)
          painter.fillRect (cx, cy, scan_res, scan_res,
                            ASBrush (image->GetPixelColor ((*it).x (),
                                        imageHeight - 1 - (*it).y ())));
        it ++;
        cx += scan_res;
      }
    }
    cy += scan_res;
    if (ctrl->scan () == num) cy += scan_res;
    num--;
  }

  // Draws upper part (left side)
  cy = ccy - (ctrl->scan () == 0 ? 2 * scan_res : scan_res);
  num = 1;
  if (cy > lowbound)
  {
    int offset = ((scan_res - 1) + cy - lowbound) / scan_res;
    num += offset;
    cy -= offset * scan_res;
  }
  int maxscan = ctrl->maxScan ();
  while (num <= maxscan && cy >= scan_area_margin)
  {
    if (ctrl->scan () == num) cy -= scan_res;
    discan = getDisplayScan (num);
    cx = lx;
    if (reversed)
    {
      it = discan->end ();
      do
      {
        it --;
        if (cx >= alti_area_width + scan_area_margin)
          painter.fillRect (cx, cy, scan_res, scan_res,
                            ASBrush (image->GetPixelColor ((*it).x (),
                                        imageHeight - 1 - (*it).y ())));
        cx += scan_res;
      }
      while (cx < w_width - scan_area_margin - scan_res
             && it != discan->begin());
    }
    else
    {
      it = discan->begin();
      while (cx < w_width - scan_area_margin - scan_res
             && it != discan->end ())
      {
        if (cx >= alti_area_width + scan_area_margin)
          painter.fillRect (cx, cy, scan_res, scan_res,
                            ASBrush (image->GetPixelColor ((*it).x (),
                                        imageHeight - 1 - (*it).y ())));
        it ++;
        cx += scan_res;
      }
    }
    cy -= scan_res;
    if (ctrl->scan () == num) cy -= scan_res;
    num++;
  }
  paintMeasureSection ();
}


void ILSDCrossProfileItem::paintMeasureSection ()
{
  if (ctrl->isMeasuring () && ctrl->isSetMeasure ())
  {
    ASPainter painter (structImage);
    painter.setPen (ASPen (ASColor::BLUE, 2));

//    int ystart = w_height / 2 - scan_res / 2;
    int ystart = w_height / 2;
    int ystop = ystart;
    if (ctrl->isSetMeasureStart () && ctrl->measureStart () != ctrl->scan ())
      ystart += ((ctrl->measureStart () > ctrl->scan () ? -1 : 1)
                + ctrl->scan () - ctrl->measureStart ()) * scan_res;
    if (ctrl->isSetMeasureStop () && ctrl->measureStop () != ctrl->scan ())
      ystop += ((ctrl->measureStop () > ctrl->scan () ? -1 : 1)
               + ctrl->scan () - ctrl->measureStop ()) * scan_res;
    if (ystart < w_height - scan_area_margin - scan_res)
      if (ystart >= scan_area_margin)
        painter.drawLine (alti_area_width + 2 * scan_area_margin, ystart,
                          alti_area_width + 4 * scan_area_margin, ystart);
      else ystart = scan_area_margin;
    else ystart = w_height - scan_area_margin - scan_res;
    if (ystop < w_height - scan_area_margin - scan_res)
      if (ystop >= scan_area_margin)
        painter.drawLine (alti_area_width + 2 * scan_area_margin, ystop,
                          alti_area_width + 4 * scan_area_margin, ystop);
      else ystop = scan_area_margin;
    else ystop = w_height - scan_area_margin - scan_res;
    if (ystart != ystop)
      painter.drawLine (alti_area_width + 2 * scan_area_margin, ystart,
                        alti_area_width + 2 * scan_area_margin, ystop);
  }
}


void ILSDCrossProfileItem::paintProfile ()
{
  ASPainter painter (structImage);
  int zr = ctrl->zRatio ();
  float zratio = sratio * zr;

  // Draws metric reference
  int meas = scale;
  while (meas > 6) meas /= 10;
  bool pair = (meas == 1);
  // while (alti_area_width > 10 * scale * sratio) // with hysteresis
  while (9 * alti_area_width > 10 * scale * sratio) // no hysteresis
  {
    scale *= (pair ? 5 : 2);
    pair = ! pair;
  }
  while (scale > 1 && 9 * alti_area_width < 10 * scale * sratio)
  {
    scale /= (pair ? 2 : 5);
    pair = ! pair;
  }
  scalength = (int) (scale * sratio + 0.5);
  painter.setPen (ASPen (ASColor::BLACK, 2));
  painter.drawLine (12, w_height - 2, 12 + scalength, w_height - 2);
  if (zr == 1) painter.drawLine (12, w_height - 2, 12, w_height - 6);
  painter.drawLine (12 + scalength, w_height - 2, 12 + scalength, w_height - 6);

  // Draws Z metric reference when a Z-ratio is applied
  if (zr != 1)
  {
    meas = zcale;
    while (meas > 6) meas /= 10;
    pair = (meas == 1);
    // while (w_height / 2 > 10 * zcale * zratio)  // with hysteresis
    while (9 * (w_height / 2) > zcale * zratio)  // no hysteresis
    {
      zcale *= (pair ? 5 : 2);
      pair = ! pair;
    }
    while (zcale > 1 && 9 * (w_height / 2) < zcale * zratio)
    {
      zcale /= (pair ? 2 : 5);
      pair = ! pair;
    }
    zcalength = (int) ((zcale * zratio) * 0.1 + 0.5);
    painter.drawLine (12, w_height - 2, 12, w_height - 2 - zcalength);
    painter.drawLine (12, w_height - 2 - zcalength,
                       16, w_height - 2 - zcalength);
  }

  // Draws altimetric reference bar
  if (ctrl->isAltiDisplay ())
  {
    painter.setPen (ASPen (ctrl->isStaticHeight () ?
                           ASColor::BLACK : ASColor::GRAY, 1));
    painter.drawLine (0, w_height / 2, alti_area_width, w_height / 2);
  }

  // Draws profile points
  int altiShift = alti_area_margin - ctrl->profileShift ();
  int lb = alti_area_margin, rb = alti_area_width - 2 * alti_area_margin;
  int tb = alti_area_margin, bb = w_height - alti_area_margin;
  painter.setPen (ASPen (ASColor::BLACK, ctrl->pointSize ()));
  std::vector<Pt2f>* ppts = getProfile (ctrl->scan ());
  if (ppts != NULL)
  {
    std::vector<Pt2f>::iterator pit = ppts->begin ();
    while (pit != ppts->end ())
    {
      int x = altiShift + (int) ((pit->x () + profshift) * sratio + 0.5);
      int y = w_height / 2 - (int) ((pit->y () - href) * zratio + 0.5);
      if (x >= lb && x <= rb && y >= tb && y <= bb)
        painter.drawPoint (x, y);
      pit ++;
    }
  }
}


void ILSDCrossProfileItem::paintInfo ()
{
  ASPainter painter (structImage);

  /** Text of the metric reference */
  std::string valid = to_string (scale) + std::string (" m");
  painter.drawText (scalength - 40, w_height - 32, valid);

  if (ctrl->zRatio () != 1)
  {
    valid = to_string (zcale / 10);
    if (zcale % 10 != 0) valid += "." + to_string (zcale % 10);
    valid += std::string (" m");
    painter.drawText (12, w_height - zcalength, valid);
  }

  /** Text of the altimetric reference */
  if (ctrl->isAltiDisplay ())
  {
    valid = format (href) + std::string (" m");
    painter.drawText (alti_area_margin,
                      w_height / 2 - 2 * alti_area_margin - 13, valid);
  }
}


float ILSDCrossProfileItem::getMeasureLineTranslationRatio () const {
  return 0.0f; }

void ILSDCrossProfileItem::incMeasureLineTranslationRatio (int inc) { }

void ILSDCrossProfileItem::setMeasureLineTranslationRatio (float val) { }

float ILSDCrossProfileItem::getMeasureLineRotationRatio () const {
  return 0.0f; }

void ILSDCrossProfileItem::incMeasureLineRotationRatio (int inc) { }

void ILSDCrossProfileItem::setMeasureLineRotationRatio (float val) { }


void ILSDCrossProfileItem::clipLine (ASPainter &painter,
                                     int p1x, int p1y, int p2x, int p2y,
                                     int bbl, int bbd, int bbr, int bbu) const
{
  int n = 0;
  Pt2i *pts = Pt2i(p1x, p1y).clipLine (Pt2i (p2x, p2y),
                                       bbl, bbd, bbr, bbu, &n);
  for (int i = 0; i < n; i++)
    painter.drawPoint (ASCanvasPos (pts[i].x (), pts[i].y ()));
  delete [] pts;
}


std::string ILSDCrossProfileItem::format (float val) const
{
  int k = 1;
  int dr = ctrl->decimalResolution ();
  int *d = new int[dr];
  int r = 0;
  float add = 0.5f;
  for (int j = 0; j < dr; j++) add /= 10.0f;
  val += (val < 0 ? -add : add);
  int pr = (int) val;
  for (int i = 0; i < dr; i++)
  {
    d[i] = (int) (val * k * 10) - 10 * (int) (val * k);
    if (d[i] != 0)
    {
      r = i + 1;
      if (d[i] < 0) d[i] = -d[i];
    }
    k *= 10;
  }
  bool n = (val < 0);
  if (n && pr == 0)
  {
    bool allnull = true;
    for (int i = 0; allnull && i < dr; i++) if (d[i] != 0) allnull = false;
    if (allnull) n = false;
  }
  std::string s = std::string (n ? "-" : "") + to_string (pr);
  if (r != 0)
  {
    s += std::string (".");
    for (int i = 0; i < r; i++) s += to_string (d[i]);
  }
  return s;
}
