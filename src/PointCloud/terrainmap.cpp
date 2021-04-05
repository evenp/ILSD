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
#include <inttypes.h>
#include <cmath>
#include "asmath.h"
#include "terrainmap.h"

#define EPS 0.001f
#define MM2M 0.001f

const int TerrainMap::SHADE_HILL = 0;
const int TerrainMap::SHADE_SLOPE = 1;
const int TerrainMap::DEFAULT_PAD_SIZE = 3;

const float TerrainMap::RELIEF_AMPLI = 5.0f;
const float TerrainMap::LIGHT_ANGLE_INCREMENT = 0.03f;


TerrainMap::TerrainMap ()
{
  nmap = NULL;
  arr_files = NULL;
  iwidth = 0;
  iheight = 0;
  twidth = 0;
  theight = 0;
  cell_size = 0.0f;
  x_min = 0.0;
  y_min = 0.0;
  fx_min = 0.0;
  fy_min = 0.0;
  no_data = 0.0;
  shading = SHADE_HILL;
  light_angle = 0.0f;
  light_v1.set (- ASF_SQRT2_2, 0.0f, ASF_SQRT2_2);
  light_v2.set (0.25f, - ASF_SQRT3_2 / 2, ASF_SQRT3_2);
  light_v3.set (0.25f, ASF_SQRT3_2 / 2, ASF_SQRT3_2);
  pad_size = DEFAULT_PAD_SIZE;
  pad_w = pad_size;
  pad_h = pad_size;
  pad_ref = -1;
  ts_cot = 1;
  ts_rot = 1;
}


TerrainMap::~TerrainMap ()
{
  clear ();
}


void TerrainMap::clear ()
{
  if (arr_files != NULL)
  {
    for (int i = 0; i < ts_cot * ts_rot; i ++)
      if (arr_files[i] != NULL) delete arr_files[i];
    delete [] arr_files;
  }
  arr_files = NULL;
  if (nmap != NULL) delete [] nmap;
  nmap = NULL;
  layout.clear ();
  dtm_files.clear ();
  nvm_files.clear ();
}


bool TerrainMap::addDtmFile (const std::string &name)
{
  std::ifstream dtmf (name.c_str (), std::ios::in);
  if (! dtmf)
  {
    std::cout << "File " << name << " can't be opened" << std::endl;
    return false;
  }
  char val[20];
  int width = 0, height = 0;
  double xllc = 0., yllc = 0., nodata = 0.;
  float csize = 0.0f;

  dtmf >> val;
  dtmf >> width;
  dtmf >> val;
  dtmf >> height;
  dtmf >> val;
  dtmf >> xllc;
  xllc = (double) ((int) xllc); // to remove the shift found in asc files ...
  dtmf >> val;
  dtmf >> yllc;
  yllc = (double) ((int) yllc);
  dtmf >> val;
  dtmf >> csize;

  if (iwidth == 0)
  {
    twidth = width;
    theight = height;
    iwidth = width;
    iheight = height;
    x_min = xllc;
    y_min = yllc;
    fx_min = xllc;
    fy_min = yllc;
    cell_size = csize;
    no_data = nodata;
    layout.push_back (Pt2i (0, 0));
  }
  else
  {
    if (width != twidth)
    {
      std::cout << "File " << name << " inconsistent width" << std::endl;
      return false;
    }
    if (height != theight)
    {
      std::cout << "File " << name << " inconsistent height" << std::endl;
      return false;
    }
    if (csize != cell_size)
    {
      std::cout << "File " << name << " inconsistent cell size" << std::endl;
      return false;
    }

    double shift = ((xllc - x_min) / csize) / width;
    int xshift = (int) (shift + (shift < 0 ? - 0.5 : 0.5));
    double err = xllc - (x_min + xshift * csize * width);
    if (err < - EPS || err > EPS)
    {
      std::cout << "File " << name << " : xllc irregular" << std::endl;
      return false;
    }
    shift = ((yllc - y_min) / csize) / height;
    int yshift = (int) (shift + (shift < 0 ? - 0.5 : 0.5));
    err = yllc - (y_min + yshift * csize * height);
    if (err < - EPS || err > EPS)
    {
      std::cout << "File " << name << " : yllc irregular" << std::endl;
      return false;
    }
    if (xshift < 0 || yshift < 0)
    {
      std::vector<Pt2i>::iterator it = layout.begin ();
      while (it != layout.end ())
      {
        if (xshift < 0) it->set (it->x () - xshift, it->y ());
        if (yshift < 0) it->set (it->x (), it->y () - yshift);
        it ++;
      }
      if (xshift < 0)
      {
        iwidth -= xshift * width;
        xshift = 0;
        x_min = xllc;
      }
      if (yshift < 0)
      {
        iheight -= yshift * height;
        yshift = 0;
        y_min = yllc;
      }
    }
    layout.push_back (Pt2i (xshift, yshift));
    if (iwidth / width <= xshift) iwidth = (xshift + 1) * width;
    if (iheight / height <= yshift) iheight = (yshift + 1) * height;
  }

  dtm_files.push_back (name);
  return true;
}


bool TerrainMap::addNormalMapFile (const std::string &name)
{
  std::ifstream dtmf (name, std::ios::in);
  if (! dtmf) return false;
  dtmf.close ();
  nvm_files.push_back (name);
  return true;
}


bool TerrainMap::create ()
{
  double *hval = new double[iwidth * iheight];
  for (int i = 0; i < iwidth * iheight; i++) hval[i] = no_data;

  std::vector<Pt2i>::iterator it = layout.begin ();
  std::vector<std::string>::iterator itn = dtm_files.begin ();
  while (it != layout.end ())
  {

    int dx = it->x () * twidth;
    int dy = (iheight / theight - 1 - it->y ()) * theight;
    std::cout << "Opening " << *itn << std::endl;
    std::ifstream dtmf (itn->c_str (), std::ios::in);
    if (! dtmf) return false;
    char val[15];
    double hv = 0.0, nodata = 0.0;
    for (int i = 0; i < 11; i++) dtmf >> val;
    dtmf >> nodata;

    for (int j = 0; j < theight; j++)
      for (int i = 0; i < twidth; i++)
      {
        dtmf >> hv;
        hval[(dy + j) * iwidth + dx + i] = (hv == nodata ? no_data : hv);
      }
    dtmf.close ();
    it ++;
    itn ++;
  }

  if (nmap != NULL) delete [] nmap;
  nmap = new Pt3f[iwidth * iheight];
  Pt3f *nval = nmap;
  double dhx, dhy;
  for (int j = 0; j < iheight; j++)
  {
    for (int i = 0; i < iwidth; i++)
    {
      if (j == iheight - 1)
        dhy = (hval[j * iwidth + i] - hval[(j-1) * iwidth + i])
              * 2 * RELIEF_AMPLI;
      else if (j == 0)
        dhy = (hval[(j+1) * iwidth + i] - hval[j * iwidth + i])
              * 2 * RELIEF_AMPLI;
      else dhy = (hval[(j+1) * iwidth + i] - hval[(j-1) * iwidth + i])
                 * RELIEF_AMPLI;
      if (i == iwidth - 1)
        dhx = (hval[j * iwidth + i] - hval[j * iwidth + i-1])
              * 2 * RELIEF_AMPLI;
      else if (i == 0)
        dhx = (hval[j * iwidth + i+1] - hval[j * iwidth + i])
              * 2 * RELIEF_AMPLI;
      else dhx = (hval[j * iwidth + i+1] - hval[j * iwidth + i-1])
                 * RELIEF_AMPLI;

      nval->set (- (float) dhx, - (float) dhy, 1.0f);
      nval->normalize ();
      nval++;
    }
  }
  delete [] hval;
    
  return true;
}


bool TerrainMap::assembleMap (int cols, int rows, int64_t xmin, int64_t ymin)
{
  int locw = 0, loch = 0, loci = 0, locj = 0;
  float wmap = 0.0f, hmap = 0.0f, locs = 0.0f, locxmin = 0.0f, locymin = 0.0f;
  twidth = 0;
  theight = 0;
  x_min = ((double) xmin) * MM2M;
  y_min = ((double) ymin) * MM2M;
  std::vector<std::string>::iterator it = nvm_files.begin ();
  while (it != nvm_files.end ())
  {
    std::ifstream nvmf (it->c_str (), std::ios::in | std::ifstream::binary);
    if (! nvmf) std::cout << "File " << *it << " can't be opened" << std::endl;
    else
    {
      nvmf.read ((char *) (&locw), sizeof (int));
      nvmf.read ((char *) (&loch), sizeof (int));
      nvmf.read ((char *) (&locs), sizeof (float));
      nvmf.read ((char *) (&locxmin), sizeof (float));
      nvmf.read ((char *) (&locymin), sizeof (float));
      if (twidth != 0)
      {
        double dx = locxmin - x_min;
        if (dx < 0.0) dx = - dx;
        int dix = (int) (dx + 0.5);
        double dy = locymin - y_min;
        if (dy < 0.0) dy = - dy;
        int diy = (int) (dy + 0.5);
        if (locw != twidth || loch != theight || locs != cell_size
            || dix % (int) wmap != 0 || diy % (int) hmap != 0)
        {
          std::cout << "Tile " << *it << " cannot be assembled" << std::endl;
          nvmf.close ();
          return false;
        }
      }
      else
      {
        twidth = locw;
        theight = loch;
        cell_size = locs;
        iwidth = cols * twidth;
        iheight = rows * theight;
        if (nmap != NULL) delete [] nmap;
        nmap = new Pt3f[iwidth * iheight];
      }
      wmap = twidth * cell_size;
      hmap = theight * cell_size;
      loci = (int) ((locxmin - x_min + wmap / 2) / wmap);
      locj = (int) ((locymin - y_min + hmap / 2) / hmap);
      Pt3f *line = nmap + iwidth * (iheight - 1);
      line -= locj * theight * iwidth;
      line += loci * twidth;
      for (int j = 0; j < theight; j++)
      {
        nvmf.read ((char *) line, twidth * sizeof (Pt3f));
        line -= iwidth;
      }
      nvmf.close ();
    }
    it ++;
  }
  return true;
}


bool TerrainMap::arrangeFiles (int cols, int rows, int64_t xmin, int64_t ymin,
                                bool loading)
{
  int locw = 0, loch = 0, loci = 0, locj = 0;
  float wmap = 0.0f, hmap = 0.0f, locs = 0.0f, locxmin = 0.0f, locymin = 0.0f;
  ts_cot = cols;
  ts_rot = rows;
  twidth = 0;
  theight = 0;
  x_min = (double) (xmin) * MM2M;
  y_min = (double) (ymin) * MM2M;
  arr_files = new std::string *[cols * rows];
  for (int i = 0; i < cols * rows; i++) arr_files[i] = NULL;
  std::vector<std::string>::iterator it = nvm_files.begin ();
  while (it != nvm_files.end ())
  {
    std::ifstream nvmf (it->c_str (), std::ios::in | std::ifstream::binary);
    if (! nvmf) std::cout << "File " << *it << " can't be opened" << std::endl;
    else
    {
      nvmf.read ((char *) (&locw), sizeof (int));
      nvmf.read ((char *) (&loch), sizeof (int));
      nvmf.read ((char *) (&locs), sizeof (float));
      nvmf.read ((char *) (&locxmin), sizeof (float));
      nvmf.read ((char *) (&locymin), sizeof (float));
      nvmf.close ();
      if (twidth != 0)
      {
        if (locw != twidth)
        {
          std::cout << "File " << *it << " inconsistent width" << std::endl;
          return false;
        }
        if (loch != theight)
        {
          std::cout << "File " << *it << " inconsistent height" << std::endl;
          return false;
        }
        if (locs != cell_size)
        {
          std::cout << "File " << *it << " inconsistent cell size" << std::endl;
          return false;
        }
      }
      else
      {
        twidth = locw;
        theight = loch;
        cell_size = locs;
        iwidth = cols * twidth;
        iheight = rows * theight;
        if (loading)
        {
          if (nmap != NULL) delete [] nmap;
          nmap = new Pt3f[iwidth * iheight];
        }
      }
      wmap = twidth * cell_size;
      hmap = theight * cell_size;
      loci = (int) ((locxmin - x_min + wmap / 2) / wmap);
      locj = (int) ((locymin - y_min + hmap / 2) / hmap);
      arr_files[locj * cols + loci] = &(*it);
    }
    it ++;
  }
  return true;
}


bool TerrainMap::loadDtmMapInfo (const std::string &name)
{
  std::ifstream dtmf (name.c_str (), std::ios::in);
  if (! dtmf)
  {
    std::cout << "File " << name << " can't be opened" << std::endl;
    return false;
  }
  char val[20];
  double xllc = 0., yllc = 0.;
  dtmf >> val;
  dtmf >> twidth;
  dtmf >> val;
  dtmf >> theight;
  dtmf >> val;
  dtmf >> xllc;
  dtmf >> val;
  dtmf >> yllc;
  dtmf >> val;
  dtmf >> cell_size;
  dtmf.close ();
  x_min = (double) ((int) xllc); // to remove the shift found in asc files ...
  y_min = (double) ((int) yllc);
std::cout << "TW = " << twidth << " et TH = " << theight << std::endl;
std::cout << "xmin = " << x_min << " et ymin = " << y_min << std::endl;
std::cout << "CS = " << cell_size << std::endl;
  iwidth = twidth;
  iheight = theight;
  return true;
}


bool TerrainMap::loadNormalMapInfo (const std::string &name)
{
  std::ifstream nvmf (name.c_str (), std::ios::in | std::ifstream::binary);
  if (! nvmf)
  {
    std::cout << "File " << name << " can't be opened" << std::endl;
    return false;
  }
  float x, y;
  nvmf.read ((char *) (&twidth), sizeof (int));
  nvmf.read ((char *) (&theight), sizeof (int));
  nvmf.read ((char *) (&cell_size), sizeof (float));
  nvmf.read ((char *) (&x), sizeof (float));
  nvmf.read ((char *) (&y), sizeof (float));
  nvmf.close ();
  x_min = (double) x;
  y_min = (double) y;
  iwidth = twidth;
  iheight = theight;
  return true;
}


int TerrainMap::get (int i, int j) const
{
  if (shading == SHADE_HILL)
  {
    float val1 = light_v1.scalar (nmap[j * iwidth + i]);
    if (val1 < 0.0f) val1 = 0.;
    float val2 = light_v2.scalar (nmap[j * iwidth + i]);
    if (val2 < 0.0f) val2 = 0.;
    float val3 = light_v3.scalar (nmap[j * iwidth + i]);
    if (val3 < 0.0f) val3 = 0.;
    float val = val1 + (val2 + val3) / 2;
    return (int) (val * 100);
  }
  else
  {
    Pt3f *pt = nmap + j * iwidth + i;
    return 255 - (int) (sqrt (pt->x () * pt->x () + pt->y () * pt->y ()) * 255);
  }
}


int TerrainMap::get (int i, int j, int shading_type) const
{
  if (shading_type == SHADE_HILL)
  {
    float val1 = light_v1.scalar (nmap[j * iwidth + i]);
    if (val1 < 0.0f) val1 = 0.;
    float val2 = light_v2.scalar (nmap[j * iwidth + i]);
    if (val2 < 0.0f) val2 = 0.;
    float val3 = light_v3.scalar (nmap[j * iwidth + i]);
    if (val3 < 0.0f) val3 = 0.;
    float val = val1 + (val2 + val3) / 2;
    return (int) (val * 100);
  }
  else
  {
    Pt3f *pt = nmap + j * iwidth + i;
    return 255 - (int) (sqrt (pt->x () * pt->x () + pt->y () * pt->y ()) * 255);
  }
}


void TerrainMap::saveFirstNormalMap (const std::string &name) const
{
  std::ofstream nvmf (name.c_str (), std::ios::out | std::ofstream::binary);
  if (! nvmf) std::cout << "File " << name << " can't be created" << std::endl;
  else
  {
    nvmf.write ((char *) (&twidth), sizeof (int));
    nvmf.write ((char *) (&theight), sizeof (int));
    nvmf.write ((char *) (&cell_size), sizeof (float));
    float fxm = (float) fx_min;
    nvmf.write ((char *) (&fxm), sizeof (float));
    float fym = (float) fy_min;
    nvmf.write ((char *) (&fym), sizeof (float));
    Pt2i txy = layout.front ();
    Pt3f *line = nmap + iwidth * (iheight - 1);
    line -= txy.y () * theight * iwidth;
    line += txy.x () * twidth;
    for (int j = 0; j < theight; j++)
    {
      nvmf.write ((char *) line, twidth * sizeof (Pt3f));
      line -= iwidth;
    }
    nvmf.close ();
  }
}


void TerrainMap::incLightAngle (int val)
{
  light_angle += LIGHT_ANGLE_INCREMENT * val;
  if (light_angle < 0.0f) light_angle += ASF_2PI;
  else if (light_angle >= ASF_2PI) light_angle -= ASF_2PI;

  float ang = light_angle;
  light_v1.set (- (float) (cos (ang) * ASF_SQRT2_2),
                - (float) (sin (ang) * ASF_SQRT2_2), ASF_SQRT2_2);
  ang += ASF_2PI_3;
  light_v2.set (- (float) (cos (ang) / 2),
                - (float) (sin (ang) / 2), ASF_SQRT3_2);
  ang += ASF_2PI_3;
  light_v3.set (- (float) (cos (ang) / 2),
                - (float) (sin (ang) / 2), ASF_SQRT3_2);
}


void TerrainMap::setLightAngle (float val)
{
  light_angle = val;
  if (light_angle < 0.0f) light_angle += ASF_2PI;
  else if (light_angle >= ASF_2PI) light_angle -= ASF_2PI;

  float ang = light_angle;
  light_v1.set (- (float) (cos (ang) * ASF_SQRT2_2),
                - (float) (sin (ang) * ASF_SQRT2_2), ASF_SQRT2_2);
  ang += ASF_2PI_3;
  light_v2.set (- (float) (cos (ang) / 2),
                - (float) (sin (ang) / 2), ASF_SQRT3_2);
  ang += ASF_2PI_3;
  light_v3.set (- (float) (cos (ang) / 2),
                - (float) (sin (ang) / 2), ASF_SQRT3_2);
}


void TerrainMap::toggleShadingType ()
{
  if (++shading > SHADE_SLOPE) shading = SHADE_HILL;
}


void TerrainMap::setPadSize (int val)
{
  if (val >= 0 && val % 2 == 1)
  {
    pad_size = val;
    pad_w = pad_size;
    pad_h = pad_size;
  }
}


void TerrainMap::adjustPadSize ()
{
  if (pad_w > ts_cot) pad_w = ts_cot;
  if (pad_h > ts_rot) pad_h = ts_rot;
}


int TerrainMap::nextPad (unsigned char *map)
{
  if (pad_ref == -1)
  {
    pad_ref = 0;
    if (nmap != NULL) delete [] nmap;
    nmap = new Pt3f[twidth];
    for (int j = 0; j < pad_h; j ++)
      for (int i = 0; i < pad_w; i ++)
        loadMap (j * ts_cot + i,
                 map + ((pad_h - j) * theight - 1) * (pad_w * twidth)
                     + i * twidth);
  }
  else if (((pad_ref / ts_cot) / (pad_h - 2)) % 2 == 1)
  {
    if (pad_ref % ts_cot == 0)
    {
      if (pad_ref + ts_cot * pad_h >= ts_cot * ts_rot)
      {
        // getting out
        pad_ref = -1;
        if (nmap != NULL) delete [] nmap;
        nmap = NULL;
      }
      else
      {
        // climbing up on left side to next row
        pad_ref += ts_cot * (pad_h - 2);
        int pad_eh = pad_h;
        if (pad_ref / ts_cot + pad_h > ts_rot)
          pad_eh -= pad_ref / ts_cot + pad_h - ts_rot;
        unsigned char *fmap = map + 2 * theight * pad_w * twidth;
        unsigned char *tmap = map + pad_h * theight * pad_w * twidth;
        for (int j = 0; j < 2 * theight; j ++)
          for (int i = 0; i < pad_w * twidth; i ++) *--tmap = *--fmap;
        for (int j = 2; j < pad_eh; j ++)
          for (int i = 0; i < pad_w; i ++)
            loadMap ((pad_ref / ts_cot + j) * ts_cot + pad_ref % ts_cot + i,
                     map + ((pad_h - j) * theight - 1) * (pad_w * twidth)
                         + i * twidth);
        for (int j = pad_eh; j < pad_h; j ++)
          for (int i = 0; i < pad_w; i ++)
            clearMap (map + ((pad_h - j) * theight - 1) * (pad_w * twidth)
                         + i * twidth, pad_w, twidth, theight);
      }
    }
    else
    {
      // going left to next column
      pad_ref -= pad_w - 2;
      int pad_eh = pad_h;
      if (pad_ref / ts_cot + pad_h > ts_rot)
        pad_eh -= pad_ref / ts_cot + pad_h - ts_rot;
      unsigned char *fmap = map + (pad_h - pad_eh) * theight * pad_w * twidth
                                + 2 * twidth;
      unsigned char *tmap = fmap + (pad_w - 2) * twidth;
      for (int j = 0; j < pad_eh * theight; j ++)
      {
        for (int i = 0; i < 2 * twidth; i ++) *--tmap = *--fmap;
        fmap += (pad_w + 2) * twidth;
        tmap += (pad_w + 2) * twidth;
      }
      for (int j = 0; j < pad_eh; j ++)
        for (int i = 0; i < pad_w - 2; i ++)
          loadMap ((pad_ref / ts_cot + j) * ts_cot + pad_ref % ts_cot + i,
                   map + ((pad_h - j) * theight - 1) * (pad_w * twidth)
                       + i * twidth);
    }
  }
  else
  {
    if ((pad_ref % ts_cot) + pad_w >= ts_cot)
    {
      if (pad_ref + ts_cot * pad_h >= ts_cot * ts_rot)
      {
        // getting out
        pad_ref = -1;
        if (nmap != NULL) delete [] nmap;
        nmap = NULL;
      }
      else
      {
        // climbing up on right side to next row
        pad_ref += ts_cot * (pad_h - 2);
        int pad_ew = pad_w;
        if (pad_ref % ts_cot + pad_w > ts_cot)
          pad_ew -= pad_ref % ts_cot + pad_w - ts_cot;
        int pad_eh = pad_h;
        if (pad_ref / ts_cot + pad_h > ts_rot)
          pad_eh -= pad_ref / ts_cot + pad_h - ts_rot;
        unsigned char *fmap = map + 2 * theight * pad_w * twidth;
        unsigned char *tmap = map + pad_h * theight * pad_w * twidth;
        for (int j = 0; j < 2 * theight; j ++)
        {
          fmap -= (pad_w - pad_ew) * twidth;
          tmap -= (pad_w - pad_ew) * twidth;
          for (int i = 0; i < pad_ew * twidth; i ++) *--tmap = *--fmap;
        }
        for (int j = 2; j < pad_eh; j ++)
          for (int i = 0; i < pad_ew; i ++)
            loadMap ((pad_ref / ts_cot + j) * ts_cot + pad_ref % ts_cot + i,
                     map + ((pad_h - j) * theight - 1) * (pad_w * twidth)
                         + i * twidth);
        for (int j = pad_eh; j < pad_h; j ++)
          for (int i = 0; i < pad_ew; i ++)
            clearMap (map + ((pad_h - j) * theight - 1) * (pad_w * twidth)
                         + i * twidth, pad_w, twidth, theight);
      }
    }
    else
    {
      // going right to next column
      pad_ref += pad_w - 2;
      int pad_ew = pad_w;
      if (pad_ref % ts_cot + pad_w > ts_cot)
        pad_ew -= pad_ref % ts_cot + pad_w - ts_cot;
      int pad_eh = pad_h;
      if (pad_ref / ts_cot + pad_h > ts_rot)
        pad_eh -= pad_ref / ts_cot + pad_h - ts_rot;
      unsigned char *tmap = map + (pad_h - pad_eh) * theight * pad_w * twidth;
      unsigned char *fmap = tmap + (pad_w - 2) * twidth;
      for (int j = 0; j < pad_eh * theight; j ++)
      {
        for (int i = 0; i < 2 * twidth; i ++) *tmap++ = *fmap++;
        fmap += (pad_w - 2) * twidth;
        tmap += (pad_w - 2) * twidth;
      }
      for (int j = 0; j < pad_eh; j ++)
      {
        for (int i = 2; i < pad_ew; i ++)
        {
          loadMap ((pad_ref / ts_cot + j) * ts_cot + pad_ref % ts_cot + i,
                   map + ((pad_h - j) * theight - 1) * (pad_w * twidth)
                       + i * twidth);
        }
        for (int i = pad_ew; i < pad_w; i ++)
        {
          clearMap (map + ((pad_h - j) * theight - 1) * (pad_w * twidth)
                        + i * twidth, pad_w, twidth, theight);
        }
      }
    }
  }
  return pad_ref;
}


bool TerrainMap::loadMap (int k, unsigned char *submap)
{
//  std::cout << "MTILE " << k << " : "
//       << (arr_files[k] == NULL ? "NULL" : *arr_files[k]) << std::endl;
  int locw = 0, loch = 0;
  float locs = 0.0f, locxmin = 0.0f, locymin = 0.0f;

  if (arr_files[k] != NULL)
  {
    std::ifstream nvmf (arr_files[k]->c_str (),
                        std::ios::in | std::ifstream::binary);
    if (! nvmf) std::cout << "File " << *arr_files[k] << " can't be opened"
                          << std::endl;
    nvmf.read ((char *) (&locw), sizeof (int));
    nvmf.read ((char *) (&loch), sizeof (int));
    nvmf.read ((char *) (&locs), sizeof (float));
    nvmf.read ((char *) (&locxmin), sizeof (float));
    nvmf.read ((char *) (&locymin), sizeof (float));
    if (locw != twidth)
    {
      std::cout << "File " << *arr_files[k] << " inconsistent width"
                << std::endl;
      nvmf.close ();
      return false;
    }
    if (loch != theight)
    {
      std::cout << "File " << *arr_files[k] << " inconsistent height"
                << std::endl;
      nvmf.close ();
      return false;
    }
    if (locs != cell_size)
    {
      std::cout << "File " << *arr_files[k] << " inconsistent cell size"
                << std::endl;
      nvmf.close ();
      return false;
    }

    unsigned char *pmap = submap;
    for (int j = 0; j < theight; j++)
    {
      nvmf.read ((char *) nmap, twidth * sizeof (Pt3f));
      //traite la ligne nmap pour construire submap
      for (int i = 0; i < twidth; i ++)
      {
        int val = 255 - (int) (sqrt (nmap[i].x () * nmap[i].x ()
                                     + nmap[i].y () * nmap[i].y ()) * 255);
        if (val < 0) val = 0;
        if (val > 255) val = 255;
        *pmap++ = val;
      }
      pmap -= (pad_w + 1) * twidth;
    }
    nvmf.close ();
  }
  else
  {
    unsigned char *pmap = submap;
    for (int j = 0; j < theight; j++)
    {
      for (int i = 0; i < twidth; i ++) *pmap ++ = 0;
      pmap -= (pad_w + 1) * twidth;
    }
  }
  return true;
}


void TerrainMap::clearMap (unsigned char *submap, int pw, int w, int h)
{
  for (int j = 0; j < h; j++)
  {
    for (int i = 0; i < w; i++) *submap++ = 0;
    submap -= (pw + 1) * w;
  }
}


void TerrainMap::checkArrangement ()
{
  for (int i = 0; i < (iheight / theight) * (iwidth / twidth); i++)
    std::cout << "DTM TILE " << i << " : "
         << (arr_files[i] == NULL ? "NULL" : *arr_files[i]) << std::endl;
}


void TerrainMap::saveSubMap (int imin, int jmin, int imax, int jmax) const
{
  int nw = imax - imin, nh = jmax - jmin;
  float xm = (float) (x_min + (double) imin * 0.5);
  float ym = (float) (y_min + (double) jmin * 0.5);

  std::ofstream nvmf ("nvm/newtile.nvm", std::ios::out | std::ofstream::binary);
  if (! nvmf) std::cout << "nvm/newtile.nvm can't be created" << std::endl;
  else
  {
    nvmf.write ((char *) (&nw), sizeof (int));
    nvmf.write ((char *) (&nh), sizeof (int));
    nvmf.write ((char *) (&cell_size), sizeof (float));
    nvmf.write ((char *) (&xm), sizeof (float));
    nvmf.write ((char *) (&ym), sizeof (float));
    Pt3f *line = nmap + iwidth * (iheight - 1);
    line -= jmin * iwidth;
    line += imin;
    for (int j = 0; j < nh; j++)
    {
      nvmf.write ((char *) line, nw * sizeof (Pt3f));
      line -= iwidth;
    }
    nvmf.close ();
  }
}
