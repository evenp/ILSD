/*  Copyright 2021 Pierre Even,
      co-author of paper:
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

#ifndef AS_PEN_H
#define AS_PEN_H

#include "asColor.h"

enum class ASPenStyle
{
	SolidLine = 0
};

enum class ASPenCapStyle
{
	RoundCap = 0
};

enum class ASPenJoinStyle
{
	RoundJoin = 0
};

/**
 * @brief Replace QtPen. (pen width is partialy suported)
*/
struct ASPen
{
	ASPen();

	ASPen(const ASColor& inPenColor);

	ASPen(const ASColor& inPenColor, const unsigned int& inPenWidth);

	ASPen(const ASColor& inPenColor, const unsigned int& inPenWidth, const ASPenStyle& inPenStyle, const ASPenCapStyle& inPenCapStyle, const ASPenJoinStyle& inPenJoinStyle);

	ASColor penColor;

	inline void setPenWidth(int inPenWidth) { penWidth = inPenWidth <= 0 ? 1 : inPenWidth; }

	inline int getPenWidth() { return penWidth; }

private:

	unsigned int penWidth = 1;
	ASPenStyle penStyle = ASPenStyle::SolidLine;
	ASPenCapStyle penCapStyle = ASPenCapStyle::RoundCap;
	ASPenJoinStyle penJoinStyle = ASPenJoinStyle::RoundJoin;
};
#endif