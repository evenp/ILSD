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
#include "ipttileset.h"

const int IPtTileSet::DEFAULT_BUF_SIZE = 3;


IPtTileSet::IPtTileSet (int buffer_size)
{
  tiles = NULL;
  cdiv = 1;
  tcols = 0;
  trows = 0;

  buf_size = buffer_size;
  if (buf_size < 0 || buf_size % 2 == 0) buf_size = DEFAULT_BUF_SIZE;
  buf_w = buf_size;
  buf_h = buf_size;
  buf_pts = NULL;
  buf_ind = NULL;
  buf_x = 0;
  buf_y = 0;
  buf_np = 0;
  buf_ni = 0;
  buf_step = 0;
}


IPtTileSet::~IPtTileSet ()
{
  clear ();
}


void IPtTileSet::clear ()
{
  if (buf_pts != NULL)
  {
    delete [] buf_pts;
    buf_pts = NULL;
  }
  if (buf_ind != NULL)
  {
    delete [] buf_ind;
    buf_ind = NULL;
  }
  if (tiles != NULL)
  {
    for (int i = 0; i < tcols * trows; i ++)
      if (tiles[i] != NULL) delete tiles[i];
    delete [] tiles;
    tiles = NULL;
  }
  vectiles.clear ();
}


bool IPtTileSet::addTile (std::string name, bool all)
{
  if (tiles == NULL || ! all)
  {
    IPtTile *tile = new IPtTile (name);
    if (tile->load (all))
    {
      vectiles.push_back (tile);
      return true;
    }
  }
  return false;
}


void IPtTileSet::addTile (IPtTile *tile)
{ 
  vectiles.push_back (tile);
}


bool IPtTileSet::create ()
{
  if (vectiles.empty ()) return false;
  std::vector<IPtTile *>::iterator it = vectiles.begin ();
  xmin = (*it)->xref ();
  ymin = (*it)->yref ();
  int64_t xmax = (*it)->xref ();
  int64_t ymax = (*it)->yref ();
  zmax = (*it)->top ();
  txspread = (*it)->xSpread ();
  tyspread = (*it)->ySpread ();
  twidth = (*it)->countOfColumns ();
  theight = (*it)->countOfRows ();
  cdiv = (*it)->cellSize () / IPtTile::MIN_CELL_SIZE;
  buf_ni = twidth * theight + 1;
  buf_np = 0;
  nb = 0;
  while (it != vectiles.end ())
  {
    if ((*it)->xref () < xmin) xmin = (*it)->xref ();
    if ((*it)->yref () < ymin) ymin = (*it)->yref ();
    if ((*it)->xref () > xmax) xmax = (*it)->xref ();
    if ((*it)->yref () > ymax) ymax = (*it)->yref ();
    if ((*it)->top () > zmax) zmax = (*it)->top ();
    nb += (*it)->size ();
    if ((*it)->size () > buf_np) buf_np = (*it)->size ();
    it ++;
  }

  tcols = 1 + (int) ((xmax - xmin + txspread / 2) / txspread);
  trows = 1 + (int) ((ymax - ymin + tyspread / 2) / tyspread);
  if (tiles == NULL)
  {
    tiles = new IPtTile*[tcols * trows];
    for (int i = 0; i < tcols * trows; i++) tiles[i] = NULL;
    do
    {
      it --;
      int ix = (int) (((*it)->xref () - xmin + txspread / 2) / txspread);
      int iy = (int) (((*it)->yref () - ymin + tyspread / 2) / tyspread);
      if (tiles[iy * tcols + ix] == NULL) tiles[iy * tcols + ix] = *it;
      else delete *it;  // already loaded
    }
    while (it != vectiles.begin ());
  }
  vectiles.clear ();
  return (true);
}


bool IPtTileSet::loadPoints ()
{
  for (int i = 0; i < tcols * trows; i ++)
    if (!tiles[i]->load ()) return false;
  return true;
}


void IPtTileSet::updateAccessType (int oldtype, int newtype,
                                   const std::string &prefix)
{
  for (int j = 0; j < trows; j++)
    for (int i = 0; i < tcols; i++)
      if (tiles[j * tcols + i] != NULL)
      {
        IPtTile *oldtile = tiles[j * tcols + i];
        std::string tname = oldtile->getName ();
        size_t last = tname.find_last_of ('/');
        if (last == std::string::npos) last = tname.find_last_of ('\\');
        if (last == std::string::npos) return;
        std::string shortname = tname.substr (last + 1, std::string::npos);
        last = shortname.find_first_of ('_');
        std::string name (prefix);
        name += shortname.substr (last + 1, std::string::npos);

        IPtTile *tile = new IPtTile (name);
        if (! tile->load ())
        {
          tile->setSize ((oldtile->countOfColumns () * oldtype) / newtype,
                         (oldtile->countOfRows () * oldtype) / newtype);
          tile->setArea (oldtile->xref (), oldtile->yref (), oldtile->top (),
                         IPtTile::MIN_CELL_SIZE * newtype);
          tile->setPoints (*oldtile);
          tile->save (name);
        }
        delete oldtile;
        tiles[j * tcols + i] = tile;
      }
  twidth = (twidth * oldtype) / newtype;
  theight = (theight * oldtype) / newtype;
  cdiv = (cdiv * newtype) / oldtype;
}


int IPtTileSet::cellSize (int i, int j) const
{
  IPtTile *tile = tiles[(j/theight)*tcols+(i/twidth)];
  return (tile != NULL ? tile->cellSize (i % twidth, j % theight) : 0);
}


bool IPtTileSet::collectPoints (std::vector<Pt3f> &pts, int i, int j) // const
{
  int icell = i / cdiv, jcell = j / cdiv;                // cdiv = 10 avec over
  int itile = icell / twidth, jtile = jcell / theight;
  IPtTile *tile = tiles[jtile * tcols + itile];
  if (tile != NULL)
  {
    if (tile->unloaded ()) return false;
    icell = icell - itile * tile->countOfColumns ();
    jcell = jcell - jtile * tile->countOfRows ();
    int nbpts = tile->cellSize (icell, jcell);
    if (nbpts != 0)
    {
      Pt3i *pt = tile->cellStartPt (icell, jcell);
      if (cdiv == 1)
      {
        for (int i = 0; i < nbpts; i++)
        {
          pts.push_back (Pt3f (((float) (txspread * itile + pt->x ())) * MM2M,
                               ((float) (tyspread * jtile + pt->y ())) * MM2M,
                               ((float) pt->z ()) * MM2M));
          pt ++;
        }
      }
      else
      {
        int cxy = tile->cellSize () / cdiv;
        int cxmin = icell * tile->cellSize () + (i % cdiv) * cxy;
        int cymin = jcell * tile->cellSize () + (j % cdiv) * cxy;
        int cxmax = cxmin + cxy;
        int cymax = cymin + cxy;

        Pt3i *ptfin = pt + nbpts;
        while (pt->y () < cymin && pt != ptfin) pt ++;
        while (pt->x () < cxmin && pt != ptfin) pt ++;
        while (pt->x () < cxmax && pt->y () < cymax && pt != ptfin)
        {
          pts.push_back (Pt3f (((float) (txspread * itile + pt->x ())) * MM2M,
                               ((float) (tyspread * jtile + pt->y ())) * MM2M,
                               ((float) pt->z ()) * MM2M));
          pt ++;
        }
      }
    }
  }
  return true;
}


void IPtTileSet::collectUnsortedPoints (std::vector<Pt3f> &pts,
                                        int i, int j) const
{
  int icell = i / cdiv, jcell = j / cdiv;
  int itile = icell / twidth, jtile = jcell / theight;
  IPtTile *tile = tiles[jtile * tcols + itile];
  if (tile != NULL)
  {
    icell = icell - itile * tile->countOfColumns ();
    jcell = jcell - jtile * tile->countOfRows ();
    int nbpts = tile->cellSize (icell, jcell);
    if (nbpts != 0)
    {
      Pt3i *pt = tile->cellStartPt (icell, jcell);
      int cxy = tile->cellSize () / cdiv;
      int cxmin = icell * tile->cellSize () + (i % cdiv) * cxy;
      int cymin = jcell * tile->cellSize () + (j % cdiv) * cxy;
      int cxmax = cxmin + cxy;
      int cymax = cymin + cxy;
      for (int i = 0; i < nbpts; i++)
      {
        if (pt->x () >= cxmin && pt->x () < cxmax
            && pt->y () >= cymin && pt->y () < cymax)
          pts.push_back (Pt3f (((float) (txspread * itile + pt->x ())) * MM2M,
                               ((float) (tyspread * jtile + pt->y ())) * MM2M,
                               ((float) pt->z ()) * MM2M));
        pt ++;
      }
    }
  }
}


int IPtTileSet::cellMaxSize () const
{
  int max = 0;
  for (int j = 0; j < trows; j++)
    for (int i = 0; i < tcols; i++)
      if (tiles[j * tcols + i] != NULL)
      {
        int cmax = tiles[j * tcols + i]->cellMaxSize ();
        if (cmax > max) max = cmax;
      }
  return max;
}


int IPtTileSet::cellMinSize (int max) const
{
  int min = max;
  for (int j = 0; j < trows; j++)
    for (int i = 0; i < tcols; i++)
      if (tiles[j * tcols + i] != NULL)
      {
        int cmin = tiles[j * tcols + i]->cellMinSize (max);
        if (cmin < min) min = cmin;
      }
  return min;
}


int IPtTileSet::bufferSize () const
{
  return buf_size;
}


void IPtTileSet::setBufferSize (int val)
{
  if (val > 0 && val % 2 == 1)
  {
    buf_size = val;
    buf_w = buf_size;
    buf_h = buf_size;
    if (buf_pts != NULL)
    {
      if (buf_w > tcols) buf_w = tcols;
      if (buf_h > trows) buf_h = trows;
      buf_pts = new Pt3i[buf_w * buf_h * buf_np];
      buf_ind = new int[buf_w * buf_h * buf_ni];
    }
  }
}


void IPtTileSet::deleteBuffers ()
{
  if (buf_pts != NULL) delete [] buf_pts;
  buf_pts = NULL;
  if (buf_ind != NULL) delete [] buf_ind;
  buf_ind = NULL;
}


void IPtTileSet::createBuffers ()
{
  if (buf_w > tcols) buf_w = tcols;
  if (buf_h > trows) buf_h = trows;
  buf_pts = new Pt3i[buf_w * buf_h * buf_np];
  buf_ind = new int[buf_w * buf_h * buf_ni];
}


int IPtTileSet::nextTile ()
{
  int k, bk;

  // SWEEP START
  if (buf_step == 0)
  {
//    std::cout << "SWEEP START IN ..." << std::endl;
    for (int j = 0; j < buf_h; j++)
      for (int i = 0; i < buf_w; i++)
      {
        k = j * tcols + i;
        bk = j * buf_w + i;
        // std::cout << "ADD " << k << " IN " << bk << std::endl;
        if (tiles[k] != NULL)
          tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
      }
    buf_x = 0;
    buf_y = 0;
    if (buf_w == 1 && buf_h == 1) buf_step = 13;
    else buf_step = (buf_w > buf_h ? 7 : 1);
//    std::cout << "SWEEP START OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // HORIZONTAL SWEEP - FIRST SAW
  else if (buf_step == 1)
  {
//    std::cout << "FIRST SAW IN ..." << std::endl;
    if ((buf_h == trows && buf_y == buf_h - 1)
        || (buf_h != trows && buf_y == buf_h / 2)) // next tooth
    {
      buf_x ++;
      buf_y = 0;
      if (buf_w != tcols && buf_x > buf_w / 2 && buf_x + buf_w / 2 < tcols)
      {
        k = buf_x - buf_w / 2 - 1;
        for (int j = 0; j < buf_h; j++)
        {
          // std::cout << "RELIZ " << k << std::endl;
          if (tiles[k] != NULL) tiles[k]->releasePoints ();
          k += tcols;
        }
        k = buf_x + buf_w / 2;
        bk = k % buf_w;
        for (int j = 0; j < buf_h; j++)
        {
          // std::cout << "ADD " << k << " IN " << bk << std::endl;
          if (tiles[k] != NULL)
            tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
          k += tcols;
          bk += buf_w;
          if (bk >= buf_w * buf_h) bk -= buf_w * buf_h;
        }
      }
    }
    else // same tooth
    {
      buf_y ++;
      if (buf_x == tcols - 1)
      {
        if (buf_y == trows - 1) buf_step = 13;
        else if (buf_h != trows && buf_y == buf_w / 2) buf_step = 2;
      }
    }
//    std::cout << "FIRST SAW OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // HORIZONTAL SWEEP - CLIMB TO NEXT ROW
  else if (buf_step == 2)
  {
//    std::cout << "CLIMB IN ..." << std::endl;
    buf_y ++;
    k = (buf_y - buf_h / 2 - 1) * tcols;
    if (buf_x != 0) k += tcols - buf_w;
    for (int i = 0; i < buf_w; i++)
    {
      // std::cout << "RELIZ " << k << std::endl;
      if (tiles[k] != NULL) tiles[k]->releasePoints ();
      k ++;
    }
    k += buf_h * tcols - buf_w;
    bk = ((k / tcols) % buf_h) * buf_w + ((k % tcols) % buf_w);
    for (int i = 0; i < buf_w; i++)
    {
      // std::cout << "ADD " << k << " IN " << bk << std::endl;
      if (tiles[k] != NULL)
        tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
      k ++;
      if (++bk % buf_w == 0) bk -= buf_w;
    }
    buf_step = (buf_y + buf_h / 2 < trows - 1 ?
                (buf_x != 0 ? 3 : 4) : (buf_x != 0 ? 6 : 5));
//    std::cout << "CLIMB OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // HORIZONTAL SWEEP - WAY BACK
  else if (buf_step == 3)
  {
//    std::cout << "WAY BACK IN ..." << std::endl;
    if (buf_x + buf_w / 2 < tcols && buf_x > buf_w / 2)
    {
      k = (buf_y - buf_h / 2) * tcols + buf_x + buf_w / 2;
      for (int j = 0; j < buf_h; j++)
      {
        // std::cout << "RELIZ " << k << std::endl;
        if (tiles[k] != NULL) tiles[k]->releasePoints ();
        k += tcols;
      }
      buf_x --;
      k -= buf_w + tcols * buf_h;
      bk = ((k / tcols) % buf_h) * buf_w + ((k % tcols) % buf_w);
      for (int j = 0; j < buf_h; j++)
      {
        // std::cout << "ADD " << k << " IN " << bk << std::endl;
        if (tiles[k] != NULL)
          tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
        k += tcols;
        bk += buf_w;
        if (bk >= buf_h * buf_w) bk -= buf_h * buf_w;
      }
    }
    else buf_x --;
    if (buf_x == 0) buf_step = 2;
//    std::cout << "WAY BACK OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // HORIZONTAL SWEEP - WAY ON
  else if (buf_step == 4)
  {
//    std::cout << "WAY ON IN ..." << std::endl;
    if (buf_x >= buf_w / 2 && buf_x + buf_w / 2 < tcols - 1)
    {
      k = (buf_y - buf_h / 2) * tcols + buf_x - buf_w / 2;
      for (int j = 0; j < buf_h; j++)
      {
        // std::cout << "RELIZ " << k << std::endl;
        if (tiles[k] != NULL) tiles[k]->releasePoints ();
        k += tcols;
      }
      buf_x ++;
      k += buf_w - buf_h * tcols;
      bk = ((k / tcols) % buf_h) * buf_w + ((k % tcols) % buf_w);
      for (int j = 0; j < buf_h; j++)
      {
        // std::cout << "ADD " << k << " IN " << bk << std::endl;
        if (tiles[k] != NULL)
          tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
        k += tcols;
        bk += buf_w;
        if (bk >= buf_h * buf_w) bk -= buf_h * buf_w;
      }
    }
    else buf_x ++;
    if (buf_x == tcols - 1) buf_step = 2;
//    std::cout << "WAY ON OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // HORIZONTAL SWEEP - LAST SAW ON
  else if (buf_step == 5)
  {
//    std::cout << "LAST SAW ON IN ..." << std::endl;
    if (buf_y == trows - 1) // next tooth
    {
      buf_x ++;
      buf_y -= buf_h / 2;
      if (buf_w != tcols && buf_x > buf_w / 2 && buf_x + buf_w / 2 < tcols)
      {
        k = (buf_y - buf_h / 2) * tcols + buf_x - buf_w / 2 - 1;
        for (int j = 0; j < buf_h; j++)
        {
          // std::cout << "RELIZ " << k << std::endl;
          if (tiles[k] != NULL) tiles[k]->releasePoints ();
          k += tcols;
        }
        k += buf_w - buf_h * tcols;
        bk = ((k / tcols) % buf_h) * buf_w + ((k % tcols) % buf_w);
        for (int j = 0; j < buf_h; j++)
        {
          // std::cout << "ADD " << k << " IN " << bk << std::endl;
          if (tiles[k] != NULL)
            tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
          k += tcols;
          bk += buf_w;
          if (bk >= buf_w * buf_h) bk -= buf_w * buf_h;
        }
      }
    }
    else // same tooth
    {
      buf_y ++;
      if (buf_y == trows - 1 && buf_x == tcols - 1) buf_step = 13;
    }
//    std::cout << "LAST SAW ON OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // HORIZONTAL SWEEP - LAST SAW BACK
  else if (buf_step == 6)
  {
//    std::cout << "LAST SAW BACK IN ..." << std::endl;
    if (buf_y == trows - 1) // next tooth
    {
      buf_x --;
      buf_y -= buf_h / 2;
      if (buf_w != tcols && buf_x >= buf_w / 2 && buf_x + buf_w / 2 < tcols - 1)
      {
        k = (buf_y - buf_h / 2) * tcols + buf_x + buf_w / 2 + 1;
        for (int j = 0; j < buf_h; j++)
        {
          // std::cout << "RELIZ " << k << std::endl;
          if (tiles[k] != NULL) tiles[k]->releasePoints ();
          k += tcols;
        }
        k = (trows - buf_h) * tcols + buf_x - buf_w / 2;
        bk = ((k / tcols) % buf_h) * buf_w + ((k % tcols) % buf_w);
        for (int j = 0; j < buf_h; j++)
        {
          // std::cout << "ADD " << k << " IN " << bk << std::endl;
          if (tiles[k] != NULL)
            tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
          k += tcols;
          bk += buf_w;
          if (bk >= buf_h * buf_w) bk -= buf_h * buf_w;
        }
      }
    }
    else // same tooth
    {
      buf_y ++;
      if (buf_y == trows - 1 && buf_x == 0) buf_step = 13;
    }
//    std::cout << "LAST SAW BACK OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // VERTICAL SWEEP - FIRST SAW
  else if (buf_step == 7)
  {
//    std::cout << "V FIRST SAW IN ..." << std::endl;
    if ((buf_w == tcols && buf_x == buf_w - 1)      // stripe end
        || (buf_w != tcols && buf_x == buf_w / 2))  // stripe center
    {
      // process next tooth
      buf_y ++;
      buf_x = 0;
      if (buf_h != trows && buf_y > buf_h / 2 && buf_y + buf_h / 2 < trows)
      {
        k = (buf_y - buf_h / 2 - 1) * tcols;
        for (int i = 0; i < buf_w; i++)
        {
          // std::cout << "RELIZ " << k << std::endl;
          if (tiles[k] != NULL) tiles[k]->releasePoints ();
          k ++;
        }
        k = (buf_y + buf_h / 2) * tcols;
        bk = (k % buf_h) * buf_w;
        for (int i = 0; i < buf_w; i++)
        {
          // std::cout << "ADD " << k << " IN " << bk << std::endl;
          if (tiles[k] != NULL)
            tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
          k ++;
          if (++bk % buf_w == 0) bk -= buf_w;
        }
      }
    }
    else // same tooth
    {
      buf_x ++;
      if (buf_y == trows - 1)
      {
        if (buf_x == tcols - 1) buf_step = 13;
        else if (buf_w != tcols && buf_x == buf_h / 2) buf_step = 8;
      }
    }
//    std::cout << "V FIRST SAW OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // VERTICAL SWEEP - CLIMB TO NEXT ROW
  else if (buf_step == 8)
  {
//    std::cout << "V CLIMB IN ..." << std::endl;
    buf_x ++;
    k = buf_x - buf_w / 2 - 1;
    if (buf_y != 0) k += (trows - buf_h) * tcols;
    for (int j = 0; j < buf_h; j++)
    {
      // std::cout << "RELIZ " << k << std::endl;
      if (tiles[k] != NULL) tiles[k]->releasePoints ();
      k += tcols;
    }
    k += buf_w - buf_h * tcols;
    bk = ((k / tcols) % buf_h) * buf_w + ((k % tcols) % buf_w);
    for (int j = 0; j < buf_h; j++)
    {
      // std::cout << "ADD " << k << " IN " << bk << std::endl;
      if (tiles[k] != NULL)
        tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
      k += tcols;
      bk += buf_w;
      if (bk >= buf_w * buf_h) bk -= buf_w * buf_h;
    }
    buf_step = (buf_x + buf_w / 2 < tcols - 1 ?
                (buf_y != 0 ? 9 : 10) : (buf_y != 0 ? 12 : 11));
//    std::cout << "V CLIMB OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // VERTICAL SWEEP - WAY BACK
  else if (buf_step == 9)
  {
//    std::cout << "V WAY BACK IN ..." << std::endl;
    if (buf_y + buf_h / 2 < trows && buf_y > buf_h / 2)
    {
      k = (buf_x - buf_w / 2) + (buf_y + buf_h / 2) * tcols;
      for (int i = 0; i < buf_w; i++)
      {
        // std::cout << "RELIZ " << k << std::endl;
        if (tiles[k] != NULL) tiles[k]->releasePoints ();
        k ++;
      }
      buf_y --;
      k -= buf_h * tcols + buf_w;
      bk = ((k / tcols) % buf_h) * buf_w + ((k % tcols) % buf_w);
      for (int j = 0; j < buf_h; j++)
      {
        // std::cout << "ADD " << k << " IN " << bk << std::endl;
        if (tiles[k] != NULL)
          tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
        k ++;
        if (++bk % buf_w == 0) bk -= buf_w;
      }
    }
    else buf_y --;
    if (buf_y == 0) buf_step = 8;
//    std::cout << "V WAY BACK OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // VERTICAL SWEEP - WAY ON
  else if (buf_step == 10)
  {
//    std::cout << "V WAY ON IN ..." << std::endl;
    if (buf_y >= buf_h / 2 && buf_y + buf_h / 2 < trows - 1)
    {
      k = (buf_x - buf_w / 2) + (buf_y - buf_h / 2) * tcols;
      for (int i = 0; i < buf_w; i++)
      {
        // std::cout << "RELIZ " << k << std::endl;
        if (tiles[k] != NULL) tiles[k]->releasePoints ();
        k ++;
      }
      buf_y ++;
      k -= buf_w + tcols * buf_h;
      bk = ((k / tcols) % buf_h) * buf_w + ((k % tcols) % buf_w);
      for (int i = 0; i < buf_w; i++)
      {
        // std::cout << "ADD " << k << " IN " << bk << std::endl;
        if (tiles[k] != NULL)
          tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
        k ++;
        if (++bk % buf_w == 0) bk -= buf_w;
      }
    }
    else buf_y ++;
    if (buf_y == trows - 1) buf_step = 8;
//    std::cout << "V WAY ON OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // VERTICAL SWEEP - LAST SAW ON
  else if (buf_step == 11)
  {
//    std::cout << "V LAST SAW ON IN ..." << std::endl;
    if (buf_x == tcols - 1) // next tooth
    {
      buf_y ++;
      buf_x -= buf_w / 2;
      if (buf_h != trows && buf_y > buf_h / 2 && buf_y + buf_h / 2 < trows)
      {
        k = (buf_x - buf_w / 2) + (buf_y - buf_h / 2 - 1) * tcols;
        for (int i = 0; i < buf_w; i++)
        {
          // std::cout << "RELIZ " << k << std::endl;
          if (tiles[k] != NULL) tiles[k]->releasePoints ();
          k ++;
        }
        k += buf_h * tcols - buf_w;
        bk = ((k / tcols) % buf_h) * buf_w + ((k % tcols) % buf_w);
        for (int i = 0; i < buf_w; i++)
        {
          // std::cout << "ADD " << k << " IN " << bk << std::endl;
          if (tiles[k] != NULL)
            tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
          k ++;
          if (++bk % buf_w == 0) bk -= buf_w;
        }
      }
    }
    else // same tooth
    {
      buf_x ++;
      if (buf_x == tcols - 1 && buf_y == trows - 1) buf_step = 13;
    }
//    std::cout << "V LAST SAW ON OUT -> " << buf_x << " " << buf_y << std::endl;
  }

  // VERTICAL SWEEP - LAST SAW BACK
  else if (buf_step == 12)
  {
//    std::cout << "V LAST SAW BACK IN ..." << std::endl;
    if (buf_x == tcols - 1) // next tooth
    {
      buf_y --;
      buf_x -= buf_w / 2;
      if (buf_h != trows && buf_y >= buf_h / 2 && buf_y + buf_h / 2 < trows - 1)
      {
        k = (buf_x - buf_w / 2) + (buf_y + buf_h / 2 + 1) * tcols;
        for (int i = 0; i < buf_w; i++)
        {
          // std::cout << "RELIZ " << k << std::endl;
          if (tiles[k] != NULL) tiles[k]->releasePoints ();
          k ++;
        }
        k = (tcols - buf_w) + (buf_y - buf_h / 2) * tcols;
        bk = ((k / tcols) % buf_h) * buf_w + ((k % tcols) % buf_w);
        for (int i = 0; i < buf_w; i++)
        {
          // std::cout << "ADD " << k << " IN " << bk << std::endl;
          if (tiles[k] != NULL)
            tiles[k]->loadPoints (buf_ind + bk * buf_ni, buf_pts + bk * buf_np);
          k ++;
          if (++bk % buf_w == 0) bk -= buf_w;
        }
      }
    }
    else // same tooth
    {
      buf_x ++;
      if (buf_x == tcols - 1 && buf_y == 0) buf_step = 13;
    }
//    std::cout << "V LAST SAW BACK OUT -> " << buf_x << " " << buf_y
//              << std::endl;
  }

  // SWEEP STOP
  else if (buf_step == 13)
  {
//    std::cout << "SWEEP STOP IN ..." << std::endl;
    k = 0;
    if (buf_x != 0) k += tcols - buf_w;
    if (buf_y != 0) k += (trows - buf_h) * tcols;
    for (int j = 0; j < buf_h; j++)
    {
      for (int i = 0; i < buf_w; i++)
      {
        // std::cout << "RELIZ " << k << std::endl;
        if (tiles[k] != NULL) tiles[k]->releasePoints ();
        k ++;
      }
      k += tcols - buf_w;
    }
    buf_x = 0;
    buf_y = 0;
    buf_step = 0;
//    std::cout << "SWEEP STOP OUT -> " << buf_x << " " << buf_y << std::endl;
    return (-1);
  }

  // std::cout << "LOADED :";
  // for (int i = 0; i < tcols * trows; i++)
  // if (! tiles[i]->unloaded ()) std::cout << " " << i;
  // std::cout << std::endl;

  return (buf_y * tcols + buf_x);
}


void IPtTileSet::saveSubTile (int imin, int jmin, int imax, int jmax) const
{
  imin *= 5;
  jmin *= 5;
  imax *= 5;
  jmax *= 5;
  int sizex = imax - imin;
  int sizey = jmax - jmin;
  int64_t nxmin = xmin + ((int64_t) imin) * 100;
  int64_t nymin = ymin + ((int64_t) jmin) * 100;
  int64_t dxmin = imin * 100;
  int64_t dymin = jmin * 100;
  
  int itile = imin / twidth, jtile = jmin / theight;
  IPtTile *tilein = tiles[jtile * tcols + itile];
  int index = 0;
  int64_t zm = 0;
  std::vector<Pt3i> pts;
  std::vector<int> inds;
  inds.push_back (index);
  for (int j = jmin; j < jmin + sizey; j++)
  {
    for (int i = imin; i < imin + sizex; i++)
    {
      int icell = i, jcell = j;
      int itile = icell / twidth, jtile = jcell / theight;
      IPtTile *tile = tiles[jtile * tcols + itile];
      if (tile != NULL && ! tile->unloaded ())
      {
        icell = icell - itile * tile->countOfColumns ();
        jcell = jcell - jtile * tile->countOfRows ();
        int nbpts = tile->cellSize (icell, jcell);
        index += nbpts;
        inds.push_back (index);
        if (nbpts != 0)
        {
          Pt3i *pt = tile->cellStartPt (icell, jcell);
          for (int i = 0; i < nbpts; i++)
          {
            if (pt->z () > zm) zm = pt->z ();
            pts.push_back (Pt3i (
                   (int) (pt->x () + tile->xSpread () * itile - dxmin),
                   (int) (pt->y () + tile->ySpread () * jtile - dymin),
                   pt->z ()));
            pt ++;
          }
        }
      }
    }
  }

  IPtTile *ntile = new IPtTile (sizey, sizex);
  ntile->setArea (nxmin, nymin, zm, tilein->cellSize ());
  ntile->setData (pts, inds);
  ntile->save ("til/top/top_newtile.til");
  delete ntile;
}


void IPtTileSet::check ()
{
  tiles[0]->check ();
}
