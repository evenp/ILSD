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

#include <filesystem>
#include "asImGuiWindow.h"
#include "DelegateSystem.h"
#include "CustomString.h"
#include <vector>

inline CustomString G_LAST_CONTENT_BROWSER_DIRECTORY = ".";

DECLARE_DELEGATE_MULTICAST(OnApplyExplorerEvent, const std::vector<std::string>&);
DECLARE_DELEGATE_MULTICAST(CancelExplorerEvent);
DECLARE_DELEGATE_MULTICAST(OnDestroyEvent);

class FileExplorer : public AsImGuiWindow {
public:
	FileExplorer(GLWindow* context, const std::string& windowName, const std::string& defaultPath, const std::vector<std::string>& inExtensionFilters = {}, bool withNoSelection = false, int itemCount = 1);

	FileExplorer(GLWindow* context, const std::string& windowName, const std::string& defaultPath, const std::vector<std::string>& inExtensionFilters, const std::vector<std::string>& alreadySelected, bool withNoSelection = false, int itemCount = 1);

	OnApplyExplorerEvent OnApplyPath;
	CancelExplorerEvent OnCancelExplorer;
	OnDestroyEvent OnDestroy;

protected:

protected:

	virtual void DrawContent(GLWindow* windowContext);

	virtual ~FileExplorer();

private:

	void SetCurrentPath(const CustomString& path);
	void UpdateCurrentPath();

	void DrawDirContent(const CustomString& dirPath);

	bool bHasBeenValidated = false;

	char currentPath[256];
	std::vector<std::filesystem::directory_entry> currentDirs;
	std::vector<std::string> alreadySelected;
	std::vector<int> numUnselect;
	std::vector<CustomString> selectedElements;
	std::vector<std::vector<CustomString>> extensionFilters;
	int currentFilter = 0;
	int itemsToSelect;

	bool bSetColumnWidth = false;
	bool withNoSel = false;
	bool noSelection = false;
        bool pruneSelected = false;

	static bool compNames (std::filesystem::directory_entry &s1, std::filesystem::directory_entry &s2);
};
