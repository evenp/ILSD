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

#include "asPainter.h"
#include "asImage.h"
#include "imgui.h"
#include "pt2i.h"
#include <stdexcept>

ASPainter::ASPainter(ASImage* inImage)
	: referencedImage(inImage) {}

void ASPainter::drawPoint(const unsigned int& posX, const unsigned int& posY)
{
	for (float x = 0; x < usedPen.getPenWidth(); ++x)
	{
		for (float y = 0; y < usedPen.getPenWidth(); ++y)
		{
			int drawPosX = (int)(posX + x - usedPen.getPenWidth() / 2);
			int drawPosY = (int)(posY + y - usedPen.getPenWidth() / 2);
			if (drawPosX >= 0 && drawPosX < (int)referencedImage->getImageResolution().x && drawPosY >= 0 && drawPosY < (int)referencedImage->getImageResolution().y)
			{
				referencedImage->setPixelColor(ASCanvasPos(drawPosX, drawPosY), usedPen.penColor);
			}
		}
	}
}

void ASPainter::drawLine(const unsigned int& posAX, const unsigned int& posAY, const unsigned int& posBX, const unsigned int& posBY)
{
	int n;
	Pt2i from(posAX, posAY);
	Pt2i to(posBX, posBY);
	Pt2i* pts = from.drawing(to, &n);
	for (int i = 0; i < n; i++)
	{
		drawPoint(ASCanvasPos(pts[i].x(), pts[i].y()));
	}
	delete[] pts;
}

void ASPainter::drawText(int offsetX, int offsetY, string text)
{
	ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(5 + offsetX + ImGui::GetWindowPos().x, 20 + offsetY + ImGui::GetWindowPos().y), ImColor(usedPen.penColor.r, usedPen.penColor.g, usedPen.penColor.b), text.data());
}

void ASPainter::drawRect(int posX, int posy, int sizeX, int sizeY)
{
	for (int x = posX; x < posX + sizeX; ++x)
	{
		if (x >= 0 && x < (int)referencedImage->getImageResolution().x && posy >= 0 && posy < (int)referencedImage->getImageResolution().y)
		{
			referencedImage->setPixelColor(ASCanvasPos(x, posy), usedPen.penColor);
		}
	}
	for (int x = posX; x < posX + sizeX; ++x)
	{
		if (x >= 0 && x < (int)referencedImage->getImageResolution().x && posy + sizeY >= 0 && posy + sizeY < (int)referencedImage->getImageResolution().y)
		{
			referencedImage->setPixelColor(ASCanvasPos(x, posy + sizeY), usedPen.penColor);
		}
	}
	for (int y = posy; y < posy + sizeY; ++y)
	{
		if (posX >= 0 && posX < (int)referencedImage->getImageResolution().x && y >= 0 && y < (int)referencedImage->getImageResolution().y)
		{
			referencedImage->setPixelColor(ASCanvasPos(posX, y), usedPen.penColor);
		}
	}
	for (int y = posy; y < posy + sizeY; ++y)
	{
		if (posX + sizeX >= 0 && posX + sizeX < (int)referencedImage->getImageResolution().x && y >= 0 && y < (int)referencedImage->getImageResolution().y)
		{
			referencedImage->setPixelColor(ASCanvasPos(posX + sizeX, y), usedPen.penColor);
		}
	}
}

void ASPainter::fillRect(int posX, int posy, int sizeX, int sizeY)
{
	fillRect(posX, posy, sizeX, sizeY, usedPen.penColor);
}

void ASPainter::fillRect(int posX, int posy, int sizeX, int sizeY, const ASColor& fillColor)
{
	for (int x = posX; x < posX + sizeX; ++x)
	{
		for (int y = posy; y < posy + sizeY; ++y)
		{
			if (x >= 0 && x < (int)referencedImage->getImageResolution().x && y >= 0 && y < (int)referencedImage->getImageResolution().y)
			{
				referencedImage->setPixelColor(ASCanvasPos(x, y), fillColor);
			}
		}
	}
}

void ASPainter::fillRect(int posX, int posy, int sizeX, int sizeY, const ASBrush& brush)
{
	fillRect(posX, posy, sizeX, sizeY, brush.brushColor);
}
