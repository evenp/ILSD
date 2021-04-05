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

#include "CustomString.h"
#include <iostream>
#include <string>

const CustomString CustomString::ENDL = CustomString((char)('\n'));

bool CustomString::operator!=(const CustomString& _Val) const
{
	if (_Val.length != length) return true;
	for (int i = 0; i < length; ++i)
		if (data[i] != _Val[i]) return true;
	return false;
}

bool CustomString::operator<(const CustomString& _Val) const
{
	return std::string(data) < std::string(_Val.data);
}

bool CustomString::SplitString(const CustomString& lineData, std::vector<char> separators, CustomString& left, CustomString& right, bool bFromStart /*= true*/)
{
	left = "";
	right = "";
	bool bParsingLeft = true;
	for (int64_t i = bFromStart ? 0 : (int64_t)lineData.length - 1; bFromStart ? i < (int64_t)lineData.length : i >= 0; i += bFromStart ? 1 : -1)
	{
		bool bContainsSep = false;
		for (const auto& sep : separators) if (lineData[i] == sep) bContainsSep = true;
		if (bContainsSep && bParsingLeft)
			bParsingLeft = false;
		else if (bParsingLeft)
		{
			if (bFromStart) left += lineData[i];
			else right = lineData[i] + right;
		}
		else
		{
			if (bFromStart) right += lineData[i];
			else left = lineData[i] + left;
		}
	}
	return !bParsingLeft;
}

CustomString ToString(int64_t value) {
	return std::to_string(value).c_str();
}

CustomString ToString(int32_t value) {
	return std::to_string(value).c_str();
}

CustomString ToString(double value) {
	return std::to_string(value).c_str();
}

CustomString ToString(float value) {
	return std::to_string(value).c_str();
}

CustomString ToString(uint64_t value)
{
	return std::to_string(value).c_str();
}

CustomString ToString(uint32_t value)
{
	return std::to_string(value).c_str();
}

int32_t Atoi(const CustomString& value)
{
	return std::atoi(value.GetData());
}

double Atof(const CustomString& value)
{
	return std::atof(value.GetData());
}

CustomString::~CustomString()
{
	delete data;
}

bool CustomString::operator==(const CustomString& _Val) const
{
	if (_Val.length != length) return false;
	for (int i = 0; i < length; ++i)
		if (data[i] != _Val[i]) return false;
	return true;
}


const CustomString CustomString::GetFileName(const CustomString& path)
{
	CustomString left, name;
	if (SplitString(path, { '/', '\\' }, left, name, false))
		return name;
	return path;
}

const CustomString CustomString::GetFileShortName(const CustomString& path)
{
	CustomString filename = GetFileName(path);
	CustomString name, ext;
	if (SplitString(filename, { '.' }, name, ext, false))
		return name;
	return filename;
}

const CustomString CustomString::GetFilePath(const CustomString& path)
{
	CustomString npath, name;
	if (SplitString(path, { '/', '\\' }, npath, name, false))
		return npath;
	return path;
}

const CustomString CustomString::GetFileExtension(const CustomString& path)
{
	CustomString filename = GetFileName(path);
	CustomString name, ext;
	if (SplitString(path, { '.' }, name, ext, false))
		return ext;
	return "";
}

const std::vector<CustomString> CustomString::ParseStringCharArray(const char* charString, size_t length)
{
	CustomString currentchr;
	std::vector<CustomString> output;

	for (int i = 0; i < length; ++i) {
		if (charString[i] == '\0') {
			if (currentchr != "") {
				output.push_back(currentchr);
			}
			currentchr = "";
		}
		else {
			currentchr += charString[i];
		}
	}
	return output;
}
