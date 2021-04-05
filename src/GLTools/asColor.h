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

#ifndef AS_COLOR_H
#define AS_COLOR_H

#include <stdint.h>
#include <string>

using namespace std;

/**
 * @brief Replace QtColor
 * handle rgba color
*/
struct ASColor
{
	/**
	 * @brief color mask used to convert from 4 single values to uint32 format
	*/
	static const uint32_t redMask = 0x000000FF; // Red mask
	static const uint32_t greenMask = 0x0000FF00; // Green mask
	static const uint32_t blueMask = 0x00FF0000; // Blue mask
	static const uint32_t alphaMask = 0xFF000000; // Alpha mask

	/**
	 * @brief default constant colors
	*/
	static const ASColor WHITE; // White
	static const ASColor GRAY; // Gray
	static const ASColor BLACK; // Black
	static const ASColor RED; // Red
	static const ASColor BLUE; // Blue
	static const ASColor GREEN; // Green
	static const ASColor YELLOW; // Yellow
	static const ASColor ALPHA; // Transparent

	/**
	 * @brief Color values (4 channels)
	*/
	uint8_t r, g, b, a;

	/**
	 * @brief Create empty color
	*/
	ASColor();

	/**
	 * @brief Create color from rgba values (4 uint8)
	*/
	ASColor(uint8_t iR, uint8_t iG, uint8_t iB, uint8_t iA);

	/**
	 * @brief Create color from uint32 value (4 channels merged into uint32)
	*/
	ASColor(const uint32_t& colorMask);

	/**
	 * @briefMake color from linear color values (float : 0.f - 1.f)
	*/
	inline static ASColor linearColor(float r, float g, float b, float a) {
		return ASColor(uint8_t(r * 255), uint8_t(g * 255), uint8_t(b * 255), uint8_t(a * 256));
	}

	/**
	 * @brief == operator
	*/
	bool operator==(const ASColor& other)
	{
		return
			r == other.r &&
			g == other.g &&
			b == other.b &&
			a == other.a;
	}

	/**
	 * @brief != operator
	*/
	bool operator!=(const ASColor& other)
	{
		return !(*this == other);
	}

	/**
	 * @brief output color into int32 format
	*/
	uint32_t asInt() const;

	/**
	 * @brief color as (r,g,b,a) string
	*/
	string toString() const;

	/**
	 * @brief ensure that color class work properly
	*/
	static bool testColor();
};
#endif
