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

#ifndef TERRAIN_MAP_H
#define TERRAIN_MAP_H

#include <string>
#include "pt3f.h"
#include "pt2i.h"


/** 
 * @class TerrainMap terrainmap.h
 * \brief Map of Ground normal vectors.
 * The map is assembled from ASC or NVM files.
 */
class TerrainMap
{
public:

  /** Hill shading type. */
  static const int SHADE_HILL;
  /** Slope shading type. */
  static const int SHADE_SLOPE;
  /** Exponential slope shading type. */
  static const int SHADE_EXP_SLOPE;
  /** Default value for the pad size (tile rows or columns). */
  static const int DEFAULT_PAD_SIZE;
  /** DTM map file suffix. */
  static const std::string NVM_SUFFIX;


  /**
   * \brief Creates a ground normal map stucture.
   */
  TerrainMap ();

  /**
   * \brief Deletes the ground normal map.
   */
  ~TerrainMap ();

  /**
   * \brief Clears the ground normal map.
   */
  void clear ();

  /**
   * \brief Returns the DTM normal map width.
   */
  inline int width () const { return iwidth; }

  /**
   * \brief Returns the DTM normal map height.
   */
  inline int height () const { return iheight; }

  /**
   * \brief Returns a DTM tile width.
   */
  inline int tileWidth () const { return twidth; }

  /**
   * \brief Returns a DTM tile height.
   */
  inline int tileHeight () const { return theight; }

  /**
   * \brief Returns the cell size.
   */
  inline float cellSize () const { return cell_size; }

  /**
   * \brief Returns the leftmost coordinate.
   */
  inline double xMin () const { return x_min; }

  /**
   * \brief Returns the lower coordinate.
   */
  inline double yMin () const { return y_min; }

  /**
   * \brief Returns a pixel from the normal map and a lighting device.
   * @param i Pixel absiscae.
   * @param j Pixel ordinate.
   */
  int get (int i, int j) const;

  /**
   * \brief Returns a pixel from the normal map and a shading type.
   * @param i Pixel absiscae.
   * @param j Pixel ordinate.
   * @param shading_type Required shading type.
   */
  int get (int i, int j, int shading_type) const;

  /**
   * \brief Returns an exponential slope value for a pixel of the normal map.
   * @param i Pixel absiscae.
   * @param j Pixel ordinate.
   * @param slp Slope angle exponential factor.
   */
  double getSlopeFactor (int i, int j, int slp) const;

  /**
   * \brief Returns the lighting device angle.
   */
  inline int shadingType () const { return shading; }

  /**
   * \brief Toggles the shading type.
   */
  void toggleShadingType ();

  /**
   * \brief Returns the lighting device angle.
   */
  inline float lightAngle () const { return light_angle; }

  /**
   * \brief Turns the lighting device of given angle.
   * @param val Rotation angle in radians.
   */
  void incLightAngle (int val);

  /**
   * \brief Sets the lighting device angle to given value.
   * @param val Rotation angle in radians.
   */
  void setLightAngle (float val);

  /**
   * \brief Returns the slope exponential factor applied.
   */
  inline int slopinessFactor () const { return slopiness; }

  /**
   * \brief Increments the slope angle exponential factor from given value.
   * @param inc Factor increment.
   */
  void incSlopinessFactor (int val);

  /**
   * \brief Sets the slope angle exponential factor.
   * @param val New factor value.
   */
  void setSlopinessFactor (int val);

  /** Return the center of the closest flat area to given point.
   * @param pt The input point.
   * @param srad The search area radius.
   * @param frad The slope integration area radius.
   * @param sfact The slope angle exponential factor.
   */
  Pt2i closestFlatArea (const Pt2i &pt, int srad, int frad, int sfact);

  /**
   * \brief Declares a new normal map file to add.
   * Returns whether the named file exists.
   * Normal maps should be assemble using assembleMap when all loaded.
   * @param name Normal vector map file name.
   */
  bool addNormalMapFile (const std::string &name);

  /**
   * \brief Creates and assembles the normal map from NVM files.
   * Returns whether creation succeeded.
   * @param cols Count of columns of normal maps to assemble.
   * @param rows Count of rows of normal maps to assemble.
   * @param xmin Left-most coordinate (in millimeters).
   * @param ymin Lower coordinate (in millimeters).
   * @param padding Pad loading mode (vector map later loaded pad by pad).
   */
  bool assembleMap (int cols, int rows, int64_t xmin, int64_t ymin,
                    bool padding = false);

  /**
   * \brief Loads normal map information from a normal vector map file.
   * Returns whether information reading was successful.
   * @param name Normal vector map file name.
   */
  bool loadNormalMapInfo (const std::string &name);;

  /**
   * \brief Returns the assigned pad size (in tile columns).
   */
  inline int padSize () const { return pad_size; }

  /**
   * \brief Returns the processed pad width (in tile columns).
   */
  inline int padWidth () const { return pad_w; }

  /**
   * \brief Returns the processed pad height (in tile rows).
   */
  inline int padHeight () const { return pad_h; }

  /**
   * \brief Sets a new value to the assigned pad size.
   * @param val New size value.
   */
  void setPadSize (int val);

  /**
   * \brief Adjusts pad size to tile set size.
   * Sets to tile set size if this size is lower.
   */
  void adjustPadSize ();

  /**
   * \brief Loads next pad tiles and returns the lower left tile index.
   * @param map Pointer to the map to be loaded with DTM tile contents.
   */
  int nextPad (unsigned char *map);

  /**
   * \brief Loads one slope-shaded DTM in given map location.
   * Returns whether loading succeeded.
   * @param k Tile index wrt tile set.
   * @param map Position in the map to be loaded with DTM tile contents.
   */
  bool loadMap (int k, unsigned char *submap);

  /**
   * \brief Clears one slope-shaded DTM in given map location.
   * @param map Position in the map to be cleared.
   * @param pw Tile pad width (count of tile columns).
   * @param w Tile width.
   * @param h Tile height.
   */
  void clearMap (unsigned char *submap, int pw, int w, int h);

  /**
   * \brief Creates a normal vector map from the first tile.
   * @param name Output file name.
   */
  void saveFirstNormalMap (const std::string &name) const;

  /**
   * \brief Adds and arranges a new DTM file.
   * Returns whether adding succeeded.
   * The new file is localized wrt already entered files.
   * The complete ground map should be built using createMapFromDtm ()
   *   when all files have been loaded.
   * @param name DTM file name (ASC format).
   * @param verb Warning display modality (optional).
   * @param grid_ref True if the input file is grid-referenced (optional) :
   *    standard is pixel-center-referenced
   */
  bool addDtmFile (const std::string &name,
                   bool verb = false, bool grid_ref = false);

  /**
   * \brief Creates the normal map from available DTM (ASC) files.
   * Returns whether creation succeeded.
   * @param verb Warning display modality.
   * @param grid_ref True if the input file is grid-referenced (optional) :
   *    standard is pixel-center-referenced
   */
  bool createMapFromDtm (bool verb = false, bool grid_ref = false);

  /**
   * \brief Loads normal map information from a DTM file.
   * Returns whether information reading was successful.
   * @param name Digital terrain model file name.
   */
  bool loadDtmMapInfo (const std::string &name);;

  /**
   * \brief Creates a new DTM tile from loaded tiles.
   * Only in top access mode. The tile is saved in resources/nvm.
   * @param imin Left column coordinate of the new tile (wrt loaded tiles).
   * @param imin Lower row coordinate of the new tile (wrt loaded tiles).
   * @param imax Upper column coordinate of the new tile + 1 (wrt loaded tiles).
   * @param imax Lower row coordinate of the new tile + 1 (wrt loaded tiles).
   */
  void saveSubMap (int imin, int jmin, int imax, int jmax) const;

  /**
   * \brief Prints the tile arrangement.
   */
  void checkArrangement ();

  /**
   * \brief Edits the contents of loaded terrain map.
   */
  // void trace ();

  /**
   * \brief Edits the header of a normal vector map file.
   * @param name Name of the nvm file.
   */
  // void traceNvmFileInfo (const std::string &name);


private:

  /** Relief artificial amplification. */
  static const float RELIEF_AMPLI;
  /** Lighting angle increment. */
  static const float LIGHT_ANGLE_INCREMENT;

  /** Conversion ratio from millimeters to meters. */
  static const float MM2M;
  /** Small value for testing non zero values. */
  static const double EPS;


  /** Tile width. */
  int twidth;
  /** Tile height. */
  int theight;
  /** Cell size. */
  float cell_size;
  /** Leftmost coordinate. */
  double x_min;
  /** Lowest coordinate. */
  double y_min;
  /** Height code for lacking data. */
  double no_data;

  /** DTM normal map width. */
  int iwidth;
  /** DTM normal map height. */
  int iheight;
  /** DTM normal map. */
  Pt3f *nmap;

  /** Applied shading type. */
  int shading;
  /** Lighting angle. */
  float light_angle;
  /** First light direction. */
  Pt3f light_v1;
  /** Second light direction. */
  Pt3f light_v2;
  /** Third light direction. */
  Pt3f light_v3;
  /** Slope exponential factor (min value : 1). */
  int slopiness;

  /** Input files layout. */
  std::vector<Pt2i> layout;
  /** First tile leftmost coordinate. */
  double fx_min;
  /** First tile lowest coordinate. */
  double fy_min;

  /** Input files names. */
  std::vector<std::string> input_files;
  /** Map of arranged tile file names. */
  std::string **arr_files;

  /** Pad layout for local seed growing: size. */
  int pad_size;
  /** Pad layout for local seed growing: width. */
  int pad_w;
  /** Pad layout for local seed growing: height. */
  int pad_h;
  /** Pad reference (index of left bottom tile). */
  int pad_ref;
  /** Count of tile columns. */
  int ts_cot;
  /** Count of tile rows. */
  int ts_rot;
};

#endif
