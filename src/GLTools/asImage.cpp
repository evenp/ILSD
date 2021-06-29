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

#include "asImage.h"
#ifdef __APPLE__
	#include "GL/glew.h"
#else
	#include "glad/glad.h"
#endif
#include "GLFW/glfw3.h"
#include "glWindow.h"
#include "imgui.h"
#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

ASImage::ASImage()
	: imageSize(ASCanvasPos(0, 0))
{
	textureData = nullptr;
	bAreTextureDataBuilt = false;
	bIsTextureDirty = false;
}

ASImage::ASImage(ASCanvasPos newImageSize)
	: imageSize(newImageSize) {
	textureData = new uint32_t[newImageSize.x * newImageSize.y];
	bIsTextureDirty = true;
	bAreTextureDataBuilt = false;
	zoom = 0;
	displayPositionX = 0;
	displayPositionY = 0;
}

bool ASImage::save(const char* newFilePath, const char* fileFormat) const
{
	return stbi_write_png(newFilePath, imageSize.x, imageSize.y, 4, textureData, 0) == 0;
}

void ASImage::operator=(const ASImage& other)
{
//	if (textureData) delete textureData;
//	textureData = new uint32_t[other.imageSize.x * other.imageSize.y];
	textureData = (uint32_t*)realloc(textureData,other.imageSize.x * other.imageSize.y * sizeof(uint32_t));
	imageSize = other.imageSize;
	memcpy(textureData, other.textureData, sizeof(int) * other.imageSize.x * other.imageSize.y);
	bIsTextureDirty = true;
}

bool ASImage::testImage()
{
	bool bFailed = false;
	ASImage img(ASCanvasPos(10, 20));

	for (int x = 0; x < 10; ++x)
	{
		for (int y = 0; y < 20; ++y)
		{
			ASColor col = ASColor(rand());

			img.setPixel(x, y, col.r, col.g, col.b, col.a);
			if (img.GetPixelColor(x, y) != col) bFailed = true;
			if (img.getPixelAsInt(x, y) != col.asInt()) bFailed = true;

			img.setPixelColor(ASCanvasPos(x, y), col);
			if (img.GetPixelColor(x, y) != col) bFailed = true;


			img.setPixelGrayscale(x, y, col.r);
			if (img.GetPixelColor(x, y).r != col.r) bFailed = true;

			if (bFailed) break;
		}
		if (bFailed) break;
	}


	cout << "Image test " << (bFailed ? "failed" : "succeed") << endl;
	return !bFailed;
}

bool ASImage::mouseToTexture(GLWindow* drawWindow, ASCanvasPos& result) const
{
	double posX, posY;
	glfwGetCursorPos(drawWindow->getGlfwContext(), &posX, &posY);

	return viewportToTexture(drawWindow, ASCanvasPos((uint32_t)posX, (uint32_t)posY), result);
}

bool ASImage::viewportToTexture(GLWindow* drawWindow, const ASCanvasPos& viewportPosition, ASCanvasPos& result) const
{
	int resX, resY;

	glfwGetWindowSize(drawWindow->getGlfwContext(), &resX, &resY);

	double sizeMult = (zoom + 1) > 0 ? (zoom + 1) : -1.0 / (zoom - 1);

	double posX = (((double)viewportPosition.x - (double)resX / 2) / sizeMult + (double)imageSize.x / 2 - (double)displayPositionX);
	double posY = (((double)viewportPosition.y - (double)resY / 2) / sizeMult + (double)imageSize.y / 2 - (double)displayPositionY);

	if (posX < 0 || posY < 0 || posX >= imageSize.x || posY >= imageSize.y)
	{
		return false;
	}

	result.x = (int32_t)posX;
	result.y = (int32_t)posY;

	return true;
}

unsigned char* testData;

void ASImage::RebuildTexture()
{
	if (!bAreTextureDataBuilt)
	{
		bAreTextureDataBuilt = true;
		glGenTextures(1, &textureId);

	}

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageSize.x, imageSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)textureData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	bIsTextureDirty = false;
	bAreTextureDataBuilt = true;
}

uint32_t ASImage::PosToPixelIndex(const uint32_t& posX, const uint32_t& posY) const
{
	if (posX < 0 || posY < 0 || posX >= imageSize.x || posY >= imageSize.y)
#if _WIN32
		throw new std::exception(string("Cannot access point outside of texture bounds : pos " + ASCanvasPos(posX, posY).toString() + " - max " + imageSize.toString()).data());
#else
		cerr << string("Cannot access point outside of texture bounds : pos " + ASCanvasPos(posX, posY).toString() + " - max " + imageSize.toString()).data() << endl;
#endif
	return posX + posY * imageSize.x;
}

GLfloat spec = 1;
GLfloat shininess = 1;
GLfloat diffusion = 1;

void ASImage::Draw(GLWindow* drawWindow)
{
	if (bIsTextureDirty)
	{
		RebuildTexture();
	}

	int resX = (int)ImGui::GetWindowSize().x;
	int resY = (int)ImGui::GetWindowSize().y;

	double sizeMult = (zoom + 1) > 0 ? (zoom + 1) : -1.0 / (zoom - 1);

	ImVec2 size = ImVec2((float)imageSize.x * (float)sizeMult, (float)imageSize.y * (float)sizeMult);
	ImVec2 center = ImVec2((float)(resX / 2 + displayPositionX * sizeMult) + ImGui::GetWindowPos().x, (float)(resY / 2 + displayPositionY * sizeMult) + ImGui::GetWindowPos().y);
	ImGui::GetWindowDrawList()->AddImageQuad(
		(void*)(intptr_t)textureId,
		ImVec2(center.x - size.x / 2, center.y - size.y / 2),
		ImVec2(center.x + size.x / 2, center.y - size.y / 2),
		ImVec2(center.x + size.x / 2, center.y + size.y / 2),
		ImVec2(center.x - size.x / 2, center.y + size.y / 2));
}

void ASImage::setPixel(const uint32_t& posX, const uint32_t& posY, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a)
{
	textureData[PosToPixelIndex(posX, posY)] = r + g * 256 + b * 256 * 256 + a * 256 * 256 * 256;
	bIsTextureDirty = true;
}

void ASImage::setPixelGrayscale(const uint32_t& posX, const uint32_t& posY, const uint8_t& greyScale)
{
	setPixel(posX, posY, greyScale, greyScale, greyScale, 255);
	bIsTextureDirty = true;
}

void ASImage::setPixelColor(const ASCanvasPos& position, const ASColor& color)
{
	textureData[PosToPixelIndex(position.x, position.y)] = color.asInt();
	bIsTextureDirty = true;
}

uint32_t** ASImage::getBitmap() const
{
	uint32_t** tabImage = new uint32_t* [imageSize.y];
	for (uint32_t i = 0; i < imageSize.y; i++)
	{
		tabImage[i] = new uint32_t[imageSize.x];
		for (uint32_t j = 0; j < imageSize.x; j++)
		{
			tabImage[i][j] = getPixelAsInt(i, j);
		}
	}
	return tabImage;
}

uint32_t ASImage::getPixelAsInt(const uint32_t& posX, const uint32_t& posY) const
{
	return textureData[PosToPixelIndex(posX, posY)];
}

ASColor ASImage::GetPixelColor(const uint32_t& posX, const uint32_t& posY) const
{
	return ASColor(textureData[PosToPixelIndex(posX, posY)]);
}

void ASImage::clear(const ASColor& clearColor /*= GLColor4::BLACK*/)
{
	int clearColorInt = clearColor.asInt();
	for (uint32_t i = 0; i < imageSize.x * imageSize.y; ++i)
	{
		textureData[i] = clearColorInt;
	}
	bIsTextureDirty = true;
}

void ASImage::copyTo(ASImage& target) const
{
	delete target.textureData;
	target.textureData = new uint32_t[imageSize.x * imageSize.y];
	target.imageSize = imageSize;
	memcpy(target.textureData, textureData, sizeof(int) * imageSize.x * imageSize.y);
	target.bIsTextureDirty = true;
}
