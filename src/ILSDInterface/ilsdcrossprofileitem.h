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

#ifndef ILSD_CROSS_PROFILE_ITEM_H
#define ILSD_CROSS_PROFILE_ITEM_H

#include "ilsditemcontrol.h"
#include "ipttileset.h"
#include "scannerprovider.h"
#include "pt2f.h"

class ASImage;
class GLWindow;
class ASPainter;
struct ASCanvasPos;


/**
 * @class ILSDCrossProfileItem ilsdcrossprofileitem.h
 * \brief Cross profile analysis widget.
 */
class ILSDCrossProfileItem
{
public:

  /**
   * \brief Creates a cross profile analysis widget.
   * @param item_ctrl Reference to applicable item controls.
   */
  ILSDCrossProfileItem (ILSDItemControl *item_ctrl);

  /**
    * \brief Delete widget.
    */
  virtual ~ILSDCrossProfileItem ();

  /**
   * \brief Declares data to be analysed.
   * @param image Reference to DTM view.
   * @param pdata Reference to point cloud.
   */
  virtual void setData (ASImage* image, IPtTileSet* pdata);

  /**
   * \brief Resets the viewer parameters after control changes.
   */
  void resetControls ();

  /**
   * \brief Returns the widget size.
   */
  ASCanvasPos size () const;

  /**
   * \brief Sets the image scan area from an initial scan.
   * @param pt1 Scan start point.
   * @param pt2 Scan end point.
   */
  virtual void buildScans (Pt2i pt1, Pt2i pt2);

  /**
   * \brief Sets the image scan area after width has changed.
   */
  virtual void rebuildScans ();

  /**
   * \brief Gets currently analyzed DTM scan.
   */
  virtual std::vector<Pt2i> *getCurrentScan ();

  /**
   * \brief Checks is the analyse strip can be reset.
   */
  virtual bool isStripAdaptable () const;

  /**
   * \brief Updates widget display contents.
   * @param context GL widget context.
   */
  virtual void paint (GLWindow* context) = 0;

  /**
   * \brief Returns the tructure profile name.
   */
  virtual std::string profileName () const = 0;

  /**
   * \brief Returns the widget display width.
   */
  inline int getWidth () const { return w_width; }

  /**
   * \brief Returns the widget display height.
   */
  inline int getHeight () const { return w_height; }

  /**
   * \brief Updates profile for a new scan.
   */
  virtual void updateProfile ();

  /**
   * \brief Returns measure line translation ratio.
   */
  virtual float getMeasureLineTranslationRatio () const;

  /**
   * \brief Increments measure line translation ratio.
   * @param inc Increment direction.
   */
  virtual void incMeasureLineTranslationRatio (int inc);

  /**
   * \brief Sets measure line translation ratio.
   * @param val New translation ratio value.
   */
  virtual void setMeasureLineTranslationRatio (float val);

  /**
   * \brief Returns measure line rotation ratio.
   */
  virtual float getMeasureLineRotationRatio () const;

  /**
   * \brief Increments measure line rotation ratio.
   * @param inc Increment direction.
   */
  virtual void incMeasureLineRotationRatio (int inc);

  /**
   * \brief Sets measure line rotation ratio.
   * @param val New rotation ratio value.
   */
  virtual void setMeasureLineRotationRatio (float val);

  /**
   * \brief Gets DTM scan to be displayed.
   * @param num Scan number.
   */
  virtual std::vector<Pt2i> *getDisplayScan (int num) = 0;

  /**
   * \brief Updates background image contents.
   */
  virtual void update () = 0;

  /**
   * \brief Saves background image in given file (directory must exist).
   * @param fname Image file name.
   */
  virtual void save (std::string fname) = 0;


protected:

  /** Drawing parameter : position of next plateau prediction. */
  static const int POS_PRED;
  /** Drawing parameter : position of estimated plateau. */
  static const int POS_EST;
  /** Drawing parameter : length of estimated direction. */
  static const int LG_DIR;
  /** Drawing parameter : default cloud points size. */
  static const int DEFAULT_POINT_SIZE;
  /** Minimum scan length allowed. */
  static const int MIN_SCAN;

  /** Analysis widget height. */
  int w_height;
  /** Analysis widget width. */
  int w_width;

  /** Altitude area width. */
  int alti_area_width;
  /** Altitude area margin. */
  int alti_area_margin;

  /** Scan area width. */
  int scan_area_width;
  /** Scan area margin width. */
  int scan_area_margin;
  /** Scan points zoom factor. */
  int scan_res;

  /** Item display controls */
  ILSDItemControl *ctrl;

  /** Analysed image. */
  ASImage* image;
  /** Rendered image. */
  ASImage* structImage;
  /* Analysed image width. */
  int imageWidth;
  /** Analyzed image height. */
  int imageHeight;
  /** Points grid. */
  IPtTileSet* ptset;

  /** Measure start status. */
//  bool mstart_on;
  /** Measure start scan index. */
//  int mstart;
  /** Measure stop status. */
//  bool mstop_on;
  /** Measure start scan index. */
//  int mstop;

  /** Image to meter ratio : inverse of cell size. */
  float iratio;
  /** Reference altitude for profile drawing. */
  float href;
  /** Position shift from structure center to scan center. */
  float profshift;
  /** Metric ratio for height profiles. */
  float sratio;
  /** Scale value for profile reference. */
  int scale;
  /** Displayed DTM scan reversal status. */
  bool reversed;

  /** Indicates whether the view can be drown. */
  bool drawable;
  /** Input stroke start point. */
  Pt2i p1;
  /** Input stroke end point. */
  Pt2i p2;
  /** Input stroke vector. */
  Vr2f p12;
  /** Input stroke length (in pixels). */
  float l12;
  /** Input stroke scan offset. */
  float d12;


  /**
   * \brief Gets point profile to be displayed.
   * @param num Scan number.
   */
  virtual std::vector<Pt2f> *getProfile (int num) = 0;

  /**
   * \brief Updates drawing parameters.
   */
  virtual void updateDrawing () = 0;

  /**
   * \brief Draws the scans used for the track detection.
   */
  virtual void paintScans ();

  /**
   * \brief Draws aligned the scans used for the track detection.
   */
  virtual void paintAlignedScans ();

  /**
   * \brief Draws measured scans section.
   */
  void paintMeasureSection ();

  /**
   * \brief Draws the altitude profile of the selected scan.
   */
  virtual void paintProfile ();

  /**
   * \brief Displays information about selected plateau.
   */
  virtual void paintInfo ();

  /**
   * \brief Displays selected scan detection status.
   */
  virtual void paintStatus () = 0;

  /**
   * \brief Draws a clipped line.
   * @param painter Reference to GL context;
   * @param p1x Line start point abscissae;
   * @param p1y Line start point ordinate;
   * @param p1x Line end point abscissae;
   * @param p1y Line end point ordinate;
   * @param bbl Left bounding line;
   * @param bbd Lower bounding line;
   * @param bbr Right bounding line;
   * @param bbu Upper bounding line;
   */
  void clipLine (ASPainter &painter,
                 int p1x, int p1y, int p2x, int p2y,
                 int bbl, int bbd, int bbr, int bbu) const;

  /**
   * \brief Returns a displayable text for floating point values.
   */
  std::string format (float val) const;

};
#endif
