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
#include "ipttile.h"


const int IPtTile::XYZ_UNIT = 1000; // assumed to be 1 meter
const int IPtTile::MIN_CELL_SIZE = 100;
const int IPtTile::TOP = 1;
const int IPtTile::MID = 5;
const int IPtTile::ECO = 10;
const std::string IPtTile::TOP_DIR = std::string ("top/");
const std::string IPtTile::MID_DIR = std::string ("mid/");
const std::string IPtTile::ECO_DIR = std::string ("eco/");
const std::string IPtTile::XYZ_DIR = std::string ("xyz/");
const std::string IPtTile::TOP_PREFIX = std::string ("top_");
const std::string IPtTile::MID_PREFIX = std::string ("mid_");
const std::string IPtTile::ECO_PREFIX = std::string ("eco_");
const std::string IPtTile::TIL_SUFFIX = std::string (".til");
const std::string IPtTile::LAB_SUFFIX = std::string (".tpl");
const std::string IPtTile::XYZ_SUFFIX = std::string (".xyz");
const std::string IPtTile::XYZL_SUFFIX = std::string (".xyzl");

const int IPtTile::R_OFF = 5;


IPtTile::IPtTile (int nbrows, int nbcols)
{
  cols = nbcols;
  rows = nbrows;
  xmin = 0;
  ymin = 0;
  zmax = 0;
  nb = 0;
  csize = 1;
  labelling = false;
  cells = new int[rows * cols + 1];
  for (int i = 0; i < rows * cols + 1; i++) cells[i] = 0;
  points = NULL;
  labels = NULL;
}


IPtTile::IPtTile (std::string name)
{
  fname = name;
  cols = 1;
  rows = 1;
  xmin = 0;
  ymin = 0;
  zmax = 0;
  nb = 0;
  csize = 1;
  labelling = false;
  cells = NULL;
  points = NULL;
  labels = NULL;
}


IPtTile::IPtTile (const std::string &dir, const std::string &name, int acc)
{
  fname = dir;
  if (acc == TOP) fname += TOP_DIR + TOP_PREFIX + name + TIL_SUFFIX;
  else if (acc == MID) fname += MID_DIR + MID_PREFIX + name + TIL_SUFFIX;
  else if (acc == ECO) fname += ECO_DIR + ECO_PREFIX + name + TIL_SUFFIX;
  cols = 1;
  rows = 1;
  xmin = 0;
  ymin = 0;
  zmax = 0;
  nb = 0;
  csize = 1;
  labelling = false;
  cells = NULL;
  points = NULL;
  labels = NULL;
}


IPtTile::~IPtTile ()
{
  if (points != NULL) delete [] points;
  if (labels != NULL) delete [] labels;
  if (cells != NULL) delete [] cells;
}


void IPtTile::setSize (int w, int h)
{
  cols = w;
  rows = h;
}


void IPtTile::setArea (int64_t xmin, int64_t ymin, int64_t zmax, int cellsize)
{
  this->xmin = xmin;
  this->ymin = ymin;
  this->zmax = zmax;
  this->csize = cellsize;
}


void IPtTile::setData (std::vector<Pt3i> pts, std::vector<int> inds)
{
  nb = (int) (pts.size ());
  points = new Pt3i[nb];
  Pt3i *pt = points;
  std::vector<Pt3i>::iterator pit = pts.begin ();
  while (pit != pts.end ()) (pt++)->set (*pit++);
  int *cl = cells;
  std::vector<int>::iterator iit = inds.begin ();
  while (iit != inds.end ()) *cl++ = *iit++;
}


void IPtTile::setCountOfPoints (int nb)
{
  this->nb = nb;
  points = new Pt3i[nb];
}


bool IPtTile::getPoints (std::vector<Pt3i> &pts, int i, int j) const
{
  int k = cells[j * cols + i];
  Pt3i *pt = points + k;
  while (k++ < cells[j * cols + i + 1]) pts.push_back (*pt++);
  return (k != cells[j * cols + i] + 1);
}


int IPtTile::collectCellPoints (std::vector<Pt3i> &pts, int i, int j) const
{
  int k = cells[j * cols + i];
  Pt3i *pt = points + k;
  while (k++ < cells[j * cols + i + 1]) pts.push_back (*pt++);
  return (cells[j * cols + i + 1] - cells[j * cols + i]);
}


int IPtTile::collectSubcellPoints (std::vector<Pt3i> &pts, int i, int j) const
{
  if (cellSize () == MIN_CELL_SIZE) return (collectCellPoints (pts, i, j));
  int nbpts = 0;
  int nbsub = cellSize () / MIN_CELL_SIZE;
  Pt3i *pt = points + cells[(j / nbsub) * cols + (i / nbsub)];
  Pt3i *ptfin = pt + cellSize (i / nbsub, j / nbsub);
  while (pt->y () < j * MIN_CELL_SIZE && pt != ptfin) pt ++;
  while (pt->x () < i * MIN_CELL_SIZE && pt != ptfin) pt ++;
  while (pt->x () < (i + 1) * MIN_CELL_SIZE
         && pt->y () < (j + 1) * MIN_CELL_SIZE && pt != ptfin)
  {
    pts.push_back (Pt3i (pt->x (), pt->y (), pt->z ()));
    nbpts ++;
    pt ++;
  }
  return (nbpts);
}


void IPtTile::setPoints (int nb, const IPtTile &tin)
{
  this->nb = nb;
  points = new Pt3i[nb];
  cells = new int[rows * cols + 1];
  int div = tin.cellSize () / csize;
  int nbpts = 0;
  int *c = cells;
  Pt3i *pout = points;
  Pt3i *fin = tin.getPointsArrayEnd ();
  *c++ = 0;
  for (int j = 0; j < rows; j++)
    for (int i = 0; i < cols; )
    {
      Pt3i *ptcell = tin.cellStartPt (i / div, j / div);
      while (ptcell->y () < j * csize && ptcell != fin) ptcell ++;
      for (int k = 0; k < div; k++)
      {
        while (ptcell->y () < (j + 1) * csize
               && ptcell->x () < (i + 1) * csize && ptcell != fin)
        {
          pout->set (*ptcell++);
          pout ++;
          nbpts++;
        }
        *c++ = nbpts;
        i++;
      }
    }
  if (nb != nbpts)
    std::cout << (xmin / 1000) << "_" << (ymin / 1000)
              << ": NB theorique = " << nb << " et NB found = " << nbpts
              << std::endl;
}


void IPtTile::setPoints (const IPtTile &tin)
{
  this->nb = tin.size ();
  points = new Pt3i[nb];
  if (cells != NULL) delete cells;
  cells = new int[rows * cols + 1];
  int subsize = csize / MIN_CELL_SIZE;

  Pt3i *pout = points;
  int *c = cells;
  *c++ = 0;

  int cumul = 0;
  std::vector<Pt3i> pts;
  std::vector<Pt3i>::iterator it;
  for (int j = 0; j < rows; j++)
    for (int i = 0; i < cols; i++)
    {
      pts.clear ();
      for (int j2 = 0; j2 < subsize; j2 ++)
        for (int i2 = 0; i2 < subsize; i2 ++)
          tin.collectSubcellPoints (pts, i * subsize + i2, j * subsize + j2);
      it = pts.begin ();
      while (it != pts.end ())
      {
        (pout++)->set (it->x (), it->y (), it->z ());
        it ++;
      }
      cumul += (int) (pts.size ());
      *c++ = cumul;
    }
}


bool IPtTile::save (std::string name) const
{
  std::ofstream fpts (name.c_str (), std::ios::out | std::ofstream::binary);
  if (! fpts.is_open ()) return false;
  fpts.write ((char *) (&cols), sizeof (int));
  fpts.write ((char *) (&rows), sizeof (int));
  fpts.write ((char *) (&xmin), sizeof (int64_t));
  fpts.write ((char *) (&ymin), sizeof (int64_t));
  fpts.write ((char *) (&zmax), sizeof (int64_t));
  fpts.write ((char *) (&csize), sizeof (int));
  fpts.write ((char *) (&nb), sizeof (int));
  fpts.write ((char *) cells, sizeof (int) * (rows * cols + 1));
  fpts.write ((char *) points, sizeof (Pt3i) * (nb));
  fpts.close ();
  return true;
}


bool IPtTile::save () const
{
  return (save (fname));
}


bool IPtTile::load (std::string name, bool all)
{
  std::ifstream fpts (name.c_str (), std::ios::in | std::ifstream::binary);
  if (! fpts.is_open ()) return false;
  fpts.read ((char *) (&cols), sizeof (int));
  fpts.read ((char *) (&rows), sizeof (int));
  fpts.read ((char *) (&xmin), sizeof (int64_t));
  fpts.read ((char *) (&ymin), sizeof (int64_t));
  fpts.read ((char *) (&zmax), sizeof (int64_t));
  fpts.read ((char *) (&csize), sizeof (int));
  fpts.read ((char *) (&nb), sizeof (int));
  if (all)
  {
    if (cells != NULL)
    {
      delete cells;
      cells = NULL;
    }
    cells = new int[rows * cols + 1];
    fpts.read ((char *) cells, sizeof (int) * (rows * cols + 1));
    if (points == NULL) points = new Pt3i[nb];
    fpts.read ((char *) points, sizeof (Pt3i) * (nb));
  }
  fpts.close ();
  return (true);
}


bool IPtTile::load (bool all)
{
  std::ifstream fpts (fname.c_str (), std::ios::in | std::ifstream::binary);
  if (! fpts.is_open ()) return false;

  fpts.read ((char *) (&cols), sizeof (int));
  fpts.read ((char *) (&rows), sizeof (int));
  fpts.read ((char *) (&xmin), sizeof (int64_t));
  fpts.read ((char *) (&ymin), sizeof (int64_t));
  fpts.read ((char *) (&zmax), sizeof (int64_t));
  fpts.read ((char *) (&csize), sizeof (int));
  fpts.read ((char *) (&nb), sizeof (int));
  if (all)
  {
    if (cells != NULL)
    {
      delete cells;
      cells = NULL;
    }
    cells = new int[rows * cols + 1];
    fpts.read ((char *) cells, sizeof (int) * (rows * cols + 1));
    if (points == NULL) points = new Pt3i[nb];
    fpts.read ((char *) points, sizeof (Pt3i) * (nb));
  }
  fpts.close ();
  return (true);
}


bool IPtTile::loadPoints (int *ind, Pt3i *pts)
{
  std::ifstream fpts (fname.c_str (), std::ios::in | std::ifstream::binary);
  if (! fpts.is_open ())
  {
    std::cout << "Loading of " << fname << " failed" << std::endl;
    return false;
  }
  fpts.read ((char *) (&cols), sizeof (int));
  fpts.read ((char *) (&rows), sizeof (int));
  fpts.read ((char *) (&xmin), sizeof (int64_t));
  fpts.read ((char *) (&ymin), sizeof (int64_t));
  fpts.read ((char *) (&zmax), sizeof (int64_t));
  fpts.read ((char *) (&csize), sizeof (int));
  fpts.read ((char *) (&nb), sizeof (int));
  cells = ind;
  fpts.read ((char *) cells, sizeof (int) * (rows * cols + 1));
  points = pts;
  fpts.read ((char *) points, sizeof (Pt3i) * (nb));
  fpts.close ();
  return (true);
}


void IPtTile::releasePoints ()
{
  // Just to avoid point and index arrays to be freed, when padding
  // Do not delete the data here !!!
  cells = NULL;
  points = NULL;
}


int IPtTile::cellMaxSize () const
{
  int max = 0;
  int * c = cells;
  for (int i = 0; i < rows * cols; i++)
  {
    if (*(c+1) - *c > max) max = *(c+1) - *c;
    c++;
  }
  return max;
}


int IPtTile::cellMinSize (int max) const
{
  int min = max;
  int * c = cells;
  for (int i = 0; i < rows * cols; i++)
  {
    if (*(c+1) - *c < min) min = *(c+1) - *c;
    c++;
  }
  return min;
}


int IPtTile::countOfLabelledPoints ()
{
  int nblp = 0;
  unsigned char *lab = labels;
  for (int i = 0; i < nb; i++) if (*lab++ == 1) nblp ++;
  return nblp;
}


std::string IPtTile::tileName () const
{
  std::string tname;
  size_t epos = fname.find (TIL_SUFFIX);
  if (fname.find (TOP_DIR) != std::string::npos)
  {
    size_t spos = fname.find (TOP_PREFIX) + TOP_PREFIX.length ();
    tname = TOP_PREFIX + fname.substr (spos, epos - spos);
  }
  else if (fname.find (MID_DIR) != std::string::npos)
  {
    size_t spos = fname.find (MID_PREFIX) + MID_PREFIX.length ();
    tname = MID_PREFIX + fname.substr (spos, epos - spos);
  }
  else if (fname.find (ECO_DIR) != std::string::npos)
  {
    size_t spos = fname.find (ECO_PREFIX) + ECO_PREFIX.length ();
    tname = ECO_PREFIX + fname.substr (spos, epos - spos);
  }
  return tname;
}


bool IPtTile::saveLabels (std::string dir) const
{
  if (! labelling) return false;
  std::string labf (dir + tileName () + LAB_SUFFIX);
  std::ofstream fpts (labf.c_str (), std::ios::out | std::ofstream::binary);
  if (! fpts.is_open ()) return false;
  fpts.write ((char *) labels, sizeof (unsigned char) * nb);
  fpts.close ();
  return true;
}


bool IPtTile::loadLabels (std::string dir)
{
  std::string labf (dir + tileName () + LAB_SUFFIX);
  std::ifstream fpts (labf.c_str (), std::ios::in | std::ifstream::binary);
  if (fpts.is_open ())
  {
    if (! labelling)
    {
      labels = new unsigned char[nb];
      labelling = true;
    }
    fpts.read ((char *) labels, sizeof (unsigned char) * nb);
    fpts.close ();
    return true;
  }
  return false;
}


void IPtTile::createLabels ()
{
  if (! labelling)
  {
    labels = new unsigned char[nb];
    unsigned char *labs = labels;
    for (int i = 0; i < nb; i++) *labs++ = (unsigned char) 0;
  }
  labelling = true;
}


void IPtTile::resetLabels ()
{
  if (labelling && labels != NULL)
  {
    delete [] labels;
    labels = NULL;
  }
  labelling = false;
}


bool IPtTile::isLabelled (int i, int j)
{
  int nbpts = cells[j * cols + i + 1] - cells[j * cols + i];
  unsigned char *lab = labels + cells[j * cols + i];
  if (csize == MIN_CELL_SIZE)
  {
    for (int k = 0; k < nbpts; k++) if (*lab++ == 1) return true;
  }
  else
  {
    int cdiv = csize / MIN_CELL_SIZE;
    int cxmin = i * csize + (i % cdiv) * MIN_CELL_SIZE;
    int cymin = j * csize + (j % cdiv) * MIN_CELL_SIZE;
    int cxmax = cxmin + MIN_CELL_SIZE;
    int cymax = cymin + MIN_CELL_SIZE;

    Pt3i *pt = points + cells[j * cols + i];
    Pt3i *ptfin = pt + nbpts;
    while (pt->y () < cymin && pt != ptfin)
    {
      pt ++;
      lab ++;
    }
    while (pt->x () < cxmin && pt != ptfin)
    {
      pt ++;
      lab ++;
    }
    while (pt->x () < cxmax && pt->y () < cymax && pt != ptfin)
    {
      if (*lab++ == 1) return true;
      pt ++;
    }
  }
  return false;
}


void IPtTile::labelAsTrack (int plab)
{
  labels[plab] = 1;
}


void IPtTile::unlabel (int i, int j)
{
  int nbpts = cells[j * cols + i + 1] - cells[j * cols + i];
  unsigned char *lab = labels + cells[j * cols + i];
  if (csize == MIN_CELL_SIZE)
  {
    for (int k = 0; k < nbpts; k++) *lab++ = (unsigned char) 0;
  }
  else
  {
    int cdiv = csize / MIN_CELL_SIZE;
    int cxmin = i * csize + (i % cdiv) * MIN_CELL_SIZE;
    int cymin = j * csize + (j % cdiv) * MIN_CELL_SIZE;
    int cxmax = cxmin + MIN_CELL_SIZE;
    int cymax = cymin + MIN_CELL_SIZE;

    Pt3i *pt = points + cells[j * cols + i];
    Pt3i *ptfin = pt + nbpts;
    while (pt->y () < cymin && pt != ptfin)
    {
      pt ++;
      lab ++;
    }
    while (pt->x () < cxmin && pt != ptfin)
    {
      pt ++;
      lab ++;
    }
    while (pt->x () < cxmax && pt->y () < cymax && pt != ptfin)
    {
      *lab++ = (unsigned char) 0;
      pt ++;
    }
  }
}


bool IPtTile::loadXYZFile (std::string ptsfile, int subdiv, bool lab_in)
{
  // Opens XYZ file
  bool labelled = (ptsfile.find (XYZL_SUFFIX) != std::string::npos);
  lab_in = lab_in && labelled;
  std::cout << "loading " << ptsfile << " ..." << std::endl;
  std::ifstream fpts (ptsfile.c_str (), std::ios::in);
  if (! fpts.is_open ()) return false;

  // Load XYZ file points
  nb = 0;
  double x, y, z;
  char lab;
  int ix, iy, iz;
  int lrow = rows * subdiv;
  int lcol = cols * subdiv;
  std::vector<Pt3i> outs;
  std::vector<Pt3i> **xyzcells = new std::vector<Pt3i>*[lrow];
  std::vector<unsigned char> **labcells
         = (lab_in ? new std::vector<unsigned char>*[lrow] : NULL);
  for (int i = 0; i < lrow; i++)
  {
    xyzcells[i] = new std::vector<Pt3i>[lcol];
    if (lab_in) labcells[i] = new std::vector<unsigned char>[lcol];
  }
  fpts >> x;
  for (int i = 0; ! fpts.eof (); i++)
  {
    fpts >> y;
    fpts >> z;
    if (labelled) fpts >> lab;
    ix = (int) ((int64_t) (x * XYZ_UNIT + 0.5) - xmin);
    iy = (int) ((int64_t) (y * XYZ_UNIT + 0.5) - ymin);
    iz = (int) (z * XYZ_UNIT + 0.5);

    int gx = (ix * subdiv) / csize;
    int gy = (iy * subdiv) / csize;
    if (gx < 0 || gy < 0 || gx >= lcol || gy >= lrow)
    {
      outs.push_back (Pt3i (ix, iy, iz));
      std::cout << "Out pt (" << ix << ", " << iy << ", " << iz << ") -> ("
                << gx << ", " << gy << ")" << std::endl;
      std::cout << "Origin " << nb << " : " << x << ", " << y << ", " << z
                << ")" << std::endl;
    }
    else
    {
      xyzcells[gy][gx].push_back (Pt3i (ix, iy, iz));
      if (lab_in) labcells[gy][gx].push_back (
                      lab == 'P' ? (unsigned char) 1 : (unsigned char) 0);
      nb ++;
      if (iz > zmax) zmax = iz;
    }
    fpts >> x;
  }
  fpts.close ();

  // Displays statistics
  std::cout << "Outliers size = " << outs.size () << std::endl;
  int cmax = 0;
  for (int j = 0; j < lrow; j++)
    for (int i = 0; i < lcol; i++)
      if ((int) xyzcells[j][i].size () > cmax)
        cmax = (int) xyzcells[j][i].size ();
  int cmin = cmax;
  for (int j = 0; j < lrow; j++)
    for (int i = 0; i < lcol; i++)
      if ((int) xyzcells[j][i].size () < cmin)
        cmin = (int) xyzcells[j][i].size ();
  std::cout << "Max cell size = " << cmax << std::endl;
  std::cout << "Min cell size = " << cmin << std::endl;
  int nz = 0, nyny = 0, nlab = 0;
  for (int j = 0; j < lrow; j++)
    for (int i = 0; i < lcol; i++)
      if (xyzcells[j][i].empty ()) nz ++;
      else
      {
        nyny ++;
        if (lab_in)
        {
          std::vector<unsigned char>::iterator lit = labcells[j][i].begin ();
          while (lit != labcells[j][i].end ())
            if (*lit++ == (unsigned char) 1) nlab ++;
        }
      }
  std::cout << nz << " cellules vides" << std::endl;
  std::cout << nyny << " cellules occupees" << std::endl;
  if (lab_in) std::cout << nlab << " labelled points" << std::endl;

  // Sets IPtTile structure
  points = new Pt3i[nb];
  if (lab_in && ! labelling)
  {
    labels = new unsigned char[nb];
    labelling = true;
  }
  Pt3i *ppoints = points;
  unsigned char *plabs = labels;
  int *pcells = cells;
  *pcells++ = 0;
  int inb = 0;
  for (int j = 0; j < rows ; j++)
    for (int i = 0; i < cols ; i++)
    {
      for (int cj = 0; cj < subdiv; cj ++)
        for (int ci = 0; ci < subdiv; ci ++)
        {
          inb += (int) (xyzcells[j * subdiv + cj][i * subdiv + ci].size ());
          const std::vector<Pt3i> *pts
            = &(xyzcells[j * subdiv + cj][i * subdiv + ci]);
          const std::vector<unsigned char> *labs = NULL;
          if (lab_in) labs = &(labcells[j * subdiv + cj][i * subdiv + ci]);
          std::vector<Pt3i>::const_iterator it = pts->begin ();
          std::vector<unsigned char>::const_iterator lit;
          if (lab_in) lit = labs->begin ();
          while (it != pts->end ())
          {
            (ppoints++)->set (it->x () + R_OFF, it->y () + R_OFF, it->z ());
            if (lab_in) *plabs++ = *lit++;
            it ++;
          }
        }
      *pcells++ = inb;
    }

  // Temporary cloud memory release
  for (int i = 0; i < lrow; i++) delete [] xyzcells[i];
  delete [] xyzcells;
  if (lab_in)
  {
    for (int i = 0; i < lrow; i++) delete [] labcells[i];
    delete [] labcells;
  }
  return true;
}


bool IPtTile::saveXYZFile (bool lab_out) const
{
  std::string pf (XYZ_DIR);
  size_t epos = fname.find (TIL_SUFFIX);
  if (fname.find (TOP_DIR) != std::string::npos)
  {
    size_t spos = fname.find (TOP_PREFIX) + TOP_PREFIX.length ();
    pf += fname.substr (spos, epos - spos);
  }
  else if (fname.find (MID_DIR) != std::string::npos)
  {
    size_t spos = fname.find (MID_PREFIX) + MID_PREFIX.length ();
    pf += fname.substr (spos, epos - spos);
  }
  else if (fname.find (ECO_DIR) != std::string::npos)
  {
    size_t spos = fname.find (ECO_PREFIX) + ECO_PREFIX.length ();
    pf += fname.substr (spos, epos - spos);
  }
  pf += (lab_out && labelling ? XYZL_SUFFIX : XYZ_SUFFIX);
  return (saveXYZFile (pf, lab_out && labelling));
}


bool IPtTile::saveXYZFile (std::string name, bool lab_out) const
{
  std::cout << "saving " << name << " (" << nb << "pts) ..." << std::endl;
  lab_out = lab_out && labelling
            && (name.find (XYZL_SUFFIX) != std::string::npos);
  std::ofstream fpts (name.c_str (), std::ios::out);
  if (! fpts.is_open ())
  {
    std::cout << "Can't save tile in " << name << std::endl;
    return false;
  }
  Pt3i *ppt = points;
  int nbl = 0;
  unsigned char *lbs = labels;
  for (int i = 0; i < nb; i++)
  {
    int64_t vx = xmin + ppt->x () - R_OFF;
    int64_t vy = ymin + ppt->y () - R_OFF;
    int64_t vz = ppt->z ();
    int dcx = (int) (vx - (vx / 1000) * 1000);
    int dcy = (int) (vy - (vy / 1000) * 1000);
    int dcz = (int) (vz - (vz / 1000) * 1000);
    std::string digx (".");
    if (dcx < 100) digx += std::string ("0");
    if (dcx < 10) digx += std::string ("0");
    std::string digy (".");
    if (dcy < 100) digy += std::string ("0");
    if (dcy < 10) digy += std::string ("0");
    std::string digz (".");
    if (dcz < 100) digz += std::string ("0");
    if (dcz < 10) digz += std::string ("0");
    if (lab_out) fpts << (vx / 1000) << digx << dcx << " "
                  << (vy / 1000) << digy << dcy << " "
                  << (vz / 1000) << digz << dcz << " "
                  << (*lbs == (unsigned char) 1 ? "P" : "N") << std::endl;
    else fpts << (vx / 1000) << digx << dcx << " "
              << (vy / 1000) << digy << dcy << " "
              << (vz / 1000) << digz << dcz << std::endl;
    if (lab_out && *lbs == (unsigned char) 1) nbl ++;
    ppt ++;
    lbs ++;
  }
  fpts.close ();
  std::cout << "  saved " << nb << " pts ("
            << nbl << " labelled)" << std::endl;
  return true;
}


void IPtTile::check () const
{
  std::cout << "TILE " << fname << std::endl;
  std::cout << rows << " rows x " << cols << " columns" << std::endl;
  std::cout << "Xmin = " << xmin << ", Ymin = " << ymin
            << ", Csize = " << csize << std::endl;
  std::cout << nb << " points, Zmax = " << zmax << std::endl;
  if (cells != NULL)
    std::cout << "Cell[112] = " << cells[112] << " et Pt[112] = ("
              << points[112].x () << ", " << points[112].y () << ", "
              << points[112].z () << ")" << std::endl;
}
