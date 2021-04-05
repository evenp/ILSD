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

#ifndef AS_PAINTER_H
#define AS_PAINTER_H

#include <string>
#include "asCanvasPos.h"
#include "asBrush.h"
#include "asPen.h"

class ASImage;

using namespace std;

/**
 * @brief Replace QtPainter. Used to draw primitives on ASImage, driven by ASPen or ASBrush.
 * All draw operations are safe, painting 'outside' the image is allowed.
*/
class ASPainter
{
public:

	/**
	 * @brief Create a painter who will use a given image to draw primitives on it
	*/
	ASPainter(ASImage* inImage);

	/**
	 * @brief set used pen
	*/
	inline void setPen(const ASPen& inPen) { usedPen = inPen; }

	/**
	 * @brief set used brush
	*/
	inline void setBrush(const ASBrush& inBrush) { usedBrush = inBrush; }

	/**
	 * @brief draw point on current image
	*/
	void drawPoint(const unsigned int& posX, const unsigned int& posY);

	/**
	 * @brief draw point on current image
	*/
	void drawPoint(const ASCanvasPos& pos) { drawPoint(pos.x, pos.y); }

	/**
	 * @brief draw line on current image from A to B
	*/
	void drawLine(const unsigned int& posAX, const unsigned int& posAY, const unsigned int& posBX, const unsigned int& posBY);

	/**
	 * @brief draw line on current image from A to B
	*/
	void drawLine(const ASCanvasPos& posA, const ASCanvasPos& posB) { drawLine(posA.x, posA.y, posB.x, posB.y); }

	/**
	 * @brief draw custom text on image (font = 12px)
	 * Note: alway call it in the imgui draw pass
	*/
	void drawText(int offsetX, int offsetY, string text);

private:

	/**
	 * @brief used pen
	*/
	ASPen usedPen;

	/**
	 * @brief used brush
	*/
	ASBrush usedBrush;

	/**
	 * @brief Image we are drawing on
	*/
	ASImage* referencedImage;

public:
	/**
	 * @brief draw a rectangle from (posX, posY) to (posX + sizeX, posY + sizeY)
	*/
	void drawRect(int posX, int posy, int sizeX, int sizeY);
	/**
	 * @brief draw a filled rectangle from (posX, posY) to (posX + sizeX, posY + sizeY)
	*/
	void fillRect(int posX, int posy, int sizeX, int sizeY);
	/**
	 * @brief draw a filled rectangle from (posX, posY) to (posX + sizeX, posY + sizeY) using custom brush
	*/
	void fillRect(int posX, int posy, int sizeX, int sizeY, const ASBrush& brush);
	/**
	 * @brief draw a filled rectangle from (posX, posY) to (posX + sizeX, posY + sizeY) using custom color
	*/
	void fillRect(int posX, int posy, int sizeX, int sizeY, const ASColor& fillColor);
};
#endif
