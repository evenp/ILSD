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

#ifndef AS_IMAGE_H
#define AS_IMAGE_H

#include <stdint.h>
#include "asCanvasPos.h"
#include "asColor.h"

using namespace std;

class GLWindow;

/**
 * @brief Replace QTImage
 * Contains a 4 channel image in an int32 table.
 * Handle loading, unloading, and drawing using openGL and ImGui
*/
class ASImage
{
public:

	/**
	 * @brief Create an empty image (not usable)
	*/
	ASImage();

	/**
	 * @brief Create an empty image with given resolution
	*/
	ASImage(ASCanvasPos newImageSize);

	/**
	 * @brief destroy image resources
	*/
	virtual ~ASImage() {}

	/**
	 * @brief Draw image onto the imgui context
	*/
	void Draw(GLWindow* drawWindow);

	/**
	 * @brief Set given pixel value from r,g,b,a values (uint8 x 4)
	*/
	void setPixel(const uint32_t& posX, const uint32_t& posY, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a);

	/**
	 * @brief Set given pixel value from given greyScale (uint8)
	*/
	void setPixelGrayscale(const uint32_t& posX, const uint32_t& posY, const uint8_t& greyScale);

	/**
	 * @brief Set given pixel value using ASColor
	*/
	void setPixelColor(const ASCanvasPos& position, const ASColor& color);

	/**
	 * @brief Get image width
	*/
	inline uint32_t width() const { return imageSize.x; }

	/**
	 * @brief Get image height
	*/
	inline uint32_t height() const { return imageSize.y; }

	/**
	 * @brief Get raw image data
	*/
	uint32_t** getBitmap() const;

	/**
	 * @brief Get pixel value as int (rgba <=> uint8 x 4 = uint32)
	*/
	uint32_t getPixelAsInt(const uint32_t& posX, const uint32_t& posY) const;

	/**
	 * @brief get pixel color as ASColor value.
	*/
	ASColor GetPixelColor(const uint32_t& posX, const uint32_t& posY) const;

	/**
	 * @brief fill image with custom color
	*/
	void clear(const ASColor& clearColor = ASColor::BLACK);

	/**
	 * @brief Copy data to another image
	*/
	void copyTo(ASImage& target) const;

	/**
	 * @brief output image data to custom png file (directory must exist)
	*/
	bool save(const char* newFilePath, const char* fileFormat = "") const;

	/**
	 * @brief Copy data from an other image
	*/
	void operator=(const ASImage& other);

	/**
	 * @brief Ensure ASImage works properly
	*/
	static bool testImage();

	/**
	 * @brief get image resolution
	*/
	inline ASCanvasPos getImageResolution() const { return imageSize; }

	/**
	 * @brief Set display zoom level
	*/
	inline void setZoom(int32_t newZoom) { zoom = newZoom; }

	/**
	 * @brief set display position offset
	*/
	inline void setDisplayPosition(const int& posX, const int& posY) { displayPositionX = posX; displayPositionY = posY; }

	/**
	 * @brief transform mouse location to texture coordinates
	*/
	bool mouseToTexture(GLWindow* drawWindow, ASCanvasPos& result) const;

	/**
	 * @brief transform viewport position to texture coordinates
	*/
	bool viewportToTexture(GLWindow* drawWindow, const ASCanvasPos& viewportPosition, ASCanvasPos& result) const;

private:

	/**
	 * @brief Send raw data to openGL
	*/
	void RebuildTexture();

	/**
	 * @brief transform 2D pos to rawData table index
	*/
	uint32_t PosToPixelIndex(const uint32_t& posX, const uint32_t& posY) const;

	/**
	 * @brief image resolution
	*/
	ASCanvasPos imageSize;

	/**
	 * @brief display zoom level
	*/
	int32_t zoom;

	/**
	 * @brief display position offset X
	*/
	int displayPositionX;

	/**
	 * @brief display position offset Y
	*/
	int displayPositionY;

	/**
	 * @brief OpenGL texture ID
	*/
	uint32_t textureId;

	/**
	 * @brief texture raw data
	*/
	uint32_t* textureData;

	/**
	 * @brief Has texture been modified an does it need to be sent back to opengl
	*/
	bool bIsTextureDirty;

	/**
	 * @brief Has openGl textureId been generated
	*/
	bool bAreTextureDataBuilt;
};
#endif
