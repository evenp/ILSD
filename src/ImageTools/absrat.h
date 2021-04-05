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

#ifndef ABSOLUTE_RATIONAL_H
#define ABSOLUTE_RATIONAL_H


/** 
 * @class AbsRat absrat.h
 * \brief Absolutely useless rational type.
 * 
 * This type is only maintained to ensure the compatibility of
 *   DigitalStraightSegment::naiveLine function used
 *   for IpolDemo and AMREL (seeds selection).
 * Better not use it anymore.
 */
class AbsRat
{
public:

  /**
   * \brief Creates an rational number with null value.
   */
  AbsRat () : r_num (0), r_den (1) { }

  /**
   * \brief Sets a value for numerator and denominator.
   * @param numerator Numerator value.
   * @param denominator Denominator value.
   */
  inline void set (int numerator, int denominator) {
    r_num = numerator; r_den = denominator; }

  /**
   * \brief Sets a integer value for the rational number.
   * @param val Integer value.
   */
  inline void set (int val) { r_num = val; r_den = 1; }

  /**
   * \brief Deletes the rational number.
   */
  ~AbsRat () { }

  /**
   * \brief Returns the numerator of the rational number.
   */
  inline int numerator () const { return r_num; }

  /**
   * \brief Returns the denominator of the rational number.
   */
  inline int denominator () const { return r_den; }

  /**
   * \brief Returns the numerator of the rational number.
   */
  inline int num () const { return r_num; }

  /**
   * \brief Returns the denominator of the rational number.
   */
  inline int den () const { return r_den; }


protected:

  /** Numerator of the rational number. */
  int r_num;
  /** Denominator of the rational number. */
  int r_den;

};
#endif
