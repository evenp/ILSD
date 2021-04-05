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

#ifndef AS_CANVASPOS_H
#define AS_CANVASPOS_H

#include <string>

using namespace std;

/**
 * @brief Represent 2D canvas position
 * cannot be negative
*/
struct ASCanvasPos
{
	/**
	 * @brief 2D coordinates
	*/
	uint32_t x, y;

	/**
	 * @brief Create position (0,0)
	*/
	ASCanvasPos()
		: x(0), y(0) {}

	/**
	 * @brief Create position from single value (x = value, y = value)
	*/
	ASCanvasPos(const unsigned int& xy)
		: x(xy), y(xy) {}

	/**
	 * @brief Create position from given coords
	*/
	ASCanvasPos(const unsigned int& x, const unsigned int y)
		: x(x), y(y) {}

	/**
	 * @brief get position as string "(x,y)"
	*/
	string toString() const;

	/**
	 * @brief == operator
	*/
	inline bool operator==(const ASCanvasPos& other) const
	{
		return x == other.x && y == other.y;
	}

	/**
	 * @brief != operator
	*/
	inline bool operator!=(const ASCanvasPos& other) const
	{
		return x != other.x || y != other.y;
	}

	/**
	 * @brief Sets position from given position
	*/
	inline void set(const int& px, const int &py) { x = px; y = py; }
};
#endif
