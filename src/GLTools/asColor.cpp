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

#include "asColor.h"
#include <iostream>

const ASColor ASColor::WHITE = ASColor(255, 255, 255, 255);
const ASColor ASColor::GRAY = ASColor(127, 127, 127, 255);
const ASColor ASColor::BLACK = ASColor(0, 0, 0, 255);
const ASColor ASColor::RED = ASColor(255, 0, 0, 255);
const ASColor ASColor::GREEN = ASColor(0, 255, 0, 255);
const ASColor ASColor::YELLOW = ASColor(255, 255, 0, 255);
const ASColor ASColor::BLUE = ASColor(0, 0, 255, 255);
const ASColor ASColor::ALPHA = ASColor(0, 0, 0, 0);

ASColor::ASColor()
	: r(0), g(0), b(0), a(255) {}

ASColor::ASColor(uint8_t iR, uint8_t iG, uint8_t iB, uint8_t iA) :
	r(iR),
	g(iG),
	b(iB),
	a(iA)
{ }

ASColor::ASColor(const uint32_t& colorMask) :
	r((colorMask& redMask)),
	g((colorMask& greenMask) >> 8),
	b((colorMask& blueMask) >> 16),
	a((colorMask& alphaMask) >> 24)
{ }

uint32_t ASColor::asInt() const
{
	return r + (g << 8) + (b << 16) + (a << 24);
}

string ASColor::toString() const
{
	return string(
		"(" +
		to_string(r) +
		", " +
		to_string(g) +
		", " +
		to_string(b) +
		", " +
		to_string(a) +
		")"
	);
}

bool ASColor::testColor()
{
	bool bFailed = false;

	for (int r = 0; r < 256; r += 7)
	{
		for (int g = 0; g < 256; g += 6)
		{
			for (int b = 0; b < 256; b += 8)
			{
				for (int a = 0; a < 256; a += 9)
				{
					ASColor col(r, g, b, a);
					int colorInt = r + g * 256 + b * 256 * 256 + a * 256 * 256;
					if (col.r != r || col.g != g || col.b != b || col.a != a) bFailed = true;
					if (ASColor(colorInt).asInt() != colorInt) bFailed = true;
					if (ASColor::linearColor((float)r / 255.f, (float)g / 255.f, (float)b / 255.f, (float)a / 255.f) != col) bFailed = true;
					if (bFailed) break;
				}
			}
		}
	}

	cout << "color test " << (bFailed ? "failed" : "succeed") << endl;
	return !bFailed;
}
