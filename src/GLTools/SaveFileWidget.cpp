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

#include "imgui.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include "SaveFileWidget.h"


SaveFileWidget::SaveFileWidget(GLWindow* context, const std::string& windowName, const std::string& defaultPath, const std::string& elementName, const std::string inExtension, bool noadd)
	: AsImGuiWindow(context, windowName)
{
	SetCurrentPath(defaultPath.data());
	CustomString::ResetCharArray(currentElementName, sizeof(currentElementName));
	CustomString::ResetCharArray(currentExtension, sizeof(currentExtension));

	std::string outName = elementName;
	size_t count = 0;
	while (std::filesystem::exists(outName.data())) {
		count++;
		outName = elementName + "_" + std::to_string(count);
	}


	memcpy(currentElementName, outName.data(), outName.size());
	memcpy(currentExtension, inExtension.data(), inExtension.size());

	this->noadd = noadd;
}

void SaveFileWidget::DrawContent(GLWindow* windowContext)
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

	ImGui::Separator();

	bool bisFileValid = true;
	CustomString outPath = (selectedDirectory / CustomString(currentElementName) + "." + currentExtension);
	if (noadd && (std::filesystem::exists(outPath.GetData()) || selectedDirectory == ""))
	{
		bisFileValid = false;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.f, .2f, .2f, .5f));
	}
	if (ImGui::BeginChild("selected_element", ImVec2(ImGui::GetContentRegionAvail().x - 450, 40), true)) {
		ImGui::InputText("file name##fileName", currentElementName, sizeof(currentElementName));
	}
	if (!bisFileValid) ImGui::PopStyleColor();
	ImGui::EndChild();
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 400);
	if (ImGui::BeginChild("extensop_text", ImVec2(ImGui::GetContentRegionAvail().x, 40), true)) {
		/** Extensions */
		ImGui::InputText("extension", currentExtension, sizeof(currentExtension));
	}
	ImGui::EndChild();

	/** Validate */
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - 320, 0));
	ImGui::SameLine();
	if (bisFileValid) {
		if (ImGui::Button("validate", ImVec2(320, 35))) {
			OnApplyPath.Execute(outPath.GetData());
			bHasBeenValidated = true;
			CloseView();
		}
	}
	ImGui::PopStyleVar();
}

SaveFileWidget::~SaveFileWidget()
{
        OnDestroy.Execute ();
	if (!bHasBeenValidated) {
		OnCancelExplorer.Execute();
	}
}

void SaveFileWidget::SetCurrentPath(const CustomString& path)
{
	for (auto& chr : currentPath) chr = 0;
	memcpy(currentPath, path.GetData(), path.Length() < 256 ? path.Length() : 256);
	selectedDirectory = currentPath;
}

void SaveFileWidget::UpdateCurrentPath()
{
        currentDirs.clear ();
        CustomString curDir(currentPath);
        for (const std::filesystem::directory_entry& elem : std::filesystem::directory_iterator(curDir.GetData()))
                currentDirs.push_back (elem);
        std::sort (currentDirs.begin (), currentDirs.end (), compNames);
}

void SaveFileWidget::DrawDirContent(const CustomString& dirPath)
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

	for (const std::filesystem::directory_entry& elem : currentDirs)
	{
		if (!elem.is_directory()) {

			CustomString elemPath = elem.path().u8string().c_str();
			CustomString Filename = CustomString::GetFileName(elemPath).GetData();
			ImGui::Text("%s", Filename.GetData());
		}
	}
}

bool SaveFileWidget::compNames (std::filesystem::directory_entry &s1, std::filesystem::directory_entry &s2)
{
  return (s1.path().compare(s2.path())<0);
}
