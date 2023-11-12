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
#include <cstdlib>
#include "ilsdcrossprofileview.h"
#include "GLFW/glfw3.h"
#include "ilsdctrackcrossprofile.h"
#include "ilsdridgecrossprofile.h"
#include "ilsdstripcrossprofile.h"
#include "ilsddetectionwidget.h"


ILSDCrossProfileView::ILSDCrossProfileView (
              GLWindow* context, bool fpos, ASCanvasPos &pos,
              CTrackDetector* detector, ILSDItemControl *ictrl,
              ILSDDetectionWidget* inwidget)
    : AsImGuiWindow (context, "Cross profiles and scans", fpos, pos),
      detection_widget (inwidget)
{
  item_ctrl = ictrl;
  item = new ILSDCTrackCrossProfile (detector, ictrl);
  resize (item->size ());
  igrab = 0;
}


ILSDCrossProfileView::ILSDCrossProfileView (
              GLWindow* context, bool fpos, ASCanvasPos &pos,
              RidgeDetector* detector, ILSDItemControl *ictrl,
              ILSDDetectionWidget* inwidget)
    : AsImGuiWindow (context, "Cross profiles and scans", fpos, pos),
      detection_widget (inwidget)
{
  item_ctrl = ictrl;
  item = new ILSDRidgeCrossProfile (detector, ictrl);
  resize (item->size ());
  igrab = 0;
  detection_widget->updateMeasuring ();
}


ILSDCrossProfileView::ILSDCrossProfileView (
              GLWindow* context, bool fpos, ASCanvasPos &pos,
              int subdiv, ILSDItemControl *ictrl,
              ILSDDetectionWidget* inwidget)
    : AsImGuiWindow (context, "Cross profiles and scans", fpos, pos),
      detection_widget (inwidget)
{
  item_ctrl = ictrl;
  item = new ILSDStripCrossProfile (subdiv, ictrl);
  resize (item->size ());
  igrab = 0;
}


ILSDCrossProfileView::~ILSDCrossProfileView ()
{
  delete item;
}


void ILSDCrossProfileView::setData (ASImage* image, IPtTileSet* pdata)
{
  item->setData (image, pdata);
}


void ILSDCrossProfileView::reset ()
{
  item_ctrl->setScan (0);
  update ();
}


void ILSDCrossProfileView::resetControls ()
{
  item->resetControls ();
  update ();
}


void ILSDCrossProfileView::resetSize ()
{
  item->resetControls ();
  resize (item->size ());
  update ();
}


void ILSDCrossProfileView::buildScans (Pt2i p1, Pt2i p2)
{
  item->buildScans (p1, p2);
}


void ILSDCrossProfileView::update ()
{
  item->update ();
}


void ILSDCrossProfileView::keyPressed (int key, int scancode,
                                       int action, int mods)
{
  if (! detection_widget->keyDisabled ()) switch (key)
  {
    case GLFW_KEY_Q :  // A
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchAligned ();
        update ();
      }
      break;

    case GLFW_KEY_B :
      if (action == GLFW_PRESS || action == GLFW_REPEAT)
      {
        item_ctrl->shiftProfile ((mods & GLFW_MOD_SHIFT) == 0 ? -1 : 1);
        update ();
      }
      break;

    case GLFW_KEY_C :
      if (action == GLFW_PRESS)
      {
        item_ctrl->incPointSize ((mods & GLFW_MOD_SHIFT) == 0 ? 1 : -1);
        update ();
      }
      break;

/*
    case GLFW_KEY_D : // For test use only !
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchInitialDetection ();
        reset ();
      }
      break;
*/

    case GLFW_KEY_E :
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchTemplateDisplay ();
        update ();
      }
      else if (action == GLFW_PRESS || action == GLFW_REPEAT)
      {
        item_ctrl->incProfileWidth ((mods & GLFW_MOD_SHIFT) == 0 ? 1 : -1);
        resetSize ();
      }
      break;

    case GLFW_KEY_F :
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchRefDisplay ();
        update ();
      }
      break;

    case GLFW_KEY_G :
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchEstimDisplay ();
        update ();
      }
      break;

    case GLFW_KEY_H :
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchPredDisplay ();
        update ();
      }
      else
      {
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
          item_ctrl->incViewHeight ((mods & GLFW_MOD_SHIFT) == 0 ? 1 : -1);
          resetSize ();
        }
      }
      break;

    case GLFW_KEY_J :
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchAltiDisplay ();
        update ();
      }
      break;

    case GLFW_KEY_K :
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchDetectionDisplay ();
        update ();
      }
      break;

    case GLFW_KEY_L :
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchDirDisplay ();
        update ();
      }
      break;

    case GLFW_KEY_SEMICOLON : // M
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchMeasuring ();
        if (item_ctrl->isMeasuring ())
        {
          int scan = item_ctrl->scan ();
          detection_widget->updateMeasuring ();
          item_ctrl->setScan (scan);
        }
        update ();
      }
      else
      {
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
          item->incMeasureLineTranslationRatio (
                                 (mods & GLFW_MOD_SHIFT) == 0 ? 1 : -1);
          update ();
        }
      }
      break;

    case GLFW_KEY_N :
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item->setMeasureLineRotationRatio (0.0f);
        update ();
      }
      else
      if (action == GLFW_PRESS || action == GLFW_REPEAT)
      {
        item->incMeasureLineRotationRatio (
                         (mods & GLFW_MOD_SHIFT) == 0 ? 1 : -1);
        update ();
      }
      break;

    case GLFW_KEY_P : // Capture
      if (action == GLFW_PRESS && mods == 0)
      {
        igrab ++;
        std::string grabnom ("captures/");
        grabnom += item->profileName ()
                   + to_string (igrab / 1000)
                   + to_string ((igrab % 1000) / 100)
                   + to_string ((igrab % 100) / 10)
                   + to_string (igrab % 10)
                   + std::string (".png");
        item->save (grabnom);
        std::cout << "Profile shot in " << grabnom.data () << std::endl;
      }
      break;

    case GLFW_KEY_R :
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchStaticHeight ();
        update ();
      }
      break;

    case GLFW_KEY_S :
      if (action == GLFW_PRESS || action == GLFW_REPEAT)
      {
        item_ctrl->incScanWidth ((mods & GLFW_MOD_SHIFT) == 0 ? 1 : -1);
        resetSize ();
      }
      break;

    case GLFW_KEY_T :
      if (action == GLFW_PRESS)
      {
        item_ctrl->incScanResolution ((mods & GLFW_MOD_SHIFT) == 0 ? 1 : -1);
        resetControls ();
      }
      break;

    case GLFW_KEY_X :
      if (item->isStripAdaptable ())
      {
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
          item_ctrl->incStraightStripWidth ((mods & GLFW_MOD_SHIFT) == 0 ?
                                            1 : -1);
          item->rebuildScans ();
          update ();
//          detection_widget->paintEvent (getContext ());
          detection_widget->updateWidget ();
        }
      }
      break;

    case GLFW_KEY_W :  // Z
      if (action == GLFW_PRESS)
      {
        item_ctrl->incZRatio ((mods & GLFW_MOD_SHIFT) == 0 ? 1 : -1);
        resetControls ();
      }
      break;

    case GLFW_KEY_M :
      if (action == GLFW_PRESS && mods == GLFW_MOD_SHIFT)  // ?
      {
        item_ctrl->switchLegendDisplay ();
        update ();
      }
      break;

    case 161 :  // GLFW_KEY_WORLD_1 (non-US)
      if (action == GLFW_PRESS)
      {
        if (mods == GLFW_MOD_CONTROL)                           // Ctrl <
          detection_widget->saveLastMeasure ();
        else if (mods == GLFW_MOD_SHIFT)                        // >
        {
          item_ctrl->setMeasureStop ();
          item->update ();
        }
        else                                                    // <
        {
          item_ctrl->setMeasureStart ();
          item->update ();
        }
      }
      break;

    case GLFW_KEY_UP :
      if (action == GLFW_PRESS || action == GLFW_REPEAT)
      {
        item_ctrl->incScan (1);
        item->updateProfile ();
        update ();
//        detection_widget->paintEvent (getContext ());
        detection_widget->updateWidget ();
      }
      break;

    case GLFW_KEY_DOWN :
      if (action == GLFW_PRESS || action == GLFW_REPEAT)
      {
        item_ctrl->incScan (-1);
        item->updateProfile ();
        update ();
//        detection_widget->paintEvent (getContext ());
        detection_widget->updateWidget ();
      }
      break;
  }
}


void ILSDCrossProfileView::rebuildScans ()
{
  item->rebuildScans ();
  update ();
//  detection_widget->paintEvent (getContext ());
  detection_widget->updateWidget ();
}


void ILSDCrossProfileView::setScan (int val)
{
  item_ctrl->setScan (val);
  item->updateProfile ();
}


void ILSDCrossProfileView::DrawContent (GLWindow* windowContext)
{
  item->paint (windowContext);
}
