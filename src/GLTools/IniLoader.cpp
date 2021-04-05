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

#include "IniLoader.h"
#if _WIN32
	#include <filesystem>
#endif
#include <iostream>
#include <fstream>

IniLoader::IniLoader(const char* filePath)
	: sourceFile(filePath) {
	LinkOrCreate();
}

IniLoader::~IniLoader() {
	Save();
	for (const auto& cat : iniCategories)
	{
		delete cat;
	}
	iniCategories.clear();
}

const std::string IniLoader::GetPropertyAsString(const char* categoryName, const char* propertyName, const char* defaultValue) {
	CustomString props = GetProperty(categoryName, propertyName);
	if (props != "")
	{
		CustomString left, center, right, finalS;
		if (CustomString::SplitString(props, { '"' }, left, center, true))
		{
			if (CustomString::SplitString(center, { '"' }, finalS, right, false))
			{
				return finalS.GetData();
			}
		}
	}
	return defaultValue;
}

const int IniLoader::GetPropertyAsInt(const char* categoryName, const char* propertyName, const int& defaultValue)
{
	CustomString props = GetProperty(categoryName, propertyName);
	if (props != "")
	{
		return Atoi(props);
	}
	return defaultValue;
}

const double IniLoader::GetPropertyAsDouble(const char* categoryName, const char* propertyName, const double& defaultValue)
{
	CustomString props = GetProperty(categoryName, propertyName);
	if (props != "")
	{
		return Atof(props);
	}
	return defaultValue;
}

const bool IniLoader::GetPropertyAsBool(const char* categoryName, const char* propertyName, const bool& defaultValue)
{
	CustomString props = GetProperty(categoryName, propertyName);
	if (props == "true") return true;
	else if (props == "false") return false;
	return defaultValue;
}

void IniLoader::Save()
{
	std::ofstream of(sourceFile.GetData());

	for (const auto& cat : iniCategories)
	{
		CustomString catString = cat->WriteCategories();
		of.write(catString.GetData(), catString.Length());
	}
	of.close();
}

const CustomString IniLoader::GetProperty(const CustomString& categoryName, const CustomString& propertyName) const
{
	for (const auto& cat : iniCategories)
	{
		if (cat->categoryName == categoryName)
		{
			for (const auto& prop : cat->properties)
			{
				if (prop.propertyName == propertyName)
				{
					return prop.value;
				}
			}
		}
	}
	return "";
}

void IniLoader::SetProperty(const CustomString& categoryName, const CustomString& propertyName, const CustomString& propertyValue)
{
	if (propertyValue == "")
	{
		ClearProperty(categoryName, propertyName);
	}
	else
	{
		for (auto& category : iniCategories)
		{
			if (category->categoryName == categoryName)
			{
				for (auto& prop : category->properties)
				{
					if (prop.propertyName == propertyName)
					{
						prop.value = propertyValue;
						return;
					}
				}
				category->properties.push_back(IniProperty(propertyName, propertyValue));
				return;
			}
		}
		IniCategory* newCat = new IniCategory(categoryName);
		newCat->properties.push_back(IniProperty(propertyName, propertyValue));
		iniCategories.push_back(newCat);
		return;
	}
}

void IniLoader::ClearProperty(const CustomString& categoryName, const CustomString& propertyName)
{
	for (int i = (int)iniCategories.size() - 1; i >= 0; --i)
	{
		if (iniCategories[i]->categoryName == categoryName)
		{
			for (int j = (int)iniCategories[i]->properties.size() - 1; j >= 0; --j)
			{
				if (iniCategories[i]->properties[j].propertyName == propertyName)
				{
					iniCategories[i]->properties.erase(iniCategories[i]->properties.begin() + j);
				}
			}
			if (iniCategories[i]->properties.size() == 0)
			{
				delete iniCategories[i];
				iniCategories.erase(iniCategories.begin() + i);
			}
			return;
		}
	}
}

bool IniLoader::DoesCategoryExist(const CustomString& propertyName) const
{
	for (const auto& cat : iniCategories)
	{
		if (cat->categoryName == propertyName) return true;
	}
	return false;
}

void IniLoader::LinkOrCreate()
{
//	if (!CustomString::IsEndingWith(sourceFile, ".ini")) { std::cerr << "Ini files must have '.ini' extension" << std::endl; }
#if _WIN32
	if (!std::filesystem::exists(sourceFile.GetData()))
	{
		CustomString left, right;
		CustomString::SplitString(sourceFile, { '/', '\'' }, left, right, false);
		std::filesystem::create_directories(left.GetData());
	}
#endif

	std::ifstream fs(sourceFile.GetData());
	char* line = new char[1000];
	IniCategory* currentCategory = nullptr;
	while (fs.getline(line, 1000, '\n'))
	{
		CustomString resultLine, right;
		if (!CustomString::SplitString(line, { ';', '#' }, resultLine, right))
		{
			resultLine = line;
		}


		if (IniCategory::IsCategoryLine(resultLine))
		{
			CustomString catName = IniCategory::GetCategoryNameFromString(resultLine);
			if (!DoesCategoryExist(catName))
			{
				currentCategory = new IniCategory(catName);
				iniCategories.push_back(currentCategory);
			}
			else
			{
				for (const auto& cat : iniCategories)
				{
					if (cat->categoryName == catName) currentCategory = cat;
				}
			}
		}
		else
		{
			if (currentCategory && IniProperty::IsPropertyLine(resultLine))
			{
				CustomString name, value;
				IniProperty::GetPropertyNameAndValueFromString(line, name, value);
				if (!currentCategory->DoesPropertyExist(name))
				{
					currentCategory->AddProperty(name, value);
				}
			}
		}
	}
	delete line;
	fs.close();
}

IniLoader::IniProperty::IniProperty(const CustomString& inPropertyName, const CustomString& inPropertyValue) {
	propertyName = inPropertyName;
	value = inPropertyValue;
}

CustomString IniLoader::IniProperty::WriteLine() const {
	return propertyName+ '=' + value + '\n';
}

void IniLoader::IniProperty::GetPropertyNameAndValueFromString(const CustomString& line, CustomString& name, CustomString& value)
{
	CustomString::SplitString(line, { '=' }, name, value);
	name = CustomString::RemoveBorderSpaces(name);
	value = CustomString::RemoveBorderSpaces(value);
}

bool IniLoader::IniProperty::IsPropertyLine(const CustomString& line)
{
	CustomString left, right;
	return (CustomString::SplitString(line, { '=' }, left, right, true) && right != "");
}

IniLoader::IniCategory::IniCategory(const CustomString& inCategoryName)
{
	categoryName = inCategoryName;
}

void IniLoader::IniCategory::AddProperty(const CustomString& propertyName, const CustomString& propertyValue)
{
	properties.push_back(IniProperty(propertyName, propertyValue));
}

CustomString IniLoader::IniCategory::WriteCategories() const
{
	CustomString outString = '[' + categoryName + "]\n";
	for (const auto& prop : properties)
	{
		outString += prop.WriteLine();
	}
	return outString + "\n";
}

bool IniLoader::IniCategory::DoesPropertyExist(const CustomString& propertyName) const
{
	for (const auto& prop : properties)
	{
		if (prop.propertyName == propertyName) return true;
	}
	return false;
}

const CustomString IniLoader::IniCategory::GetCategoryNameFromString(const CustomString& line)
{
	CustomString left, center, right, categoryName;
	CustomString::SplitString(line, { '[' }, left, center);
	CustomString::SplitString(center, { ']' }, categoryName, right);
	return categoryName;
}

bool IniLoader::IniCategory::IsCategoryLine(const CustomString& line)
{
	return CustomString::IsStartingWith(line, '[') && CustomString::IsEndingWith(line, ']');
}
