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

#ifndef IPT_TILE_SET_H
#define IPT_TILE_SET_H

#include "ipttile.h"
#include "pt3f.h"

#define MM2M 0.001f


/** 
 * @class ipttileset.h
 * \brief Set of 3D integer point tiles (unit is millimeter).
 * CARE WITH FLOATING POINT TYPES USED !!!
 */
class IPtTileSet
{
public:

  /**
   * \brief Creates a point tile set.
   * @param Tile set buffer size value (optional).
   */
  IPtTileSet (int buffer_size = 0);

  /**
   * \brief Deletes the point tile set.
   */
  ~IPtTileSet ();

  /**
   * \brief Clears the point tile set.
   */
  void clear ();

  /**
   * \brief Creates and adds a new point tile to the tile vector.
   * Return true in case of success, false otherwise.
   * @param name Tile file name.
   * @param complete Only loads headers if set to false.
   */
  bool addTile (std::string name, bool complete = true);

  /**
   * \brief Creates and adds a new point tile to the tile vector.
   * Return true in case of success, false otherwise.
   * If the access type is not available, a new tile is created and saved
   *   from an alternative access.
   * @param dir Tile file directory.
   * @param name Tile name.
   * @param access Required access type.
   */
  bool addTile (const std::string &dir, const std::string &name, int access);

  /**
   * \brief Adds an already created point tile to the tile vector.
   * @param tile Created point tile to be added.
   */
  void addTile (IPtTile *tile);

  /**
   * \brief Creates the tile array from tile vector to start exploitation.
   * Returns false of no tile is loaded, true otherwise.
   */
  bool create ();

  /**
   * \brief Loads the tile points.
   * Returns whether loading succeeded.
   */
  bool loadPoints ();

  /**
   * \brief Returns whether a specifc tile is effectively loaded.
   * @param num Number of the tile to check in the tile set.
   */
  inline bool isLoaded (int num) const {
    return (tiles != NULL && num < tcols * trows && tiles[num] != NULL); }

  /**
   * \brief Updates access type of the tiles.
   * @param oldtype Previous access type.
   * @param newtype New access type.
   * @param prefix New file prefix.
   */
  void updateAccessType (int oldtype, int newtype, const std::string &prefix);

  /**
   * \brief Returns the size of a divided tile cell (in mm).
   */
  inline int cellSize () const { return (tiles[0]->cellSize () / cdiv); }

  /**
   * \brief Returns the width of a tile.
   */
  inline int tileWidth () const { return twidth; }

  /**
   * \brief Returns the height of a tile.
   */
  inline int tileHeight () const { return theight; }

  /**
   * \brief Returns the X-spread of a tile (in mm).
   */
  inline int tileXSpread () const { return txspread; }

  /**
   * \brief Returns the Y-spread of a tile (in mm).
   */
  inline int tileYSpread () const { return tyspread; }

  /**
   * \brief Returns the X-spread of the point tile (array size in mm).
   */
  inline int xSpread () const { return (tcols * txspread); }

  /**
   * \brief Returns the Y-spread of the point tile (array size in mm).
   */
  inline int ySpread () const { return (trows * tyspread); }

  /**
   * \brief Returns the X-spread of the point tile (array size in m).
   */
  inline float xmSpread () const {
    return (((float) (tcols * txspread)) * MM2M); }

  /**
   * \brief Returns the Y-spread of the point tile (array size in m).
   */
  inline float ymSpread () const {
    return (((float) (trows * tyspread)) * MM2M); }

  /**
   * \brief Returns the left coordinate in millimeters.
   */
  inline int64_t xref () const { return (xmin); }

  /**
   * \brief Returns the lower coordinate in millimeters.
   */
  inline int64_t yref () const { return (ymin); }

  /**
   * \brief Returns the left coordinate in meter unit.
   */
  inline double xmref () const { return (((double) xmin) * MM2M); }

  /**
   * \brief Returns the lower coordinate in meter unit.
   */
  inline double ymref () const { return (((double) ymin) * MM2M); }

  /**
   * \brief Returns the highest height in millimeters.
   */
  inline int64_t top () const { return (zmax); }

  /**
   * \brief Returns the highest height in meter unit.
   */
  inline float mtop () const { return (((float) zmax) * MM2M); }

  /**
   * \brief Returns the size (count of points) of the tile set.
   */
  inline int size () const { return nb; }

  /**
   * \brief Returns the count of tile rows in the set.
   */
  inline int rowsOfTiles () const { return trows; }

  /**
   * \brief Returns the count of tile columns in the set.
   */
  inline int columnsOfTiles () const { return tcols; }

  /**
   * \brief Returns the count of divided cell rows in the set.
   */
  inline int rowsOfSubCells () const {
    return trows * theight * cdiv; }

  /**
   * \brief Returns the count of divided cell columns in the set.
   */
  inline int columnsOfSubCells () const {
    return tcols * twidth * cdiv; }

  /**
   * \brief Returns the size of a tile cell.
   * @param i Tile cell column.
   * @param j Tile cell row.
   */
  int cellSize (int i, int j) const;

  /**
   * \brief Pushes the points of given tile subcell in provided vector.
   *   Points are transfered in meter unit.
   *   Tiles are assumed to be organized in sorted sub-cells.
   * Returns whether tile points are effectively loaded.
   * @param pts Provided vector of points.
   * @param i Tile subcell column.
   * @param j Tile subcell row.
   */
  bool collectPoints (std::vector<Pt3f> &pts, int i, int j);// const;

  /**
   * \brief Pushes the points of given tile subcell in provided vector.
   *   Points are transfered in meter unit.
   *   No sub-cells sort is assumed here.
   * @param pts Provided vector of points.
   * @param i Tile subcell column.
   * @param j Tile subcell row.
   */
  void collectUnsortedPoints (std::vector<Pt3f> &pts, int i, int j) const;

  /**
   * \brief Returns the count of points in the most populated subcell.
   */
  int cellMaxSize () const;

  /**
   * \brief Returns the count of points in the less populated subcell.
   * @param max Sufficiently high value
   */
  int cellMinSize (int max) const;

  /**
   * \brief Returns the size of local buffers.
   */
  int bufferSize () const;

  /**
   * \brief Sets the size of local buffers.
   * @param val New size value (only positive odd).
   */
  void setBufferSize (int val);

  /**
   * \brief Deletes tile point and index buffers.
   */
  void deleteBuffers ();

  /**
   * \brief Creates tile point and index buffers.
   */
  void createBuffers ();

  /**
   * \brief Returns the next traversed tile index.
   */
  int nextTile ();

  /**
   * \brief Creates a new point tile from loaded tiles.
   * Only in top access mode. The tile is saved in resources/til/top.
   * @param imin Left column coordinate of the new tile (wrt loaded tiles).
   * @param imin Lower row coordinate of the new tile (wrt loaded tiles).
   * @param imax Upper column coordinate of the new tile + 1 (wrt loaded tiles).
   * @param imax Lower row coordinate of the new tile + 1 (wrt loaded tiles).
   */
  void saveSubTile (int imin, int jmin, int imax, int jmax) const;

  /**
   * \brief Prints features of the set first tile.
   */
  void check ();


private:

  /** Default value for local tile set size. */
  static const int DEFAULT_BUF_SIZE;

  /** X offset. */
  int64_t xmin;
  /** Y offset. */
  int64_t ymin;
  /** Cloud summit. */
  int64_t zmax;
  /** Tile width */
  int twidth;
  /** Tile height */
  int theight;
  /** Tile X spread (in millimeters) */
  int txspread;
  /** Tile Y spread (in millimeters) */
  int tyspread;
  /** Total number of points. */
  int nb;
  /** Temporary vector of tiles (used before storing in array). */
  std::vector<IPtTile *> vectiles;
  /** Count of tile columns. */
  int tcols;
  /** Count of tile rows. */
  int trows;
  /** Divide factor of tile cells for fine requests. */
  int cdiv;
  /** Tile array. */
  IPtTile **tiles;

  /** Local tile set size. */
  int buf_size;
  /** Local tile set width. */
  int buf_w;
  /** Local tile set height. */
  int buf_h;
  /** Current tile X position (wrt loaded tile set). */
  int buf_x;
  /** Current tile Y position (wrt loaded tile set). */
  int buf_y;
  /** Maximum number of loaded points in a tile. */
  int buf_np;
  /** Number of loaded indices in a tile. */
  int buf_ni;
  /** Loaded points in local tiles. */
  Pt3i *buf_pts;
  /** Loaded indices in local tiles. */
  int *buf_ind;
  /** Current step of tile set traversal. */
  int buf_step;

};

#endif
