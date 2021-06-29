/*  Copyright 2021 Philippe Even, Phuc Ngo and Pierre Even,
      co-authors of paper:
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

#include <iostream>
#include "ilsdmenu.h"
#include "ilsddetectionwidget.h"
#include "ilsditemcontrol.h"
#include "ctrackdetector.h"
#include "ridgedetector.h"
#include "terrainmap.h"
#include "asmath.h"

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "asWidgets.h"
#include "glWindow.h"
#include "asPainter.h"
#include "IniLoader.h"
#include "FileExplorer.h"
#include "SaveFileWidget.h"

#define DEFAULT_SETTING_DIR std::string("./config/")
#define DEFAULT_SETTING_FILE std::string("contextASD")
#define THIS_DIR std::string("./")
#define DEFAULT_TILE_NAME std::string("")
#define NVM_DIR std::string("./nvm/")
#define TIL_DIR std::string("./til/")
#define DEFAULT_STROKE_DIR std::string("./tests/")
#define DEFAULT_EXPORT_DIR std::string("./exports/")
#define DEFAULT_SELECTION_DIR std::string("./selections/")
#define DEFAULT_SELECTION_FILE std::string("last_selection")
#define DEFAULT_STRUCTURE_FILE std::string("last_structure")
#define DEFAULT_EXPORT_FILE std::string("last_export")
#define DEFAULT_MEASURE_FILE std::string("last_measure")
#define SHAPE_SUFFIX std::string("shp")
#define STRUCTURE_SUFFIX std::string("asd")
#define MEASURE_SUFFIX std::string("msr")
#define SCUTPOS 420


ILSDMenu::ILSDMenu ()
{
  det_widget = NULL;
  dtm_map = NULL;
  ictrl = NULL;
  tdetector = NULL;
  rdetector = NULL;
  showDemoWindow = false;
  import_parent = NULL;
}


void ILSDMenu::setDetectionWidget (ILSDDetectionWidget *widget)
{
  det_widget = widget;
  dtm_map = det_widget->getDtmMap ();
  ictrl = det_widget->getAnalysisController ();
  tdetector = det_widget->getCTrackDetector ();
  rdetector = det_widget->getRidgeDetector ();
}


void ILSDMenu::drawUI (GLWindow* parentWindow)
{
  int sizeX, sizeY;
  glfwGetWindowSize (parentWindow->getGlfwContext (), &sizeX, &sizeY);

  if (ImGui::BeginMainMenuBar ())
  {
    if (drawFileMenu (parentWindow))
    {
      drawDisplayMenu (parentWindow);
      drawSelectionMenu (parentWindow);
      drawDetectionMenu (parentWindow);
      drawCrossProfileMenu (parentWindow);
      drawLongProfileMenu (parentWindow);
    }
//    drawPrefMenu (parentWindow);
    drawMouseInfo (parentWindow, sizeX);
    ImGui::EndMainMenuBar ();
  }
  drawSelectionInfo (parentWindow, sizeY);
  if (showDemoWindow) ImGui::ShowDemoWindow (&showDemoWindow);
}


bool ILSDMenu::drawFileMenu (GLWindow* parent)
{
  if (ImGui::BeginMenu ("Files"))
  {
    {
      int newclac = det_widget->cloudAccess ();
      ImGui::Text ("Cloud access:");
      ImGui::SameLine ();
      if (ImGui::RadioButton ("fast", newclac == IPtTile::TOP))
        newclac = IPtTile::TOP;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("medium", newclac == IPtTile::MID))
        newclac = IPtTile::MID;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("eco", newclac == IPtTile::ECO))
        newclac = IPtTile::ECO;
      ImGui::SameLine (SCUTPOS);
      ImGui::TextDisabled ("Ctrl B");
      if (newclac != det_widget->cloudAccess ())
        det_widget->setCloudAccess (newclac);
    }
    if (ImGui::MenuItem ("Select tiles"))
    {
      det_widget->disableKeys ();
      FileExplorer* explo = new FileExplorer (parent, "Select tiles",
                                              NVM_DIR, { "nvm" }, false, 0);
      explo->OnApplyPath.Add (det_widget, &ILSDDetectionWidget::selectTiles);
      explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
      explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
    }
    if (ImGui::MenuItem ("Import tile"))
    {
      import_parent = parent;
      det_widget->disableKeys ();
      FileExplorer* explo = new FileExplorer (parent,
            				      "Select XYZ point tile", ".");
      explo->OnApplyPath.Add (this, &ILSDMenu::importPointTile);
      explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
      explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
    }
    ImGui::Separator();

    if (det_widget->tilesLoaded ())
    {
      if (ImGui::MenuItem ("Save settings"))
      {
        det_widget->disableKeys ();
        SaveFileWidget* explo = new SaveFileWidget (
              parent, "Save settings",
              DEFAULT_SETTING_DIR, DEFAULT_SETTING_FILE, "ini");
        explo->OnApplyPath.Add (det_widget,
                                &ILSDDetectionWidget::saveSettings);
        explo->OnCancelExplorer.Add (det_widget,
                                     &ILSDDetectionWidget::noAction);
        explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
      }

      if (ImGui::MenuItem ("Load settings"))
      {
        det_widget->disableKeys ();
        FileExplorer* explo = new FileExplorer (
              parent, "Load settings", "./config", { "ini" });
        explo->OnApplyPath.Add (det_widget,
                                &ILSDDetectionWidget::loadSettings);
        explo->OnCancelExplorer.Add (det_widget,
                                     &ILSDDetectionWidget::noAction);
        explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
      }
      ImGui::Separator();

      if (ImGui::MenuItem ("Save screenshot"))
      {
        det_widget->disableKeys ();
        SaveFileWidget* explo = new SaveFileWidget (
              parent, "Save screenshot", "./captures", "capture", "png");
        explo->OnApplyPath.Add (det_widget,
                                &ILSDDetectionWidget::saveScreen);
        explo->OnCancelExplorer.Add (det_widget,
                                     &ILSDDetectionWidget::noAction);
        explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
      }
      ImGui::Separator();
    }

    if (ImGui::MenuItem ("Close", "Escape"))
    {
      glfwSetWindowShouldClose (parent->getGlfwContext (), 1);
    }

    ImGui::EndMenu ();
  }
  return (det_widget->tilesLoaded ());
}


void ILSDMenu::drawDisplayMenu (GLWindow* parent)
{
  if (ImGui::BeginMenu ("Display"))
  {
    int newbgnd = det_widget->getBackground ();
    ImGui::Text ("Background:");
    ImGui::SameLine ();
    if (ImGui::RadioButton ("image", newbgnd == 2)) newbgnd = 2;
    ImGui::SameLine ();
    if (ImGui::RadioButton ("black", newbgnd == 0)) newbgnd = 0;
    ImGui::SameLine ();
    if (ImGui::RadioButton ("white", newbgnd == 1)) newbgnd = 1;
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl B");
    if (newbgnd != det_widget->getBackground ())
    {
      det_widget->setBackground (newbgnd);
      det_widget->display ();
    }
    ImGui::Separator ();

    {
      bool status = det_widget->isDisplayTileOn ();
      if (ImGui::Checkbox ("Tile border displayed", &status))
      {
        if (status != det_widget->isDisplayTileOn ())
        {
          det_widget->switchDisplayTile ();
          det_widget->display ();
        }
      }
      ImGui::SameLine (SCUTPOS);
      ImGui::TextDisabled ("/");
    }

    if (det_widget->mode () != ILSDDetectionWidget::MODE_NONE)
    {
      bool status = det_widget->isDisplayDetectionOn ();
      if (ImGui::Checkbox ("Detected structure displayed", &status))
      {
        if (status != det_widget->isDisplayDetectionOn ())
        {
          det_widget->switchDisplayDetection ();
          det_widget->display ();
        }
      }
      ImGui::SameLine (SCUTPOS);
      ImGui::TextDisabled ("(");
    }

    if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
    {
      {
        bool status = det_widget->isPlateauSmoothedOn ();
        if (ImGui::Checkbox ("Only accepted profiles displayed", &status))
        {
          if (status != det_widget->isPlateauSmoothedOn ())
          {
            det_widget->switchPlateauSmoothed ();
            det_widget->display ();
          }
        }
        ImGui::SameLine (SCUTPOS);
        ImGui::TextDisabled ("!");
      }
    }
    else if (det_widget->mode () & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
    {
      {
        bool status = det_widget->isBumpSmoothedOn ();
        if (ImGui::Checkbox ("Only accepted profiles displayed##2", &status))
        {
          if (status != det_widget->isBumpSmoothedOn ())
          {
            det_widget->switchBumpSmoothed ();
            det_widget->display ();
          }
        }
        ImGui::SameLine (SCUTPOS);
        ImGui::TextDisabled ("!");
      }
    }

    if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
    {
      int newstyle = det_widget->getTrackStyle ();
      ImGui::Text ("Track style:");
      ImGui::SameLine ();
      if (ImGui::RadioButton ("disconnected##8", newstyle == 0)) newstyle = 0;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("connected##2", newstyle == 1)) newstyle = 1;
      ImGui::SameLine (SCUTPOS);
      ImGui::TextDisabled ("*");
      if (newstyle != det_widget->getTrackStyle ())
      {
        det_widget->setTrackStyle (newstyle);
        det_widget->display ();
      }
    }
    else if (det_widget->mode () & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
    {
      int newstyle = det_widget->getRidgeStyle ();
      ImGui::Text ("Ridge style:");
      ImGui::SameLine ();
      if (ImGui::RadioButton ("disconnected##1", newstyle == 0)) newstyle = 0;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("connected##1", newstyle == 1)) newstyle = 1;
      ImGui::SameLine (SCUTPOS);
      ImGui::TextDisabled ("*");
      ImGui::Text (" ");
      ImGui::SameLine (80);
      if (ImGui::RadioButton ("bounds", newstyle == 2)) newstyle = 2;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("summit", newstyle == 3)) newstyle = 3;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("center", newstyle == 4)) newstyle = 4;
      if (newstyle != det_widget->getRidgeStyle ())
      {
        det_widget->setRidgeStyle (newstyle);
        det_widget->display ();
      }
    }

    {
      int newstyle = det_widget->getSelectionStyle ();
      ImGui::Text ("Selection style:");
      ImGui::SameLine ();
      if (ImGui::RadioButton ("thin", newstyle == 1)) newstyle = 1;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("thick", newstyle == 2)) newstyle = 2;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("hidden", newstyle == 0)) newstyle = 0;
      ImGui::SameLine (SCUTPOS);
      ImGui::TextDisabled (".");
      if (newstyle != det_widget->getSelectionStyle ())
      {
        det_widget->setSelectionStyle (newstyle);
        det_widget->display ();
      }
    }
    ImGui::Separator ();

    {
      int newstyle = dtm_map->shadingType ();
      ImGui::Text ("Shading:");
      ImGui::SameLine ();
      if (ImGui::RadioButton ("hill", newstyle == 0)) newstyle = 0;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("slope", newstyle == 1)) newstyle = 1;
      ImGui::SameLine (SCUTPOS);
      ImGui::TextDisabled ("Ctrl V");
      if (newstyle != dtm_map->shadingType ())
      {
        dtm_map->toggleShadingType ();
        det_widget->rebuildImage ();
        det_widget->display ();
      }
    }
    {
      int newlevel = det_widget->getBlackLevel ();
      ImGui::SliderInt ("Light intensity (B)", &newlevel, 0, 200);
      if (newlevel != det_widget->getBlackLevel ())
      {
        det_widget->setBlackLevel (newlevel);
        det_widget->rebuildImage ();
        det_widget->display();
      }
    }
    if (dtm_map->shadingType () != TerrainMap::SHADE_SLOPE)
    {
      float newLightAngle = dtm_map->lightAngle () * (float) ASF_RAD2DEG;
      ImGui::SliderFloat ("Light direction (V)", &newLightAngle, 0.f, 360.f);
      if (newLightAngle != dtm_map->lightAngle () * (float) ASF_RAD2DEG)
      {
        dtm_map->setLightAngle (newLightAngle * (float) ASF_DEG2RAD);
        det_widget->rebuildImage ();
        det_widget->display ();
      }
    }
    ImGui::Separator ();

    {
      int newZoom = det_widget->getZoom ();
      ImGui::SliderInt ("Zoom level", &newZoom, -10, 10);
      if (newZoom != det_widget->getZoom ())
      {
        det_widget->setZoom (newZoom);
      }
    }
    {
      int pX = det_widget->getXShift () + det_widget->getWidth () / 2;
      ImGui::SliderInt ("Translation X", &pX, 0, det_widget->getWidth ());
      if (pX != det_widget->getXShift () + det_widget->getWidth () / 2)
      {
        det_widget->setXShift (pX - det_widget->getWidth () / 2);
      }
    }
    {
      int pY = det_widget->getYShift () + det_widget->getHeight () / 2;
      ImGui::SliderInt ("Translation Y", &pY, 0, det_widget->getHeight ());
      if (pY != det_widget->getYShift () + det_widget->getHeight () / 2)
      {
        det_widget->setYShift (pY - det_widget->getHeight () / 2);
      }
    }
    ImGui::EndMenu ();
  }
}


void ILSDMenu::drawSelectionMenu (GLWindow* parent)
{
  if (ImGui::BeginMenu ("Selection"))
  {
    if (ImGui::MenuItem ("Replay last", "   U"))
    {
      det_widget->detectAndDisplay ();
    }
    if (ImGui::MenuItem ("Revert last", "   :"))
    {
      det_widget->invertInputStroke ();
      std::cout << "P1 <-> P2" << endl;
    }
    if (ImGui::MenuItem ("Save current stroke", "   Enter", false,
                         det_widget->isInputStrokeValid ()))
    {
      det_widget->disableKeys ();
      SaveFileWidget* explo = new SaveFileWidget (
            parent, "Save stroke", DEFAULT_STROKE_DIR, "test", "txt");
      explo->OnApplyPath.Add (det_widget, &ILSDDetectionWidget::saveStroke);
      explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
      explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
    }
    if (ImGui::MenuItem ("Load/run stroke", "   0"))
    {
      det_widget->disableKeys ();
      FileExplorer* explo = new FileExplorer (
            parent, "Load stroke", DEFAULT_STROKE_DIR, { "txt" });
      explo->OnApplyPath.Add (det_widget, &ILSDDetectionWidget::loadStroke);
      explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
      explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
    }
    ImGui::Separator ();

    if (ImGui::MenuItem ("Save structure", "  ", false,
                         det_widget->isInputStrokeValid ()))
    {
      string dirname (DEFAULT_SELECTION_DIR);
      if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        dirname += string ("ctracks");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_RIDGE)
        dirname += string ("ridges");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_HOLLOW)
        dirname += string ("hollows");
      det_widget->disableKeys ();
      SaveFileWidget* explo = new SaveFileWidget (
            parent, "Save structure", dirname,
            DEFAULT_STRUCTURE_FILE, STRUCTURE_SUFFIX, false);
      explo->OnApplyPath.Add (det_widget, &ILSDDetectionWidget::saveStructure);
      explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
      explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
    }
    if (ImGui::MenuItem ("Load structure", "  "))
    {
      det_widget->setSelectionDisplay (true);
      string dirname (DEFAULT_SELECTION_DIR);
      if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        dirname += string ("ctracks");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_RIDGE)
        dirname += string ("ridges");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_HOLLOW)
        dirname += string ("hollows");
      det_widget->disableKeys ();
      FileExplorer* explo = new FileExplorer (
            parent, "Load structure", dirname, { STRUCTURE_SUFFIX });
      explo->OnApplyPath.Add (det_widget, &ILSDDetectionWidget::loadStructure);
      explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
      explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
    }
    if (ImGui::MenuItem ("Export to shape", "   '"))
    {
      string dirname (DEFAULT_EXPORT_DIR);
      det_widget->disableKeys ();
      SaveFileWidget* explo = new SaveFileWidget (
            parent, "Export shape", dirname,
            DEFAULT_EXPORT_FILE, SHAPE_SUFFIX, false);
      explo->OnApplyPath.Add (det_widget, &ILSDDetectionWidget::exportShape);
      explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
      explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
    }
    ImGui::Separator ();

    if (ImGui::MenuItem ("Save measure", "  ", false,
                 det_widget->isCrossProfileVisible () && ictrl->isMeasuring ()))
    {
      string dirname (DEFAULT_SELECTION_DIR);
      if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        dirname += string ("ctracks");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_RIDGE)
        dirname += string ("ridges");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_HOLLOW)
        dirname += string ("hollows");
      det_widget->disableKeys ();
      SaveFileWidget* explo = new SaveFileWidget (
            parent, "Save measure", dirname,
            DEFAULT_MEASURE_FILE, MEASURE_SUFFIX, false);
      explo->OnApplyPath.Add (det_widget, &ILSDDetectionWidget::saveMeasure);
      explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
      explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
    }
    if (ImGui::MenuItem ("Load measure", "  "))
    {
      det_widget->setSelectionDisplay (true);
      string dirname (DEFAULT_SELECTION_DIR);
      if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        dirname += string ("ctracks");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_RIDGE)
        dirname += string ("ridges");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_HOLLOW)
        dirname += string ("hollows");
      det_widget->disableKeys ();
      FileExplorer* explo = new FileExplorer (parent,
        "Load measure", dirname, { MEASURE_SUFFIX });
      explo->OnApplyPath.Add (det_widget, &ILSDDetectionWidget::loadMeasure);
      explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
      explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
    }
    ImGui::Separator();

    if (ImGui::MenuItem ("Add to selection", "   a", false,
                         det_widget->isInputStrokeValid ()))
    {
      string dirname (DEFAULT_SELECTION_DIR);
      if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        dirname += string ("ctracks");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_RIDGE)
        dirname += string ("ridges");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_HOLLOW)
        dirname += string ("hollows");
      det_widget->disableKeys ();
      SaveFileWidget* explo = new SaveFileWidget (
            parent, "Add to selection", dirname,
            DEFAULT_SELECTION_FILE, "txt", false);
      explo->OnApplyPath.Add (det_widget, &ILSDDetectionWidget::addToSelection);
      explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
      explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
    }
    if (ImGui::MenuItem ("Load selection", "   A"))
    {
      det_widget->setSelectionDisplay (true);
      string dirname (DEFAULT_SELECTION_DIR);
      if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        dirname += string ("ctracks");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_RIDGE)
        dirname += string ("ridges");
      else if (det_widget->mode () == ILSDDetectionWidget::MODE_HOLLOW)
        dirname += string ("hollows");
      det_widget->disableKeys ();
      FileExplorer* explo = new FileExplorer (
            parent, "Load selection", dirname, { "txt" });
      explo->OnApplyPath.Add (det_widget, &ILSDDetectionWidget::loadSelection);
      explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
      explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
    }
    if (ImGui::MenuItem ("Clear out selection", "   A", false,
                         det_widget->getSelectionDisplay ()))
    {
      det_widget->setSelectionDisplay (false);
      det_widget->display ();
    }
    ImGui::EndMenu ();
  }
}


void ILSDMenu::drawDetectionMenu (GLWindow* parent)
{
  if (ImGui::BeginMenu ("Detection"))
  {
    {
      int newmode = det_widget->mode ();
      ImGui::Text ("Structure:");
      ImGui::SameLine ();
      if (ImGui::RadioButton ("ridge",
                det_widget->mode () == ILSDDetectionWidget::MODE_RIDGE))
        newmode = ILSDDetectionWidget::MODE_RIDGE;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("hollow",
                det_widget->mode () == ILSDDetectionWidget::MODE_HOLLOW))
        newmode = ILSDDetectionWidget::MODE_HOLLOW;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("track",
                det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK))
        newmode = ILSDDetectionWidget::MODE_CTRACK;
      ImGui::SameLine ();
      if (ImGui::RadioButton ("none",
                det_widget->mode () == ILSDDetectionWidget::MODE_NONE))
        newmode = ILSDDetectionWidget::MODE_NONE;
      ImGui::SameLine (SCUTPOS);
      ImGui::TextDisabled ("Ctrl M");
      if (newmode != det_widget->mode ())
      {
        det_widget->setTitleChanged (true);
        det_widget->setDetectionMode (newmode);
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::Separator ();

    if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
      drawCTrackDetectionSubmenu (parent);
    else if (det_widget->mode () & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
      drawRidgeDetectionSubmenu (parent);

    ImGui::EndMenu ();
  }
}


void ILSDMenu::drawCTrackDetectionSubmenu (GLWindow* parent)
{
  {
    bool status = tdetector->isInitializationOn ();
    if (ImGui::Checkbox ("Pre-detection", &status))
    {
      if (status != tdetector->isInitializationOn ())
      {
        tdetector->switchInitialization ();
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl P");
  }

  {
    bool status = tdetector->isDensitySensitive ();
    if (ImGui::Checkbox ("Plateau density test", &status))
    {
      if (status != tdetector->isDensitySensitive ())
      {
        tdetector->switchDensitySensitivity ();
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl R");
  }

  {
    int newTilt = tdetector->model()->bsMaxTilt ();
    ImGui::SliderInt ("Plateau max tilt % (E)", &newTilt, 1, 60);
    if (newTilt != tdetector->model()->bsMaxTilt ())
    {
      tdetector->model()->setBSmaxTilt (newTilt);
      det_widget->detectAndDisplay ();
    }
  }

  {
    float newMinLength = tdetector->model()->minLength ();
    ImGui::SliderFloat ("Plateau minimal length (L)", &newMinLength,
                        PlateauModel::MIN_MIN_LENGTH_TOLERANCE,
                        PlateauModel::MAX_MIN_LENGTH_TOLERANCE);
    if (newMinLength != tdetector->model()->minLength ())
    {
      tdetector->model()->setMinLength (newMinLength);
      det_widget->detectAndDisplay ();
    }
  }

  {
    float newMaxLength = tdetector->model()->maxLength ();
    ImGui::SliderFloat ("Plateau maximal length (M)", &newMaxLength,
                        PlateauModel::MIN_MAX_LENGTH_TOLERANCE,
                        PlateauModel::MAX_MAX_LENGTH_TOLERANCE);
    if (newMaxLength != tdetector->model()->maxLength ())
    {
      tdetector->model()->setMaxLength (newMaxLength);
      det_widget->detectAndDisplay ();
    }
  }

  {
    float newThickness = tdetector->model()->thicknessTolerance ();
    ImGui::SliderFloat ("Max thickness shift (T)", &newThickness,
                        PlateauModel::MIN_THICKNESS_TOLERANCE,
                        PlateauModel::MAX_THICKNESS_TOLERANCE);
    if (newThickness != tdetector->model()->thicknessTolerance ())
    {
      tdetector->model()->setThicknessTolerance (newThickness);
      det_widget->detectAndDisplay ();
    }
  }

  {
    float newSideShift = tdetector->model()->sideShiftTolerance ();
    ImGui::SliderFloat ("Max position shift (W)", &newSideShift,
                        PlateauModel::MIN_SIDE_SHIFT_TOLERANCE,
                        PlateauModel::MAX_SIDE_SHIFT_TOLERANCE);
    if (newSideShift != tdetector->model()->sideShiftTolerance ())
    {
      tdetector->model()->setSideShiftTolerance (newSideShift);
      det_widget->detectAndDisplay ();
    }
  }

  {
    float newSlope = tdetector->model()->slopeTolerance ();
    ImGui::SliderFloat ("Max altitude shift (Z)", &newSlope,
                        PlateauModel::MIN_SLOPE_TOLERANCE,
                        PlateauModel::MAX_SLOPE_TOLERANCE);
    if (newSlope != tdetector->model()->slopeTolerance ())
    {
      tdetector->model()->setSlopeTolerance (newSlope);
      det_widget->detectAndDisplay ();
    }
  } 
  ImGui::Separator ();

  {
    bool status = tdetector->model()->isDeviationPredictionOn ();
    if (ImGui::Checkbox ("Direction aware##1", &status))
    {
      if (status != tdetector->model()->isDeviationPredictionOn ())
      {
        tdetector->model()->switchDeviationPrediction ();
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl D");
    if (ImGui::IsItemHovered ())
      ImGui::SetTooltip ("Helps to localize next plateau");
  }

  {
    bool status = tdetector->model()->isSlopePredictionOn ();
    if (ImGui::Checkbox ("Slope aware##1", &status))
    {
      if (status != tdetector->model()->isSlopePredictionOn ())
      {
        tdetector->model()->switchSlopePrediction ();
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl S");
    if (ImGui::IsItemHovered ())
      ImGui::SetTooltip ("Helps to localize next plateau");
  }

  {
    int newMissCount = tdetector->getPlateauLackTolerance ();
    ImGui::SliderInt ("Plateau lack tolerance (Q)", &newMissCount, 0, 50);
    if (newMissCount != tdetector->getPlateauLackTolerance ())
    {
      tdetector->setPlateauLackTolerance(newMissCount);
      det_widget->detectAndDisplay ();
    }
  }
  ImGui::Separator ();

  {
    bool status = tdetector->isShiftLengthPruning ();
    if (ImGui::Checkbox ("Plateau stability control", &status))
    {
      if (status != tdetector->isShiftLengthPruning ())
      {
        tdetector->switchShiftLengthPruning ();
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl F");
  }
  {
    int outputVal;
    if (AsWidgets::MenuCartesianSlider (
          "Max cumulated shift", "F / SHIFT+F",
          (float) (tdetector->maxShiftLength ()), outputVal))
    {
      tdetector->incMaxShiftLength (outputVal);
      det_widget->detectAndDisplay ();
    }
  }

  {
    bool status = tdetector->isDensityPruning ();
    if (ImGui::Checkbox ("Ridge density control", &status))
    {
      if (status != tdetector->isDensityPruning ())
      {
        tdetector->switchDensityPruning ();
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl K");
  }
  {
    int outputVal;
    if (AsWidgets::MenuCartesianSlider (
          "Min ridge density", "K / SHIFT+K",
          (float) (tdetector->minDensity ()), outputVal))
    {
      tdetector->incMinDensity (outputVal);
      det_widget->detectAndDisplay ();
    }
  }

  {
    bool status = (tdetector->tailPruning () != 0);
    if (ImGui::Checkbox ("Sparse tail pruning", &status))
    {
      if (status != (tdetector->tailPruning () != 0))
      {
        tdetector->switchTailPruning ();
        if (status != (tdetector->tailPruning () != 0))
          tdetector->switchTailPruning ();
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl N");
  }
  {
    int outputVal;
    if (AsWidgets::MenuCartesianSlider (
          "Tail minimal size", "N / SHIFT+N",
          (float) (tdetector->model()->tailMinSize ()), outputVal))
    {
      tdetector->model()->incTailMinSize (outputVal);
      det_widget->detectAndDisplay ();
    }
  }
}


void ILSDMenu::drawRidgeDetectionSubmenu (GLWindow* parent)
{
  {
    bool status = rdetector->model()->isDetectingTrend ();
    if (ImGui::Checkbox ("Trend detection", &status))
    {
      if (status != rdetector->model()->isDetectingTrend ())
      {
        rdetector->model()->switchDetectingTrend ();
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl T");
    if (ImGui::IsItemHovered ())
      ImGui::SetTooltip ("For enhanced bump localization");
  }
  {
    int newPinch = rdetector->model()->trendMinPinch ();
    ImGui::SliderInt ("Trend roughness (T)", &newPinch,
                      BumpModel::MIN_TREND_MIN_PINCH,
                      BumpModel::MAX_TREND_MIN_PINCH);
    if (newPinch != rdetector->model()->trendMinPinch ())
    {
      rdetector->model()->setTrendMinPinch (newPinch);
      det_widget->detectAndDisplay ();
    }
  }
  ImGui::Separator ();

  {
    bool status = rdetector->model()->massReferenced ();
    if (ImGui::Checkbox ("Mass-center referenced", &status))
    {
      if (status != rdetector->model()->massReferenced ())
      {
        rdetector->model()->switchCenterReference ();
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("$");
    if (ImGui::IsItemHovered ())
      ImGui::SetTooltip ("surface center otherwise");
  }

  {
    int newmode = rdetector->model()->positionControl ();
    ImGui::Text ("Position control:");
    ImGui::SameLine ();
    if (ImGui::RadioButton ("absolute##1", newmode == 1)) newmode = 1;
    ImGui::SameLine ();
    if (ImGui::RadioButton ("relative##1", newmode == 2)) newmode = 2;
    ImGui::SameLine ();
    if (ImGui::RadioButton ("unchecked##1", newmode == 0)) newmode = 0;
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl Y");
    if (newmode != rdetector->model()->positionControl ())
    {
      rdetector->model()->setPositionControl(newmode);
      det_widget->detectAndDisplay ();
    }
  }

  if (rdetector->model()->positionControl () == 1)
  {
    float newPositionShift = rdetector->model()->positionShiftTolerance ();
    ImGui::SliderFloat ("Maximal position shift (Y)", &newPositionShift,
                        BumpModel::MIN_SHIFT, BumpModel::MAX_SHIFT);
    if (newPositionShift != rdetector->model()->positionShiftTolerance ())
    {
      rdetector->model()->setPositionShiftTolerance (newPositionShift);
      det_widget->detectAndDisplay ();
    }
  }
  else if (rdetector->model()->positionControl () == 2)
  {
    float newPositionRShift = rdetector->model()->positionRelShiftTolerance ();
    ImGui::SliderFloat ("Max position shift / width (Y)", &newPositionRShift,
                        BumpModel::MIN_REL_SHIFT, BumpModel::MAX_REL_SHIFT);
    if (newPositionRShift != rdetector->model()->positionRelShiftTolerance ())
    {
      rdetector->model()->setPositionRelShiftTolerance (newPositionRShift);
      det_widget->detectAndDisplay ();
    }
  }

  {
    int newmode = rdetector->model()->altitudeControl ();
    ImGui::Text ("Altitude control:");
    ImGui::SameLine ();
    if (ImGui::RadioButton ("absolute##2", newmode == 1)) newmode = 1;
    ImGui::SameLine ();
    if (ImGui::RadioButton ("relative##2", newmode == 2)) newmode = 2;
    ImGui::SameLine ();
    if (ImGui::RadioButton ("unchecked##2", newmode == 0)) newmode = 0;
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl Z");
    if (newmode != rdetector->model()->altitudeControl ())
    {
      rdetector->model()->setAltitudeControl (newmode);
      det_widget->detectAndDisplay ();
    }
  }

  if (rdetector->model()->altitudeControl () == 1)
  {
    float newAltiShift = rdetector->model()->altitudeShiftTolerance ();
    ImGui::SliderFloat ("Maximal altitude shift (Z)", &newAltiShift,
                        BumpModel::MIN_SHIFT, BumpModel::MAX_SHIFT);
    if (newAltiShift != rdetector->model()->altitudeShiftTolerance ())
    {
      rdetector->model()->setAltitudeShiftTolerance (newAltiShift);
      det_widget->detectAndDisplay ();
    }
  }
  else if (rdetector->model()->altitudeControl () == 2)
  {
    float newAltitudeRShift = rdetector->model()->altitudeRelShiftTolerance ();
    ImGui::SliderFloat ("Max altitude shift / height (Z)", &newAltitudeRShift,
                        BumpModel::MIN_REL_SHIFT, BumpModel::MAX_REL_SHIFT);
    if (newAltitudeRShift != rdetector->model()->altitudeRelShiftTolerance ())
    {
      rdetector->model()->setAltitudeRelShiftTolerance (newAltitudeRShift);
      det_widget->detectAndDisplay ();
    }
  }

  {
    int newmode = rdetector->model()->widthControl ();
    ImGui::Text ("Width control:");
    ImGui::SameLine ();
    if (ImGui::RadioButton ("absolute##3", newmode == 1)) newmode = 1;
    ImGui::SameLine ();
    if (ImGui::RadioButton ("relative##3", newmode == 2)) newmode = 2;
    ImGui::SameLine ();
    if (ImGui::RadioButton ("unchecked##3", newmode == 0)) newmode = 0;
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl W");
    if (newmode != rdetector->model()->widthControl ())
    {
      rdetector->model()->setWidthControl (newmode);
      det_widget->detectAndDisplay ();
    }
  }

  if (rdetector->model()->widthControl () == 1)
  {
    float newWidthShift = rdetector->model()->widthShiftTolerance ();
    ImGui::SliderFloat ("Maximal width shift (W)", &newWidthShift,
                        BumpModel::MIN_SHIFT, BumpModel::MAX_SHIFT);
    if (newWidthShift != rdetector->model()->widthShiftTolerance ())
    {
      rdetector->model()->setWidthShiftTolerance (newWidthShift);
      det_widget->detectAndDisplay ();
    }
  }
  else if (rdetector->model()->widthControl () == 2)
  {
    float newWidthRShift = rdetector->model()->widthRelShiftTolerance ();
    ImGui::SliderFloat ("Max width shift / width (W)", &newWidthRShift,
                        BumpModel::MIN_REL_SHIFT, BumpModel::MAX_REL_SHIFT);
    if (newWidthRShift != rdetector->model()->widthRelShiftTolerance ())
    {
      rdetector->model()->setWidthRelShiftTolerance (newWidthRShift);
      det_widget->detectAndDisplay ();
    }
  }

  {
    int newmode = rdetector->model()->heightControl ();
    ImGui::Text ("Height control:");
    ImGui::SameLine ();
    if (ImGui::RadioButton ("absolute##4", newmode == 1)) newmode = 1;
    ImGui::SameLine ();
    if (ImGui::RadioButton ("relative##4", newmode == 2)) newmode = 2;
    ImGui::SameLine ();
    if (ImGui::RadioButton ("unchecked##4", newmode == 0)) newmode = 0;
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl H");
    if (newmode != rdetector->model()->heightControl ())
    {
      rdetector->model()->setHeightControl (newmode);
      det_widget->detectAndDisplay ();
    }
  }

  if (rdetector->model()->heightControl () == 1)
  {
    float newHeightShift = rdetector->model()->heightShiftTolerance ();
    ImGui::SliderFloat ("Maximal height shift (H)", &newHeightShift,
                        BumpModel::MIN_SHIFT, BumpModel::MAX_SHIFT);
    if (newHeightShift != rdetector->model()->heightShiftTolerance ())
    {
      rdetector->model()->setHeightShiftTolerance (newHeightShift);
      det_widget->detectAndDisplay ();
    }
  }
  else if (rdetector->model()->heightControl () == 2)
  {
    float newHeightRShift = rdetector->model()->heightRelShiftTolerance ();
    ImGui::SliderFloat ("Max height shift / height (H)", &newHeightRShift,
                        BumpModel::MIN_REL_SHIFT, BumpModel::MAX_REL_SHIFT);
    if (newHeightRShift != rdetector->model()->heightRelShiftTolerance ())
    {
      rdetector->model()->setHeightRelShiftTolerance (newHeightRShift);
      det_widget->detectAndDisplay ();
    }
  }
  ImGui::Separator ();

  {
    bool status = rdetector->model()->isDeviationPredictionOn ();
    if (ImGui::Checkbox ("Direction aware##2", &status))
    {
      if (status != rdetector->model()->isDeviationPredictionOn ())
      {
        rdetector->model()->switchDeviationPrediction ();
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl D");
    if (ImGui::IsItemHovered ())
      ImGui::SetTooltip ("Helps to localize next bump");
  }

  {
    bool status = rdetector->model()->isSlopePredictionOn ();
    if (ImGui::Checkbox ("Slope aware##2", &status))
    {
      if (status != rdetector->model()->isSlopePredictionOn ())
      {
        rdetector->model()->switchSlopePrediction ();
        det_widget->detectAndDisplay ();
      }
    }
    ImGui::SameLine (SCUTPOS);
    ImGui::TextDisabled ("Ctrl S");
    if (ImGui::IsItemHovered ())
      ImGui::SetTooltip ("Helps to localize next bump");
  }

  {
    int newMissCount = rdetector->getBumpLackTolerance ();
    ImGui::SliderInt ("Bump lack tolerance (Q)", &newMissCount, 0, 50);
    if (newMissCount != rdetector->getBumpLackTolerance ())
    {
      rdetector->setBumpLackTolerance (newMissCount);
      det_widget->detectAndDisplay ();
    }
  }

  {
    float newMinWidth = rdetector->model()->minWidth ();
    ImGui::SliderFloat ("Bump minimal width (L)", &newMinWidth,
                        BumpModel::MIN_MIN_WIDTH, BumpModel::MAX_MIN_WIDTH);
    if (newMinWidth != rdetector->model()->minWidth ())
    {
      rdetector->model()->setMinWidth (newMinWidth);
      det_widget->detectAndDisplay ();
    }
  }

  {
    float newMinHeight = rdetector->model()->minHeight ();
    ImGui::SliderFloat ("Bump minimal height (K)", &newMinHeight,
                        BumpModel::MIN_MIN_HEIGHT, BumpModel::MAX_MIN_HEIGHT);
    if (newMinHeight != rdetector->model()->minHeight ())
    {
      rdetector->model()->setMinHeight (newMinHeight);
      det_widget->detectAndDisplay ();
    }
  }
}


void ILSDMenu::drawCrossProfileMenu (GLWindow* parent)
{
  if (ImGui::BeginMenu ("X profiles"))
  {
    {
      bool status = det_widget->isCrossProfileVisible ();
      if (ImGui::Checkbox ("Profile view visible", &status))
      {
        if (status != det_widget->isCrossProfileVisible ())
        {
          det_widget->switchCrossProfileAnalyzer ();
          det_widget->updateMeasuring ();
        }
      }
      ImGui::SameLine (SCUTPOS);
      ImGui::TextDisabled ("&");
    }

    if (det_widget->isCrossProfileVisible ())
    {
      ImGui::Separator ();
      /* For test use only
      if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
      {
        {
          bool status = ictrl->isInitialDetection ();
          if (ImGui::Checkbox ("Initial detection only", &status))
          {
            if (status != ictrl->isInitialDetection ())
            {
              ictrl->switchInitialDetection ();
              det_widget->getCrossProfileView()->reset ();
            }
          }
          ImGui::SameLine (SCUTPOS);
          ImGui::TextDisabled ("Ctrl D");
        }
        ImGui::Separator ();
      }
      */

      {
        bool status = ictrl->isTemplateDisplay ();
        if (ImGui::Checkbox ("Template displayed", &status))
        {
          if (status != ictrl->isTemplateDisplay ())
          {
            ictrl->switchTemplateDisplay ();
            det_widget->getCrossProfileView()->update ();
          }
        }
        ImGui::SameLine (SCUTPOS);
        ImGui::TextDisabled ("Ctrl E");
      }

      {
        bool status = ictrl->isRefDisplay ();
        if (ImGui::Checkbox ("Reference displayed", &status))
        {
          if (status != ictrl->isRefDisplay ())
          {
            ictrl->switchRefDisplay ();
            det_widget->getCrossProfileView()->update ();
          }
        }
        ImGui::SameLine (SCUTPOS);
        ImGui::TextDisabled ("Ctrl F");
      }

      if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
      {
        {
          bool status = ictrl->isEstimDisplay ();
          if (ImGui::Checkbox ("Estimation displayed", &status))
          {
            if (status != ictrl->isEstimDisplay ())
            {
              ictrl->switchEstimDisplay ();
              det_widget->getCrossProfileView()->update ();
            }
          }
          ImGui::SameLine (SCUTPOS);
          ImGui::TextDisabled ("Ctrl G");
        }

        {
          bool status = ictrl->isPredDisplay ();
          if (ImGui::Checkbox ("Prediction displayed", &status))
          {
            if (status != ictrl->isPredDisplay ())
            {
              ictrl->switchPredDisplay ();
              det_widget->getCrossProfileView()->update ();
            }
          }
          ImGui::SameLine (SCUTPOS);
          ImGui::TextDisabled ("Ctrl H");
        }
      }

      {
        bool status = ictrl->isDirDisplay ();
        if (ImGui::Checkbox ("Direction displayed", &status))
        {
          if (status != ictrl->isDirDisplay ())
          {
            ictrl->switchDirDisplay ();
            det_widget->getCrossProfileView()->update ();
          }
        }
        ImGui::SameLine (SCUTPOS);
        ImGui::TextDisabled ("Ctrl L");
      }
      {
        bool status = ictrl->isLegendDisplay ();
        if (ImGui::Checkbox ("Legend displayed", &status))
        {
          if (status != ictrl->isLegendDisplay ())
          {
            ictrl->switchLegendDisplay ();
            det_widget->getCrossProfileView()->update ();
          }
        }
        ImGui::SameLine (SCUTPOS);
        ImGui::TextDisabled ("?");
      }

      {
        bool status = ictrl->isStaticHeight ();
        if (ImGui::Checkbox ("Height ref on first profile", &status))
        {
          if (status != ictrl->isStaticHeight ())
          {
            ictrl->switchStaticHeight ();
            det_widget->getCrossProfileView()->update ();
          }
        }
        ImGui::SameLine (SCUTPOS);
        ImGui::TextDisabled ("Ctrl R");
        if (ImGui::IsItemHovered ())
          ImGui::SetTooltip ("Current profile otherwise");
      }
      ImGui::Separator ();

      if (det_widget->mode () != ILSDDetectionWidget::MODE_NONE)
      {
        bool status = ictrl->isAligned ();
        if (ImGui::Checkbox ("Scans aligned", &status))
        {
          if (status != ictrl->isAligned ())
          {
            ictrl->switchAligned ();
            det_widget->getCrossProfileView()->update ();
          }
        }
        ImGui::SameLine (SCUTPOS);
        ImGui::TextDisabled ("Ctrl A");
      }
  
      {
        int srlevel = ictrl->scanResolutionLevel ();
        ImGui::SliderInt ("Stripe resolution (W)", &srlevel, 0, 4);
        if (srlevel != ictrl->scanResolutionLevel ())
        {
          ictrl->setScanResolution (srlevel);
          det_widget->getCrossProfileView()->resetControls ();
        }
      }
      if (det_widget->mode () == ILSDDetectionWidget::MODE_NONE)
      {
        int swdth = ictrl->straightStripWidth ();
        ImGui::SliderInt ("Scan stripe width (X)", &swdth,
                          ILSDItemControl::MIN_STRAIGHT_STRIP_WIDTH,
                          ILSDItemControl::MAX_STRAIGHT_STRIP_WIDTH);
        if (swdth != ictrl->straightStripWidth ())
        {
          ictrl->setStraightStripWidth (swdth);
          det_widget->getCrossProfileView()->rebuildScans ();
        }
      }
      ImGui::Separator ();

      {
        int pwidth = ictrl->profileWidth ();
        ImGui::SliderInt ("Profile width (E)", &pwidth,
                          ILSDItemControl::MIN_PROF_WIDTH,
                          ILSDItemControl::MAX_PROF_WIDTH);
        if (pwidth != ictrl->profileWidth ())
        {
          ictrl->setProfileWidth (pwidth);
          det_widget->getCrossProfileView()->resetSize ();
        }
      }

      {
        int swidth = ictrl->scanWidth ();
        ImGui::SliderInt ("Scan width (S)", &swidth,
                          ILSDItemControl::MIN_SCAN_WIDTH,
                          ILSDItemControl::MAX_SCAN_WIDTH);
        if (swidth != ictrl->scanWidth ())
        {
          ictrl->setScanWidth (swidth);
          det_widget->getCrossProfileView()->resetSize ();
        }
      }

      {
        int vheight = ictrl->viewHeight ();
        ImGui::SliderInt ("View height (H)", &vheight,
                          ILSDItemControl::MIN_VIEW_HEIGHT,
                          ILSDItemControl::MAX_VIEW_HEIGHT);
        if (vheight != ictrl->viewHeight ())
        {
          ictrl->setViewHeight (vheight);
          det_widget->getCrossProfileView()->resetSize ();
        }
      }

      {
        bool reset;
        int outval;
        if (AsWidgets::MenuCartesianSlider ("Profile shift (B)", "",
                                            (float) (ictrl->profileShift ()),
                                            outval, &reset))
        {
          if (reset) ictrl->resetProfileShift ();
          else ictrl->shiftProfile (- outval);
          det_widget->getCrossProfileView()->update ();
        }
      }

      {
        int psize = ictrl->pointSize ();
        ImGui::SliderInt ("Point size (C)", &psize, 1, 7);
        if (psize != ictrl->pointSize ())
        {
          ictrl->setPointSize (psize);
          det_widget->getCrossProfileView()->update ();
        }
      }

      ImGui::Separator ();
      {
        bool reset;
        int outval;
        if (AsWidgets::MenuCartesianSlider ("Displayed scan (up/down)", "",
                                            (float) (ictrl->scan ()),
                                            outval, &reset))
        {
          if (reset) ictrl->setScan (0);
          else ictrl->incScan (outval);
          det_widget->getCrossProfileView()->update ();
          det_widget->updateWidget ();
        }
      }

      if (det_widget->mode () & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
      {
        {
          bool status = ictrl->isMeasuring ();
          if (ImGui::Checkbox ("Area measure", &status))
          {
            if (status != ictrl->isMeasuring ())
            {
              ictrl->switchMeasuring ();
              if (ictrl->isMeasuring ())
              {
                det_widget->updateMeasuring ();
                det_widget->getCrossProfileView()->update ();
              }
            }
          }
          ImGui::SameLine (SCUTPOS);
          ImGui::TextDisabled ("Ctrl M");
        }

        if (ictrl->isMeasuring ())
        {
          {
            if (ImGui::Button (" Start measure "))
            {
              ictrl->setMeasureStart ();
              det_widget->getCrossProfileView()->update ();
            }
            std::string msval ("-----");
            if (ictrl->isSetMeasureStart ())
              msval = to_string (ictrl->measureStart ());
            ImGui::SameLine (160);
            ImGui::TextUnformatted (msval.c_str ());
            ImGui::SameLine (SCUTPOS);
            ImGui::TextDisabled ("<");
          }

          {
            if (ImGui::Button (" Stop  measure "))
            {
              ictrl->setMeasureStop ();
              det_widget->getCrossProfileView()->update ();
            }
            std::string msval ("-----");
            if (ictrl->isSetMeasureStop ())
              msval = to_string (ictrl->measureStop ());
            ImGui::SameLine (160);
            ImGui::TextUnformatted (msval.c_str ());
            ImGui::SameLine (SCUTPOS);
            ImGui::TextDisabled (">");
          }
          ImGui::Separator ();

          {
            ILSDCrossProfileView *view = det_widget->getCrossProfileView ();
            float newTrslRatio = view->getMeasureLineTranslationRatio ();
            ImGui::SliderFloat ("Lower line translation", &newTrslRatio, 0.0f,
                                ILSDItemControl::MAX_LINE_TRANSLATION_RATIO);
            if (newTrslRatio != view->getMeasureLineTranslationRatio ())
            {
              view->setMeasureLineTranslationRatio (newTrslRatio);
              view->update ();
            }
            ImGui::SameLine (SCUTPOS);
            ImGui::TextDisabled ("m/M");
            if (ImGui::IsItemHovered ())
              ImGui::SetTooltip ("Lifts measure line");
          }

          {
            ILSDCrossProfileView *view = det_widget->getCrossProfileView ();
            if (ImGui::Button ("0")
                && view->getMeasureLineRotationRatio () != 0.0f)
            {
              view->setMeasureLineRotationRatio (0.0f);
              view->update ();
            }
            ImGui::SameLine ();
            float newRotRatio = view->getMeasureLineRotationRatio ();
            float minval = view->getMeasureLineTranslationRatio ()
                           - Bump::MAX_LINE_ROTATION_RATIO;
            float maxval = Bump::MAX_LINE_ROTATION_RATIO
                           - view->getMeasureLineTranslationRatio ();
            ImGui::SliderFloat ("Lower line rotation", &newRotRatio,
                                minval, maxval);
            if (newRotRatio != view->getMeasureLineRotationRatio ())
            {
              view->setMeasureLineRotationRatio (newRotRatio);
              view->update ();
            }
            ImGui::SameLine (SCUTPOS);
            ImGui::TextDisabled ("n/N");
            if (ImGui::IsItemHovered ())
              ImGui::SetTooltip ("Tilts measure line");
          }
        }
      }
    }
    ImGui::EndMenu ();
  }
}


void ILSDMenu::drawLongProfileMenu (GLWindow* parent)
{
  if (ImGui::BeginMenu ("L profiles"))
  {
    {
      bool status = det_widget->isLongProfileVisible ();
      if (ImGui::Checkbox ("Profile view visible", &status))
      {
        if (status != det_widget->isLongProfileVisible ())
        {
          det_widget->switchLongProfileAnalyzer ();
//          det_widget->display ();
        }
      }
      ImGui::SameLine (SCUTPOS);
      ImGui::TextDisabled (")");
      ImGui::Separator ();
    }

    if (det_widget->isLongProfileVisible ())
    {
      {
        bool status = ictrl->isThinLongStrip ();
        if (ImGui::Checkbox ("Straight strip thin longitudinal profile",
                             &status))
        {
          if (status != ictrl->isThinLongStrip ())
          {
            ictrl->switchThinLongStrip ();
            det_widget->getLongProfileView()->rebuildProfile ();
            det_widget->getLongProfileView()->update ();
          }
        }
        ImGui::SameLine (SCUTPOS);
        ImGui::TextDisabled ("Ctrl T");
        ImGui::Separator ();
      }

      {
        int pwidth = ictrl->longViewWidth ();
        ImGui::SliderInt ("Profile width (E)", &pwidth,
                          ILSDItemControl::MIN_LONG_VIEW_WIDTH,
                          ILSDItemControl::MAX_LONG_VIEW_WIDTH);
        if (pwidth != ictrl->longViewWidth ())
        {
          ictrl->setLongViewWidth (pwidth);
          det_widget->getLongProfileView()->resetSize ();
        }
      }
      {
        int pheight = ictrl->longViewHeight ();
        ImGui::SliderInt ("Profile height (H)", &pheight,
                          ILSDItemControl::MIN_LONG_VIEW_HEIGHT,
                          ILSDItemControl::MAX_LONG_VIEW_HEIGHT);
        if (pheight != ictrl->longViewHeight ())
        {
          ictrl->setLongViewHeight (pheight);
          det_widget->getLongProfileView()->resetSize ();
        }
      }
    }
    ImGui::EndMenu ();
  }
}



void ILSDMenu::drawPrefMenu (GLWindow* parent)
{
  if (ImGui::BeginMenu ("Preferences"))
  {
    // Display demo window
    ImGui::Checkbox ("Show ImGui demo window", &showDemoWindow);
    ImGui::EndMenu ();
  }
}


void ILSDMenu::drawMouseInfo (GLWindow* parent, int sx)
{
  if (! GLWindow::getMainWindow()->IsBackgroundHovered ()) return;
  ASCanvasPos pos;
  if (det_widget->getMousePosition (parent, pos))
  {
    ImGui::Dummy (ImVec2 (sx - 650.0f, 0.0f));
    ImGui::Text ("%s", string ("pos : "
            + ASCanvasPos (pos.x, det_widget->getHeight () - 1 - pos.y)
              .toString()).c_str ());
  }
}


void ILSDMenu::drawSelectionInfo (GLWindow *parent, int sy)
{
  if (! GLWindow::getMainWindow()->IsBackgroundHovered ()) return;
  if (glfwGetMouseButton (parent->getGlfwContext(), GLFW_MOUSE_BUTTON_1)
      == GLFW_PRESS)
  {
    ImGui::SetNextWindowSize (ImVec2 (300, 60));
    ImGui::SetNextWindowPos (ImVec2 (0, (float) sy - 65));
    if (ImGui::Begin ("Overlay", NULL, ImGuiWindowFlags_NoMove
                                       | ImGuiWindowFlags_NoDecoration
                                       | ImGuiWindowFlags_AlwaysAutoResize
                                       | ImGuiWindowFlags_NoSavedSettings
                                       | ImGuiWindowFlags_NoFocusOnAppearing
                                       | ImGuiWindowFlags_NoNav))
    {
      Pt2i p1, p2;
      det_widget->getInputStroke (p1, p2);
      ImGui::Columns (2);
      ImGui::Text ("Selection");
      ImGui::NextColumn ();
      ImGui::Text ("%s", string ("start : ("
                                 + to_string (p1.x ()) + ","
                                 + to_string (p1.y ()) + ")").c_str());
      ImGui::NextColumn ();
      ImGui::NextColumn ();
      ImGui::Text ("%s", string ("end : ("
                                 + to_string (p2.x ()) + ","
                                 + to_string (p2.y ()) + ")").c_str());
      ImGui::End ();
    }
  }
}


void ILSDMenu::importPointTile (const std::vector<std::string>& paths)
{
  // Registers input tile (XYZ, then NVM)
  import_tile = (paths.size () == 0 ? "" : paths[0]);
  if (import_tile != "")
  {
    size_t last = import_tile.find_last_of ('\\');
    if (last == std::string::npos) last = import_tile.find_last_of ('/');
    if (last == std::string::npos) return; 
    std::string import_dir = import_tile.substr (0, last); 

    // Requires next input tile
    std::string name;
    det_widget->disableKeys ();
    FileExplorer* explo = new FileExplorer (import_parent,
          "Select first central tile, then 8 neighbour tiles",
          import_dir, { "asc" }, false, 9); // @NEW FILEEXPLORER
    explo->OnApplyPath.Add (this, &ILSDMenu::importDTMTiles);
    explo->OnCancelExplorer.Add (det_widget, &ILSDDetectionWidget::noAction);
    explo->OnDestroy.Add (det_widget, &ILSDDetectionWidget::enableKeys);
  }
}


void ILSDMenu::importDTMTiles (const std::vector<std::string>& paths)
{
  // Prepares the new NVM file
  TerrainMap mappy;
  if (! mappy.addDtmFile (paths[0]))
  {
    std::cout << "Problem with file " << paths[0] << std::endl;
    return;
  }
  int tilwidth = mappy.width ();
  int tilheight = mappy.height ();
  int64_t tilxmin = (int64_t) (mappy.xMin () * IPtTile::XYZ_UNIT + 0.5f);
  int64_t tilymin = (int64_t) (mappy.yMin () * IPtTile::XYZ_UNIT + 0.5f);
  double tilcs = mappy.cellSize () * IPtTile::XYZ_UNIT;

  // Extract the name of the new tile
  size_t suff = paths[0].find_last_of ('.');
  if (suff == std::string::npos) suff = paths[0].length ();
  size_t last = paths[0].find_last_of ('\\');
  if (last == std::string::npos) last = paths[0].find_last_of ('/');
  if (last == std::string::npos) last = -1; // if no '/' is found
  suff = suff - last - 1;
  std::string outname = paths[0].substr (last + 1, suff); 
  bool unspaced = true;
  while (unspaced)
  {
    size_t spos = outname.find_last_of (' ');
    if (spos == std::string::npos) unspaced = false;
    else outname = outname.substr (0, spos) + std::string ("_")
                   + outname.substr (spos + 1, std::string::npos);
  }

  // Creates the new NVM tile
  std::string nvmfile = NVM_DIR + outname + std::string (".nvm");
  for (int i = 1; i < (int) (paths.size ()); i ++)
  {
    if (paths[i] != "")
    {
      if (! mappy.addDtmFile (paths[i]))
      {
        std::cout << "Problem with file " << paths[i] << std::endl;
        return;
      }
    }
  }
  if (! mappy.create ())
  {
    std::cout << "DTM fusion failed" << std::endl;
    return;
  }
  mappy.saveFirstNormalMap (nvmfile);

  // Creates the new TIL tile
  std::string tilfile = TIL_DIR;
  if (det_widget->cloudAccess () == IPtTile::TOP)
    tilfile += std::string (TILE_ACCESS_DIR_TOP)
               + std::string (TILE_ACCESS_PREF_TOP);
  else if (det_widget->cloudAccess () == IPtTile::MID)
    tilfile += std::string (TILE_ACCESS_DIR_MID)
               + std::string (TILE_ACCESS_PREF_MID);
  else if (det_widget->cloudAccess () == IPtTile::ECO)
    tilfile += std::string (TILE_ACCESS_DIR_ECO)
               + std::string (TILE_ACCESS_PREF_ECO);
  tilfile += outname + std::string (".til");
  int subdiv = det_widget->gridSubdivisionFactor ();
  IPtTile tile ((tilheight * subdiv) / det_widget->cloudAccess (),
                (tilwidth * subdiv) / det_widget->cloudAccess ());
  tile.setArea (tilxmin, tilymin, (int64_t) 0,
                (int) ((tilcs * det_widget->cloudAccess ()) / subdiv + 0.5));
  tile.loadXYZFile (import_tile, det_widget->cloudAccess ());
  tile.save (tilfile);
  std::vector<std::string> outnames;
  outnames.push_back (paths[0]);
  det_widget->selectTiles (outnames);
}
