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

#include "CustomString.h"
#include <string>

/**
 * @IniLoader - Read and write ini files
 *
 * 1) Create a new IniLoader with desired file path
 *			IniLoader myLoader("path/config.ini");
 *
 * 2) Read and write property from your loader
 *			String value = myLoader.GetPropertyAsString("categoryName", "propertyName", "defaultValue");
 *			myLoader.SetPropertyAsString("categoryName", "propertyName", "value");
 *
 * 3) Clear a property value by writing an empty string
 *			myLoader.SetPropertyAsString("categoryName", "propertyName", "");
 *
 * 3) Don't forget to save modifications
 *			myLoader.Save();
 *
 */


class IniLoader
{
public:

	/** Create a new ini loader - automatically load file at designed path or create path if file doesn't exist. Ini file must have '.ini extension' */
	IniLoader(const char* filePath);

	/** save ini modifications, then delete resources */
	~IniLoader();

	/** Get ini property from category and property name. Default value is returned if we can't find any occurrence */
	const std::string GetPropertyAsString(const char* categoryName, const char* propertyName, const char* defaultValue = "");
	const double GetPropertyAsDouble(const char* categoryName, const char* propertyName, const double& defaultValue = 0.f);
	const int GetPropertyAsInt(const char* categoryName, const char* propertyName, const int& defaultValue = 0);
	const bool GetPropertyAsBool(const char* categoryName, const char* propertyName, const bool& defaultValue = false);

	/** Set ini property (Save() must be called to save modifications on disk) */
	void SetPropertyAsString(const char* categoryName, const char* propertyName, const char* propertyValue) { SetProperty(categoryName, propertyName, CustomString('"' + CustomString(propertyValue) + '"'))	; }
	void SetPropertyAsDouble(const char* categoryName, const char* propertyName, const double& propertyValue) { SetProperty(categoryName, propertyName, ToString(propertyValue)); }
	void SetPropertyAsInt(const char* categoryName, const char* propertyName, const int& propertyValue) { SetProperty(categoryName, propertyName, ToString(propertyValue)); }
	void SetPropertyAsBool(const char* categoryName, const char* propertyName, const bool& propertyValue) { SetProperty(categoryName, propertyName, propertyValue ? "true" : "false"); }

	/** Create or update ini file on disk */
	void Save();

private:

	/** Ini structures */
	struct IniProperty
	{
		IniProperty(const CustomString& propertyName, const CustomString& propertyValue);

		CustomString propertyName;
		CustomString value;

		CustomString WriteLine() const;
		static void GetPropertyNameAndValueFromString(const CustomString& line, CustomString& name, CustomString& value);
		static bool IsPropertyLine(const CustomString& line);
	};

	struct IniCategory
	{
		IniCategory(const CustomString& categoryname);

		CustomString categoryName;
		std::vector<IniProperty> properties;

		void AddProperty(const CustomString& propertyName, const CustomString& propertyValue);
		CustomString WriteCategories() const;

		bool DoesPropertyExist(const CustomString& propertyName) const;
		static const CustomString GetCategoryNameFromString(const CustomString& line);
		static bool IsCategoryLine(const CustomString& line);
	};

	/** Internal methods */
	const CustomString GetProperty(const CustomString& categoryName, const CustomString& propertyName) const;
	void SetProperty(const CustomString& categoryName, const CustomString& propertyName, const CustomString& propertyValue);
	void ClearProperty(const CustomString& categoryName, const CustomString& propertyName);
	bool DoesCategoryExist(const CustomString& propertyName) const;

	void LinkOrCreate();

	/** ini file path */
	CustomString sourceFile;

	/** Ini categories (each category contains a property vector) */
	std::vector<IniCategory*> iniCategories;
};
