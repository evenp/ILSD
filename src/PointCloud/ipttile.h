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

#ifndef IPT_TILE_H
#define IPT_TILE_H

#include <vector>
#include <string>
#include <inttypes.h>
#include "pt2i.h"
#include "pt3i.h"


/** 
 * @class ipttile.h
 * \brief 3D integer point tile (unit is millimeter).
 */
class IPtTile
{
public:

  /** Ratio of lidar file unit (1 m) on local unit (1 mm). */
  static const int XYZ_UNIT;
  /** Minimal size (in millimeters) of a cell. */
  static const int MIN_CELL_SIZE;
  /** Fast tile access mode. */
  static const int TOP;
  /** Medium tile access mode. */
  static const int MID;
  /** Sustainable tile access mode. */
  static const int ECO;
  /** Relative path to top mode tile directory. */
  static const std::string TOP_DIR;
  /** Relative path to mid mode tile directory. */
  static const std::string MID_DIR;
  /** Relative path to eco mode tile directory. */
  static const std::string ECO_DIR;
  /** Relative path to point file directory. */
  static const std::string XYZ_DIR;
  /** Top tile file prefix. */
  static const std::string TOP_PREFIX;
  /** Mid tile file prefix. */
  static const std::string MID_PREFIX;
  /** Eco tile file prefix. */
  static const std::string ECO_PREFIX;
  /** Point file suffix. */
  static const std::string TIL_SUFFIX;
  /** Point label file suffix. */
  static const std::string LAB_SUFFIX;
  /** Point text file suffix. */
  static const std::string XYZ_SUFFIX;
  /** Labelled point text file suffix. */
  static const std::string XYZL_SUFFIX;


  /**
   * \brief Creates a point tile.
   * @params nbrows Number of cell rows.
   * @params nbcols Number of cell columns.
   */
  IPtTile (int nbrows = 1, int nbcols = 1);

  /**
   * \brief Creates a point tile.
   * @params name Tile file name.
   */
  IPtTile (std::string name);

  /**
   * \brief Creates a point tile.
   * @params dir Tile file directory.
   * @params name Tile name.
   * @params acc Access type.
   */
  IPtTile (const std::string &dir, const std::string &name, int acc);

  /**
   * \brief Deletes the point tile.
   */
  ~IPtTile ();

  /**
   * Declares the tile size.
   * @params h Tile height.
   * @params w Tile width.
   */
  void setSize (int w, int h);

  /**
   * Declares the playground area.
   * @params xmin X-coordinate of lower left corner.
   * @params ymin Y-coordinate of lower left corner.
   * @params zmax Maximal height.
   * @params cellsize Cell size (in millimeters).
   */
  void setArea (int64_t xmin, int64_t ymin, int64_t zmax, int cellsize);

  /**
   * Determines the layout of the point tile.
   * Returns (-1, -1) if the layout is not found.
   * @params name Name of the point tile file.
   * @params dtmx Left bound of DTM tile set.
   * @params dtmy Lower bound of DTM tile set.
   * @params dtmw Width of a DTM tile (in meters).
   * @params dtmh Height of a DTM tile (in meters).
   */
  Pt2i findLayout (std::string name, double dtmx, double dtmy,
                                     double dtmw, double dtmh);

  /**
   * Declares the point and index data.
   * @params pts Vector of points to declare.
   * @params inds Vector of indexes to declare.
   */
  void setData (std::vector<Pt3i> pts, std::vector<int> inds);

  /**
   * Declares the number of points to load.
   * @params nb Count of points.
   */
  void setCountOfPoints (int nb);

  /**
   * \brief Returns the size of a tile cell (in mm).
   */
  inline int cellSize () const { return (csize); }

  /**
   * \brief Returns the X-spread of the point tile (array size in mm).
   */
  inline int xSpread () const { return (cols * csize); }

  /**
   * \brief Returns the Y-spread of the point tile (array size in mm).
   */
  inline int ySpread () const { return (rows * csize); }

  /**
   * \brief Returns the left coordinate.
   */
  inline int64_t xref () const { return (xmin); }

  /**
   * \brief Returns the lower coordinate.
   */
  inline int64_t yref () const { return (ymin); }

  /**
   * \brief Returns the highest height.
   */
  inline int64_t top () const { return (zmax); }

  /**
   * \brief Returns the size (count of points) of the point tile.
   */
  inline int size () const { return nb; }

  /**
   * \brief Returns the count of rows of the point tile.
   */
  inline int countOfRows () const { return rows; }

  /**
   * \brief Returns the count of columns of the point tile.
   */
  inline int countOfColumns () const { return cols; }

  /**
   * \brief Returns the name of the tile file.
   */
  inline std::string getName () const { return fname; }

  /**
   * \brief Returns the size of a tile cell.
   * @param i Tile cell column.
   * @param j Tile cell row.
   */
  inline int cellSize (int i, int j) const {
    return (cells[j * cols + i + 1] - cells[j * cols + i]); }

  /**
   * \brief Pushes the points of given cell in the provided vector.
   * Returns false if the cell is empty, true otherwise.
   * @param pts Provided vector of points.
   * @param i Tile cell column.
   * @param j Tile cell row.
   */
  bool getPoints (std::vector<Pt3i> &pts, int i, int j) const;

  /**
   * \brief Adds the points of given cell in the provided vector.
   * Returns the count of collected points.
   * @param pts Provided vector of points.
   * @param i Tile cell column.
   * @param j Tile cell row.
   */
  int collectCellPoints (std::vector<Pt3i> &pts, int i, int j) const;

  /**
   * \brief Adds the points of given subcell in the provided vector.
   * Returns the count of collected points.
   * @param pts Provided vector of points.
   * @param i Tile subcell column.
   * @param j Tile subcell row.
   */
  int collectSubcellPoints (std::vector<Pt3i> &pts, int i, int j) const;

  /**
   * \brief Arranges provided tile points in the cells and creates indices.
   * @param pts Count of provided points.
   * @param tin Provided tile.
   */
  void setPoints (int nb, const IPtTile &tin);

  /**
   * \brief Arranges provided tile points in the cells and creates indices.
   * @param tin Provided tile.
   */
  void setPoints (const IPtTile &tin);

  /**
   * \brief Returns the first point of a tile cell.
   * @param i Tile cell column.
   * @param j Tile cell row.
   */
  inline Pt3i *cellStartPt (int i, int j) const {
    return (points + cells[j * cols + i]); }

  /**
   * \brief Returns the label of the first point of a tile cell.
   * @param i Tile cell column.
   * @param j Tile cell row.
   */
  inline int cellStart (int i, int j) const { return (cells[j * cols + i]); }

  /**
   * \brief Returns the points array.
   */
  inline Pt3i *getPointsArray () { return points; }

  /**
   * \brief Returns the points array end (end iterator address).
   */
  inline Pt3i *getPointsArrayEnd () const { return points + nb; }

  /**
   * \brief Returns the cells address array.
   */
  inline int *getCellsArray () { return cells; }

  /**
   * \brief Returns whether poînts are loaded in the tile.
   */
  inline bool unloaded () const { return (points == NULL); }

  /**
   * \brief Saves the tile in a file.
   * Returns whether saving succeeded.
   * @param name Specific tile name.
   */
  bool save (std::string name) const;

  /**
   * \brief Saves the tile in a file.
   * Returns whether saving succeeded.
   */
  bool save () const;

  /**
   * \brief Loads the tile from a file.
   * Returns whether loading succeeded.
   * @param name File name.
   * @param all Only loads header if false.
   */
  bool load (std::string name, bool all = true);

  /**
   * \brief Loads tile index and point tables.
   * Returns whether loading succeeded.
   * @param all Only loads header if false.
   */
  bool load (bool all = true);

  /**
   * \brief Loads the tile data in given arrays.
   * Returns whether loading succeeded.
   * @param ind Index array.
   * @param pts Point array.
   */
  bool loadPoints (int *ind, Pt3i *pts);

  /**
   * \brief Releases the tile data in given arrays.
   */
  void releasePoints ();

  /**
   * \brief Returns the count of points in the most populated cell.
   */
  int cellMaxSize () const;

  /**
   * \brief Returns the count of points in the less populated cell.
   * @param max Sufficiently high value
   */
  int cellMinSize (int max) const;

  /**
   * \brief Returns the count of labelled points.
   */
  int countOfLabelledPoints ();

  /**
   * \brief Saves the labels in a binary file.
   * Returns whether saving succeeded.
   * @param name File directory name.
   */
  bool saveLabels (std::string dir) const;

  /**
   * \brief Reads the labels in a binary file.
   * Returns whether the file exists.
   * @param name File directory name.
   */
  bool loadLabels (std::string dir);

  /**
   * \brief Activates the point labelling modality.
   */
  void createLabels ();

  /**
   * \brief Deactivates the point labelling modality.
   */
  void resetLabels ();

  /**
   * \brief Returns if a cell contains a point labelled as carriage track.
   * @param i Tile cell X coordinate.
   * @param j Tile cell Y coordinate.
   */
  bool isLabelled (int i, int j);

  /**
   * \brief Labels a point as carriage track.
   * @param plab Index of the point in the tile.
   */
  void labelAsTrack (int plab);

  /**
   * \brief Resets all labels in a cell.
   * @param i Tile cell X coordinate.
   * @param j Tile cell Y coordinate.
   */
  void unlabel (int i, int j);

  /**
   * Loads the point tile from a XYZ or XYZL file.
   * Returns whether the XYZ file was found.
   * @params ptsfile XYZ points file name.
   * @params subdiv Tile structure resolution: number of grouped columns.
   * @params lab_in Point label loading modality.
   */
  bool loadXYZFile (std::string ptsfile, int subdiv, bool lab_in = true);

  /**
   * Saves the point tile into an XYZ or XYZL file.
   * Returns whether saving succeeded.
   * @params lab_out Point label saving modality.
   */
  bool saveXYZFile (bool lab_out = true) const;

  /**
   * Saves the point tile into an XYZ or XYZL file.
   * Returns whether saving succeeded.
   * @params ptsfile XYZ points file name.
   * @params lab_out Point label saving modality.
   */
  bool saveXYZFile (std::string ptsfile, bool lab_out = true) const;

  /**
   * \brief Checks the tile.
   */
  void check () const;


private:

  /** Rounding offset used for XYZ file loading or saving.
   * Arbitrarily set to 5 mm to account for 10mm coordinate rounding.
   */
  static const int R_OFF;


  /** Count of rows. */
  int rows;
  /** Count of columns. */
  int cols;
  /** X offset. */
  int64_t xmin;
  /** Y offset. */
  int64_t ymin;
  /** Cloud summit. */
  int64_t zmax;
  /** Grid cell size (in millimeters) */
  int csize;
  /** Total number of points. */
  int nb;
  /** Point labelling modality. */
  bool labelling;
  /** Registered point file full name. */
  std::string fname;
  /** Point array. */
  Pt3i *points;
  /** Point label array. */
  unsigned char *labels;
  /** Tile cell addresses in the point array. */
  int *cells;


  /**
   * \brief Returns the name of the tile from registered name.
   */
  std::string tileName () const;
};

#endif
