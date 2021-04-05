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
#include "ilsdlongprofileview.h"
#include "GLFW/glfw3.h"
#include "ilsdctracklongprofile.h"
#include "ilsdridgelongprofile.h"
#include "ilsdstriplongprofile.h"
#include "ilsddetectionwidget.h"


ILSDLongProfileView::ILSDLongProfileView (
              GLWindow* context, bool fpos, ASCanvasPos &pos,
              CTrackDetector* detector, ILSDItemControl *ictrl,
              ILSDDetectionWidget* inwidget)
    : AsImGuiWindow (context, "Longitudinal profile", fpos, pos),
      detection_widget (inwidget)
{
  item_ctrl = ictrl;
  item = new ILSDCTrackLongProfile (detector, ictrl);
  resize (item->size ());
  igrab = 0;
}


ILSDLongProfileView::ILSDLongProfileView (
              GLWindow* context, bool fpos, ASCanvasPos &pos,
              RidgeDetector* detector, ILSDItemControl *ictrl,
              ILSDDetectionWidget* inwidget)
    : AsImGuiWindow (context, "Longitudinal profile", fpos, pos),
      detection_widget (inwidget)
{
  item_ctrl = ictrl;
  item = new ILSDRidgeLongProfile (detector, ictrl);
  resize (item->size ());
  igrab = 0;
}


ILSDLongProfileView::ILSDLongProfileView (
              GLWindow* context, bool fpos, ASCanvasPos &pos,
              int subdiv, ILSDItemControl *ictrl,
              ILSDDetectionWidget* inwidget)
    : AsImGuiWindow (context, "Longitudinal profile", fpos, pos),
      detection_widget (inwidget)
{
  item_ctrl = ictrl;
  item = new ILSDStripLongProfile (subdiv, ictrl);
  resize (item->size ());
  igrab = 0;
}


ILSDLongProfileView::~ILSDLongProfileView ()
{
  delete item;
}


void ILSDLongProfileView::setData (ASImage* image, IPtTileSet* pdata)
{
  item->setData (image, pdata);
}


void ILSDLongProfileView::reset ()
{
  update ();
}


void ILSDLongProfileView::resetControls ()
{
  item->resetControls ();
  update ();
}


void ILSDLongProfileView::resetSize ()
{
  item->resetControls ();
  resize (item->size ());
  update ();
}


void ILSDLongProfileView::buildProfile (Pt2i p1, Pt2i p2)
{
  item->buildProfile (p1, p2);
}


void ILSDLongProfileView::rebuildProfile ()
{
  item->rebuildProfile ();
}


void ILSDLongProfileView::update ()
{
  item->update ();
}


void ILSDLongProfileView::keyPressed (int key, int scancode,
                                      int action, int mods)
{
  if (! detection_widget->keyDisabled ()) switch (key)
  {
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

    case GLFW_KEY_E :
      if (action == GLFW_PRESS || action == GLFW_REPEAT)
      {
        item_ctrl->incLongViewWidth ((mods & GLFW_MOD_SHIFT) == 0 ? 1 : -1);
        resetSize ();
      }
      break;

    case GLFW_KEY_H :
      if (action == GLFW_PRESS || action == GLFW_REPEAT)
      {
        item_ctrl->incLongViewHeight ((mods & GLFW_MOD_SHIFT) == 0 ? 1 : -1);
        resetSize ();
      }
      break;

    case GLFW_KEY_P : // Capture
      if (action == GLFW_PRESS && mods == 0)
      {
        igrab ++;
        std::string grabnom ("captures/long_");
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

    case GLFW_KEY_T :
      if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
      {
        item_ctrl->switchThinLongStrip ();
        item->rebuildProfile ();
        item->update ();
      }
      break;
  }
}


void ILSDLongProfileView::DrawContent (GLWindow* windowContext)
{
  item->paint (windowContext);
}
