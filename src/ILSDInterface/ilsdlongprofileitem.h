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

#ifndef ILSD_LONG_PROFILE_ITEM_H
#define ILSD_LONG_PROFILE_ITEM_H

#include "ilsditemcontrol.h"
#include "ipttileset.h"
#include "scannerprovider.h"
#include "pt2f.h"

class ASImage;
class GLWindow;
class ASPainter;
struct ASCanvasPos;


/**
 * @class ILSDLongProfileItem ilsdlongprofileitem.h
 * \brief Longitudinal profile analysis widget.
 */
class ILSDLongProfileItem
{
public:

  /**
   * \brief Creates a longitudinal profile analysis widget.
   * @param item_ctrl Reference to applicable item controls.
   */
  ILSDLongProfileItem (ILSDItemControl *item_ctrl);

  /**
    * \brief Delete widget.
    */
  virtual ~ILSDLongProfileItem ();

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
   * \brief Sets longitudinal profile from an initial scan.
   * @param pt1 Scan start point.
   * @param pt2 Scan end point.
   */
  virtual void buildProfile (Pt2i pt1, Pt2i pt2);

  /**
   * \brief Updates longitudinal profile.
   */
  virtual void rebuildProfile ();

  /**
   * \brief Sets the metric to viewport ratio and adapts the scale reference.
   */
  virtual void setScale ();

  /**
   * \brief Updates widget display contents.
   * @param context GL widget context.
   */
  virtual void paint (GLWindow* context) = 0;

  /**
   * \brief Returns the structure profile name.
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
   * \brief Updates background image contents.
   */
  virtual void update () = 0;

  /**
   * \brief Saves background image in given file (directory must exist).
   * @param fname Image file name.
   */
  virtual void save (std::string fname) = 0;


protected:

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
  /** Altitude area height. */
  int alti_area_height;
  /** Altitude area margin. */
  int alti_area_margin;
  /** Altitude area left margin. */
  int alti_left_margin;
  /** Altitude area right margin. */
  int alti_right_margin;
  /** Altitude area top margin. */
  int alti_top_margin;
  /** Altitude area bottom margin. */
  int alti_bottom_margin;
  /** Scale area height. */
  int scale_height;
  /** Level area width. */
  int level_width;
  /** Half length of cursor bar. */
  int index_length;

  /** Item display controls */
  ILSDItemControl *ctrl;

  /** Rendered image. */
  ASImage* structImage;
  /** Analysed image width. */
  int imageWidth;
  /** Analyzed image height. */
  int imageHeight;
  /** Points grid. */
  IPtTileSet* ptset;
  /** Image to meter ratio : inverse of cell size. */
  float iratio;

  /** Longitudinal profile of points. */
  std::vector<Pt2f> profile;
  /** Indexes of profile points. */
  std::vector<int> index;
  /** Minimal profile height. */
  float zmin;
  /** Maximal profile height. */
  float zmax;
  /** Profile length. */
  float profile_length;

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

  /** Indicates whether the view can be drawn. */
  bool drawable;
  /** Input stroke start point. */
  Pt2i p1;
  /** Input stroke end point. */
  Pt2i p2;


  /**
   * \brief Updates drawing parameters.
   */
  virtual void updateDrawing () = 0;

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
   * \brief Returns the curvilinear position of a scan.
   * @param num Scan index.
   */
  float profileIndexPosition (int num);

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
