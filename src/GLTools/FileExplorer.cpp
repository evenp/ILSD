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

#include "FileExplorer.h"
#include "imgui.h"
#include <iostream>
#include <vector>
#include <algorithm>


FileExplorer::FileExplorer(GLWindow* context, const std::string& windowName, const std::string& defaultPath, const std::vector<std::string>& inExtensionFilters, bool withNoSelection, int itemCount)
	: itemsToSelect(itemCount), AsImGuiWindow(context, windowName)
{
//	if (std::filesystem::exists(G_LAST_CONTENT_BROWSER_DIRECTORY.GetData()))
//		SetCurrentPath(G_LAST_CONTENT_BROWSER_DIRECTORY);
//	else
		SetCurrentPath(defaultPath.data());

	extensionFilters.push_back({});
	if (extensionFilters.size() != 0) {
		for (const auto& ext : inExtensionFilters)
		{
			extensionFilters.push_back({ ext.data() });
		}
		if (inExtensionFilters.size() != 1)
		{
			std::vector<CustomString> extGroup;
			for (const auto& subExt : inExtensionFilters) extGroup.push_back(subExt.data());
				extensionFilters.push_back(extGroup);
		}
		currentFilter = (int)extensionFilters.size() - 1;
	}
	withNoSel = withNoSelection;
}

FileExplorer::FileExplorer(GLWindow* context, const std::string& windowName, const std::string& defaultPath, const std::vector<std::string>& inExtensionFilters, const std::vector<std::string>& alreadySelected, bool withNoSelection, int itemCount)
	: itemsToSelect(itemCount), AsImGuiWindow(context, windowName)
{
	SetCurrentPath(defaultPath.data());

	extensionFilters.push_back({});
	if (extensionFilters.size() != 0) {
		for (const auto& ext : inExtensionFilters)
		{
			extensionFilters.push_back({ ext.data() });
		}
		if (inExtensionFilters.size() != 1)
		{
			std::vector<CustomString> extGroup;
			for (const auto& subExt : inExtensionFilters) extGroup.push_back(subExt.data());
				extensionFilters.push_back(extGroup);
		}
		currentFilter = (int)extensionFilters.size() - 1;
	}
	withNoSel = withNoSelection;
	this->alreadySelected = alreadySelected;
	pruneSelected = true;
}

void FileExplorer::DrawContent(GLWindow* windowContext)
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 2));
	ImGui::Separator();
	bool bIsPathValid = true;
	if (!std::filesystem::exists(currentPath))
	{
		bIsPathValid = false;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.f, .2f, .2f, .5f));
	}

	/** Search bar */
	ImGui::InputText("", currentPath, 256);
	if (std::filesystem::path(currentPath).has_parent_path())
	{
		ImGui::SameLine();
		if (ImGui::Button("<<")) {
			SetCurrentPath(std::filesystem::path(currentPath).parent_path().u8string().c_str());
		}
	}
	if (!bIsPathValid) ImGui::PopStyleColor();
	ImGui::Separator();

	float windowSize = max(ImGui::GetContentRegionAvail().x * .15f, 150.f);

	ImGui::Columns(2);
	if (!bSetColumnWidth) {
		ImGui::SetColumnWidth(0, windowSize);
		bSetColumnWidth = true;
	}
	/**  default path */

	if (ImGui::Button("project", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
		SetCurrentPath(".");
	}	
	if (ImGui::Button("root", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
		SetCurrentPath("/");
	}
	ImGui::NextColumn();

	/** Content */
	if (ImGui::BeginChild("outer_child", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 100), false)) {
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0.5));
		if (bIsPathValid) DrawDirContent(currentPath);
		ImGui::PopStyleVar();
	}
	ImGui::EndChild();

	ImGui::NextColumn();
	ImGui::Columns(1);

	if (ImGui::BeginChild("selected_element", ImVec2(ImGui::GetContentRegionAvail().x - 450, 32), true)) {
		ImGui::Text("%s", CustomString::ConcatenateArray(selectedElements).GetData());
		// Show selected items tooltip
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			for (int i = 0; i < selectedElements.size(); ++i) {
				ImGui::TextUnformatted(selectedElements[i].GetData());
			}
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
	
	ImGui::EndChild();
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 400);

	/** Extensions */
	char** extensionItems = new char*[extensionFilters.size()];
	for (int i = 0; i < extensionFilters.size(); ++i) {
		CustomString filtValue = CustomString::ConcatenateArray(extensionFilters[i]).GetData();
		extensionItems[i] = new char[filtValue.Length()];
		memcpy(extensionItems[i], filtValue.GetData(), filtValue.Length() + 1);
	}
	if (ImGui::BeginChild("extenshop_text", ImVec2(ImGui::GetContentRegionAvail().x, 40), true)) {
		ImGui::Text("Extensions");
		ImGui::SameLine();
		ImGui::Combo("", &currentFilter, extensionItems, (int)extensionFilters.size());
	}
	ImGui::EndChild();

	delete extensionItems;

	ImGui::Dummy(ImVec2(0, 10));
	/** No selection */
	if (withNoSel)
	{
		if (ImGui::Button("no selection", ImVec2(ImGui::GetContentRegionAvail().x - 600, 0))) {
			noSelection = true;
			selectedElements = {};
		}
	ImGui::SameLine();
	}
	/** Validate */
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - 500, 0));
	ImGui::SameLine();
	if (noSelection || (selectedElements.size() > 0)) {
		if (ImGui::Button("validate", ImVec2(320, 35))) {
			std::vector<std::string> elements;
			for (const auto& elem : selectedElements) elements.push_back(elem.GetData());
			OnApplyPath.Execute(elements);
			bHasBeenValidated = true;
			CloseView();
		}
	}
	ImGui::PopStyleVar();
}

FileExplorer::~FileExplorer()
{
	G_LAST_CONTENT_BROWSER_DIRECTORY = currentPath;
	OnDestroy.Execute ();
	if (!bHasBeenValidated) {
		OnCancelExplorer.Execute();
	}
}

void FileExplorer::SetCurrentPath(const CustomString& path)
{
	for (auto& chr : currentPath) chr = 0;
	memcpy(currentPath, path.GetData(), path.Length() < 256 ? path.Length() : 256);
	currentDirs.clear ();
}

void FileExplorer::UpdateCurrentPath()
{
	currentDirs.clear ();
	CustomString curDir(currentPath);
        for (const std::filesystem::directory_entry& elem : std::filesystem::directory_iterator(curDir.GetData()))
                currentDirs.push_back (elem);
        std::sort (currentDirs.begin (), currentDirs.end (), compNames);
	if (pruneSelected)
	{
		numUnselect.clear ();
		int i = 0;
		std::vector<std::filesystem::directory_entry>::iterator dit = currentDirs.begin ();
		while (dit != currentDirs.end ())
		{
			std::vector<std::string>::iterator it = alreadySelected.begin ();
			bool searching = true;
			while (searching && it != alreadySelected.end ())
			{
				if (dit->path() == *it)
				{
					numUnselect.push_back (i);
					searching = false;
				}
				it ++;
			}
			i ++;
			dit ++;
		}
	}
}

void FileExplorer::DrawDirContent(const CustomString& dirPath)
{
	if (!std::filesystem::exists(dirPath.GetData())) return;
	CustomString curDir(dirPath);

        for (const std::filesystem::directory_entry& elem : currentDirs)
	{
		if (elem.is_directory()) {
			CustomString elemPath = elem.path().u8string().c_str();
			CustomString Filename = CustomString::GetFileName(elemPath).GetData();
			if (ImGui::Button(elemPath.GetData(), ImVec2(ImGui::GetContentRegionAvail().x * 0.8f, 0))) {
				SetCurrentPath(elemPath);
			}
		}
	}
	UpdateCurrentPath();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.7f, .7f, .8f, .5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.8f, .8f, .9f, .7f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(.6f, .6f, .8f, .5f));
	int i = 0;
	std::vector<int>::iterator nit = numUnselect.begin ();
	int nextUnsel = (numUnselect.empty () ? -1 : *nit++);
	for (const std::filesystem::directory_entry& elem : currentDirs)
	{
		if (extensionFilters[currentFilter].size() > 0)
		{
			bool bfound = false;
			CustomString fileExt = CustomString::GetFileExtension(elem.path().u8string().c_str());
			for (const auto& ext : extensionFilters[currentFilter]) {
				if (fileExt == ext) {
					bfound = true;
				}
			}
			if (!bfound)
			{
				i++;
			 	continue;
			}
		}
		if (!elem.is_directory()) {

			CustomString elemPath = elem.path().u8string().c_str();
			CustomString Filename = CustomString::GetFileName(elemPath).GetData();
			bool but = true;
			if (pruneSelected)
			{
				if (i == nextUnsel)
				{
					ImGui::Text(Filename.GetData(), ImVec2(ImGui::GetContentRegionAvail().x * 0.8f, 0));
					nextUnsel = *nit++;
					but = false;
				}
			}
			if (but)
			{
				// Disable background on selected items
				bool isSelected = false;
				if (std::find(selectedElements.begin(), selectedElements.end(), elemPath) != selectedElements.end()) {
					isSelected = true;
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.7f, .7f, .8f, 0.1f));
				}

				if (ImGui::Button(Filename.GetData(), ImVec2(ImGui::GetContentRegionAvail().x * 0.8f, 0))) {
					if (isSelected) selectedElements.erase(std::find(selectedElements.begin(), selectedElements.end(), elemPath));
					else {
						if (itemsToSelect == 1) selectedElements.clear();
						selectedElements.push_back(elemPath);
					}
					noSelection = false;
				}

				if (isSelected) {
					ImGui::PopStyleColor();
				}
			}
		}
		i ++;
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

bool FileExplorer::compNames (std::filesystem::directory_entry &s1, std::filesystem::directory_entry &s2)
{
  return (s1.path().compare(s2.path())<0);
}

