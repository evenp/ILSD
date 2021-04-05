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

#pragma once

#include <vector>
#include <inttypes.h>
#include <cstdlib>


#ifndef STRING_MINIMAL_LENGTH
	#define STRING_MINIMAL_LENGTH 16
#endif

class CustomString;
class IStringable
{
public:
	virtual CustomString ToString() const = 0;
};

class CustomString;

/* Value To String */
CustomString ToString(int64_t value);
CustomString ToString(uint64_t value);
CustomString ToString(int32_t value);
CustomString ToString(uint32_t value);
CustomString ToString(double value);
CustomString ToString(float value);

int32_t Atoi(const CustomString& value);
double Atof(const CustomString& value);

class CustomString
{
public:
	static const CustomString ENDL;

	/* Constructors */
	CustomString() = default;
	~CustomString();

	inline CustomString(const char* value) {
		if (value)
		{
			length = 0;
			while (value[length] != '\0')
				length++;

			SetLength(length);

			for (int i = 0; i < length; ++i)
				data[i] = value[i];
		}
	}

	inline CustomString(const char& value) { SetLength(1); data[length - 1] = value; }
	inline CustomString(CustomString&& other) {	CopyTo(other, this); }
	inline CustomString(const CustomString& other) { CopyTo(other, this); }
	inline CustomString(const IStringable& other) { CopyTo(other.ToString(), this); }
	inline CustomString(int32_t other) : CustomString(ToString(other)) {}
	inline CustomString(uint32_t other) : CustomString(ToString(other)) {}
	inline CustomString(int64_t other) : CustomString(ToString(other)) {}
	inline CustomString(uint64_t other) : CustomString(ToString(other)) {}
	inline CustomString(float other) : CustomString(ToString(other)) {}
	inline CustomString(double other) : CustomString(ToString(other)) {}

	/* Getters */
	inline const size_t& Length() const { return length; }
	inline const char* GetData() const { data[length] = '\0'; return data; }

	/* Operator == */
	bool operator==(const CustomString& _Val) const;

	/* Operator != */
	bool operator!=(const CustomString& _Val) const;

	/* Operator < */
	bool operator<(const CustomString& _Val) const;

	/* Operator /= */
	inline CustomString& operator/=(const CustomString& _Val) { return Append(CustomString('/') + _Val); }

	/* Operator / */
	inline CustomString operator/(const CustomString& _Val) const { return Concatenate(CustomString(GetData()), CustomString(CustomString('/') + _Val)); }

	/* Operator * */
	inline const char* operator*() { return GetData(); }

	/* Operator [] */
	inline const char& operator[](size_t pos) const { return data[pos]; }

	/* Operator -= */
	inline CustomString& operator-=(const size_t& _Val) { SetLength(length - _Val); return *this; }

	/* Operator << */
	inline CustomString& operator<<(const CustomString& _Val) { return Append(_Val); }
	inline CustomString& operator<<(const char* _Val) { return Append(_Val); }
	inline CustomString& operator<<(const char& _Val) { return Append(_Val); }
	inline CustomString& operator<<(void* _Val) { return Append("ptr"); }
	inline CustomString& operator<<(const int64_t& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator<<(const uint64_t& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator<<(const int32_t& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator<<(const uint32_t& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator<<(const float& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator<<(const double& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator<<(const IStringable& _Val) { return Append(_Val.ToString()); }

	/* Operator += */
	inline CustomString& operator+=(const CustomString& _Val) { return Append(_Val); }
	inline CustomString& operator+=(const char* _Val) { return Append(_Val); }
	inline CustomString& operator+=(const char& _Val) { return Append(_Val); }
	inline CustomString& operator+=(void* _Val) { return Append("ptr"); }
	inline CustomString& operator+=(const int64_t& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator+=(const uint64_t& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator+=(const int32_t& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator+=(const uint32_t& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator+=(const float& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator+=(const double& _Val) { return Append(ToString(_Val)); }
	inline CustomString& operator+=(const IStringable& _Val) { return Append(_Val.ToString()); }

	/* Operator + */
	inline CustomString operator+(const CustomString& _Val) const { return Concatenate(CustomString(GetData()), CustomString(_Val)); }
	inline CustomString operator+(const char* _Val) const { return Concatenate(CustomString(GetData()), CustomString(_Val)); }
	inline CustomString operator+(const char& _Val) const { return Concatenate(CustomString(GetData()), CustomString(_Val)); }
	inline CustomString operator+(const int64_t& _Val) const { return Concatenate(CustomString(GetData()), ToString(_Val)); }
	inline CustomString operator+(const uint64_t& _Val) const { return Concatenate(CustomString(GetData()), ToString(_Val)); }
	inline CustomString operator+(const int32_t& _Val) const { return Concatenate(CustomString(GetData()), ToString(_Val)); }
	inline CustomString operator+(const uint32_t& _Val) const { return Concatenate(CustomString(GetData()), ToString(_Val)); }
	inline CustomString operator+(const double& _Val) const { return Concatenate(CustomString(GetData()), ToString(_Val)); }
	inline CustomString operator+(const float& _Val) const { return Concatenate(CustomString(GetData()), ToString(_Val)); }
	inline CustomString operator+(const IStringable& _Val) const { return Concatenate(CustomString(GetData()), _Val.ToString()); }

	/* Operator = */
	inline CustomString operator=(const CustomString& other) { return CopyTo(other, this); }
	inline CustomString operator=(const char& other) { return CopyTo(other, this); }
	inline CustomString operator=(const char* other) { return CopyTo(other, this); }
	inline CustomString operator=(const int64_t& other) { return CopyTo(ToString(other), this); }
	inline CustomString operator=(const int32_t& other) { return CopyTo(ToString(other), this); }
	inline CustomString operator=(const double& other) { return CopyTo(ToString(other), this); }
	inline CustomString operator=(const float& other) { return CopyTo(ToString(other), this); }
	inline CustomString operator=(const IStringable& other) { return CopyTo(other.ToString(), this); }

	/* Tests */
	inline static bool IsAlpha(const char& chr) { return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z'); }
	inline static bool IsNumeric(const char& chr) { return (chr >= '0' && chr <= '9'); }
	inline static bool IsAlphanumeric(const char& chr) { return IsAlpha(chr) || IsNumeric(chr); }

	/* String operations */
	inline static const CustomString Concatenate(const CustomString& left, const CustomString& right) {
		CustomString res;
		res.SetLength(left.length + right.length);
		for (size_t i = 0; i < left.length; ++i)
			res.data[i] = left[i];
		for (size_t i = left.length; i < res.length; ++i)
			res.data[i] = right[i - left.length];
		return res;	
	}

	inline CustomString& Append(const CustomString& other) {
		size_t thisLength = length;
		SetLength(thisLength + other.length);
		for (size_t i = thisLength; i < length; ++i)
			data[i] = other.data[i - thisLength];
		return *this;
	}

	inline bool IsStartingWith(const CustomString& start) const {
		if (start.length > length) return false;
		for (size_t i = 0; i < start.length; ++i)
			if ((*this)[i] != start[i])
				return false;
		return true;
	}

	template<typename T>
	inline static CustomString ConcatenateArray(T& tarray, CustomString separator = ", ")
	{
		CustomString result = "";
		size_t elemCount = tarray.size();
		for (auto& elem : tarray)
		{
			result += CustomString(elem);
			elemCount--;
			if (elemCount > 0) result += separator;
		}
		return result;
	}

	static bool SplitString(const CustomString& lineData, std::vector<char> separators, CustomString& left, CustomString& right, bool bFromStart = true);

	inline static bool const IsStartingWith(const CustomString& base, const CustomString& start) {
		if (start.length > base.length) return false;
		for (int i = 0; i < start.length; ++i)
			if (base[i] != start[i])
				return false;
		return true;
	}

	inline static bool const IsEndingWith(const CustomString& base, const CustomString& end) {
		if (end.length > base.length) return false;
		for (int i = 1; i <= end.length; ++i)
			if (base[base.length - i] != end[end.length - i])
				return false;
		return true;
	}

	inline static CustomString RemoveBorderSpaces(const CustomString& line)	{
		if (line.length <= 0) return "";
		CustomString out = "";
		size_t start = 0, stop = line.length - 1;
		for (size_t i = 0; i < line.length; ++i)
		{
			if (line[i] != ' ' && line[i] != '\t')
			{
				start = i;
				break;
			}
		}
		for (size_t i = line.length - 1; i > 0; --i)
		{
			if (line[i] != ' ' && line[i] != '\t')
			{
				stop = i;
				break;
			}
		}
		if (start > stop) return "";
		for (size_t i = start; i <= stop; ++i)
			out += line[i] == '\t' ? ' ' : line[i];

		return out;
	}



	static const CustomString GetFileName(const CustomString& path);
	static const CustomString GetFileShortName(const CustomString& path);
	static const CustomString GetFilePath(const CustomString& path);
	static const CustomString GetFileExtension(const CustomString& path);

	static const std::vector<CustomString> ParseStringCharArray(const char* charString, size_t length);

	inline static void ResetCharArray(char* str, const size_t& strLength) { for (int i = 0; i < strLength; ++i) str[i] = '\0'; }

private:

	inline static const CustomString& CopyTo(const CustomString& from, CustomString* to) {
		to->SetLength(from.length);
		for (int i = 0; i < to->length; ++i)
		{
			to->data[i] = from.data[i];
		}
		return *to;
	}


	inline void SetLength(const size_t& newSize) {
		if (allocLength < newSize + 1 || newSize + 1 + STRING_MINIMAL_LENGTH < allocLength)
		{
			allocLength = newSize + 1 + STRING_MINIMAL_LENGTH;
			data = (char*)realloc(data, allocLength);
		}
		length = newSize;
	}

	char* data = new char[STRING_MINIMAL_LENGTH];

	size_t length = 0;
	size_t allocLength = STRING_MINIMAL_LENGTH;
};

inline CustomString operator+(const char& chr, const CustomString& str) { return CustomString(chr) + str; }
inline CustomString operator+(const char* chr, const CustomString& str) { return CustomString(chr) + str; }
