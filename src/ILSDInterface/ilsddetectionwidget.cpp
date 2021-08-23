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
#include <fstream>
#include <ctime>
#include <cmath>
#include "asmath.h"
#include "digitalstraightline.h"
#include "ilsddetectionwidget.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "asWidgets.h"
#include "glWindow.h"
#include "asPainter.h"
#include "IniLoader.h"
#include "FileExplorer.h"
#include "SaveFileWidget.h"
#include "shapefil.h" // SHP

#define DEFAULT_SETTING_DIR std::string("./config/")
#define DEFAULT_SETTING_FILE std::string("ILSD")
#define DEFAULT_TILE_DIR std::string("./tiles/")
#define DEFAULT_TILE_FILE std::string("last")
#define NVM_DIR std::string("./nvm/")
#define TIL_DIR std::string("./til/")
#define DEFAULT_STROKE_DIR std::string("./tests/")
#define DEFAULT_STROKE_FILE std::string("test")
#define DEFAULT_MLINE_SUFFIX std::string("_cplt.mln")
#define TILE_NAME_MAX_LENGTH 200


const std::string ILSDDetectionWidget::VERSION = "1.1.3";

const int ILSDDetectionWidget::MODE_NONE = 0;
const int ILSDDetectionWidget::MODE_CTRACK = 1;
const int ILSDDetectionWidget::MODE_RIDGE = 2;
const int ILSDDetectionWidget::MODE_HOLLOW = 4;
const int ILSDDetectionWidget::MODE_RIDGE_OR_HOLLOW = 6;

const int ILSDDetectionWidget::SEL_NO = 0;
const int ILSDDetectionWidget::SEL_THIN = 1;
const int ILSDDetectionWidget::SEL_THICK = 2;

const int ILSDDetectionWidget::BACK_BLACK = 0;
const int ILSDDetectionWidget::BACK_WHITE = 1;
const int ILSDDetectionWidget::BACK_IMAGE = 2;

const int ILSDDetectionWidget::THIN_PEN = 1;
const int ILSDDetectionWidget::THICK_PEN = 3;
const int ILSDDetectionWidget::DEFAULT_MEAN_TRACK_WIDTH = 6;
const int ILSDDetectionWidget::SELECT_TOL = 5;

const int ILSDDetectionWidget::SUBDIV = 5;
const int ILSDDetectionWidget::MOVE_SHIFT = 10;


ILSDDetectionWidget::ILSDDetectionWidget ()
{
  // Sets default user inputs parameters
  cloud_access = IPtTile::ECO;
  det_mode = MODE_RIDGE;
  tiles_loaded = false;
  picking = false;
  udef = false;
  oldudef = false;
  nodrag = true;
  bMousePressed = false;
  with_aux_update = false;
  to_update = false;
  tiledisp = true;
  ctrack_style = CTRACK_DISP_SCANS;
  ridge_style = RIDGE_DISP_CENTER;
  sel_style = SEL_THICK;
  cuts_displayed = true;
  disp_saved = false;
  disp_detection = true;
  perf_mode = false;
  popup_nb = 0;

  // Initializes the maps and the auxiliary views
  cp_view = NULL;
  lp_view = NULL;
  iratio = 1.0f;
  cellsize = 1.0f;

  // Sets initial user outputs parameters
  verbose = false;
  statsOn = false;
  smoothed_plateaux = true;
  smoothed_bumps = true;
  background = BACK_IMAGE;
  blevel = 0;

  // Sets display parameters
  structure_color = ASColor::YELLOW;
  selection_color = ASColor::GREEN;
  tiles_color = ASColor::GREEN;
  analyzed_color = ASColor::BLACK;
  track_width = DEFAULT_MEAN_TRACK_WIDTH;

  width = 0;
  height = 0;
  maxWidth = 768;
  maxHeight = 512;
  xMaxShift = 0;
  yMaxShift = 0;
  xShift = 0;
  yShift = 0;
  zoom = 0;

  string defload = DEFAULT_SETTING_DIR
                   + DEFAULT_SETTING_FILE + string (".ini");
  ini_load = new IniLoader (defload.c_str ());
  loadSettings (ini_load);
  new_title = true;
}


ILSDDetectionWidget::~ILSDDetectionWidget ()
{
  saveSettings (ini_load);
  delete ini_load;
}


void ILSDDetectionWidget::reset ()
{
  udef = false;
  p1.set (0, 0);
  p2.set (0, 0);
  if (lp_view != NULL) switchLongProfileAnalyzer ();
  if (cp_view != NULL) switchCrossProfileAnalyzer ();
  rdetector.clear ();
  tdetector.clear ();
  display ();
}


void ILSDDetectionWidget::saveSettings (const std::string& path)
{
  IniLoader iload (path.c_str ());
  saveSettings (&iload);
}

void ILSDDetectionWidget::saveSettings (IniLoader *iload)
{
  iload->SetPropertyAsInt("ASD", "CloudAccess", cloud_access);
  iload->SetPropertyAsInt("ASD", "DetectionMode", det_mode);

  if (cp_view != NULL)
  {
    iload->SetPropertyAsInt ("AnalysisView", "Xpos",
                             cp_view->GetWindowPos().x);
    iload->SetPropertyAsInt ("AnalysisView", "Ypos",
                             cp_view->GetWindowPos().y);
  }
  iload->SetPropertyAsInt ("AnalysisView", "StraightStripWidth",
                           ictrl.straightStripWidth ());
  iload->SetPropertyAsInt ("AnalysisView", "ProfileWidth",
                           ictrl.profileWidth ());
  iload->SetPropertyAsInt ("AnalysisView", "ScanWidth",
                           ictrl.scanWidth ());
  iload->SetPropertyAsInt ("AnalysisView", "ViewHeight",
                           ictrl.viewHeight ());

  if (lp_view != NULL)
  {
    iload->SetPropertyAsInt ("LongProfileView", "Xpos",
                             lp_view->GetWindowPos().x);
    iload->SetPropertyAsInt ("LongProfileView", "Ypos",
                             lp_view->GetWindowPos().y);
  }
  iload->SetPropertyAsInt ("LongProfileView", "ViewWidth",
                           ictrl.longViewWidth ());
  iload->SetPropertyAsInt ("LongProfileView", "ViewHeight",
                           ictrl.longViewHeight ());
  iload->SetPropertyAsBool ("LongProfileView", "ThinLongStrip",
                            ictrl.isThinLongStrip ());
  
  saveRidge (iload);
  saveCarTrack (iload);
  iload->Save ();
}


void ILSDDetectionWidget::loadSettings (const std::vector<std::string>& paths)
{
  std::string path = (paths.size () == 0 ? "" : paths[0]);
  IniLoader iload (path.c_str ());
  loadSettings (&iload);
}

void ILSDDetectionWidget::loadSettings (IniLoader *iload)
{
  int access = iload->GetPropertyAsInt ("ASD", "CloudAccess", cloud_access);
  if (access != cloud_access) setCloudAccess (access);

  int mode = iload->GetPropertyAsInt ("ASD", "DetectionMode", det_mode);
  if (mode != det_mode)
  {
    setDetectionMode (mode);
    new_title = true;
  }
  ini_load->SetPropertyAsInt ("AnalysisView", "Xpos",
    iload->GetPropertyAsInt ("AnalysisView", "Xpos", -12345));
  ini_load->SetPropertyAsInt ("AnalysisView", "Ypos",
    iload->GetPropertyAsInt ("AnalysisView", "Ypos", -12345));

  ictrl.setStraightStripWidth (iload->GetPropertyAsInt (
      "AnalysisView", "StraightStripWidth",
      ILSDItemControl::DEFAULT_STRAIGHT_STRIP_WIDTH));
  ictrl.setProfileWidth (iload->GetPropertyAsInt (
      "AnalysisView", "ProfileWidth", ILSDItemControl::DEFAULT_PROF_WIDTH));
  ictrl.setScanWidth (iload->GetPropertyAsInt (
      "AnalysisView", "ScanWidth", ILSDItemControl::DEFAULT_SCAN_WIDTH));
  ictrl.setViewHeight (iload->GetPropertyAsInt (
      "AnalysisView", "ViewHeight", ILSDItemControl::DEFAULT_VIEW_HEIGHT));
  if (cp_view != NULL) cp_view->setPosition (
          iload->GetPropertyAsInt ("AnalysisView", "Xpos", -12345),
          iload->GetPropertyAsInt ("AnalysisView", "Ypos", -12345));

  ictrl.setLongViewWidth (iload->GetPropertyAsInt ("LongProfileView",
                 "ViewWidth", ILSDItemControl::DEFAULT_LONG_VIEW_WIDTH));
  ictrl.setLongViewHeight (iload->GetPropertyAsInt ("LongProfileView",
                 "ViewHeight", ILSDItemControl::DEFAULT_LONG_VIEW_HEIGHT));
  if (ictrl.isThinLongStrip () != iload->GetPropertyAsBool (
      "LongProfileView", "ThinLongStrip", true))
    ictrl.switchThinLongStrip ();
  if (lp_view != NULL) cp_view->setPosition (
    iload->GetPropertyAsInt ("LongProfileView", "Xpos", -12345),
    iload->GetPropertyAsInt ("LongProfileView", "Ypos", -12345));

  loadRidge (iload);
  loadCarTrack (iload);
}


void ILSDDetectionWidget::saveStructure (const std::string& path)
{
  IniLoader iload (path.c_str ());
  iload.SetPropertyAsInt ("ASD", "DetectionMode", det_mode);
  if (det_mode == MODE_RIDGE || det_mode == MODE_HOLLOW) saveRidge (&iload);
  else if (det_mode == MODE_CTRACK) saveCarTrack (&iload);
  saveStroke (&iload);
  iload.Save ();

  if (det_mode == MODE_RIDGE || det_mode == MODE_HOLLOW)
  {
    if (iload.GetPropertyAsInt ("Ridge", "NumberOfMeasureLines", 0) != 0)
    {
      size_t suff = path.find_last_of ('.');
      if (suff != std::string::npos)
      {
        string cpltname = path.substr (0, suff) + DEFAULT_MLINE_SUFFIX;
        ofstream output (cpltname.c_str (), ios::out);
        if (output)
        {
          Ridge *rdg = rdetector.getRidge ();
          if (rdg != NULL)
          {
            int m1 = - rdg->getRightScanCount ();
            int m2 = rdg->getLeftScanCount ();
            for (int i = m1; i <= m2; i++)
            {
              Bump *bmp = rdg->bump (i);
              if (bmp->hasMeasureLine ())
              {
                float trsl = bmp->getMeasureLineTranslationRatio ();
                float rot = bmp->getMeasureLineRotationRatio ();
                output << i << " " << trsl << " " << rot << std::endl;
              }
            }
          }
          output.close ();
        }
      }
    }
  }
}


void ILSDDetectionWidget::loadStructure (const std::vector<std::string>& paths)
{
  std::string path = (paths.size () == 0 ? "" : paths[0]);
  IniLoader iload (path.c_str ());
  int mode = iload.GetPropertyAsInt ("ASD", "DetectionMode", det_mode);
  if (! checkStroke (&iload))
  {
    std::cout << "Not the correct tiles" << std::endl;
    return;
  }

  if (mode != det_mode)
  {
    setDetectionMode (mode);
    new_title = true;
  }
  if (det_mode == MODE_RIDGE || det_mode == MODE_HOLLOW) loadRidge (&iload);
  else if (det_mode == MODE_CTRACK) loadCarTrack (&iload);
  if (! loadStroke (&iload))
  {
    std::cout << "Not the correct tiles" << std::endl;
    return;
  }

  if (det_mode == MODE_RIDGE || det_mode == MODE_HOLLOW)
  {
    if (iload.GetPropertyAsInt ("Ridge", "NumberOfMeasureLines", 0) != 0)
    {
      Ridge *rdg = rdetector.getRidge ();
      if (rdg != NULL)
      {
        size_t suff = path.find_last_of ('.');
        if (suff != std::string::npos)
        {
          string cpltname = path.substr (0, suff) + DEFAULT_MLINE_SUFFIX;
          ifstream input (cpltname.c_str (), ios::in);
          if (input)
          {
            int num;
            float trsl, rot;
            input >> num;
            while (! input.eof ())
            {
              input >> trsl;
              input >> rot;
              Bump *bmp = rdg->bump (num);
              if (bmp != NULL)
              {
                bmp->setMeasureLineTranslationRatio (trsl);
                bmp->setMeasureLineRotationRatio (rot);
              }
              input >> num;
            }
          }
          input.close ();
        }
      }
    }
  }
}


void ILSDDetectionWidget::exportShape (const std::string& path)
{
  std::vector<Pt2i> pts;
  std::vector<Pt2i> pts2;
  if (det_mode == MODE_CTRACK)
  {
    CarriageTrack* ct = tdetector.getCarriageTrack ();
    if (ct != NULL)
      ct->getPosition (pts, pts2, ctrack_style, iratio, smoothed_plateaux);
  }
  else if (det_mode & MODE_RIDGE_OR_HOLLOW)
  {
    Ridge* rdg = rdetector.getRidge ();
    if (rdg != NULL)
      rdg->getPosition (pts, pts2, ridge_style, iratio, smoothed_bumps);
  }

  if (! pts.empty ())
  {
    bool bounds = (det_mode == MODE_CTRACK)
                  || ((det_mode & MODE_RIDGE_OR_HOLLOW)
                      && (ridge_style <= RIDGE_DISP_BOUNDS));
    int sz = (int) (pts.size ());
    if (bounds) sz = 2 * sz + 1;
    double *x = new double[sz];
    double *y = new double[sz];
    double *ax = x;
    double *ay = y;
    std::vector<Pt2i>::iterator it = pts.begin ();
    while (it != pts.end ())
    {
      *ax++ = ((double) (ptset.xref () + it->x () * 500 + 25)) / 1000;
      *ay++ = ((double) (ptset.yref () + it->y () * 500 + 25)) / 1000;
      it ++;
    }
    if (bounds)
    {
      it = pts2.end ();
      do
      {
        it --;
        *ax++ = ((double) (ptset.xref () + it->x () * 500 + 25)) / 1000;
        *ay++ = ((double) (ptset.yref () + it->y () * 500 + 25)) / 1000;
      }
      while (it != pts2.begin ());
      *ax = *x;
      *ay = *y;
    }

    SHPHandle hSHPHandle = SHPCreate (path.c_str (), SHPT_ARC);
    SHPObject *shp = SHPCreateObject (SHPT_ARC, -1, 0, NULL, NULL, sz,
                                      x, y, NULL, NULL);
    SHPWriteObject (hSHPHandle, -1, shp);
    SHPDestroyObject (shp);
    SHPClose (hSHPHandle);
    delete [] x;
    delete [] y;
  }
}


void ILSDDetectionWidget::saveMeasure (const std::string& path)
{
  if (det_mode == MODE_NONE || det_mode == MODE_CTRACK
      || cp_view == NULL || ! ictrl.isMeasuring ()) return;
  IniLoader iload (path.c_str ());
  iload.SetPropertyAsInt ("ASD", "DetectionMode", det_mode);
  if (det_mode & MODE_RIDGE_OR_HOLLOW)
  {
    saveRidge (&iload);
    saveStroke (&iload);
    iload.SetPropertyAsBool ("Bump", "MeasureSet", ictrl.isSetMeasure ());
    if (ictrl.isSetMeasure ())
    {
      int m1 = ictrl.measureIntervalStart ();
      int m2 = ictrl.measureIntervalStop ();
      int nbb = (m2 < m1 ? m1 - m2 : m2 - m1);
      float lg2 = 0.0f, lg3 = 0.0f, zmin = 0.0f, zmax = 0.0f;
      float vlow = 0.0f, vhigh = 0.0f;
      float mwidth = 0.0f, sigw = 0.0f;
      float mheight = 0.0f, sigh = 0.0f;
      Ridge *rdg = rdetector.getRidge ();
      float mslope = rdg->estimateSlope (m1, m2, iratio, lg2, lg3, zmin, zmax);
      float vol = rdg->estimateVolume (m1, m2, iratio, vlow, vhigh);
      int nbmeas = rdg->meanWidth (m1, m2, 0.5f, mwidth, sigw);
      nbmeas = rdg->meanHeight (m1, m2, mheight, sigh);
      iload.SetPropertyAsInt ("Bump", "MeasureStart", m1);
      iload.SetPropertyAsInt ("Bump", "MeasureStop", m2);
      iload.SetPropertyAsDouble ("Measure", "MinHeight", (double) zmin);
      iload.SetPropertyAsDouble ("Measure", "MaxHeight", (double) zmax);
      iload.SetPropertyAsDouble ("Measure", "HeightDiff",
                                 (double) (zmax - zmin));
      iload.SetPropertyAsDouble ("Measure", "HorLength", (double) lg2);
      iload.SetPropertyAsDouble ("Measure", "Length", (double) lg3);
      iload.SetPropertyAsDouble ("Measure", "MeanSlope", (double) mslope);
      iload.SetPropertyAsDouble ("Measure", "Volume", (double) vol);
      iload.SetPropertyAsDouble ("Measure", "LowVolume", (double) vlow);
      iload.SetPropertyAsDouble ("Measure", "HighVolume", (double) vhigh);
      iload.SetPropertyAsInt ("Measure", "MeasuredBumps", nbmeas);
      iload.SetPropertyAsInt ("Measure", "CrossedBumps", nbb);
      iload.SetPropertyAsDouble ("Measure", "MeanWidth", (double) mwidth);
      iload.SetPropertyAsDouble ("Measure", "WidthStdDev", (double) sigw);
      iload.SetPropertyAsDouble ("Measure", "MeanHeight", (double) mheight);
      iload.SetPropertyAsDouble ("Measure", "HeightStdDev", (double) sigh);
    }
    else
    {
      iload.SetPropertyAsInt ("Bump", "SelectedScan", ictrl.scan ());
    }
  }
  iload.Save ();
}


void ILSDDetectionWidget::loadMeasure (const std::vector<std::string>& paths)
{
  std::string path = (paths.size () == 0 ? "" : paths[0]);
  IniLoader iload (path.c_str ());
  int mode = iload.GetPropertyAsInt ("ASD", "DetectionMode", det_mode);
  if (! checkStroke (&iload))
  {
    std::cout << "Not the correct tiles" << std::endl;
    return;
  }

  if (mode != det_mode)
  {
    setDetectionMode (mode);
    new_title = true;
  }
  if (det_mode & MODE_RIDGE_OR_HOLLOW) loadRidge (&iload);
  else if (det_mode == MODE_CTRACK) loadCarTrack (&iload);
  if (! loadStroke (&iload))
  {
    std::cout << "Not the correct tiles" << std::endl;
    return;
  }
  if (cp_view == NULL) switchCrossProfileAnalyzer ();
  if (! ictrl.isMeasuring ())
  {
    ictrl.switchMeasuring ();
    if (! rdetector.model()->isMeasured ())
      rdetector.model()->switchMeasured ();
    detect ();
  }
  if (iload.GetPropertyAsBool ("Bump", "MeasureSet", false))
  {
    ictrl.setScan (iload.GetPropertyAsInt ("Bump", "MeasureStart", 0));
    ictrl.setMeasureStart ();
    ictrl.setScan (iload.GetPropertyAsInt ("Bump", "MeasureStop", 0));
    ictrl.setMeasureStop ();
  }
  else
  {
    if (det_mode & MODE_RIDGE_OR_HOLLOW)
      cp_view->setScan (iload.GetPropertyAsInt ("Bump", "SelectedScan", 0));
    ictrl.setMeasureStart ();
    ictrl.setMeasureStop ();
  }
  cp_view->update ();
}


void ILSDDetectionWidget::saveRidge (IniLoader *ild)
{
  ild->SetPropertyAsBool ("Ridge", "DirectionAware",
                rdetector.model()->isDeviationPredictionOn ());
  ild->SetPropertyAsBool ("Ridge", "SlopeAware",
                rdetector.model()->isSlopePredictionOn ());
  ild->SetPropertyAsInt ("Ridge", "BumpLackTolerance",
               rdetector.getBumpLackTolerance ());
  ild->SetPropertyAsDouble ("Ridge", "BumpMinWidth",
               (double) (rdetector.model()->minWidth ()));
  ild->SetPropertyAsDouble ("Ridge", "BumpMinHeight",
               (double) (rdetector.model()->minHeight ()));
  ild->SetPropertyAsBool ("Ridge", "MassCenterRef",
                rdetector.model()->massReferenced ());
  ild->SetPropertyAsInt ("Ridge", "PositionControl",
               rdetector.model()->positionControl ());
  ild->SetPropertyAsDouble ("Ridge", "MaxPositionShift",
               (double) (rdetector.model()->positionShiftTolerance ()));
  ild->SetPropertyAsDouble ("Ridge", "MaxPositionRelShift",
               (double) (rdetector.model()->positionRelShiftTolerance ()));
  ild->SetPropertyAsInt ("Ridge", "AltitudeControl",
               rdetector.model()->altitudeControl ());
  ild->SetPropertyAsDouble ("Ridge", "MaxAltitudeShift",
               (double) (rdetector.model()->altitudeShiftTolerance ()));
  ild->SetPropertyAsDouble ("Ridge", "MaxAltitudeRelShift",
               (double) (rdetector.model()->altitudeRelShiftTolerance ()));
  ild->SetPropertyAsInt ("Ridge", "WidthControl",
               rdetector.model()->widthControl ());
  ild->SetPropertyAsDouble ("Ridge", "MaxWidthShift",
               (double) (rdetector.model()->widthShiftTolerance ()));
  ild->SetPropertyAsDouble ("Ridge", "MaxWidthRelShift",
               (double) (rdetector.model()->widthRelShiftTolerance ()));
  ild->SetPropertyAsInt ("Ridge", "HeightControl",
               rdetector.model()->heightControl ());
  ild->SetPropertyAsDouble ("Ridge", "MaxHeightShift",
               (double) (rdetector.model()->heightShiftTolerance ()));
  ild->SetPropertyAsDouble ("Ridge", "MaxHeightRelShift",
               (double) (rdetector.model()->heightRelShiftTolerance ()));
  ild->SetPropertyAsBool ("Ridge", "WithTrend",
               rdetector.model()->isDetectingTrend ());
  ild->SetPropertyAsInt ("Ridge", "TrendMinPinch",
               rdetector.model()->trendMinPinch ());
  if (rdetector.getRidge () != NULL)
    ild->SetPropertyAsInt ("Ridge", "NumberOfMeasureLines",
                           rdetector.getRidge()->countOfMeasureLines ());
}


void ILSDDetectionWidget::loadRidge (IniLoader *ild)
{
  bool val = rdetector.model()->isDeviationPredictionOn ();
  if (ild->GetPropertyAsBool ("Ridge", "DirectionAware", val) != val)
        rdetector.model()->switchDeviationPrediction ();
  val = rdetector.model()->isSlopePredictionOn ();
  if (ild->GetPropertyAsBool ("Ridge", "SlopeAware", val) != val)
        rdetector.model()->switchSlopePrediction ();
  rdetector.setBumpLackTolerance (ild->GetPropertyAsInt("Ridge",
        "BumpLackTolerance", rdetector.getBumpLackTolerance ()));
  rdetector.model()->setMinWidth (
        (float) ild->GetPropertyAsDouble ("Ridge", "BumpMinWidth",
                (double) (rdetector.model()->minWidth ())));
  rdetector.model()->setMinHeight (
        (float) ild->GetPropertyAsDouble ("Ridge", "BumpMinHeight",
                (double) (rdetector.model()->minHeight ())));
  val = rdetector.model()->massReferenced ();
  if (ild->GetPropertyAsBool ("Ridge", "MassCenterRef", val) != val)
        rdetector.model()->switchCenterReference ();
  rdetector.model()->setPositionControl (ild->GetPropertyAsInt (
        "Ridge", "PositionControl", rdetector.model()->positionControl ()));
  rdetector.model()->setPositionShiftTolerance(
        (float) ild->GetPropertyAsDouble ("Ridge", "MaxPositionShift",
                (double) (rdetector.model()->positionShiftTolerance ())));
  rdetector.model()->setPositionRelShiftTolerance (
        (float) ild->GetPropertyAsDouble("Ridge", "MaxPositionRelShift",
		(double) (rdetector.model()->positionRelShiftTolerance())));
  rdetector.model()->setAltitudeControl (ild->GetPropertyAsInt (
        "Ridge", "AltitudeControl", rdetector.model()->altitudeControl ()));
  rdetector.model()->setAltitudeShiftTolerance (
        (float) ild->GetPropertyAsDouble ("Ridge", "MaxAltitudeShift",
                (double) (rdetector.model()->altitudeShiftTolerance ())));
  rdetector.model()->setAltitudeRelShiftTolerance (
        (float) ild->GetPropertyAsDouble ("Ridge", "MaxAltitudeRelShift",
                (double) (rdetector.model()->altitudeRelShiftTolerance ())));
  rdetector.model()->setWidthControl (ild->GetPropertyAsInt (
        "Ridge", "WidthControl", rdetector.model()->widthControl ()));
  rdetector.model()->setWidthShiftTolerance (
        (float) ild->GetPropertyAsDouble ("Ridge", "MaxWidthShift",
                (double) (rdetector.model()->widthShiftTolerance ())));
  rdetector.model()->setWidthRelShiftTolerance (
        (float) ild->GetPropertyAsDouble ("Ridge", "MaxWidthRelShift",
                (double) (rdetector.model()->widthRelShiftTolerance ())));
  rdetector.model()->setHeightControl (ild->GetPropertyAsInt (
        "Ridge", "HeightControl", rdetector.model()->heightControl ()));
  rdetector.model()->setHeightShiftTolerance (
        (float) ild->GetPropertyAsDouble ("Ridge", "MaxHeightShift",
                (double) (rdetector.model()->heightShiftTolerance ())));
  rdetector.model()->setHeightRelShiftTolerance (
        (float) ild->GetPropertyAsDouble ("Ridge", "MaxHeightRelShift",
                (double) (rdetector.model()->heightRelShiftTolerance ())));
  val = rdetector.model()->isDetectingTrend ();
  if (ild->GetPropertyAsBool ("Ridge", "WithTrend", val) != val)
        rdetector.model()->switchDetectingTrend ();
  rdetector.model()->setTrendMinPinch (ild->GetPropertyAsInt (
        "Ridge", "TrendMinPinch", rdetector.model()->trendMinPinch ()));
}


void ILSDDetectionWidget::saveCarTrack (IniLoader *ild)
{
  ild->SetPropertyAsBool ("CTrack", "InitialDetection",
        tdetector.isInitializationOn ());
  ild->SetPropertyAsBool ("CTrack", "DensityCheck",
        tdetector.isDensitySensitive ());
  ild->SetPropertyAsBool ("CTrack", "DirectionAware",
        tdetector.model()->isDeviationPredictionOn ());
  ild->SetPropertyAsBool ("CTrack", "SlopeAware",
        tdetector.model()->isSlopePredictionOn ());
  ild->SetPropertyAsInt ("CTrack", "PlateauLackTolerance",
        tdetector.getPlateauLackTolerance ());
  ild->SetPropertyAsInt ("CTrack", "PlateauMaxTilt",
        tdetector.model()->bsMaxTilt ());
  ild->SetPropertyAsDouble ("CTrack", "PlateauMinLength",
        (double) (tdetector.model()->minLength ()));
  ild->SetPropertyAsDouble ("CTrack", "PlateauMaxLength",
        (double) (tdetector.model()->maxLength ()));
  ild->SetPropertyAsDouble ("CTrack", "MaxThicknessShift",
        (double) (tdetector.model()->thicknessTolerance ()));
  ild->SetPropertyAsDouble ("CTrack", "MaxSlopeShift",
        (double) (tdetector.model()->slopeTolerance ()));
  ild->SetPropertyAsDouble ("CTrack", "MaxPositionShift",
        (double) (tdetector.model()->sideShiftTolerance ()));
  ild->SetPropertyAsBool ("CTrack", "CenterStabilityTest",
        tdetector.isShiftLengthPruning ());
  ild->SetPropertyAsDouble ("CTrack", "MaxCenterShift",
        (double) (tdetector.maxShiftLength ()));
  ild->SetPropertyAsBool ("CTrack", "DetectionRatioTest",
        tdetector.isDensityPruning ());
  ild->SetPropertyAsInt ("CTrack", "MaxUndetectedRatio",
        tdetector.minDensity ());
  ild->SetPropertyAsBool ("CTrack", "TailCompactnessTest",
        (tdetector.tailPruning() != 0));
  ild->SetPropertyAsInt ("CTrack", "MinTailLength",
        tdetector.model()->tailMinSize ());
}


void ILSDDetectionWidget::loadCarTrack (IniLoader *ild)
{
  bool val = tdetector.isInitializationOn ();
  if (ild->GetPropertyAsBool ("CTrack", "InitialDetection", val) != val)
      tdetector.switchInitialization ();
  val = tdetector.isDensitySensitive ();
  if (ild->GetPropertyAsBool ("CTrack", "DensityCheck", val) != val)
      tdetector.switchDensitySensitivity ();
  val = tdetector.model()->isDeviationPredictionOn ();
  if (ild->GetPropertyAsBool ("CTrack", "DirectionAware", val) != val)
      tdetector.model()->switchDeviationPrediction ();
  val = tdetector.model()->isSlopePredictionOn ();
  if (ild->GetPropertyAsBool ("CTrack", "SlopeAware", val) != val)
      tdetector.model()->switchSlopePrediction ();
  tdetector.setPlateauLackTolerance (ild->GetPropertyAsInt (
      "CTrack", "PlateauLackTolerance", tdetector.getPlateauLackTolerance ()));
  tdetector.model()->setBSmaxTilt (ild->GetPropertyAsInt (
      "CTrack", "PlateauMaxTilt", tdetector.model()->bsMaxTilt ()));
  tdetector.model()->setMinLength (
      (float) ild->GetPropertyAsDouble ("CTrack", "PlateauMinLength",
            (double) (tdetector.model()->minLength ())));
  tdetector.model()->setMaxLength (
      (float) ild->GetPropertyAsDouble ("CTrack", "PlateauMaxLength",
            (double) (tdetector.model()->maxLength ())));
  tdetector.model()->setThicknessTolerance (
      (float) ild->GetPropertyAsDouble ("CTrack", "MaxThicknessShift",
            (double) (tdetector.model()->thicknessTolerance ())));
  tdetector.model()->setSlopeTolerance (
      (float) ild->GetPropertyAsDouble ("CTrack", "MaxSlopeShift",
            (double) (tdetector.model()->slopeTolerance ())));
  tdetector.model()->setSideShiftTolerance (
      (float) ild->GetPropertyAsDouble ("CTrack", "MaxPositionShift",
            (double) (tdetector.model()->sideShiftTolerance ())));
  val = tdetector.isShiftLengthPruning ();
  if (ild->GetPropertyAsBool ("CTrack", "CenterStabilityTest", val) != val)
      tdetector.switchShiftLengthPruning ();
  tdetector.setMaxShiftLength (
      (float) ild->GetPropertyAsDouble ("CTrack", "MaxCenterShift",
            (double) (tdetector.maxShiftLength ())));
  val = tdetector.isDensityPruning ();
  if (ild->GetPropertyAsBool ("CTrack", "DetectionRatioTest", val) != val)
      tdetector.switchDensityPruning ();
  tdetector.setMinDensity (ild->GetPropertyAsInt (
      "CTrack", "MaxUndetectedRatio", tdetector.minDensity ()));
  val = (tdetector.tailPruning () != 0);
  if (val)
  {
    if (ild->GetPropertyAsBool ("CTrack", "TailCompactnessTest", val) != val)
      tdetector.switchTailPruning ();
    if (tdetector.tailPruning() != 0) tdetector.switchTailPruning ();
  }
  else
  {
    if (ild->GetPropertyAsBool ("CTrack", "TailCompactnessTest", val) != val)
      tdetector.switchTailPruning ();
  }
  tdetector.model()->setTailMinSize (ild->GetPropertyAsInt (
      "CTrack", "MinTailLength", tdetector.model()->tailMinSize ()));
}


void ILSDDetectionWidget::saveStroke (IniLoader *ild)
{
  ild->SetPropertyAsInt ("Stroke", "TileX", (int) (ptset.xref () / 100000));
  ild->SetPropertyAsInt ("Stroke", "TileY", (int) (ptset.yref () / 100000));
  if (ptset.xref () % 100000 != 0)
    ild->SetPropertyAsInt ("Stroke", "BalanceX",
                           (int) (ptset.xref () % 100000));
  if (ptset.yref () % 100000 != 0)
    ild->SetPropertyAsInt ("Stroke", "BalanceY",
                           (int) (ptset.yref () % 100000));
  ild->SetPropertyAsInt ("Stroke", "StartPointX", p1.x ());
  ild->SetPropertyAsInt ("Stroke", "StartPointY", p1.y ());
  ild->SetPropertyAsInt ("Stroke", "EndPointX", p2.x ());
  ild->SetPropertyAsInt ("Stroke", "EndPointY", p2.y ());
}


bool ILSDDetectionWidget::checkStroke (IniLoader *ild)
{
  int64_t tx, ty, bx, by;
  int sx, sy, ex, ey;
  tx = ild->GetPropertyAsInt("Stroke", "TileX", 0);
  ty = ild->GetPropertyAsInt("Stroke", "TileY", 0);
  bx = ild->GetPropertyAsInt("Stroke", "BalanceX", 0);
  by = ild->GetPropertyAsInt("Stroke", "BalanceY", 0);
  sx = ild->GetPropertyAsInt("Stroke", "StartPointX", 0);
  sy = ild->GetPropertyAsInt("Stroke", "StartPointY", 0);
  ex = ild->GetPropertyAsInt("Stroke", "EndPointX", 0);
  ey = ild->GetPropertyAsInt("Stroke", "EndPointY", 0);
  int x1 = sx + (int) (tx * 200 + (bx - ptset.xref ()) / 500);
  if (x1 < 0 || x1 >= dtm_map.width ()) return false;
  int y1 = sy + (int) (ty * 200 + (by - ptset.yref ()) / 500);
  if (y1 < 0 || y1 >= dtm_map.height ()) return false;
  int x2 = ex + (int) (tx * 200 + (bx - ptset.xref ()) / 500);
  if (x2 < 0 || x2 >= dtm_map.width ()) return false;
  int y2 = ey + (int) (ty * 200 + (by - ptset.yref ()) / 500);
  if (y2 < 0 || y2 >= dtm_map.height ()) return false;
  return true;
}


bool ILSDDetectionWidget::loadStroke (IniLoader *ild)
{
  int64_t tx, ty, bx, by;
  int sx, sy, ex, ey;
  tx = ild->GetPropertyAsInt("Stroke", "TileX", 0);
  ty = ild->GetPropertyAsInt("Stroke", "TileY", 0);
  bx = ild->GetPropertyAsInt("Stroke", "BalanceX", 0);
  by = ild->GetPropertyAsInt("Stroke", "BalanceY", 0);
  sx = ild->GetPropertyAsInt("Stroke", "StartPointX", 0);
  sy = ild->GetPropertyAsInt("Stroke", "StartPointY", 0);
  ex = ild->GetPropertyAsInt("Stroke", "EndPointX", 0);
  ey = ild->GetPropertyAsInt("Stroke", "EndPointY", 0);
  int x1 = sx + (int) (tx * 200 + (bx - ptset.xref ()) / 500);
  if (x1 < 0 || x1 >= dtm_map.width ()) return false;
  int y1 = sy + (int) (ty * 200 + (by - ptset.yref ()) / 500);
  if (y1 < 0 || y1 >= dtm_map.height ()) return false;
  int x2 = ex + (int) (tx * 200 + (bx - ptset.xref ()) / 500);
  if (x2 < 0 || x2 >= dtm_map.width ()) return false;
  int y2 = ey + (int) (ty * 200 + (by - ptset.yref ()) / 500);
  if (y2 < 0 || y2 >= dtm_map.height ()) return false;
  p1.set (x1, y1);
  p2.set (x2, y2);
  udef = true;
  detect (p1, p2);
  display ();
  std::cout << "Structure from (" << p1.x () << ", " << p1.y ()
            << ") (" << p2.x () << ", " << p2.y () << ")" << std::endl;
  return true;
}


void ILSDDetectionWidget::saveScreen (const std::string& path)
{
  augmentedImage.save (path.c_str ());
}


void ILSDDetectionWidget::selectTiles (const std::vector<std::string>& paths)
{
  if (paths.size () > 0)
  {
    reset ();
    std::string tname (std::string (DEFAULT_TILE_DIR)
                       + std::string (DEFAULT_TILE_FILE) + string (".txt"));
    ofstream output (tname.c_str (), ios::out);
    if (output)
    {
      std::vector<std::string>::const_iterator it = paths.begin ();
      while (it != paths.end ())
      {
        size_t suff = it->find_last_of ('.');
        if (suff == std::string::npos) suff = it->length ();
        size_t last = it->find_last_of ('\\');
        if (last == std::string::npos) last = it->find_last_of ('/');
        if (last == std::string::npos) return;
        output << it->substr (last + 1, suff - last - 1) << std::endl;
        it ++;
      }
      output.close ();
      loadTiles ();
    }
  }
}

void ILSDDetectionWidget::loadTiles ()
{
  loadTiles (DEFAULT_TILE_DIR + DEFAULT_TILE_FILE + string (".txt"));
}

void ILSDDetectionWidget::loadTiles (const std::string& path)
{
  dtm_map.clear ();
  ptset.clear ();
  tiles_loaded = false;

  char sval[TILE_NAME_MAX_LENGTH];
  ifstream input (path, ios::in);
  if (input)
  {
    bool reading = true;
    while (reading)
    {
      input >> sval;
      if (input.eof ()) reading = false;
      // else
      else if (sval != "")
      {
        string nvmfile (NVM_DIR);
        string ptsfile (TIL_DIR);
        nvmfile += sval + TerrainMap::NVM_SUFFIX;
        bool tl = dtm_map.addNormalMapFile (nvmfile);
        if (tl) tiles_loaded = ptset.addTile (TIL_DIR, std::string (sval),
                                              cloud_access) || tiles_loaded;
      }
    }
  }
  else std::cerr << "Failed to open file " << path << std::endl;
  if (tiles_loaded) createMap ();
}


void ILSDDetectionWidget::createMap ()
{
  tiles_loaded = ptset.create ();
  if (tiles_loaded)
    tiles_loaded = dtm_map.assembleMap (
                     ptset.columnsOfTiles (), ptset.rowsOfTiles (),
                     ptset.xref (), ptset.yref ());
  if (tiles_loaded)
  {
    width = dtm_map.width ();
    height = dtm_map.height ();
    cellsize = dtm_map.cellSize ();

    tdetector.setPointsGrid (&ptset, width, height, SUBDIV, cellsize);
    rdetector.setPointsGrid (&ptset, width, height, SUBDIV, cellsize);
    iratio = width / ptset.xmSpread ();

    loadedImage = ASImage (ASCanvasPos (width, height));
    for (int j = 0; j < height; j++)
      for (int i = 0; i < width; i++)
      {
        int val = dtm_map.get (i, j);
        loadedImage.setPixelGrayscale (i, j, val);
      }
    augmentedImage = loadedImage;

    if (cp_view != NULL) cp_view->setData (&loadedImage, &ptset);
    if (lp_view != NULL) lp_view->setData (&loadedImage, &ptset);

    xMaxShift = (width > maxWidth ? maxWidth - width : 0);
    yMaxShift = (height > maxHeight ? maxHeight - height : 0);
  }
}


ASCanvasPos ILSDDetectionWidget::widgetSize ()
{
  return ASCanvasPos (width, height);
}


void ILSDDetectionWidget::rebuildImage ()
{
  for (int j = 0; j < height; j++)
    for (int i = 0; i < width; i++)
    {
      int val = dtm_map.get (i, j);
      loadedImage.setPixelGrayscale (i, j, (char) val);
    }
  augmentedImage = loadedImage;
}


bool ILSDDetectionWidget::titleChanged ()
{
  if (new_title)
  {
    new_title = false;
    return true;
  }
  return false;
}

string ILSDDetectionWidget::getTitle () const
{
  if (det_mode == MODE_CTRACK) return string ("ILSD: track");
  else if (det_mode == MODE_RIDGE) return string ("ILSD: ridge");
  else if (det_mode == MODE_HOLLOW) return string ("ILSD: hollow");
  else return string ("ILSD");
}


int** ILSDDetectionWidget::getBitmap (const ASImage& image)
{
  return (int**) image.getBitmap ();
}


void ILSDDetectionWidget::setCloudAccess (int type)
{
  string prefix (TIL_DIR);
  if (type == IPtTile::TOP)
    prefix += IPtTile::TOP_DIR + IPtTile::TOP_PREFIX;
  else if (type == IPtTile::MID)
    prefix += IPtTile::MID_DIR + IPtTile::MID_PREFIX;
  else if (type == IPtTile::ECO)
    prefix += IPtTile::ECO_DIR + IPtTile::ECO_PREFIX; 
  ptset.updateAccessType (cloud_access, type, prefix);
  cloud_access = type;
}


void ILSDDetectionWidget::toggleBackground ()
{
  if (background++ == BACK_IMAGE) background = BACK_BLACK;
}


void ILSDDetectionWidget::incZoom (int dir)
{
  zoom += dir;
  if (zoom < -10) zoom = -10;
  if (zoom > 10) zoom = 10;
}


bool ILSDDetectionWidget::saveAugmentedImage (const string& fileName,
                                              const char* fileFormat)
{
  ASImage aImage = augmentedImage;
  return (aImage.save (fileName.data (), fileFormat));
}


void ILSDDetectionWidget::invertInputStroke ()
{
  Pt2i tmp (p1);
  p1.set (p2);
  p2.set (tmp);
  if (udef) detect (p1, p2);
  if (cp_view && udef && ! p1.equals (p2)) cp_view->update ();
  if (lp_view && udef && ! p1.equals (p2)) lp_view->update ();
  display ();
}

const string ILSDDetectionWidget::getStrokeFileName () const
{
  return (DEFAULT_STROKE_DIR + DEFAULT_STROKE_FILE + string (".txt"));
}

void ILSDDetectionWidget::saveStroke (const std::string& path)
{
  ofstream output (path.c_str (), ios::out);
  if (output)
  {
    output << ptset.xref () + p1.x () * 500 + 25 << " "
           << ptset.yref () + p1.y () * 500 + 25 << std::endl;
    output << ptset.xref () + p2.x () * 500 + 25 << " "
           << ptset.yref () + p2.y () * 500 + 25 << std::endl;
    output.close ();
  }
}

void ILSDDetectionWidget::loadStroke (const std::vector<std::string>& paths)
{
  std::string path = (paths.size () == 0 ? "" : paths[0]);
  int64_t val[4], i = 0;
  ifstream input (path.c_str (), ios::in);
  bool reading = true;
  if (input)
  {
    while (reading)
    {
      input >> val[i];
      if (input.eof ()) reading = false;
      else if (++i == 4) reading = false;
    }
    if (i == 4)
    {
      udef = true;
      p1.set ((int) ((val[0] - ptset.xref ()) / 500),
              (int) ((val[1] - ptset.yref ()) / 500));
      p2.set ((int) ((val[2] - ptset.xref ()) / 500),
              (int) ((val[3] - ptset.yref ()) / 500));
      std::cout << "Run test on (" << val[0] << ", " << val[1]
                << ") (" << val[2] << ", " << val[3] << ")" << std::endl;
    }
    else
    {
      std::cout << "Test file damaged (test.txt)" << std::endl;
      return;
    }
  }
  else
  {
    std::cout << "No test file (test.txt)" << std::endl;
    return;
  }

  detect (p1, p2);
  display ();
  std::cout << "Test run on (" << p1.x () << ", " << p1.y ()
            << ") (" << p2.x () << ", " << p2.y () << ")" << std::endl;
}


void ILSDDetectionWidget::addToSelection (const std::string& path)
{
  ofstream output (path.c_str (), ios::app);
  output << ptset.xref () + p1.x () * 500 + 25 << " "
         << ptset.yref () + p1.y () * 500 + 25 << " "
         << ptset.xref () + p2.x () * 500 + 25 << " "
         << ptset.yref () + p2.y () * 500 + 25 << std::endl;
  output.close ();
  std::cout << "Selection added to " << path << std::endl;
  if (disp_saved) loadSelection ({ path });
  else display ();
}


void ILSDDetectionWidget::loadSelection (const std::vector<std::string>& paths)
{
  std::string path = (paths.size () == 0 ? "" : paths[0]);
  int64_t x1, y1, x2, y2;
  ifstream strk (path.c_str (), ios::in);
  if (! strk) std::cout << path << " file not found" << std::endl;
  else
  {
    savmap.clear ();
    savstroke.clear ();
    augmentedImage.clear (ASColor::WHITE);
    ASPainter painter (&augmentedImage);
    strk >> x1;
    while (! strk.eof ())
    {
      strk >> y1;
      strk >> x2;
      strk >> y2;
      Pt2i pi1 ((int) ((x1 - ptset.xref ()) / 500),
                (int) ((y1 - ptset.yref ()) / 500));
      Pt2i pi2 ((int) ((x2 - ptset.xref ()) / 500),
                (int) ((y2 - ptset.yref ()) / 500));
      savstroke.push_back (pi1);
      savstroke.push_back (pi2);
      if (pi1.x () >= 0 && pi1.y () >= 0
          && pi1.x () < width && pi1.y () < height
          && pi2.x () >= 0 && pi2.y () >= 0
          && pi2.x () < width && pi2.y () < height)
      {
        drawSelection (painter, pi1, pi2);
        painter.setPen (ASPen (structure_color, track_width));
        if (det_mode == MODE_CTRACK)
        {
          tdetector.detect (pi1, pi2);
          if (ctrack_style != CTRACK_DISP_SCANS)
            displayConnectedTrack (painter, ASColor::BLACK);
          else
            displayCarriageTrack (painter, ASColor::BLACK);
        }
        else if (det_mode & MODE_RIDGE_OR_HOLLOW)
        {
          rdetector.detect (pi1, pi2);
          if (ridge_style != RIDGE_DISP_SCANS)
            displayConnectedRidge (painter, ASColor::BLACK);
          else displayRidge (painter, ASColor::BLACK);
        }
      }
      strk >> x1;
    }
    strk.close ();
    if (det_mode == MODE_CTRACK) tdetector.clear ();
    else if (det_mode & MODE_RIDGE_OR_HOLLOW) rdetector.clear ();
    for (int j = 0; j < height; j++)
    {
      for (int i = 0; i < width; i++)
      {
        if (augmentedImage.GetPixelColor (i, height - 1 - j).r < 10)
        {
          savmap.push_back (Pt2i (i, j));
        }
      }
    }
    display ();
  }
}


void ILSDDetectionWidget::clearImage ()
{
  augmentedImage.clear (ASColor::WHITE);
}


void ILSDDetectionWidget::closeCrossProfileView ()
{
  if (cp_view)
  {
    cp_view->CloseView ();
    cp_view = nullptr;
  }
}


void ILSDDetectionWidget::closeLongProfileView ()
{
  if (lp_view)
  {
    lp_view->CloseView ();
    lp_view = nullptr;
  }
}


void ILSDDetectionWidget::updateWidget ()
{
  to_update = true;
  with_aux_update = true;
}


void ILSDDetectionWidget::paintEvent (GLWindow* drawWindow)
{
  int resX, resY;
  glfwGetWindowSize (drawWindow->getGlfwContext (), &resX, &resY);
  ImGui::SetNextWindowSize (ImVec2 ((float) resX, (float) resY));
  ImGui::SetNextWindowPos (ImVec2 (0, 0));
  if (tiles_loaded)
  {
    if (ImGui::Begin ("DebugWindow", NULL,
                      ImGuiWindowFlags_NoBringToFrontOnFocus
                      | ImGuiWindowFlags_NoSavedSettings
                      | ImGuiWindowFlags_NoBackground
                      | ImGuiWindowFlags_NoMove
                      | ImGuiWindowFlags_NoNav
                      | ImGuiWindowFlags_NoDecoration
                      | ImGuiWindowFlags_NoInputs))
    {
//      with_aux_update = false;
      if (to_update) displayDetectionResult ();
//      with_aux_update = true;
      augmentedImage.setZoom (zoom);
      augmentedImage.setDisplayPosition (xShift, yShift);
      augmentedImage.Draw (drawWindow);
      ImGui::End ();
    }
  }
}


void ILSDDetectionWidget::setDetectionMode (int mode)
{
  if (mode == det_mode) return;
  bool cp_rh = false;
  bool lp_rh = false;
  bool cp_open = (cp_view != NULL);
  if (cp_open && ((det_mode | mode) == MODE_RIDGE_OR_HOLLOW))
  {
    cp_rh = true;
    cp_open = false; // Useless to close to change ridge <-> hollow
  }
  bool lp_open = (lp_view != NULL);
  if (lp_open && ((det_mode | mode) == MODE_RIDGE_OR_HOLLOW))
  {
    lp_rh = true;
    lp_open = false; // Useless to close to change ridge <-> hollow
  }
  if (cp_open) switchCrossProfileAnalyzer ();
  if (lp_open) switchLongProfileAnalyzer ();
  disp_saved = false;
  det_mode = mode;
  if (det_mode & MODE_RIDGE_OR_HOLLOW)
    rdetector.setOver (det_mode == MODE_RIDGE);
  if (cp_open) switchCrossProfileAnalyzer ();
  else if (cp_rh) cp_view->update ();
  if (lp_open) switchLongProfileAnalyzer ();
  else if (lp_rh) lp_view->update ();
  if (udef) detect (p1, p2);
}


ILSDCrossProfileView *ILSDDetectionWidget::getCrossProfileView () const
{
  return (cp_view);
}

ILSDLongProfileView *ILSDDetectionWidget::getLongProfileView () const
{
  return (lp_view);
}


void ILSDDetectionWidget::toggleDetectionMode ()
{
  if (det_mode == MODE_NONE) setDetectionMode (MODE_RIDGE);
  else if (det_mode == MODE_RIDGE) setDetectionMode (MODE_HOLLOW);
  else if (det_mode == MODE_HOLLOW) setDetectionMode (MODE_CTRACK);
  else if (det_mode == MODE_CTRACK) setDetectionMode (MODE_NONE);
}


void ILSDDetectionWidget::switchCrossProfileAnalyzer ()
{
  if (cp_view)
  {
    if (lp_view == NULL)
    {
      if (det_mode == MODE_CTRACK) tdetector.recordProfile (false);
      else if (det_mode & MODE_RIDGE_OR_HOLLOW) rdetector.recordProfile (false);
    }
    ini_load->SetPropertyAsInt ("AnalysisView", "Xpos",
                                cp_view->GetWindowPos().x);
    ini_load->SetPropertyAsInt ("AnalysisView", "Ypos",
                                cp_view->GetWindowPos().y);
    closeCrossProfileView ();
    cp_view = nullptr;
  }
  else
  {
    ASCanvasPos pos;
    bool exists = previousCrossProfilePosition (pos);
    if (det_mode == MODE_CTRACK)
    {
      if (lp_view == NULL)
      {
        tdetector.recordProfile (true);
        detect ();
      }
      cp_view = new ILSDCrossProfileView (GLWindow::getMainWindow (), exists,
                                          pos, &tdetector, &ictrl, this);
    }
    else if (det_mode & MODE_RIDGE_OR_HOLLOW)
    {
      if (lp_view == NULL)
      {
        std::vector<float> measures;
        Ridge *rdg = rdetector.getRidge ();
        if (rdg != NULL) rdg->getMeasureLines (measures);
        rdetector.recordProfile (true);
        detect ();
        if (rdg != NULL)
        {
          rdg = rdetector.getRidge ();
          if (rdg != NULL) rdg->setMeasureLines (measures);
        }
      }
      cp_view = new ILSDCrossProfileView (GLWindow::getMainWindow (), exists,
                                          pos, &rdetector, &ictrl, this);
    }
    else cp_view = new ILSDCrossProfileView (GLWindow::getMainWindow (), exists,
                                             pos, SUBDIV, &ictrl, this);
    cp_view->setData (&loadedImage, &ptset);
    cp_view->buildScans (p1, p2);
    cp_view->update ();
  }
}


void ILSDDetectionWidget::switchLongProfileAnalyzer ()
{
  if (lp_view)
  {
    if (cp_view == NULL)
    {
      if (det_mode == MODE_CTRACK) tdetector.recordProfile (false);
      else if (det_mode & MODE_RIDGE_OR_HOLLOW) rdetector.recordProfile (false);
    }
    ini_load->SetPropertyAsInt ("LongProfileView", "Xpos",
                                lp_view->GetWindowPos().x);
    ini_load->SetPropertyAsInt ("LongProfileView", "Ypos",
                                lp_view->GetWindowPos().y);
    closeLongProfileView ();
    lp_view = nullptr;
  }
  else
  {
    ASCanvasPos pos;
    bool exists = previousLongProfilePosition (pos);
    if (det_mode == MODE_CTRACK)
    {
      if (cp_view == NULL)
      {
        tdetector.recordProfile (true);
        detect ();
      }
      lp_view = new ILSDLongProfileView (GLWindow::getMainWindow (), exists,
                                         pos, &tdetector, &ictrl, this);
    }
    else if (det_mode & MODE_RIDGE_OR_HOLLOW)
    {
      if (cp_view == NULL)
      {
        std::vector<float> measures;
        Ridge *rdg = rdetector.getRidge ();
        if (rdg != NULL) rdg->getMeasureLines (measures);
        rdetector.recordProfile (true);
        detect ();
        if (rdg != NULL)
        {
          rdg = rdetector.getRidge ();
          if (rdg != NULL) rdg->setMeasureLines (measures);
        }
      }
      lp_view = new ILSDLongProfileView (GLWindow::getMainWindow (), exists,
                                         pos, &rdetector, &ictrl, this);
    }
    else lp_view = new ILSDLongProfileView (GLWindow::getMainWindow (), exists,
                                            pos, SUBDIV, &ictrl, this);
    lp_view->setData (&loadedImage, &ptset);
    lp_view->buildProfile (p1, p2);
    lp_view->update ();
  }
}


bool ILSDDetectionWidget::previousCrossProfilePosition (ASCanvasPos &pos) const
{
  pos.set (ini_load->GetPropertyAsInt ("AnalysisView", "Xpos", -12345),
           ini_load->GetPropertyAsInt ("AnalysisView", "Ypos", -12345));
  return (pos.x != -12345);
}


bool ILSDDetectionWidget::previousLongProfilePosition (ASCanvasPos &pos) const
{
  pos.set (ini_load->GetPropertyAsInt ("LongProfileView", "Xpos", -12345),
           ini_load->GetPropertyAsInt ("LongProfileView", "Ypos", -12345));
  return (pos.x != -12345);
}


void ILSDDetectionWidget::updateMeasuring ()
{
  if (ictrl.isMeasuring () != rdetector.isMeasured ())
  {
    rdetector.switchMeasured ();
    if (rdetector.isMeasured ())
    {
      Ridge *rdg = rdetector.getRidge ();
      if (rdg != NULL) rdg->updateMeasure ();
    }
    display ();
  }
}


void ILSDDetectionWidget::saveLastMeasure () const
{
}


void ILSDDetectionWidget::itemDeleted (AsImGuiWindow *item)
{
  if (item == cp_view)
  {
    ini_load->SetPropertyAsInt ("AnalysisView", "Xpos",
                                item->GetWindowPos().x);
    ini_load->SetPropertyAsInt ("AnalysisView", "Ypos",
                                item->GetWindowPos().y);
    cp_view = NULL;
  }
  else if (item == lp_view)
  {
    ini_load->SetPropertyAsInt ("LongProfileView", "Xpos",
                                item->GetWindowPos().x);
    ini_load->SetPropertyAsInt ("LongProfileView", "Ypos",
                                item->GetWindowPos().y);
    lp_view = NULL;
  }
}


void ILSDDetectionWidget::enableKeys ()
{
  popup_nb --;
}

void ILSDDetectionWidget::disableKeys ()
{
  popup_nb ++;
}


void ILSDDetectionWidget::mousePressEvent (GLWindow* parentWindow)
{
  if (ImGui::IsWindowHovered (ImGuiHoveredFlags_ChildWindows)
      || ! tiles_loaded) return;
  ASCanvasPos texPos;
  if (! augmentedImage.mouseToTexture (parentWindow, texPos)) return;

  int ex = ((int) texPos.x);
  int ey = height - 1 - ((int) texPos.y);

  oldp1.set (p1);
  oldp2.set (p2);
  oldudef = udef;
  p1.set (ex, ey);
  if (p1.manhattan (p2) < 10) p1.set (oldp1);
  else if (p1.manhattan (oldp1) < 10) p1.set (oldp2);
  else p2.set (p1);
  udef = true;
  bMousePressed = true;
}


void ILSDDetectionWidget::mouseReleaseEvent (GLWindow* parentWindow)
{
  if (! (bMousePressed && tiles_loaded)) return;
  bMousePressed = false;
  if (! GLWindow::getMainWindow()->IsBackgroundHovered ()) return;

  ASCanvasPos texPos;
  bool bFailed = ! augmentedImage.mouseToTexture (parentWindow, texPos);

  if (! picking)
  {
    int ex = ((int) texPos.x);
    int ey = ((int) texPos.y);
    p2.set (ex, height - 1 - ey);
    if (p1.equals (p2) || bFailed)
    {
      if (!savstroke.empty ()) selectStroke (p1);
      p1.set (oldp1);
      p2.set (oldp2);
      udef = oldudef;
    }
    else
    {
      std::cerr << "p1 (" << p1.x () << ", " << p1.y () << ") defined: "
                << (ptset.xref () + p1.x () * 500 + 25) << " "
                << (ptset.yref () + p1.y () * 500 + 25) << std::endl;
      std::cerr << "p2 (" << p2.x () << ", " << p2.y () << ") defined: "
                << (ptset.xref () + p2.x () * 500 + 25) << " "
                << (ptset.yref () + p2.y () * 500 + 25) << std::endl;
      if (udef) detect (p1, p2);
      display();
    }
  }
  nodrag = true;
}


void ILSDDetectionWidget::mouseMoveEvent (GLWindow* parentWindow)
{
  if (! GLWindow::getMainWindow()->IsBackgroundHovered ()
      || ! (bMousePressed && tiles_loaded)) return;

  ASCanvasPos texPos;
  if (! augmentedImage.mouseToTexture (parentWindow, texPos)) return;

  if (picking) picking = false;
  else
  {
    int ex = (int) (texPos.x);
    int ey = (int) (texPos.y);
    p2.set (ex, height - 1 - ey);
    if (verbose) std::cerr << "(" << p1.x () << ", " << p1.y () << ") ("
			   << p2.x () << ", " << p2.y () << ")" << std::endl;
    if (p1.manhattan (p2) > 5
        && (width > p2.x () && height > p2.y () && p2.x () > 0 && p2.y () > 0))
    {
      nodrag = false;
      if (udef) display ();
      nodrag = true;
    }
  }
}


void ILSDDetectionWidget::drawPoints (ASPainter& painter,
                                      vector<Pt2i> pts, ASColor color)
{
  vector<Pt2i>::iterator iter = pts.begin();
  while (iter != pts.end())
  {
    Pt2i p = *iter++;
    painter.setPen (ASPen (color, THIN_PEN, ASPenStyle::SolidLine,
                           ASPenCapStyle::RoundCap, ASPenJoinStyle::RoundJoin));
    if (p.x () < width && p.y () < height && p.x () >= 0 && p.y () >= 0)
      painter.drawPoint (ASCanvasPos (p.x (), height - 1 - p.y ()));
  }
}


void ILSDDetectionWidget::drawPixels (ASPainter& painter, vector<Pt2i> pix)
{
  vector<Pt2i>::iterator iter = pix.begin();
  while (iter != pix.end())
  {
    Pt2i p = *iter++;
    painter.setPen (ASPen (loadedImage.GetPixelColor (
                                p.x (), loadedImage.height () - 1 - p.y ()),
			   THIN_PEN, ASPenStyle::SolidLine,
			   ASPenCapStyle::RoundCap, ASPenJoinStyle::RoundJoin));
    if (p.x () < width && p.y () < height && p.x () >= 0 && p.y () >= 0)
      painter.drawPoint (ASCanvasPos (p.x (), height - 1 - p.y ()));
  }
}


void ILSDDetectionWidget::drawLine (ASPainter& painter,
                                    const Pt2i from, const Pt2i to,
                                    ASColor color, int width)
{
  int n;
  Pt2i* pts = from.drawing(to, &n);
  painter.setPen (ASPen(color, width, ASPenStyle::SolidLine,
                  ASPenCapStyle::RoundCap, ASPenJoinStyle::RoundJoin));
  for (int i = 0; i < n; i++)
    painter.drawPoint (ASCanvasPos (pts[i].x (), height - 1 - pts[i].y ()));
  delete[] pts;
}


void ILSDDetectionWidget::drawSelection (ASPainter& painter,
                                         const Pt2i from, const Pt2i to)
{
  if (sel_style == SEL_THICK)
    drawLine (painter, from, to, selection_color, THICK_PEN);
  else if (sel_style == SEL_THIN)
    drawLine (painter, from, to, selection_color, THIN_PEN);
}


void ILSDDetectionWidget::drawTiles (ASPainter& painter)
{
  int tw = (ptset.tileWidth () * cloud_access) / SUBDIV;
  int th = (ptset.tileHeight () * cloud_access) / SUBDIV;
  for (int i = tw; i < width - 50; i += tw)
  {
    drawLine (painter, Pt2i (i, 0), Pt2i (i, height - 1),
              tiles_color, THICK_PEN);
  }
  for (int i = th; i < height - 50; i += th)
    drawLine (painter, Pt2i (0, i), Pt2i (width - 1, i),
              tiles_color, THICK_PEN);
}


void ILSDDetectionWidget::incBlackLevel (int val)
{
  setBlackLevel (blevel + 5 * val);
}

void ILSDDetectionWidget::setBlackLevel (int val)
{
  blevel = val;
  if (blevel < 0) blevel = 0;
  else if (blevel > 200) blevel = 200;
}


void ILSDDetectionWidget::capture (string fname)
{
  augmentedImage.save (fname.c_str ());
}


void ILSDDetectionWidget::lighten (ASImage& im)
{
  if (blevel != 0 && background != BACK_BLACK && background != BACK_WHITE)
  {
    for (unsigned int i = 0; i < im.height (); i++)
      for (unsigned int j = 0; j < im.width (); j++)
      {
        int col = blevel + ((im.GetPixelColor (j, i)).r * (255 - blevel)) / 255;
        im.setPixelGrayscale (j, i, col);
      }
  }
}


void ILSDDetectionWidget::detectAndDisplay()
{
  if (udef) detect (p1, p2);
  display ();
}


void ILSDDetectionWidget::detect (const Pt2i& p1, const Pt2i& p2)
{
  if (det_mode == MODE_CTRACK) tdetector.detect (p1, p2);
  else if (det_mode & MODE_RIDGE_OR_HOLLOW) rdetector.detect (p1, p2);
  if (cp_view != NULL)
  {
    cp_view->reset ();
    cp_view->buildScans (p1, p2);
  }
  if (lp_view != NULL)
  {
    lp_view->reset ();
    lp_view->buildProfile (p1, p2);
  }
}


void ILSDDetectionWidget::display ()
{
  if (udef)
  {
    if (p1.equals (p2)) displayBackground ();
    else updateWidget ();
  }
  else updateWidget ();
}


void ILSDDetectionWidget::displayBackground ()
{
  loadedImage.save ("displayed.png");
  if (background == BACK_BLACK) augmentedImage.clear (ASColor::BLACK);
  else if (background == BACK_WHITE) augmentedImage.clear (ASColor::WHITE);
  else if (background == BACK_IMAGE) augmentedImage = loadedImage;
}


void ILSDDetectionWidget::displayDetectionResult ()
{
  if (background == BACK_BLACK) augmentedImage.clear (ASColor::BLACK);
  else if (background == BACK_WHITE) augmentedImage.clear (ASColor::WHITE);
  else if (background == BACK_IMAGE) augmentedImage = loadedImage;
  lighten (augmentedImage);
  ASPainter painter (&augmentedImage);

  if (tiledisp) drawTiles (painter);
  if (disp_saved) drawPoints (painter, savmap, ASColor::WHITE);

  if (disp_saved)
  {
    vector<Pt2i>::iterator it = savstroke.begin ();
    while (it != savstroke.end ())
    {
      Pt2i pt = *it++;
      drawSelection (painter, pt, *it);
      it ++;
    }
  }

  if (disp_detection)
  {
    if (det_mode == MODE_CTRACK)
    {
      if (ctrack_style != CTRACK_DISP_SCANS)
        displayConnectedTrack (painter, structure_color);
      else displayCarriageTrack (painter, structure_color);
    }
    else if (det_mode & MODE_RIDGE_OR_HOLLOW)
    {
      if (ridge_style != RIDGE_DISP_SCANS)
        displayConnectedRidge (painter, structure_color);
      else displayRidge (painter, structure_color);
    }
    else if (cp_view != NULL && udef && ! p1.equals (p2))
      displayStraightStrip (painter, p1, p2);
  }
  if (udef) drawSelection (painter, p1, p2);

  // Update auxiliary view if not dragging
  if (nodrag)
  {
    if (with_aux_update && cp_view != NULL) cp_view->update ();
    if (lp_view != NULL) lp_view->update ();
  }

  if (cp_view != NULL && udef && ! p1.equals (p2))
    displayAnalyzedScan (painter);

  to_update = false;
  with_aux_update = false;
}


void ILSDDetectionWidget::displayStraightStrip (ASPainter& painter,
                                                const Pt2i from, const Pt2i to)
{
  double dist = ictrl.straightStripWidth ()
                / (2 * sqrt (p1.vectorTo(p2).norm2 ()));
  Vr2i orth = p1.vectorTo(p2).orthog ();
  Pt2i spt ((int) (from.x () + dist * orth.x () + 0.5),
            (int) (from.y () + dist * orth.y () + 0.5));
  Pt2i ept ((int) (to.x () + dist * orth.x () + 0.5),
            (int) (to.y () + dist * orth.y () + 0.5));
  drawLine (painter, spt, ept, ASColor::GREEN);
  spt.set ((int) (from.x () - dist * orth.x () + 0.5),
           (int) (from.y () - dist * orth.y () + 0.5));
  ept.set ((int) (to.x () - dist * orth.x () + 0.5),
           (int) (to.y () - dist * orth.y () + 0.5));
  drawLine (painter, spt, ept, ASColor::GREEN);
}


void ILSDDetectionWidget::displayAnalyzedScan (ASPainter& painter)
{
  std::vector<Pt2i> *pix = NULL;
  if (cp_view != NULL) pix = cp_view->getCurrentScan ();
  if (pix == NULL) return;
  painter.setPen (ASPen (analyzed_color, THICK_PEN,
                         ASPenStyle::SolidLine, ASPenCapStyle::RoundCap,
                         ASPenJoinStyle::RoundJoin));
  int nb = 0, count = (int) pix->size () / 4;
  std::vector<Pt2i>::iterator it = pix->begin ();
  while (nb++ < count)
  {
    if (it->x() < width && it->y() < height && it->x() >= 0 && it->y() >= 0)
      painter.drawPoint (ASCanvasPos (it->x(), height - 1 - it->y()));
    it ++;
  }
  it = pix->end ();
  while (nb-- > 0)
  {
    it --;
    if (it->x() < width && it->y() < height && it->x() >= 0 && it->y() >= 0)
      painter.drawPoint (ASCanvasPos (it->x(), height - 1 - it->y()));
  }

  if (det_mode & MODE_RIDGE_OR_HOLLOW
      && (ictrl.isMeasuring () && ictrl.isSetMeasure ()))
  {
    int m1 = ictrl.measureIntervalStart ();
    int m2 = ictrl.measureIntervalStop ();
    painter.setPen (ASPen (ASColor::BLUE, THICK_PEN,
                           ASPenStyle::SolidLine, ASPenCapStyle::RoundCap,
                           ASPenJoinStyle::RoundJoin));
    pix = cp_view->getDisplayScan (m1);
    int nb = 0, count = (int) pix->size () / 4;
    it = pix->begin ();
    while (nb++ < count)
    {
      if (it->x() < width && it->y() < height && it->x() >= 0 && it->y() >= 0)
        painter.drawPoint (ASCanvasPos (it->x(), height - 1 - it->y()));
      it ++;
    }
    it = pix->end ();
    while (nb-- > 0)
    {
      it --;
      if (it->x() < width && it->y() < height && it->x() >= 0 && it->y() >= 0)
        painter.drawPoint (ASCanvasPos (it->x(), height - 1 - it->y()));
    }

    pix = cp_view->getDisplayScan (m2);
    nb = 0;
    count = (int) pix->size () / 4;
    it = pix->begin ();
    while (nb++ < count)
    {
      if (it->x() < width && it->y() < height && it->x() >= 0 && it->y() >= 0)
        painter.drawPoint (ASCanvasPos (it->x(), height - 1 - it->y()));
      it ++;
    }
    it = pix->end ();
    while (nb-- > 0)
    {
      it --;
      if (it->x() < width && it->y() < height && it->x() >= 0 && it->y() >= 0)
        painter.drawPoint (ASCanvasPos (it->x(), height - 1 - it->y()));
    }
  }
}


void ILSDDetectionWidget::displayCarriageTrack (ASPainter& painter, ASColor col)
{
  CarriageTrack* ct = tdetector.getCarriageTrack ();
  if (ct != NULL)
  {
    Plateau* pl = ct->plateau (0);
    if (pl != NULL)
    {
      painter.setPen (ASPen (col, THIN_PEN));

      Pt2i pp1, pp2;
      tdetector.getInputStroke (pp1, pp2);
      Vr2i p12 = pp1.vectorTo (pp2);
      float l12 = (float) (sqrt (p12.norm2 ()));
      int mini = - ct->getRightScanCount ();
      int maxi = ct->getLeftScanCount ();
      for (int i = mini; i <= maxi; i++)
      {
        pl = ct->plateau (i);
        if (pl != NULL && pl->inserted (smoothed_plateaux))
        {
          float sint = pl->internalStart () * iratio;
          float eint = pl->internalEnd () * iratio;
          vector<Pt2i>* scan = ct->getDisplayScan (i);
          vector<Pt2i>::iterator it = scan->begin ();
          while (it != scan->end ())
          {
            Vr2i p1x = pp1.vectorTo (*it);
            float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
            if (dist >= sint && dist < eint)
              painter.drawPoint (ASCanvasPos (it->x (), height - 1 - it->y ()));
            it++;
          }
        }
      }
    }
  }
}


void ILSDDetectionWidget::displayConnectedTrack (ASPainter& painter,
                                                 ASColor col)
{
  CarriageTrack* ct = tdetector.getCarriageTrack ();
  if (ct != NULL)
  {
    Plateau* pl = ct->plateau (0);
    if (pl != NULL)
    {
      painter.setBrush (ASBrush (col));
      painter.setPen (ASPen (col));

      Pt2i pp1, pp2;
      tdetector.getInputStroke (pp1, pp2);
      Vr2i p12 = pp1.vectorTo (pp2);
      float l12 = (float) (sqrt (p12.norm2 ()));
      int mini = -ct->getRightScanCount ();
      int maxi = ct->getLeftScanCount ();
      Pt2i pt0, pt1;
      int miss = 0;
      float slast = 0.f, elast = 0.f;
      bool disp_on = true;
      bool rev = ct->isScanReversed (0);
      for (int num = 0; disp_on && num <= maxi; num++)
      {
        disp_on = displayConnectedPlateau (painter, ct, num, rev,
                                           pt0, pt1, miss, slast, elast,
                                           pp1, p12, l12);
      }
      miss = 0;
      slast = 0.f;
      elast = 0.f;
      disp_on = true;
      for (int num = 0; disp_on && num >= mini; num--)
      {
        disp_on = displayConnectedPlateau (painter, ct, num, rev,
                                           pt0, pt1, miss, slast, elast,
                                           pp1, p12, l12);
      }
    }
  }
}


bool ILSDDetectionWidget::displayConnectedPlateau (ASPainter& painter,
                                  CarriageTrack* ct, int num, bool rev,
                                  Pt2i& pt0, Pt2i& pt1,
                                  int& miss, float& slast, float& elast,
                                  Pt2i pp1, Vr2i p12, float l12)
{
  Plateau* pl = ct->plateau (num);
  if (pl != NULL && pl->inserted (smoothed_plateaux))
  {
    int sdraw = -1, edraw = -1, snum = 0;
    float sint = pl->internalStart () * iratio;
    float eint = pl->internalEnd () * iratio;
    vector<Pt2i>* scan = ct->getDisplayScan (num);
    vector<Pt2i>::iterator it = scan->begin ();
    while (it != scan->end ())
    {
      Vr2i p1x = pp1.vectorTo (*it);
      float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
      if (rev)
      {
        if (edraw == -1 && dist <= eint) edraw = snum;
        if (sdraw == -1 && dist < sint) sdraw = snum - 1;
      }
      else
      {
        if (sdraw == -1 && dist >= sint) sdraw = snum;
        if (edraw == -1 && dist > eint) edraw = snum - 1;
      }
      snum ++;
      it ++;
    }
    if (rev)
    {
      if (sdraw == -1) sdraw = (int) (scan->size ()) - 1;
    }
    else
    {
      if (edraw == -1) edraw = (int) (scan->size ()) - 1;
    }
    if (num == 0)
    {
      pt0.set ((*scan)[edraw].x (), height - 1 - (*scan)[edraw].y ());
      pt1.set ((*scan)[sdraw].x (), height - 1 - (*scan)[sdraw].y ());
    }
    else
    {
      Pt2i pt2 ((*scan)[sdraw].x (), height - 1 - (*scan)[sdraw].y ());
      Pt2i pt3 ((*scan)[edraw].x (), height - 1 - (*scan)[edraw].y ());
 
      vector<Pt2i>::iterator it = scan->begin ();
      while (it != scan->end ())
      {
        Vr2i p1x = pp1.vectorTo (*it);
        float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
        if (dist >= sint && dist < eint)
          painter.drawPoint (ASCanvasPos (it->x (), height - 1 - it->y ()));
        it ++;
      }

      if (miss != 0)
      {
        float ds = (sint - slast) / (miss + 1);
        float de = (eint - elast) / (miss + 1);
        while (miss-- != 0)
        {
          if (num < 0) num++;
          else num--;
          scan = ct->getDisplayScan (num);
          sint -= ds;
          eint -= de;
          vector<Pt2i>::iterator it = scan->begin ();
          while (it != scan->end ())
          {
            Vr2i p1x = pp1.vectorTo (*it);
            float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
            if (sint > eint)
            {
              if (dist >= eint && dist < sint)
                painter.drawPoint (ASCanvasPos (it->x (),
                                                height - 1 - it->y ()));
            }
            else
            {
              if (dist >= sint && dist < eint)
                painter.drawPoint (ASCanvasPos (it->x (),
                                                height - 1 - it->y ()));
            }
            it ++;
          }
        }
      }
      pt0.set(pt3);
      pt1.set(pt2);
    }
    miss = 0;
    slast = sint;
    elast = eint;
  }
  else miss ++;
  return true;
}


void ILSDDetectionWidget::displayRidge (ASPainter& painter, ASColor col)
{
  Ridge* ridge = rdetector.getRidge ();
  if (ridge != NULL)
  {
    Bump* bump = ridge->bump (0);
    if (bump != NULL && bump->inserted (smoothed_bumps))
    {
      painter.setPen (ASPen (col, THIN_PEN));

      Pt2i pp1, pp2;
      rdetector.getInputStroke (pp1, pp2);
      Vr2i p12 = pp1.vectorTo (pp2);
      float l12 = (float) (sqrt (p12.norm2 ()));
      int mini = - ridge->getRightScanCount ();
      int maxi = ridge->getLeftScanCount ();
      for (int i = mini; i <= maxi; i++)
      {
        bump = ridge->bump (i);
        if (bump != NULL)
        {
          float sint = bump->internalStart () * iratio;
          float eint = bump->internalEnd () * iratio;
          const vector<Pt2i>* scan = ridge->getDisplayScan (i);
          vector<Pt2i>::const_iterator it = scan->begin ();
          while (it != scan->end ())
          {
            Vr2i p1p = pp1.vectorTo (*it);
            float dist = (p12.x () * p1p.x () + p12.y () * p1p.y ()) / l12;
            if (dist >= sint && dist < eint)
              painter.drawPoint (ASCanvasPos (it->x (), height - 1 - it->y ()));
            it ++;
          }
        }
      }
    }
  }
}


void ILSDDetectionWidget::displayConnectedRidge (ASPainter& painter,
                                                 ASColor col)
{
  Ridge* rdg = rdetector.getRidge ();
  if (rdg != NULL)
  {
    Bump* bmp = rdg->bump (0);
    if (bmp != NULL)
    {
      if (ridge_style == RIDGE_DISP_CONNECT)
      {
        painter.setBrush (ASBrush (col));
        painter.setPen (ASPen (col));
      }
      else painter.setPen (ASPen (col, THICK_PEN));

      Pt2i pp1, pp2;
      rdetector.getInputStroke (pp1, pp2);
      Vr2i p12 = pp1.vectorTo (pp2);
      float l12 = (float) (sqrt (p12.norm2 ()));
      int mini = - rdg->getRightScanCount ();
      int maxi = rdg->getLeftScanCount ();
      Pt2i pt0, pt1;
      int miss = 0;
      float slast = 0.0f, elast = 0.0f;
      bool disp_on = true;
      bool rev = rdg->isScanReversed (0);
      for (int num = 0; disp_on && num <= maxi; num++)
      {
        disp_on = displayConnectedBump (painter, rdg, num, rev,
                                        pt0, pt1, miss, slast, elast,
                                        pp1, p12, l12);
      }
      if (ridge_style == RIDGE_DISP_BOUNDS)
        painter.drawLine (pt0.x (), pt0.y (), pt1.x (), pt1.y ());
      miss = 0;
      slast = 0.0f;
      elast = 0.0f;
      disp_on = true;
      for (int num = 0; disp_on && num >= mini; num--)
      {
        disp_on = displayConnectedBump (painter, rdg, num, rev,
                                        pt0, pt1, miss, slast, elast,
                                        pp1, p12, l12);
      }
      if (ridge_style == RIDGE_DISP_BOUNDS)
        painter.drawLine (pt0.x (), pt0.y (), pt1.x (), pt1.y ());
    }
  }
}


bool ILSDDetectionWidget::displayConnectedBump (ASPainter& painter,
                                      Ridge* rdg, int num, bool rev,
                                      Pt2i& pt0, Pt2i& pt1,
                                      int &miss, float &slast, float &elast,
                                      Pt2i pp1, Vr2i p12, float l12)
{
  Bump* bmp = rdg->bump (num);
  if (bmp != NULL && bmp->inserted (smoothed_bumps))
  {
    int sdraw = -1, edraw = -1, snum = 0;
    float sint = 0.0f, eint = 0.0f;
    if (ridge_style == RIDGE_DISP_SPINE)
      sint = bmp->estimatedSummit().x () * iratio;
    else if (ridge_style == RIDGE_DISP_CENTER)
      sint = bmp->estimatedCenter().x () * iratio;
    else
    {
      sint = bmp->internalStart () * iratio;
      eint = bmp->internalEnd () * iratio;
    }
    const vector<Pt2i>* scan = rdg->getDisplayScan (num);
    vector<Pt2i>::const_iterator it = scan->begin ();
    while (it != scan->end ())
    {
      Vr2i p1x = pp1.vectorTo (*it);
      float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
      if (rev)
      {
        if (edraw == -1 && dist <= eint) edraw = snum;
        if (sdraw == -1 && dist < sint) sdraw = snum - 1;
      }
      else
      {
        if (sdraw == -1 && dist >= sint) sdraw = snum;
        if (edraw == -1 && dist > eint) edraw = snum - 1;
      }
      snum ++;
      it ++;
    }
    if (rev)
    {
      if (sdraw == -1) sdraw = (int) (scan->size ()) - 1;
    }
    else
    {
      if (edraw == -1) edraw = (int) (scan->size ()) - 1;
    }
    if (num == 0)
    {
      if (ridge_style == RIDGE_DISP_CONNECT
          || ridge_style == RIDGE_DISP_BOUNDS)
        pt0.set ((*scan)[edraw].x (), height - 1 - (*scan)[edraw].y ());
      pt1.set ((*scan)[sdraw].x (), height - 1 - (*scan)[sdraw].y ());
    }
    else
    {
      if (ridge_style == RIDGE_DISP_CONNECT)
      {
        Pt2i pt2 ((*scan)[sdraw].x (), height - 1 - (*scan)[sdraw].y ());
        Pt2i pt3 ((*scan)[edraw].x (), height - 1 - (*scan)[edraw].y ());
        it = scan->begin ();
        while (it != scan->end ())
        {
          Vr2i p1x = pp1.vectorTo (*it);
          float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
          if (dist >= sint && dist < eint)
            painter.drawPoint (ASCanvasPos (it->x (), height - 1 - it->y ()));
          it ++;
        }
        if (miss != 0)
        {
          float ds = (sint - slast) / (miss + 1);
          float de = (eint - elast) / (miss + 1);
          while (miss -- != 0)
          {
            if (num < 0) num ++;
            else num --;
            scan = rdg->getDisplayScan (num);
            sint -= ds;
            eint -= de;
            it = scan->begin ();
            while (it != scan->end ())
            {
              Vr2i p1x = pp1.vectorTo (*it);
              float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
              if (sint > eint)
              {
                if (dist >= eint && dist < sint)
                  painter.drawPoint (ASCanvasPos (it->x (),
                                                  height - 1 - it->y ()));
              }
              else
              {
                if (dist >= sint && dist < eint)
                  painter.drawPoint (ASCanvasPos (it->x (),
                                                  height - 1 - it->y ()));
              }
              it ++;
            }
          }
        }
        pt0.set (pt3);
        pt1.set (pt2);
      }
      else if (ridge_style == RIDGE_DISP_BOUNDS)
      {
        Pt2i pt2 ((*scan)[sdraw].x (), height - 1 - (*scan)[sdraw].y ());
        Pt2i pt3 ((*scan)[edraw].x (), height - 1 - (*scan)[edraw].y ());
        painter.drawLine (pt0.x (), pt0.y (), pt3.x (), pt3.y ());
        painter.drawLine (pt1.x (), pt1.y (), pt2.x (), pt2.y ());
        pt0.set (pt3);
        pt1.set (pt2);
      }
      else if (ridge_style == RIDGE_DISP_SPINE
               || ridge_style == RIDGE_DISP_CENTER)
      {
        Pt2i pt2 ((*scan)[sdraw].x (), height - 1 - (*scan)[sdraw].y ());
        painter.drawLine (pt1.x (), pt1.y (), pt2.x (), pt2.y ());
        pt1.set (pt2);
      }
      miss = 0;
      slast = sint;
      elast = eint;
    }
  }
  else miss ++;
  return true;
}


void ILSDDetectionWidget::toggleSelectionStyle ()
{
  if (++sel_style > SEL_THICK) sel_style = SEL_NO;
}


void ILSDDetectionWidget::toggleStructureStyle ()
{
  if (det_mode == MODE_CTRACK)
  {
    if (++ctrack_style > CTRACK_DISP_MAX) ctrack_style = CTRACK_DISP_SCANS;
  }
  else if (det_mode & MODE_RIDGE_OR_HOLLOW)
  {
    if (++ridge_style > RIDGE_DISP_MAX) ridge_style = RIDGE_DISP_SCANS;
  }
}


void ILSDDetectionWidget::selectStroke (Pt2i pt)
{
  int num = 0;
  bool searching = true;
  std::cout << "Selecting (" << pt.x () << ", " << pt.y () << ")" << std::endl;
  vector<Pt2i>::iterator it = savstroke.begin ();
  while (searching && it != savstroke.end ())
  {
    Pt2i spt1 (*it++);
    Pt2i spt2 (*it++);
    if (spt1.x () >= 0 && spt1.y () >= 0
        && spt1.x () < width && spt1.y () < height
        && spt2.x () >= 0 && spt2.y () >= 0
        && spt2.x () < width && spt2.y () < height)
    {
      tdetector.detect (spt1, spt2);
      if (selectConnectedTrack (pt))
      {
        std::cout << "Selected stroke " << num << "("
                  << spt1.x () << ", " << spt1.y () << ") ("
                  << spt2.x () << ", " << spt2.y () << ")" << std::endl;
        searching = false;
      }
    }
    num++;
  }
  if (searching)
  {
    tdetector.clear ();
    std::cout << "Missed" << std::endl;
  }
  else updateWidget ();
}


bool ILSDDetectionWidget::selectConnectedTrack (Pt2i pt)
{
  CarriageTrack* ct = tdetector.getCarriageTrack ();
  if (ct != NULL)
  {
    Plateau* pl = ct->plateau (0);
    if (pl != NULL)
    {
      Pt2i pp1, pp2;
      tdetector.getInputStroke (pp1, pp2);
      Vr2i p12 = pp1.vectorTo (pp2);
      float l12 = (float) (sqrt (p12.norm2 ()));
      int mini = - ct->getRightScanCount ();
      int maxi = ct->getLeftScanCount ();
      Pt2i pt0, pt1;
      bool rev = ct->isScanReversed (0);
      for (int num = 0; num <= maxi; num++)
      {
        if (selectConnectedPlateau (pt, ct, num, rev, pt0, pt1, pp1, p12, l12))
        {
          std::cout << "Touched stroke " << num << " ("
                    << pp1.x () << ", " << pp1.y () << ") ("
                    << pp2.x () << ", " << pp2.y () << ")" << std::endl;
          return true;
        }
      }
      for (int num = 0; num >= mini; num--)
      {
        if (selectConnectedPlateau (pt, ct, num, rev, pt0, pt1, pp1, p12, l12))
        {
          std::cout << "Touched stroke " << num << " ("
                    << pp1.x () << ", " << pp1.y () << ") ("
                    << pp2.x () << ", " << pp2.y () << ")" << std::endl;
          return true;
        }
      }
    }
  }
  return false;
}


bool ILSDDetectionWidget::selectConnectedPlateau (Pt2i pt,
                                     CarriageTrack* ct, int num, bool rev,
                                     Pt2i& pt0, Pt2i& pt1,
                                     Pt2i pp1, Vr2i p12, float l12)
{
  Plateau* pl = ct->plateau (num);
  if (pl != NULL && pl->inserted (smoothed_plateaux))
  {
    int sdraw = -1, edraw = -1, snum = 0;
    float sint = pl->internalStart () * iratio;
    float eint = pl->internalEnd () * iratio;
    vector<Pt2i>* scan = ct->getDisplayScan (num);
    vector<Pt2i>::iterator it = scan->begin ();
    while (it != scan->end ())
    {
      Vr2i p1x = pp1.vectorTo (*it);
      float dist = (p12.x () * p1x.x () + p12.y () * p1x.y ()) / l12;
      if (rev)
      {
        if (edraw == -1 && dist <= eint) edraw = snum;
        if (sdraw == -1 && dist < sint) sdraw = snum - 1;
      }
      else
      {
        if (sdraw == -1 && dist >= sint) sdraw = snum;
        if (edraw == -1 && dist > eint) edraw = snum - 1;
      }
      snum ++;
      it ++;
    }
    if (rev)
    {
      if (sdraw == -1) sdraw = (int) (scan->size ()) - 1;
    }
    else
    {
      if (edraw == -1) edraw = (int) (scan->size ()) - 1;
    }
    if (num == 0)
    {
      pt0.set ((*scan)[edraw].x (), height - 1 - (*scan)[edraw].y ());
      pt1.set ((*scan)[sdraw].x (), height - 1 - (*scan)[sdraw].y ());
    }
    else
    {
      Pt2i pt2 ((*scan)[sdraw].x (), height - 1 - (*scan)[sdraw].y ());
      Pt2i pt3 ((*scan)[edraw].x (), height - 1 - (*scan)[edraw].y ());

      Pt2i hpt (pt.x (), height - 1 - pt.y ());
      if (hpt.inTriangle (pt0, pt1, pt3)) return true;
      if (hpt.inTriangle (pt1, pt2, pt3)) return true;
      pt0.set (pt3);
      pt1.set (pt2);
    }
  }
  return false;
}


void ILSDDetectionWidget::performanceTest ()
{
  if (perf_mode)
  {
    udef = true;
    std::cout << "Run test" << std::endl;
    clock_t start = clock ();
    for (int i = 0; i < 1000; i++) detect (p1, p2);
    double diff = (clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "Test run : " << diff << std::endl;
    display ();
  }
}


void ILSDDetectionWidget::saveSubTile ()
{
  int old_access = cloud_access;
  if (cloud_access != IPtTile::TOP) setCloudAccess (IPtTile::TOP);
  ptset.saveSubTile (800, 300, 1100, 600);
  dtm_map.saveSubMap (800, 300, 1100, 600);
  if (old_access != IPtTile::TOP) setCloudAccess (old_access);
}
