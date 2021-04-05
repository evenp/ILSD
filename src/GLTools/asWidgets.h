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

#ifndef AS_WIDGET_H
#define AS_WIDGET_H
#include <vector>

/**
 * @brief Static function library with custom imgui widgets
*/
class AsWidgets
{
public:

	/**
	 * @brief menu 'Slider' with -10, -1, +1, +10 buttons.
	*/
	static bool MenuCartesianSlider(const char* title, const char* shortcut, float currentValue, int& result, bool* resetBool = nullptr);

	/**
	 * @brief Allow to select an item within different possibilities
	*/
	static bool MenuComboSwitcher(const char* title, const char* shortcut, std::vector<const char*> options, const int& currentSelectedOption, int& selectedOptionIndex);
};
#endif
