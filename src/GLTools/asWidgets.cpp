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

#include <string>
#include <iostream>
#include "asWidgets.h"
#include "imgui.h"
#include "imgui_internal.h"

using namespace std;

bool AsWidgets::MenuCartesianSlider(const char* title, const char* shortcut, float value, int& result, bool* resetBool)
{
	result = 0;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, ImGui::GetStyle().ItemSpacing.y));
	if (ImGui::Button(string("<<##" + string(title)).data(), ImVec2((float)25, 20)))
	{
		result = -10;
	}
	ImGui::SameLine();

	if (ImGui::Button(string("<##" + string(title)).data(), ImVec2((float)50, 20)))
	{
		result = -1;
	}
	ImGui::SameLine();

	if (resetBool) {
		(*resetBool) = (ImGui::Button(string("o##" + string(title)).data(), ImVec2((float)50, 20)));
	}
	ImGui::SameLine();

	if (ImGui::Button(string(">##" + string(title)).data(), ImVec2((float)50, 20)))
	{
		result = 1;
	}
	ImGui::SameLine();

	if (ImGui::Button(string(">>##" + string(title)).data(), ImVec2((float)25, 20)))
	{
		result = 10;
	}
	ImGui::PopStyleVar();
	ImGui::SameLine();


	ImGui::Dummy(ImVec2(10, 0));
	ImGui::SameLine();

	std::string valueStr = to_string(value);
	valueStr = valueStr.erase(valueStr.find_last_not_of('0') + 1, string::npos);
	string text = string(valueStr + string(" | ") + title).data();
	ImGui::Text("%s", string(text.data() + string("                ")).c_str());

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImVec2 pos = window->DC.CursorPos;
	ImGuiStyle& style = g.Style;

	float label_size = 25 + 50 + 50 + 25 + 10 + ImGui::CalcTextSize(text.data()).x;
	float shortcut_w = shortcut ? ImGui::CalcTextSize(shortcut, NULL).x : 0.0f;
	float min_w = window->DC.MenuColumns.DeclColumns(label_size, shortcut_w, floor(g.FontSize * 1.20f));
	float extra_w = ImMax(0.0f, ImGui::GetContentRegionAvail().x - min_w);
	if (shortcut_w > 0.0f)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
		pos.x += window->DC.MenuColumns.Pos[1] + extra_w;
		pos.y -= 30;
		ImGui::RenderText(pos, shortcut, NULL, false);
		ImGui::PopStyleColor();
	}



	return result != 0 || (resetBool && *resetBool);
}

bool AsWidgets::MenuComboSwitcher(const char* baseTitle, const char* shortcut, std::vector<const char*> options, const int& currentSelectedOption, int& selectedOptionIndex)
{
	size_t optionCount = options.size();

	std::string titleString = std::string(baseTitle + std::string(" : [") + options[currentSelectedOption] + "]").c_str();

	const char* title = titleString.c_str();

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(title);
	bool menu_is_open = ImGui::IsPopupOpen(id);

	// Sub-menus are ChildWindow so that mouse can be hovering across them (otherwise top-most popup menu would steal focus and not allow hovering on parent menu)
	ImGuiWindowFlags flags = ImGuiWindowFlags_ChildMenu | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
	if (window->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildMenu))
		flags |= ImGuiWindowFlags_ChildWindow;

	// If a menu with same the ID was already submitted, we will append to it, matching the behavior of Begin().
	// We are relying on a O(N) search - so O(N log N) over the frame - which seems like the most efficient for the expected small amount of BeginMenu() calls per frame.
	// If somehow this is ever becoming a problem we can switch to use e.g. a ImGuiStorager mapping key to last frame used.
	if (g.MenusIdSubmittedThisFrame.contains(id))
	{
		if (menu_is_open)
			menu_is_open = ImGui::BeginPopupEx(id, flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
		else
			g.NextWindowData.ClearFlags();          // we behave like Begin() and need to consume those values
		return menu_is_open;
	}

	// Tag menu as used. Next time BeginMenu() with same ID is called it will append to existing menu
	g.MenusIdSubmittedThisFrame.push_back(id);

	ImVec2 label_size = ImGui::CalcTextSize(title, NULL, true);
	bool pressed;
	bool menuset_is_open = !(window->Flags & ImGuiWindowFlags_Popup) && (g.OpenPopupStack.Size > g.BeginPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].OpenParentId == window->IDStack.back());
	ImGuiWindow* backed_nav_window = g.NavWindow;
	if (menuset_is_open)
		g.NavWindow = window;  // Odd hack to allow hovering across menus of a same menu-set (otherwise we wouldn't be able to hover parent)

	// The reference position stored in popup_pos will be used by Begin() to find a suitable position for the child menu,
	// However the final position is going to be different! It is chosen by FindBestWindowPosForPopup().
	// e.g. Menus tend to overlap each other horizontally to amplify relative Z-ordering.
	ImVec2 popup_pos, pos = window->DC.CursorPos;		


	float Shortcut_label_size = 25 + 50 + 50 + 25 + 10 + ImGui::CalcTextSize(title).x;
	float shortcut_w = shortcut ? ImGui::CalcTextSize(shortcut, NULL).x : 0.0f;
	float min_w = window->DC.MenuColumns.DeclColumns(Shortcut_label_size, shortcut_w, floor(g.FontSize * 1.20f));
	float extra_w = ImMax(0.0f, ImGui::GetContentRegionAvail().x - min_w);
	if (shortcut_w > 0.0f)
	{
		ImVec2 ShortcutPos = pos;
		ImGui::PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
		ShortcutPos.x += window->DC.MenuColumns.Pos[1] + extra_w;
		ImGui::RenderText(ShortcutPos, shortcut, NULL, false);
		ImGui::PopStyleColor();
	}


	if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
	{
		// Menu inside an horizontal menu bar
		// Selectable extend their highlight by half ItemSpacing in each direction.
		// For ChildMenu, the popup position will be overwritten by the call to FindBestWindowPosForPopup() in Begin()
		popup_pos = ImVec2(pos.x - 1.0f - IM_FLOOR(style.ItemSpacing.x * 0.5f), pos.y - style.FramePadding.y + window->MenuBarHeight());
		window->DC.CursorPos.x += IM_FLOOR(style.ItemSpacing.x * 0.5f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
		float w = label_size.x;
		pressed = ImGui::Selectable(title, menu_is_open, ImGuiSelectableFlags_NoHoldingActiveID | ImGuiSelectableFlags_SelectOnClick | ImGuiSelectableFlags_DontClosePopups, ImVec2(w, 0.0f));

		ImGui::PopStyleVar();
		window->DC.CursorPos.x += IM_FLOOR(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
	}
	else
	{
		// Menu inside a menu
		// (In a typical menu window where all items are BeginMenu() or MenuItem() calls, extra_w will always be 0.0f.
		//  Only when they are other items sticking out we're going to add spacing, yet only register minimum width into the layout system.
		popup_pos = ImVec2(pos.x, pos.y - style.WindowPadding.y);
		float min_w = window->DC.MenuColumns.DeclColumns(label_size.x, 0.0f, IM_FLOOR(g.FontSize * 1.20f)); // Feedback to next frame
		float extra_w = ImMax(0.0f, ImGui::GetContentRegionAvail().x - min_w);
		pressed = ImGui::Selectable(title, menu_is_open, ImGuiSelectableFlags_NoHoldingActiveID | ImGuiSelectableFlags_SelectOnClick | ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_SpanAvailWidth, ImVec2(min_w, 0.0f));
		ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
		ImVec2 posBase = ImVec2(window->DC.MenuColumns.Pos[2] + extra_w + g.FontSize * 0.30f, 0.0f);
		ImGui::RenderArrow(window->DrawList, ImVec2(pos.x + posBase.x, pos.y + posBase.y), text_col, ImGuiDir_Right);
	}

	const bool hovered = ImGui::ItemHoverable(window->DC.LastItemRect, id);
	if (menuset_is_open)
		g.NavWindow = backed_nav_window;

	bool want_open = false;
	bool want_close = false;
	if (window->DC.LayoutType == ImGuiLayoutType_Vertical) // (window->Flags & (ImGuiWindowFlags_Popup|ImGuiWindowFlags_ChildMenu))
	{
		// Close menu when not hovering it anymore unless we are moving roughly in the direction of the menu
		// Implement http://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown to avoid using timers, so menus feels more reactive.
		bool moving_toward_other_child_menu = false;

		ImGuiWindow* child_menu_window = (g.BeginPopupStack.Size < g.OpenPopupStack.Size&& g.OpenPopupStack[g.BeginPopupStack.Size].SourceWindow == window) ? g.OpenPopupStack[g.BeginPopupStack.Size].Window : NULL;
		if (g.HoveredWindow == window && child_menu_window != NULL && !(window->Flags & ImGuiWindowFlags_MenuBar))
		{
			// FIXME-DPI: Values should be derived from a master "scale" factor.
			ImRect next_window_rect = child_menu_window->Rect();
			ImVec2 ta = ImVec2(g.IO.MousePos.x - g.IO.MouseDelta.x, g.IO.MousePos.y - g.IO.MouseDelta.y);
			ImVec2 tb = (window->Pos.x < child_menu_window->Pos.x) ? next_window_rect.GetTL() : next_window_rect.GetTR();
			ImVec2 tc = (window->Pos.x < child_menu_window->Pos.x) ? next_window_rect.GetBL() : next_window_rect.GetBR();
			float extra = ImClamp(ImFabs(ta.x - tb.x) * 0.30f, 5.0f, 30.0f);    // add a bit of extra slack.
			ta.x += (window->Pos.x < child_menu_window->Pos.x) ? -0.5f : +0.5f; // to avoid numerical issues
			tb.y = ta.y + ImMax((tb.y - extra) - ta.y, -100.0f);                // triangle is maximum 200 high to limit the slope and the bias toward large sub-menus // FIXME: Multiply by fb_scale?
			tc.y = ta.y + ImMin((tc.y + extra) - ta.y, +100.0f);
			moving_toward_other_child_menu = ImTriangleContainsPoint(ta, tb, tc, g.IO.MousePos);
			//GetForegroundDrawList()->AddTriangleFilled(ta, tb, tc, moving_within_opened_triangle ? IM_COL32(0,128,0,128) : IM_COL32(128,0,0,128)); // [DEBUG]
		}
		if (menu_is_open && !hovered && g.HoveredWindow == window && g.HoveredIdPreviousFrame != 0 && g.HoveredIdPreviousFrame != id && !moving_toward_other_child_menu)
			want_close = true;

		if (!menu_is_open && hovered && !moving_toward_other_child_menu) // Hover to open
			want_open = true;

		if (g.NavActivateId == id)
		{
			want_close = menu_is_open;
			want_open = !menu_is_open;
		}
		if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Right) // Nav-Right to open
		{
			want_open = true;
			ImGui::NavMoveRequestCancel();
		}
	}
	else
	{
		// Menu bar
		if (menu_is_open && pressed && menuset_is_open) // Click an open menu again to close it
		{
			want_close = true;
			want_open = menu_is_open = false;
		}
		else if (pressed || (hovered && menuset_is_open && !menu_is_open)) // First click to open, then hover to open others
		{
			want_open = true;
		}
		else if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Down) // Nav-Down to open
		{
			want_open = true;
			ImGui::NavMoveRequestCancel();
		}
	}
	if (want_close && ImGui::IsPopupOpen(id))
		ImGui::ClosePopupToLevel(g.BeginPopupStack.Size, true);

	IMGUI_TEST_ENGINE_ITEM_INFO(id, title, window->DC.ItemFlags | ImGuiItemStatusFlags_Openable | (menu_is_open ? ImGuiItemStatusFlags_Opened : 0));

	if (!menu_is_open && want_open && g.OpenPopupStack.Size > g.BeginPopupStack.Size)
	{
		// Don't recycle same menu level in the same frame, first close the other menu and yield for a frame.
		ImGui::OpenPopup(title);
		return false;
	}

	menu_is_open |= want_open;
	if (want_open)
		ImGui::OpenPopup(title);

	if (menu_is_open)
	{
		ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Always);
		menu_is_open = ImGui::BeginPopupEx(id, flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
	}
	else
	{
		g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
	}


	if (menu_is_open)
	{
 		for (int i = 0; i < optionCount; ++i)
		{
			if (ImGui::MenuItem(options[i], NULL, currentSelectedOption == i))
			{
				selectedOptionIndex = i;
				ImGui::EndMenu();
				return true;
			}
		}
		ImGui::EndMenu();
	}
	if (pressed)
	{
		selectedOptionIndex = (currentSelectedOption + 1) % optionCount;
		std::cout << selectedOptionIndex << std::endl;
		return true;
	}

	return false;
}
