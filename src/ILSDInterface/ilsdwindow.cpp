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

#include "ilsdwindow.h"
#include "ilsdmenu.h"
#include "ilsdkeycontrol.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include <iostream>


ILSDWindow::ILSDWindow () : GLWindow ("ILSD", 400)
{
  showAnal = false;
  detection_widget = new ILSDDetectionWidget ();
  control = new ILSDKeyControl ();
  control->setDetectionWidget (detection_widget);
  menu = new ILSDMenu ();
  menu->setDetectionWidget (detection_widget);
}


ILSDWindow::~ILSDWindow ()
{
  delete menu;
  delete control;
  delete detection_widget;
}


bool ILSDWindow::loadTiles ()
{
  detection_widget->loadTiles ();
  if (detection_widget->tilesLoaded ())
  {
    resize (detection_widget->widgetSize ());
    detection_widget->display ();
  }
  return (detection_widget->tilesLoaded ());
}


void ILSDWindow::setDetectionMode (int mode)
{
  detection_widget->setDetectionMode (mode);
}


void ILSDWindow::runOptions ()
{
  if (showAnal) detection_widget->switchCrossProfileAnalyzer ();
}


void ILSDWindow::runTest ()
{
  if (detection_widget->tilesLoaded ())
    detection_widget->loadStroke ({ detection_widget->getStrokeFileName () });
}


void ILSDWindow::drawWindow ()
{
  if (detection_widget)
  {
    if (detection_widget->titleChanged ())
      glfwSetWindowTitle (GLWindow::getMainWindow()->getGlfwContext (),
                          detection_widget->getTitle().c_str ());
    detection_widget->paintEvent (this);
    menu->drawUI (this);
  }
}


void ILSDWindow::processKey (int key, int scancode, int action, int mods)
{
  if (detection_widget != NULL && detection_widget->tilesLoaded ())
    control->processKey (this, key, scancode, action, mods);
}


void ILSDWindow::processMouseButtonKey (int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_1)
  {
    if (action == GLFW_PRESS)
    {
      if (detection_widget) detection_widget->mousePressEvent (this);
    }
    else if (action == GLFW_RELEASE)
    {
      if (detection_widget) detection_widget->mouseReleaseEvent (this);
    }
  }
}


void ILSDWindow::moveCursor (double posX, double posY)
{
  if (glfwGetMouseButton (getGlfwContext (), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
  {
    if (detection_widget) detection_widget->mouseMoveEvent (this);
  }
}


void ILSDWindow::onChildWindowToBeDeleted (AsImGuiWindow *child)
{
  detection_widget->itemDeleted (child);
}
