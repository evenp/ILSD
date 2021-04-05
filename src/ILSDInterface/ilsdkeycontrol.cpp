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
#include "ilsdkeycontrol.h"
#include "ilsddetectionwidget.h"
#include "ctrackdetector.h"
#include "ridgedetector.h"
#include "terrainmap.h"
#include "asmath.h"

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "glWindow.h"

#define DEFAULT_STROKE_DIR std::string("./tests/")
#define DEFAULT_STROKE_FILE std::string("test")
#define DEFAULT_SELECTION_DIR std::string("./selections/")
#define DEFAULT_SELECTION_FILE std::string("last_selection")


ILSDKeyControl::ILSDKeyControl ()
{
  det_widget = NULL;
  dtm_map = NULL;
  tdetector = NULL;
  rdetector = NULL;
}


void ILSDKeyControl::setDetectionWidget (ILSDDetectionWidget *widget)
{
  det_widget = widget;
  dtm_map = det_widget->getDtmMap ();
  tdetector = det_widget->getCTrackDetector ();
  rdetector = det_widget->getRidgeDetector ();
}


void ILSDKeyControl::processKey (GLWindow* parent,
                                 int key, int scancode, int action, int mods)
{
  if (det_widget->keyDisabled () || action == GLFW_RELEASE) return;

  // Display pressed key by pressing ALT modifier (for dev mode)
  // if (mods & GLFW_MOD_ALT && key != GLFW_KEY_LEFT_ALT)
  //   std::cout << key << std::endl;

  switch (key)
  {
    case GLFW_KEY_3 :
      det_widget->exportShape (string ("exports/structure.shx"));
      std::cout << "resources/exports/structure.shx file available"
                << std::endl;
      break;

    case GLFW_KEY_9:
      det_widget->performanceTest ();
      break;

    case GLFW_KEY_0:
      det_widget->loadStroke ({ DEFAULT_STROKE_DIR
                                + DEFAULT_STROKE_FILE + string (".txt") });
      break;
  }

  if (GLWindow::getMainWindow()->IsBackgroundHovered ())
  {
    switch (key)
    {
      case GLFW_KEY_Q: // A
        if (mods & GLFW_MOD_SHIFT)
        {
          if (det_widget->mode () != ILSDDetectionWidget::MODE_NONE)
          {
            bool seldisp = ! det_widget->getSelectionDisplay ();
            det_widget->setSelectionDisplay (seldisp);
            if (seldisp)
            {
              string dirname (DEFAULT_SELECTION_DIR);
              if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
                dirname += string ("ctracks/");
              else if (det_widget->mode () == ILSDDetectionWidget::MODE_RIDGE)
                dirname += string ("ridges/");
              else if (det_widget->mode () == ILSDDetectionWidget::MODE_HOLLOW)
                dirname += string ("hollows/");
              det_widget->loadSelection ({ dirname + DEFAULT_SELECTION_FILE
                                                   + string (".txt") });
            }
            else det_widget->display ();
          }
        }
        else if (det_widget->isInputStrokeValid ())
        {
          if (det_widget->mode () != ILSDDetectionWidget::MODE_NONE)
          {
            string dirname = DEFAULT_SELECTION_DIR;
            if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
              dirname += string ("ctracks/");
            else if (det_widget->mode () == ILSDDetectionWidget::MODE_RIDGE)
              dirname += string ("ridges/");
            else if (det_widget->mode () == ILSDDetectionWidget::MODE_HOLLOW)
              dirname += string ("hollows/");
            det_widget->addToSelection (dirname + DEFAULT_SELECTION_FILE
                                                + string (".txt"));
          }
        }
        break;

      case GLFW_KEY_B :
        if (mods & GLFW_MOD_CONTROL)
        {
          // Toggles background image
          det_widget->toggleBackground ();
          det_widget->display ();
        }
        else
        {
          // Tunes the background image black level
          det_widget->incBlackLevel (mods & GLFW_MOD_SHIFT ? -1 : 1);
          std::cout << "Background black level = "
                    << det_widget->getBlackLevel () << std::endl;
          det_widget->display ();
        }
        break;

      case GLFW_KEY_C :
        det_widget->rebuildImage ();
        det_widget->display ();
        break;

      case GLFW_KEY_D :
        if (mods & GLFW_MOD_CONTROL)
        {
          if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
          {
            // Switches on or off deviation addition to the template.
            tdetector->model()->switchDeviationPrediction ();
            std::cout << "Direction-aware template: "
                      << (tdetector->model()->isDeviationPredictionOn () ?
                          "on" : "off") << std::endl;
            det_widget->detectAndDisplay ();
          }
          else if (det_widget->mode ()
                   & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
          {
            // Switches on or off deviation addition to the template.
            rdetector->model()->switchDeviationPrediction ();
            std::cout << "Direction-aware template: "
                      << (rdetector->model()->isDeviationPredictionOn() ?
                          "on" : "off") << std::endl;
            det_widget->detectAndDisplay ();
          }
        }
        break;

      case GLFW_KEY_E :
        if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        {
          // Tunes the max tilt of detected tracks.
          tdetector->model()->incBSmaxTilt ((mods & GLFW_MOD_SHIFT) ? -1 : 1);
          std::cout << "BS max tilt (%) = "
                    << (tdetector->model()->bsMaxTilt()) << std::endl;
          det_widget->detectAndDisplay ();
        }
        break;

      case GLFW_KEY_F :
        if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        {
          if (mods & GLFW_MOD_CONTROL)
          {
            // Switches on or off absolute shift length pruning
            tdetector->switchShiftLengthPruning ();
            std::cout << "Shift length pruning modality: "
                      << (tdetector->isShiftLengthPruning () ? "on" : "off")
                      << std::endl;
            det_widget->detectAndDisplay ();
          }
          else
          {
            // Tunes maximal threshold for shift length pruning
            tdetector->incMaxShiftLength ((mods & GLFW_MOD_SHIFT) ? -1 : 1);
            std::cout << "Max shift length = "
                      << (tdetector->maxShiftLength ()) << std::endl;
            det_widget->detectAndDisplay ();
          }
        }
        break;

      case GLFW_KEY_H :
        if (det_widget->mode () & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
        {
          if (mods & GLFW_MOD_CONTROL)
          {
            // Toggles bump height control mode
            int ctrl = rdetector->model()->heightControl ();
            if (++ctrl == 3) ctrl = 0;
            rdetector->model()->setHeightControl (ctrl);
            if (ctrl == 0)
              std::cout << "No bump height control" << std::endl;
            else if (ctrl == 1)
              std::cout << "Absolute bump height control" << std::endl;
            else if (ctrl == 2)
              std::cout << "Relative bump height control" << std::endl;
            det_widget->detectAndDisplay ();
          }
          else if (rdetector->model()->heightControl () == 1)
          {
            // Tunes max shift of bump height
            rdetector->model()->incHeightShiftTolerance (
                                       mods & GLFW_MOD_SHIFT ? -1 : 1);
            std::cout << "Maximal height shift = "
                      << (rdetector->model()->heightShiftTolerance ())
                      << std::endl;
            det_widget->detectAndDisplay ();
          }
          else if (rdetector->model()->heightControl () == 2)
          {
            // Tunes max relative shift of bump height
            rdetector->model()->incHeightRelShiftTolerance (
                                       mods & GLFW_MOD_SHIFT ? -1 : 1);
            std::cout << "Maximal relative height shift = "
                      << (rdetector->model()->heightRelShiftTolerance())
                      << std::endl;
            det_widget->detectAndDisplay ();
          }
        }
        break;

      case GLFW_KEY_K :
        if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        {
          if (mods & GLFW_MOD_CONTROL)
          {
            // Switches on or off plateaux density pruning
            tdetector->switchDensityPruning ();
            std::cout << "Density pruning modality: "
                      << (tdetector->isDensityPruning () ? "on" : "off")
                      << std::endl;
            det_widget->detectAndDisplay ();
          }
          else
          {
            // Tunes minimal density of plateaux for validation
            tdetector->incMinDensity ((mods & GLFW_MOD_SHIFT) ? -5 : 5);
            std::cout << "Min plateaux density = "
                      << (tdetector->minDensity ()) << std::endl;
            det_widget->detectAndDisplay ();
          }
        }
        else if (det_widget->mode ()
                 & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
        {
          // Tunes minimal height of bumps
          rdetector->model()->incMinHeight ((mods & GLFW_MOD_SHIFT) ? -1 : 1);
          std::cout << "Minimal bump height = "
                    << (rdetector->model()->minHeight ()) << std::endl;
          det_widget->detectAndDisplay ();
        }
        break;

      case GLFW_KEY_L :
        if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        {
          // Tunes plateau minimal length
          tdetector->model()->incMinLength ((mods & GLFW_MOD_SHIFT) ? -1 : 1);
          std::cout << "Plateau minimal length = "
                    << (tdetector->model()->minLength()) << std::endl;
          det_widget->detectAndDisplay ();
        }
        else if (det_widget->mode ()
                 & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
        {
          // Tunes bump minimal width
          rdetector->model()->incMinWidth ((mods & GLFW_MOD_SHIFT) ? -1 : 1);
          std::cout << "Bump minimal width = "
                    << (rdetector->model()->minWidth ()) << std::endl;
          det_widget->detectAndDisplay ();
        }
        break;

      case GLFW_KEY_SEMICOLON : // M
        if (mods & GLFW_MOD_CONTROL)
        {
          // Toggles detection mode
          det_widget->toggleDetectionMode ();
          det_widget->setTitleChanged (true);
          if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
            std::cout << "Detection of carriage tracks" << std::endl;
          else if (det_widget->mode () == ILSDDetectionWidget::MODE_RIDGE)
            std::cout << "Detection of ridges" << std::endl;
          else if (det_widget->mode () == ILSDDetectionWidget::MODE_HOLLOW)
            std::cout << "Detection of hollows" << std::endl;
          else std::cout << "Simple analysis" << std::endl;
          det_widget->detectAndDisplay ();
        }
        else if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        {
          // Tunes plateau maximal length
          tdetector->model()->incMaxLength ((mods & GLFW_MOD_SHIFT) ? -1 : 1);
          std::cout << "Plateau maximal length = "
                    << (tdetector->model()->maxLength ()) << std::endl;
          det_widget->detectAndDisplay ();
        }
        break;

      case GLFW_KEY_N :
        if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        {
          if (mods & GLFW_MOD_CONTROL)
          {
            // Switches tail plateaux pruning
            tdetector->switchTailPruning ();
            int tpr = tdetector->tailPruning ();
            if (tpr == 0)
              cout << "Tail pruning modality off" << std::endl;
            else if (tpr == 1)
              cout << "Tail pruning modality on" << std::endl;
            else if (tpr == 2)
              cout << "Hard tail pruning modality on" << std::endl;
            det_widget->detectAndDisplay ();
          }
          else
          {
            // Tunes tail min size for track tail pruning
            tdetector->model()->incTailMinSize (
                                     (mods & GLFW_MOD_SHIFT) ? -1 : 1);
            std::cout << "Track tail min size = "
                      << (tdetector->model()->tailMinSize ()) << std::endl;
            det_widget->detectAndDisplay ();
          }
        }
        break;

      case GLFW_KEY_P :
        if (mods & GLFW_MOD_CONTROL)
        {
          if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
          {
            // Switches on or off initial detection
            tdetector->switchInitialization ();
            std::cout << "Initial detection "
                      << (tdetector->isInitializationOn () ? "on" : "off")
                      << endl;
            det_widget->detectAndDisplay ();
          }
        }
        else if (mods == 0)
        {
          // Captures main window
          std::cout << "Saves main window in capture.png" << std::endl;
          det_widget->capture ("captures/capture.png");
        }
        break;

      case GLFW_KEY_A : // Q
        if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        {
          // Tunes accepted count of successive plateau detection failures
          tdetector->incPlateauLackTolerance ((mods & GLFW_MOD_SHIFT) ? -1 : 1);
          std::cout << "Plateau lack tolerance = "
                    << (tdetector->getPlateauLackTolerance()) << std::endl;
          det_widget->detectAndDisplay ();
        }
        else if (det_widget->mode ()
                 & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
        {
          // Tunes accepted count of successive bump detection failures
          rdetector->incBumpLackTolerance ((mods & GLFW_MOD_SHIFT) ? -1 : 1);
          std::cout << "Bump lack tolerance = "
                    << (rdetector->getBumpLackTolerance ()) << std::endl;
          det_widget->detectAndDisplay ();
        }
        break;

      case GLFW_KEY_R :
        if (mods & GLFW_MOD_CONTROL)
        {
          if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
          {
            // Switches on or off point density use for tracking.
            tdetector->switchDensitySensitivity ();
            std::cout << "Point density use for tracking: "
                      << (tdetector->isDensitySensitive () ? "on" : "off")
                      << std::endl;
            det_widget->detectAndDisplay ();
          }
        }
        break;

      case GLFW_KEY_S:
        if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        {
          if (mods & GLFW_MOD_CONTROL)
          {
            // Switches on or off slope addition to the template.
            tdetector->model()->switchSlopePrediction ();
            std::cout << "Slope-aware template: "
                      << (tdetector->model()->isSlopePredictionOn () ?
                          "on" : "off") << std::endl;
            det_widget->detectAndDisplay ();
          }
          else
          {
            // Tunes the slope tolerance
            tdetector->model()->incSlopeTolerance (
                                         (mods & GLFW_MOD_SHIFT) ? -1 : 1);
            std::cout << "Max slope shift = "
                      << (tdetector->model()->slopeTolerance ()) << std::endl;
            det_widget->detectAndDisplay ();
          }
        }
        else if (det_widget->mode ()
                 & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
        {
          if (mods & GLFW_MOD_CONTROL)
          {
            // Switches on or off slope addition to the template.
            rdetector->model()->switchSlopePrediction ();
            std::cout << "Slope-aware template: "
                      << (rdetector->model()->isSlopePredictionOn () ?
                          "on" : "off") << std::endl;
            det_widget->detectAndDisplay ();
          }
        }
        break;

      case GLFW_KEY_T :
        if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        {
          // Tunes plateau thickness tolerance
          tdetector->model()->incThicknessTolerance (
                                         (mods & GLFW_MOD_SHIFT) ? -1 : 1);
          std::cout << "Max thickness shift = "
                    << (tdetector->model()->thicknessTolerance ()) << std::endl;
          det_widget->detectAndDisplay ();
        }
        else if (det_widget->mode ()
                 & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
        {
          if (mods & GLFW_MOD_CONTROL)
          {
            // Switches trend detection modality.
            rdetector->model()->switchDetectingTrend ();
            std::cout << "Trend detection modality: "
                      << (rdetector->model()->isDetectingTrend () ?
                          "on" : "off") << std::endl;
            det_widget->detectAndDisplay ();
          }
          else
          {
            // Tunes trend pinch thickness
            rdetector->model()->incTrendMinPinch (
                                       (mods & GLFW_MOD_SHIFT) ? -1 : 1);
            std::cout << "Trend min pinch thickness = "
                      << (rdetector->model()->trendMinPinch ()) << std::endl;
              det_widget->detectAndDisplay ();
          }
        }
        break;

      case GLFW_KEY_U :
        // Replays last extraction
        {
          Pt2i pt1, pt2;
          det_widget->getInputStroke (pt1, pt2);
          std::cerr << "p1 update: " << pt1.x () << " "
                                     << pt1.y () << std::endl;
          std::cerr << "p2 update: " << pt2.x () << " "
                                     << pt2.y () << std::endl;
          det_widget->detectAndDisplay ();
        }
        break;

      case GLFW_KEY_V :
        if (mods & GLFW_MOD_CONTROL)
        {
          // Toggles shading type
          dtm_map->toggleShadingType ();
          std::cout << "Shading " << dtm_map->shadingType () << std::endl;
          det_widget->rebuildImage ();
          det_widget->display ();
        }
        else
        {
          // Tunes lighting direction
          dtm_map->incLightAngle ((mods & GLFW_MOD_SHIFT) ? -1 : 1);
          std::cout << "Light angle = "
                    << (dtm_map->lightAngle () * ASF_RAD2DEG) << std::endl;
          det_widget->rebuildImage ();
          det_widget->display ();
        }
        break;

      case GLFW_KEY_Z : // W
        if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        {
          // Tunes side shift tolerance for successive plateaux bounds
          tdetector->model()->incSideShiftTolerance (
                                      (mods & GLFW_MOD_SHIFT) ? -1 : 1);
          std::cout << "Max position shift = "
                    << (tdetector->model()->sideShiftTolerance ()) << std::endl;
          det_widget->detectAndDisplay ();
        }
        else if (det_widget->mode ()
                 & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
        {
          if (mods & GLFW_MOD_CONTROL)
          {
            // Toggles bump width control mode
            int ctrl = rdetector->model()->widthControl ();
            if (++ctrl == 3) ctrl = 0;
            rdetector->model()->setWidthControl (ctrl);
            if (ctrl == 0)
              std::cout << "No bump width control" << std::endl;
            else if (ctrl == 1)
              std::cout << "Absolute bump width control" << std::endl;
            else if (ctrl == 2)
              std::cout << "Relative bump width control" << std::endl;
            det_widget->detectAndDisplay ();
          }
          else if (rdetector->model()->widthControl () == 1)
          {
            // Tunes max shift of bump width
            rdetector->model()->incWidthShiftTolerance (
                                          mods & GLFW_MOD_SHIFT ? -1 : 1);
            std::cout << "Maximal width shift = "
                      << (rdetector->model()->widthShiftTolerance ())
                      << std::endl;
            det_widget->detectAndDisplay ();
          }
          else if (rdetector->model()->widthControl () == 2)
          {
            // Tunes max relative shift of bump width
            rdetector->model()->incWidthRelShiftTolerance (
                                         mods & GLFW_MOD_SHIFT ? -1 : 1);
            std::cout << "Maximal relative width shift = "
                      << (rdetector->model()->widthRelShiftTolerance ())
                      << std::endl;
            det_widget->detectAndDisplay ();
          }
        }
        break;

      case GLFW_KEY_Y :
        if (det_widget->mode () & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
        {
          if (mods & GLFW_MOD_CONTROL)
          {
            // Toggles bump position control mode
            int ctrl = rdetector->model()->positionControl ();
            if (++ctrl == 3) ctrl = 0;
            rdetector->model()->setPositionControl (ctrl);
            if (ctrl == 0)
              std::cout << "No bump position control" << std::endl;
            else if (ctrl == 1)
              std::cout << "Absolute bump position control" << std::endl;
            else if (ctrl == 2)
              std::cout << "Relative bump position control" << endl;
            det_widget->detectAndDisplay ();
          }
          else if (rdetector->model()->positionControl () == 1)
          {
            // Tunes max shift of bump position
            rdetector->model()->incPositionShiftTolerance (
                                          mods & GLFW_MOD_SHIFT ? -1 : 1);
            std::cout << "Maximal position shift = "
                      << (rdetector->model()->positionShiftTolerance ())
                      << std::endl;
            det_widget->detectAndDisplay ();
          }
          else if (rdetector->model()->positionControl () == 2)
          {
            // Tunes max relative shift of bump position
            rdetector->model()->incPositionRelShiftTolerance (
                                         mods & GLFW_MOD_SHIFT ? -1 : 1);
            std::cout << "Maximal position relative shift = "
                      << (rdetector->model()->positionRelShiftTolerance ())
                      << std::endl;
            det_widget->detectAndDisplay ();
          }
        }
        break;

      case GLFW_KEY_W : // Z
        if (det_widget->mode () & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
        {
          if (mods & GLFW_MOD_CONTROL)
          {
            // Toggles bump altitude control mode
            int ctrl = rdetector->model()->altitudeControl ();
            if (++ctrl == 3) ctrl = 0;
            rdetector->model()->setAltitudeControl (ctrl);
            if (ctrl == 0)
              std::cout << "No bump altitude control" << std::endl;
            else if (ctrl == 1)
              std::cout << "Absolute bump altitude control" << std::endl;
            else if (ctrl == 2)
              std::cout << "Relative bump altitude control" << std::endl;
            det_widget->detectAndDisplay ();
          }
          else if (rdetector->model()->altitudeControl () == 1)
          {
            // Tunes max shift of bump altitude
            rdetector->model()->incAltitudeShiftTolerance (
                                         mods & GLFW_MOD_SHIFT ? -1 : 1);
            std::cout << "Maximal altitude shift = "
                      << (rdetector->model()->altitudeShiftTolerance ())
                      << std::endl;
            det_widget->detectAndDisplay ();
          }
          else if (rdetector->model()->altitudeControl () == 2)
          {
            // Tunes max relative shift of bump altitude
            rdetector->model()->incAltitudeRelShiftTolerance (
                                         mods & GLFW_MOD_SHIFT ? -1 : 1);
            std::cout << "Maximal altitude relative shift = "
                      << (rdetector->model()->altitudeRelShiftTolerance ())
                      << endl;
            det_widget->detectAndDisplay ();
          }
        }
        break;

      case GLFW_KEY_ENTER :
        std::cout << "User input saved" << std::endl;
        det_widget->saveStroke (DEFAULT_STROKE_DIR
                                + DEFAULT_STROKE_FILE + string (".txt"));
        break;

      case GLFW_KEY_1 :
        if (mods == 0) // &
        {
          det_widget->switchCrossProfileAnalyzer ();
          det_widget->updateMeasuring ();
        }
        break;

      case GLFW_KEY_2 :
        if (mods == 0) // &
        {
          det_widget->saveSubTile ();
        }
        break;

      case GLFW_KEY_4 :
        if (mods == 0) // '
        {
          det_widget->exportShape (string ("exports/last_export.shx"));
          std::cout << "resources/exports/last_export.shx file available"
                    << std::endl;
        }
        break;

      case GLFW_KEY_PERIOD :
        if (mods == 0) // key ':'
        {
          det_widget->invertInputStroke ();
          std::cout << "P1 <-> P2" << std::endl;
        }
        else // key '/'
        {
          det_widget->switchDisplayTile ();
          std::cout << "Tile display: "
                    << (det_widget->isDisplayTileOn () ? "on" : "off")
                    << std::endl;
          det_widget->display ();
        }
        break;

      case GLFW_KEY_5 :
        if (mods == 0)  // (
        {
          det_widget->switchDisplayDetection ();
          std::cout << "Detection display: "
                    << (det_widget->isDisplayDetectionOn () ? "on" : "off")
                    << std::endl;
          det_widget->display ();
        }
        break;

      case GLFW_KEY_MINUS :
        if (mods == 0)  // )
        {
          det_widget->switchLongProfileAnalyzer ();
//          det_widget->detectAndDisplay ();
        }
        break;

      case GLFW_KEY_COMMA : // ;
        det_widget->toggleSelectionStyle ();
        det_widget->display ();
        break;

      case GLFW_KEY_RIGHT_BRACKET:  // $
        if (det_widget->mode ()    // $
            & ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW)
        {
          // Switches bump center selection (mass center or point above)
          rdetector->model()->switchCenterReference ();
          std::cout << "Bump reference: "
                    << (rdetector->model()->massReferenced () ?
                        "mass" : "surface") << " center" << std::endl;
          det_widget->detectAndDisplay ();
        }
      
      case GLFW_KEY_BACKSLASH:  // *
        det_widget->toggleStructureStyle ();
        det_widget->display ();
        break;

      case GLFW_KEY_SLASH:  // !
        if (det_widget->mode () == ILSDDetectionWidget::MODE_CTRACK)
        {
          det_widget->switchPlateauSmoothed ();
          if (det_widget->isPlateauSmoothedOn ())
            std::cout << "Smoothed plateaux insertion" << std::endl;
          else std::cout << "Strict plateaux insertion" << std::endl;
        }
        else
        {
          det_widget->switchBumpSmoothed ();
          if (det_widget->isBumpSmoothedOn ())
            std::cout << "Smoothed bumps insertion" << std::endl;
          else std::cout << "Strict bumps insertion" << std::endl;
        }
        det_widget->display ();
        break;

      case GLFW_KEY_PAGE_UP:
        det_widget->incZoom (1);
        break;

      case GLFW_KEY_PAGE_DOWN:
        det_widget->incZoom (-1);
        break;

      case GLFW_KEY_LEFT:
        det_widget->incXShift (1);
        break;

      case GLFW_KEY_RIGHT:
        det_widget->incXShift (-1);
        break;

      case GLFW_KEY_UP:
        det_widget->incYShift (1);
        break;

      case GLFW_KEY_DOWN:
        det_widget->incYShift (-1);
        break;
    }
  }
}
