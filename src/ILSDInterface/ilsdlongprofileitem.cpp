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
#include "ilsdlongprofileitem.h"
#include "directionalscanner.h"
#include "asImage.h"
#include "asPainter.h"

const int ILSDLongProfileItem::MIN_SCAN = 8;


ILSDLongProfileItem::ILSDLongProfileItem (ILSDItemControl *item_ctrl)
{
  ctrl = item_ctrl;
  drawable = false;
  zmin = 0.0f;
  zmax = 0.0f;

  alti_area_margin = 6;
  alti_left_margin = alti_area_margin;
  alti_right_margin = 1;
  alti_bottom_margin = -3;
  alti_top_margin = 14;
  scale_height = 22;
  level_width = 76;
  index_length = 16;

  ptset = NULL;
  href = 0.0f;
  zmin = 0.0f;
  zmax = 1.0f;
  profile_length = 1.0f;
  profshift = 0.0f;
  sratio = 1.0f;
  scale = 1;
  reversed = false;
  resetControls ();

  iratio = 1.0f; // (1/csize)
  structImage = new ASImage (size ());
}


ILSDLongProfileItem::~ILSDLongProfileItem ()
{
  delete structImage;
}


ASCanvasPos ILSDLongProfileItem::size () const
{
  return ASCanvasPos (w_width + 6, w_height + 6);
}


void ILSDLongProfileItem::buildProfile (Pt2i pt1, Pt2i pt2)
{
  p1.set (pt1);
  p2.set (pt2);
  int scanx = p2.x () - p1.x ();
  int scany = p2.y () - p1.y ();
  drawable = (scanx * scanx + scany * scany >= MIN_SCAN * MIN_SCAN);
}


void ILSDLongProfileItem::rebuildProfile ()
{
  buildProfile (p1, p2);
}


void ILSDLongProfileItem::setScale ()
{
  // Find metric to window ratio
  float rwidth = alti_area_width / profile_length;
  float rheight = (float) (alti_area_height - 1);
  if (zmax > zmin + 1.0f) rheight /= (zmax - zmin);
  sratio = (rwidth < rheight ? rwidth : rheight);

  // Adapt scale reference to window
  if (scale == 0) scale = 1;  // security
  int meas = scale;
  while (meas > 6) meas /= 10;
  bool pair = (meas == 1);
  while (alti_area_width > 10 * scale * sratio)
  {
    scale *= (pair ? 5 : 2);
    pair = ! pair;
  }
  while (scale > 1 && 9 * alti_area_width < 10 * scale * sratio)
  {
    scale /= (pair ? 2 : 5);
    pair = ! pair;
  }
}


void ILSDLongProfileItem::setData (ASImage* image, IPtTileSet* pdata)
{
  imageWidth = image->width ();
  imageHeight = image->height ();
  ptset = pdata;
  iratio = imageWidth / ptset->xmSpread ();  // (1/csize)
}


void ILSDLongProfileItem::resetControls ()
{
  w_width = ctrl->longViewWidth ();
  alti_area_width = w_width - level_width
                    - alti_left_margin - alti_right_margin;
  w_height = ctrl->longViewHeight ();
  alti_area_height = w_height - scale_height
                     - alti_top_margin - alti_bottom_margin;
  setScale ();
}


void ILSDLongProfileItem::paintProfile ()
{
  ASPainter painter (structImage);

  // Draws metric reference
  int scalength = (int) (scale * sratio / 2 + 0.5f);
  painter.setPen (ASPen (ASColor::BLACK, 2));
  painter.drawLine (alti_area_width / 2 - scalength, w_height - 2,
                    alti_area_width / 2 + scalength, w_height - 2);
  painter.drawLine (alti_area_width / 2 - scalength, w_height - 2,
                    alti_area_width / 2 - scalength, w_height - 6);
  painter.drawLine (alti_area_width / 2 + scalength, w_height - 2,
                    alti_area_width / 2 + scalength, w_height - 6);

  // Draws middle altitude line
  painter.setPen (ASPen (ASColor::GRAY, 2));
  int mh = w_height - alti_bottom_margin - scale_height
                     - (int) ((zmax - zmin) * sratio / 2 + 0.5f);
  painter.drawLine (alti_left_margin, mh, w_width - alti_right_margin, mh);

  // Draws start scan
  painter.setPen (ASPen (ASColor::GREEN, 3));
  float xpos = profileIndexPosition (0);
  int ipos = alti_left_margin + level_width + (int) (xpos * sratio + 0.5f);
  if (ipos > 0 && ipos < w_width - 1)
    painter.drawLine (ipos, mh - index_length, ipos, mh + index_length);
  
  // Draws current scan
  painter.setPen (ASPen (ASColor::BLACK, 3));
  xpos = profileIndexPosition (ctrl->scan ());
  ipos = alti_left_margin + level_width + (int) (xpos * sratio + 0.5f);
  if (ipos > 0 && ipos < w_width - 1)
  {
    painter.drawLine (ipos, mh - index_length, ipos, mh - index_length / 3);
    painter.drawLine (ipos, mh + index_length / 3, ipos, mh + index_length);
  }

  // Draws current measure
  if (ctrl->isMeasuring () && ctrl->isSetMeasure ())
  {
    painter.setPen (ASPen (ASColor::BLUE, 3));
    xpos = profileIndexPosition (ctrl->measureIntervalStart ());
    ipos = alti_left_margin + level_width + (int) (xpos * sratio + 0.5f);
    if (ipos > 0 && ipos < w_width - 1)
    {
      painter.drawLine (ipos, mh - index_length, ipos, mh - index_length / 3);
      painter.drawLine (ipos, mh + index_length / 3, ipos, mh + index_length);
    }
    xpos = profileIndexPosition (ctrl->measureIntervalStop ());
    ipos = alti_left_margin + level_width + (int) (xpos * sratio + 0.5f);
    if (ipos > 0 && ipos < w_width - 1)
    {
      painter.drawLine (ipos, mh - index_length, ipos, mh - index_length / 3);
      painter.drawLine (ipos, mh + index_length / 3, ipos, mh + index_length);
    }
  }

  // Draws profile points
  painter.setPen (ASPen (ASColor::BLACK, ctrl->pointSize ()));
  std::vector<Pt2f>::iterator it = profile.begin ();
  while (it != profile.end ())
  {
    painter.drawPoint (
        alti_left_margin + level_width + (int) (it->x () * sratio + 0.5f),
        w_height - alti_bottom_margin - scale_height
                  - (int) ((it->y () - zmin) * sratio + 0.5f));
    it ++;
  }

  // Draws frame
//  painter.setPen (ASPen (ASColor::GREEN, 1));
//  painter.drawRect (alti_left_margin + level_width, alti_top_margin,
//                    alti_area_width, alti_area_height);
}


void ILSDLongProfileItem::paintInfo ()
{
  if (! drawable) return;
  ASPainter painter (structImage);

  /** Text of the metric reference */
  std::string valid = to_string (scale) + std::string (" m");
  painter.drawText (alti_area_width / 2 - 20, w_height - 27, valid);

  /** Text of middle altitude */
  int mh = w_height - scale_height - alti_bottom_margin
                     - (int) ((zmax - zmin) * sratio / 2 + 0.5f);
  valid = format ((zmin + zmax) / 2) + std::string (" m");
  painter.drawText (alti_area_margin, mh - 24, valid);
}


float ILSDLongProfileItem::profileIndexPosition (int num)
{
  int ind = 0, i = 0;
  bool between = false;
  float pos = 0.0f;
  bool search = true;
  std::vector<int>::iterator it = index.begin ();
  if (num < *it)
  {
    search = false;
    between = true;
    pos = (num - *it) / (float) (*(it+1) - *it);
  }
  else if (num > *it++)
  {
    i ++;
    while (search && it != index.end ())
    {
      if (num == *it)
      {
        ind = i;
        search = false;
      }
      else if (num < *it)
      {
        ind = i - 1;
        between = true;
        pos = (num - *(it-1)) / (float) (*it - *(it-1));
        search = false;
      }
      i ++;
      it ++;
    }
    if (search)
    {
      it --;
      between = true;
      ind = i - 2; 
      pos = (num - *(it-1)) / (float) (*it - *(it-1));
    }
  }
  float val = profile[ind].x ();
  if (between) val += pos * (profile[ind+1].x () - profile[ind]. x());
  return val;
}


void ILSDLongProfileItem::clipLine (ASPainter &painter,
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


std::string ILSDLongProfileItem::format (float val) const
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
