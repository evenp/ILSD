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
#include <string>
#include <ostream>
#include <fstream>
#include "ilsdwindow.h"
#include "GLFW/glfw3.h"

using namespace std;


int main(int argc, char* argv[])
{
  bool testing = false;
  bool autotesting = false;

  ILSDWindow* window = new ILSDWindow();

  for (int i = 1; i < argc; i++)
  {
    if (string(argv[i]).at(0) == '-')
    {
      if (string(argv[i]) == string("--autotest")) autotesting = true;
      else if (string(argv[i]) == string("--test")) testing = true;
      else if (string(argv[i]) == string("--profile"))
        window->setDetectionMode(ILSDDetectionWidget::MODE_NONE);
      else if (string(argv[i]) == string("--ctrack"))
        window->setDetectionMode(ILSDDetectionWidget::MODE_CTRACK);
      else if (string(argv[i]) == string("--ridge"))
        window->setDetectionMode(ILSDDetectionWidget::MODE_RIDGE);
      else if (string(argv[i]) == string("--hollow"))
        window->setDetectionMode(ILSDDetectionWidget::MODE_HOLLOW);
      else if (string(argv[i]) == string("--analysis"))
        window->switchAnalysisWindow();
      else
      {
        int l = static_cast<int>(string(argv[i]).length());
        for (int j = 1; j < l; j++)
        {
          char carac = string(argv[i]).at(j);
          if (carac == 'p')
            window->setDetectionMode(ILSDDetectionWidget::MODE_NONE);
          else if (carac == 'c')
            window->setDetectionMode(ILSDDetectionWidget::MODE_CTRACK);
          else if (carac == 'r')
            window->setDetectionMode(ILSDDetectionWidget::MODE_RIDGE);
          else if (carac == 'h')
            window->setDetectionMode(ILSDDetectionWidget::MODE_HOLLOW);
          else if (carac == 'a') window->switchAnalysisWindow();
          else if (carac == 't') testing = true;
          else
          {
            cout << "Unknown argument: " << argv[i] << endl;
            return 0;
          }
        }
      }
    }
  }

  window->loadTiles ();
  if (autotesting)
  {
    window->runTest();
    return (EXIT_SUCCESS);
  }
  window->runOptions ();
  if (testing) window->runTest ();

  glfwMaximizeWindow (GLWindow::getMainWindow()->getGlfwContext ());
  GLWindow::run ();
  delete window;
  return 0;
}
