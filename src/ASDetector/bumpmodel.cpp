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

#include "bumpmodel.h"

const int BumpModel::MIN_PTS = 6;
const float BumpModel::HOLE_MAX_RELATIVE_LENGTH = 0.6f;

const float BumpModel::DEFAULT_MIN_WIDTH = 1.0f;
const float BumpModel::MIN_MIN_WIDTH = 0.1f;
const float BumpModel::MAX_MIN_WIDTH = 10.0f;
const float BumpModel::DEFAULT_MIN_HEIGHT = 0.4f;
const float BumpModel::MIN_MIN_HEIGHT = 0.1f;
const float BumpModel::MAX_MIN_HEIGHT = 10.0f;
const float BumpModel::MAX_HEIGHT_RATIO = 0.9f;

const float BumpModel::DEFAULT_POS_SHIFT_TOLERANCE = 1.1f;
const float BumpModel::DEFAULT_ALTI_SHIFT_TOLERANCE = 0.1f;
const float BumpModel::DEFAULT_WIDTH_SHIFT_TOLERANCE = 2.0f;
const float BumpModel::DEFAULT_HEIGHT_SHIFT_TOLERANCE = 0.1f;
const float BumpModel::DEFAULT_POS_REL_SHIFT_TOLERANCE = 0.16f;
const float BumpModel::DEFAULT_ALTI_REL_SHIFT_TOLERANCE = 0.125f;
const float BumpModel::DEFAULT_WIDTH_REL_SHIFT_TOLERANCE = 0.26f;
const float BumpModel::DEFAULT_HEIGHT_REL_SHIFT_TOLERANCE = 0.05f;

const float BumpModel::MIN_SHIFT = 0.05f;
const float BumpModel::MAX_SHIFT = 5.00f;
const float BumpModel::MIN_REL_SHIFT = 0.05f;
const float BumpModel::MAX_REL_SHIFT = 1.0f;
const float BumpModel::LENGTH_INCREMENT = 0.05f;
const float BumpModel::POS_INCREMENT = 0.05f;
const float BumpModel::HEIGHT_INCREMENT = 0.05f;
const float BumpModel::WIDTH_INCREMENT = 0.05f;
const float BumpModel::REL_INCREMENT = 0.05f;
const float BumpModel::HEIGHT_RATIO_INCREMENT = 0.01f;
const float BumpModel::UNDETECTED_BOUND_DISTANCE = 6.0f;

const int BumpModel::TREND_PERSISTENCE = 6;
const int BumpModel::DEFAULT_TREND_MIN_PINCH = 136;
const int BumpModel::MIN_TREND_MIN_PINCH = 50;
const int BumpModel::MAX_TREND_MIN_PINCH = 1000;
const int BumpModel::THICKNESS_INCREMENT = 50;



BumpModel::BumpModel ()
{
  over = true;
  mass_ref = false;
  position_control = 1;
  altitude_control = 1;
  width_control = 2;
  height_control = 2;
  with_trend = false;
  trend_min_pinch = DEFAULT_TREND_MIN_PINCH;
  deviation_prediction_on = true;
  slope_prediction_on = true;
  min_width = DEFAULT_MIN_WIDTH;
  min_height = DEFAULT_MIN_HEIGHT;
  pos_tolerance = DEFAULT_POS_SHIFT_TOLERANCE;
  alti_tolerance = DEFAULT_ALTI_SHIFT_TOLERANCE;
  width_tolerance = DEFAULT_WIDTH_SHIFT_TOLERANCE;
  height_tolerance = DEFAULT_HEIGHT_SHIFT_TOLERANCE;
  pos_rel_tolerance = DEFAULT_POS_REL_SHIFT_TOLERANCE;
  alti_rel_tolerance = DEFAULT_ALTI_REL_SHIFT_TOLERANCE;
  width_rel_tolerance = DEFAULT_WIDTH_REL_SHIFT_TOLERANCE;
  height_rel_tolerance = DEFAULT_HEIGHT_REL_SHIFT_TOLERANCE;
  measures_req = false;
}


BumpModel::~BumpModel ()
{
}


void BumpModel::incMinWidth (int dir)
{
  setMinWidth (min_width + dir * LENGTH_INCREMENT);
}

void BumpModel::setMinWidth (float val)
{
  min_width = val;
  if (min_width < MIN_MIN_WIDTH) min_width = MIN_MIN_WIDTH;
  else if (min_width > MAX_MIN_WIDTH) min_width = MAX_MIN_WIDTH;
}

void BumpModel::incMinHeight (int dir)
{
  setMinHeight (min_height + dir * HEIGHT_INCREMENT);
}

void BumpModel::setMinHeight (float val)
{
  min_height = val;
  if (min_height < MIN_MIN_HEIGHT) min_height = MIN_MIN_HEIGHT;
  else if (min_height > MAX_MIN_HEIGHT) min_height = MAX_MIN_HEIGHT;
}


void BumpModel::incPositionShiftTolerance (int dir)
{
  setPositionShiftTolerance (pos_tolerance + dir * POS_INCREMENT);
}

void BumpModel::setPositionShiftTolerance (float val)
{
  pos_tolerance = val;
  if (pos_tolerance < MIN_SHIFT) pos_tolerance = MIN_SHIFT;
  if (pos_tolerance > MAX_SHIFT) pos_tolerance = MAX_SHIFT;
}

void BumpModel::incAltitudeShiftTolerance (int dir)
{
  setAltitudeShiftTolerance (alti_tolerance + dir * HEIGHT_INCREMENT);
}

void BumpModel::setAltitudeShiftTolerance (float val)
{
  alti_tolerance = val;
  if (alti_tolerance < MIN_SHIFT) alti_tolerance = MIN_SHIFT;
  if (alti_tolerance > MAX_SHIFT) alti_tolerance = MAX_SHIFT;
}

void BumpModel::incWidthShiftTolerance (int dir)
{
  setWidthShiftTolerance (width_tolerance + dir * WIDTH_INCREMENT);
}

void BumpModel::setWidthShiftTolerance (float val)
{
  width_tolerance = val;
  if (width_tolerance < MIN_SHIFT) width_tolerance = MIN_SHIFT;
  if (width_tolerance > MAX_SHIFT) width_tolerance = MAX_SHIFT;
}

void BumpModel::incHeightShiftTolerance (int dir)
{
  setHeightShiftTolerance (height_tolerance + dir * HEIGHT_INCREMENT);
}

void BumpModel::setHeightShiftTolerance (float val)
{
  height_tolerance = val;
  if (height_tolerance < MIN_SHIFT) height_tolerance = MIN_SHIFT;
  if (height_tolerance > MAX_SHIFT) height_tolerance = MAX_SHIFT;
}


void BumpModel::incPositionRelShiftTolerance (int dir)
{
  setPositionRelShiftTolerance (pos_rel_tolerance + dir * REL_INCREMENT);
}

void BumpModel::setPositionRelShiftTolerance (float val)
{
  pos_rel_tolerance = val;
  if (pos_rel_tolerance < MIN_REL_SHIFT) pos_rel_tolerance = MIN_REL_SHIFT;
  if (pos_rel_tolerance > MAX_REL_SHIFT) pos_rel_tolerance = MAX_REL_SHIFT;
}

void BumpModel::incAltitudeRelShiftTolerance (int dir)
{
  setAltitudeRelShiftTolerance (alti_rel_tolerance + dir * REL_INCREMENT);
}

void BumpModel::setAltitudeRelShiftTolerance (float val)
{
  alti_rel_tolerance = val;
  if (alti_rel_tolerance < MIN_REL_SHIFT) alti_rel_tolerance = MIN_REL_SHIFT;
  if (alti_rel_tolerance > MAX_REL_SHIFT) alti_rel_tolerance = MAX_REL_SHIFT;
}

void BumpModel::incWidthRelShiftTolerance (int dir)
{
  setWidthRelShiftTolerance (width_rel_tolerance + dir * REL_INCREMENT);
}

void BumpModel::setWidthRelShiftTolerance (float val)
{
  width_rel_tolerance = val;
  if (width_rel_tolerance < MIN_REL_SHIFT) width_rel_tolerance = MIN_REL_SHIFT;
  if (width_rel_tolerance > MAX_REL_SHIFT) width_rel_tolerance = MAX_REL_SHIFT;
}

void BumpModel::incHeightRelShiftTolerance (int dir)
{
  setHeightRelShiftTolerance (height_rel_tolerance + dir * REL_INCREMENT);
}

void BumpModel::setHeightRelShiftTolerance (float val)
{
  height_rel_tolerance = val;
  if (height_rel_tolerance < MIN_REL_SHIFT)
    height_rel_tolerance = MIN_REL_SHIFT;
  if (height_rel_tolerance > MAX_REL_SHIFT)
    height_rel_tolerance = MAX_REL_SHIFT;
}


void BumpModel::incTrendMinPinch (int dir)
{
  setTrendMinPinch (trend_min_pinch + dir * THICKNESS_INCREMENT);
}

void BumpModel::setTrendMinPinch (int val)
{
  trend_min_pinch = val;
  if (trend_min_pinch < MIN_TREND_MIN_PINCH)
    trend_min_pinch = MIN_TREND_MIN_PINCH;
  else if (trend_min_pinch > MAX_TREND_MIN_PINCH)
    trend_min_pinch = MAX_TREND_MIN_PINCH;
}


void BumpModel::switchDeviationPrediction ()
{
  deviation_prediction_on = ! deviation_prediction_on;
}

void BumpModel::switchSlopePrediction ()
{
  slope_prediction_on = ! slope_prediction_on;
}
