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

#include "ilsditemcontrol.h"
#include <iostream>
#include <fstream>


const int ILSDItemControl::DEFAULT_STRAIGHT_STRIP_WIDTH = 100;
const int ILSDItemControl::MIN_STRAIGHT_STRIP_WIDTH = 20;
const int ILSDItemControl::MAX_STRAIGHT_STRIP_WIDTH = 500;
const int ILSDItemControl::DEFAULT_PROF_WIDTH = 400;
const int ILSDItemControl::MIN_PROF_WIDTH = 300;
const int ILSDItemControl::MAX_PROF_WIDTH = 800;
const int ILSDItemControl::DEFAULT_SCAN_WIDTH = 200;
const int ILSDItemControl::MIN_SCAN_WIDTH = 100;
const int ILSDItemControl::MAX_SCAN_WIDTH = 400;
const int ILSDItemControl::DEFAULT_VIEW_HEIGHT = 600;
const int ILSDItemControl::MIN_VIEW_HEIGHT = 400;
const int ILSDItemControl::MAX_VIEW_HEIGHT = 800;
const int ILSDItemControl::DEFAULT_LONG_VIEW_WIDTH = 800;
const int ILSDItemControl::DEFAULT_LONG_VIEW_HEIGHT = 200;
const int ILSDItemControl::MIN_LONG_VIEW_WIDTH = 200;
const int ILSDItemControl::MAX_LONG_VIEW_WIDTH = 1600;
const int ILSDItemControl::MIN_LONG_VIEW_HEIGHT = 100;
const int ILSDItemControl::MAX_LONG_VIEW_HEIGHT = 1000;
const int ILSDItemControl::SIZE_INC = 5;
const int ILSDItemControl::MAX_ZRATIO = 20;
const int ILSDItemControl::DEFAULT_POINT_SIZE = 2;
const int ILSDItemControl::DEFAULT_SCAN_RESOL = 4;
const int ILSDItemControl::PROF_SHIFT_INC = 10;
const float ILSDItemControl::MAX_LINE_TRANSLATION_RATIO = 0.8f;
const float ILSDItemControl::MAX_LINE_ROTATION_RATIO = 0.95f;
const float ILSDItemControl::RATIO_INC = 0.01f;



ILSDItemControl::ILSDItemControl ()
{
  straight_strip_width = DEFAULT_STRAIGHT_STRIP_WIDTH;
  prof_width = DEFAULT_PROF_WIDTH;
  scan_width = DEFAULT_SCAN_WIDTH;
  view_height = DEFAULT_VIEW_HEIGHT;
  long_view_width = DEFAULT_LONG_VIEW_WIDTH;
  long_view_height = DEFAULT_LONG_VIEW_HEIGHT;
  scan_resol = DEFAULT_SCAN_RESOL;
  point_size = DEFAULT_POINT_SIZE;
  aligned = false;
  initial_detection = false;
  static_height = true;
  display_template = true;
  display_reference = false;
  display_alti = true;
  display_estimation = false;
  display_det = true;
  display_direction = false;
  display_prediction = false;
  display_legend = false;
  z_ratio = 1;
  profile_shift = 0;
  measuring = false;
  thin_long_strip = true;
  min_scan = 0;
  max_scan = 0;
  cur_scan = 0;
  mstart_on = false;
  mstart = 0;
  mstop_on = false;
  mstop = 0;
  decimal_resolution = 3;
}


void ILSDItemControl::incStraightStripWidth (int inc)
{
  setStraightStripWidth (straight_strip_width + inc * SIZE_INC);
}

void ILSDItemControl::setStraightStripWidth (int val)
{
  straight_strip_width = val;
  if (straight_strip_width > MAX_STRAIGHT_STRIP_WIDTH)
    straight_strip_width = MAX_STRAIGHT_STRIP_WIDTH;
  else if (straight_strip_width < MIN_STRAIGHT_STRIP_WIDTH)
    straight_strip_width = MIN_STRAIGHT_STRIP_WIDTH;
}


void ILSDItemControl::incProfileWidth (int inc)
{
  setProfileWidth (prof_width + inc * SIZE_INC);
}

void ILSDItemControl::setProfileWidth (int val)
{
  prof_width = val;
  if (prof_width > MAX_PROF_WIDTH) prof_width = MAX_PROF_WIDTH;
  else if (prof_width < MIN_PROF_WIDTH) prof_width = MIN_PROF_WIDTH;
}


void ILSDItemControl::incScanWidth (int inc)
{
  setScanWidth (scan_width + inc * SIZE_INC);
}

void ILSDItemControl::setScanWidth (int val)
{
  scan_width = val;
  if (scan_width > MAX_SCAN_WIDTH) scan_width = MAX_SCAN_WIDTH;
  else if (scan_width < MIN_SCAN_WIDTH) scan_width = MIN_SCAN_WIDTH;
}


void ILSDItemControl::incViewHeight (int inc)
{
  setViewHeight (view_height + inc * SIZE_INC);
}

void ILSDItemControl::setViewHeight (int val)
{
  view_height = val;
  if (view_height > MAX_VIEW_HEIGHT) view_height = MAX_VIEW_HEIGHT;
  else if (view_height < MIN_VIEW_HEIGHT) view_height = MIN_VIEW_HEIGHT;
}


void ILSDItemControl::incLongViewWidth (int inc)
{
  setLongViewWidth (long_view_width + inc * SIZE_INC);
}

void ILSDItemControl::setLongViewWidth (int val)
{
  long_view_width = val;
  if (long_view_width > MAX_LONG_VIEW_WIDTH)
    long_view_width = MAX_LONG_VIEW_WIDTH;
  else if (long_view_width < MIN_LONG_VIEW_WIDTH)
    long_view_width = MIN_LONG_VIEW_WIDTH;
}


void ILSDItemControl::incLongViewHeight (int inc)
{
  setLongViewHeight (long_view_height + inc * SIZE_INC);
}

void ILSDItemControl::setLongViewHeight (int val)
{
  long_view_height = val;
  if (long_view_height > MAX_LONG_VIEW_HEIGHT)
    long_view_height = MAX_LONG_VIEW_HEIGHT;
  else if (long_view_height < MIN_LONG_VIEW_HEIGHT)
    long_view_height = MIN_LONG_VIEW_HEIGHT;
}


int ILSDItemControl::scanResolutionLevel () const
{
  int level = 0;
  int sr = scan_resol;
  while (sr > 1)
  {
    sr /= 2;
    level ++;
  }
  return (level);
}

void ILSDItemControl::incScanResolution (int inc)
{
  if (inc > 0)
  {
    if (scan_resol < 16) scan_resol *= 2;
  }
  else
  {
    if (scan_resol > 1) scan_resol /= 2;
  }
}

void ILSDItemControl::setScanResolution (int val)
{
  if (val >= 0 && val <= 4)
  {
    scan_resol = 1;
    while (val --) scan_resol *= 2;
  }
}


void ILSDItemControl::incZRatio (int inc)
{
  int div = z_ratio;
  while (div >= 10) div /= 10;
  if (inc > 0)
  {
    if (div == 2) setZRatio (z_ratio * 5);
    else setZRatio (z_ratio * 2);
  }
  else
  {
    if (div == 2) setZRatio (z_ratio / 2);
    else setZRatio (z_ratio / 5);
  }
}


void ILSDItemControl::setZRatio (int val)
{
  int div = val, dec = 0, rem = 0;
  while (div >= 10)
  {
    rem = div;
    div /= 10;
    rem -= div * 10;
    dec ++;
  }
  if (div >= 8)
  {
    div = 1;
    dec ++;
  }
  else if (div >= 4) div = 5;
  else if (div == 3) div = 2;
  else if (div == 1 && rem > 5) div = 2;
  z_ratio = div;
  while (dec-- != 0) z_ratio *= 10;
  if (z_ratio <= 0) z_ratio = 1;
  if (z_ratio > MAX_ZRATIO) z_ratio = MAX_ZRATIO;
}


void ILSDItemControl::incPointSize (int inc)
{
  setPointSize (point_size + inc);
}

void ILSDItemControl::setPointSize (int val)
{
  point_size = val;
  if (point_size <= 0) point_size = 1;
  else if (point_size > 7) point_size = 7;
}


void ILSDItemControl::setMinScan (int val)
{
  min_scan = val;
  if (cur_scan < min_scan) cur_scan = 0;
}

void ILSDItemControl::setMaxScan (int val)
{
  max_scan = val;
  if (cur_scan > max_scan) cur_scan = 0;
}

void ILSDItemControl::incScan (int inc)
{
  if (cur_scan + inc <= max_scan && cur_scan + inc >= min_scan) cur_scan += inc;
}

void ILSDItemControl::setScan (int val)
{
  cur_scan = (val <= max_scan && val >= min_scan ? val : 0);
}

void ILSDItemControl::resetScan ()
{
  min_scan = 0;
  max_scan = 0;
  cur_scan = 0;
}


bool ILSDItemControl::isCurrentScanMeasured () const
{
  if (mstart_on)
    if (mstop_on)
      if (mstart > mstop) return (cur_scan <= mstart && cur_scan >= mstop);
      else return (cur_scan <= mstop && cur_scan >= mstart);
    else return true;
  else return mstop_on;
}


void ILSDItemControl::incDecimalResolution (int inc)
{
  if (inc > 0) decimal_resolution ++;
  else if (decimal_resolution > 1) decimal_resolution --;
}
