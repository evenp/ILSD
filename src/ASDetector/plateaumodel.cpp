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

#include "plateaumodel.h"

const int PlateauModel::PLATEAU_MIN_PTS = 6;
const float PlateauModel::PLATEAU_CRITICAL_LENGTH = 1.0f;
const float PlateauModel::PLATEAU_START_LENGTH = 3.0f;

const float PlateauModel::DEFAULT_MIN_LENGTH = 0.8f;
const float PlateauModel::MIN_MIN_LENGTH_TOLERANCE = 0.1f;
const float PlateauModel::MAX_MIN_LENGTH_TOLERANCE = 5.0f;
const float PlateauModel::DEFAULT_MAX_LENGTH = 6.0f;
const float PlateauModel::MIN_MAX_LENGTH_TOLERANCE = 2.0f;
const float PlateauModel::MAX_MAX_LENGTH_TOLERANCE = 12.0f;

const float PlateauModel::DEFAULT_THICKNESS_TOLERANCE = 0.23f;
const float PlateauModel::MIN_THICKNESS_TOLERANCE = 0.05f;
const float PlateauModel::MAX_THICKNESS_TOLERANCE = 0.5f;
const float PlateauModel::DEFAULT_SLOPE_TOLERANCE = 0.15f;
const float PlateauModel::MIN_SLOPE_TOLERANCE = 0.05f;
const float PlateauModel::MAX_SLOPE_TOLERANCE = 0.75f;
const float PlateauModel::DEFAULT_SIDE_SHIFT_TOLERANCE = 1.3f;
const float PlateauModel::MIN_SIDE_SHIFT_TOLERANCE = 0.05f;
const float PlateauModel::MAX_SIDE_SHIFT_TOLERANCE = 6.0f;

const float PlateauModel::HEIGHT_FLEXIBILITY = 0.10f;
const float PlateauModel::DEFAULT_PLATEAU_WIDTH_MOVE_TOLERANCE = 0.5f;
const float PlateauModel::MIN_WIDTH_MOVE_TOLERANCE = 0.05f;
const float PlateauModel::MAX_WIDTH_MOVE_TOLERANCE = 6.0f;
const float PlateauModel::PLATEAU_BOUND_ACCURACY = 0.5f;
const float PlateauModel::MIN_POS_TOLERANCE = 0.05f;
const float PlateauModel::OPT_HEIGHT_MIN_USE = 0.7f;
const float PlateauModel::DEFAULT_PLATEAU_SEARCH_DISTANCE = 1.0f;

const float PlateauModel::HEIGHT_INCREMENT = 0.05f;
const float PlateauModel::POS_INCREMENT = 0.05f;
const float PlateauModel::LENGTH_INCREMENT = 0.1f;

const int PlateauModel::PLATEAU_MAX_INTERRUPTION = 0;
const int PlateauModel::BS_PINCH_MARGIN = 50;
const int PlateauModel::DEFAULT_BS_MAX_TILT = 14;
const int PlateauModel::DEFAULT_TAIL_MIN_SIZE = 10;



PlateauModel::PlateauModel ()
{
  deviation_prediction_on = false;
  slope_prediction_on = false;
  min_length = DEFAULT_MIN_LENGTH;
  max_length = DEFAULT_MAX_LENGTH;
  thickness_tolerance = DEFAULT_THICKNESS_TOLERANCE;
  slope_tolerance = DEFAULT_SLOPE_TOLERANCE;
  side_shift_tolerance = DEFAULT_SIDE_SHIFT_TOLERANCE;
  width_move_tolerance = DEFAULT_PLATEAU_WIDTH_MOVE_TOLERANCE;
  opt_height_min_use = OPT_HEIGHT_MIN_USE;
  bs_max_tilt = DEFAULT_BS_MAX_TILT;
  tail_min_size = DEFAULT_TAIL_MIN_SIZE;
}


PlateauModel::~PlateauModel ()
{
}


void PlateauModel::incThicknessTolerance (int dir)
{
  setThicknessTolerance (thickness_tolerance + dir * HEIGHT_INCREMENT);
}

void PlateauModel::setThicknessTolerance (float val)
{
  thickness_tolerance = val;
  if (thickness_tolerance < MIN_THICKNESS_TOLERANCE)
    thickness_tolerance = MIN_THICKNESS_TOLERANCE;
  if (thickness_tolerance > MAX_THICKNESS_TOLERANCE)
    thickness_tolerance = MAX_THICKNESS_TOLERANCE;
}


void PlateauModel::incSlopeTolerance (int dir)
{
  setSlopeTolerance (slope_tolerance + dir * HEIGHT_INCREMENT);
}


void PlateauModel::setSlopeTolerance (float val)
{
  slope_tolerance = val;
  if (slope_tolerance < MIN_SLOPE_TOLERANCE)
    slope_tolerance = MIN_SLOPE_TOLERANCE;
  if (slope_tolerance > MAX_SLOPE_TOLERANCE)
    slope_tolerance = MAX_SLOPE_TOLERANCE;
}


void PlateauModel::incMinLength (int dir)
{
  setMinLength (min_length + dir * LENGTH_INCREMENT);
}

void PlateauModel::setMinLength (float val)
{
  min_length = val;
  if (min_length < MIN_MIN_LENGTH_TOLERANCE)
    min_length = MIN_MIN_LENGTH_TOLERANCE;
  if (min_length > MAX_MIN_LENGTH_TOLERANCE)
    min_length = MAX_MIN_LENGTH_TOLERANCE;
  if (min_length > max_length - PLATEAU_CRITICAL_LENGTH)
    min_length = max_length - PLATEAU_CRITICAL_LENGTH;
}


void PlateauModel::incMaxLength (int dir)
{
  setMaxLength (max_length + dir * LENGTH_INCREMENT);
}

void PlateauModel::setMaxLength (float val)
{
  max_length = val;
  if (max_length < MIN_MAX_LENGTH_TOLERANCE)
    max_length = MIN_MAX_LENGTH_TOLERANCE;
  if (max_length > MAX_MAX_LENGTH_TOLERANCE)
    max_length = MAX_MAX_LENGTH_TOLERANCE;
  if (max_length < min_length + PLATEAU_CRITICAL_LENGTH)
    max_length = min_length + PLATEAU_CRITICAL_LENGTH;
}


void PlateauModel::incSideShiftTolerance (int dir)
{
  setSideShiftTolerance (side_shift_tolerance + dir * POS_INCREMENT);
}

void PlateauModel::setSideShiftTolerance (float val)
{
  side_shift_tolerance = val;
  if (side_shift_tolerance < MIN_SIDE_SHIFT_TOLERANCE)
    side_shift_tolerance = MIN_SIDE_SHIFT_TOLERANCE;
  if (side_shift_tolerance > MAX_SIDE_SHIFT_TOLERANCE)
    side_shift_tolerance = MAX_SIDE_SHIFT_TOLERANCE;
}

void PlateauModel::incWidthMoveTolerance (int dir)
{
  width_move_tolerance += dir * POS_INCREMENT;
  if (width_move_tolerance < MIN_POS_TOLERANCE)
    width_move_tolerance = MIN_POS_TOLERANCE;
}

void PlateauModel::setWidthMoveTolerance (float val)
{
  width_move_tolerance = val;
  if (width_move_tolerance < MIN_WIDTH_MOVE_TOLERANCE)
    width_move_tolerance = MIN_WIDTH_MOVE_TOLERANCE;
  if (width_move_tolerance > MAX_WIDTH_MOVE_TOLERANCE)
    width_move_tolerance = MAX_WIDTH_MOVE_TOLERANCE;
}


void PlateauModel::incBSmaxTilt (int dir)
{
  setBSmaxTilt (bs_max_tilt + dir);
}

void PlateauModel::setBSmaxTilt (int val)
{
  bs_max_tilt = val;
  if (bs_max_tilt < 1) bs_max_tilt = 1;
}


void PlateauModel::incTailMinSize (int inc)
{
  setTailMinSize (tail_min_size + inc);
}

void PlateauModel::setTailMinSize (int val)
{
  tail_min_size = val;
  if (tail_min_size < 0) tail_min_size = 0;
}


void PlateauModel::switchDeviationPrediction ()
{
  deviation_prediction_on = ! deviation_prediction_on;
}

void PlateauModel::switchSlopePrediction ()
{
  slope_prediction_on = ! slope_prediction_on;
}
