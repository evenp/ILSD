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

#include <cstdlib>
#include <iostream>
#include "ilsdstriplongprofile.h"
#include "directionalscanner.h"
#include "asImage.h"
#include "math.h"
#include "asPainter.h"


ILSDStripLongProfile::ILSDStripLongProfile (int subdiv,
                                            ILSDItemControl *item_ctrl)
                     : ILSDLongProfileItem (item_ctrl)
{
  this->subdiv = subdiv;
  href0 = 0.0f;
  hrefc = 0.0f;
}


std::string ILSDStripLongProfile::profileName () const
{
  return ("lprofile");
}


void ILSDStripLongProfile::setData (ASImage* image, IPtTileSet* pdata)
{
  ILSDLongProfileItem::setData (image, pdata);
  scanp.setSize (subdiv * imageWidth, subdiv * imageHeight);
}


void ILSDStripLongProfile::buildProfile (Pt2i pt1, Pt2i pt2)
{
  // Checks drawability
  ILSDLongProfileItem::buildProfile (pt1, pt2);
  profile.clear ();
  index.clear ();
  profile_length = 1.0f;
  if (! drawable) return;

  int scanx2 = pt2.x () - pt1.x ();
  scanx2 *= scanx2;
  int scany2 = pt2.y () - pt1.y ();
  scany2 *= scany2;
  float dist = (float) (sqrt (scanx2 + scany2) / iratio);
  reversed = (pt1.x () > pt2.x ());

  if (ctrl->isThinLongStrip ())
  {
    Pt2i spt (pt1.x () * subdiv + subdiv / 2, pt1.y () * subdiv + subdiv / 2);
    Pt2i ept (pt2.x () * subdiv + subdiv / 2, pt2.y () * subdiv + subdiv / 2);
    std::vector<Pt2i> scan;
    spt.draw (scan, ept);
    dist /= (int) (scan.size ());
    int pos = 0;
    bool heightToFix = true;
    if (reversed)
    {
      std::vector<Pt2i>::iterator it = scan.end ();
      while (it != scan.begin ())
      {
        it --;
        std::vector<Pt3f> pts;
        ptset->collectPoints (pts, it->x (), it->y ());
        if (! pts.empty ())
        {
          float hm = 0.0f;
          std::vector<Pt3f>::iterator pit = pts.begin ();
          while (pit != pts.end ())
          {
            hm += pit->z ();
            pit ++;
          }
          hm /= (int) (pts.size ());   // average height
          if (heightToFix)
          {
            zmin = hm;
            zmax = hm;
            heightToFix = false;
          }
          else
          {
            if (hm < zmin) zmin = hm;
            else if (hm > zmax) zmax = hm;
          }
          profile.push_back (Pt2f (pos * dist, hm));
          index.push_back (pos - ((int) (scan.size ())) / 2);
        }
        pos ++;
      }
    }
    else
    {
      std::vector<Pt2i>::iterator it = scan.begin ();
      while (it != scan.end ())
      {
        std::vector<Pt3f> pts;
        ptset->collectPoints (pts, it->x (), it->y ());
        if (! pts.empty ())
        {
          float hm = 0.0f;
          std::vector<Pt3f>::iterator pit = pts.begin ();
          while (pit != pts.end ())
          {
            hm += pit->z ();
            pit ++;
          }
          hm /= (int) (pts.size ());   // average height
          if (heightToFix)
          {
            zmin = hm;
            zmax = hm;
            heightToFix = false;
          }
          else
          {
            if (hm < zmin) zmin = hm;
            else if (hm > zmax) zmax = hm;
          }
          profile.push_back (Pt2f (pos * dist, hm));
          index.push_back (pos - ((int) (scan.size ())) / 2);
        }
        pos ++;
        it ++;
      }
    }
    profile_length = pos * dist;
  }
  else
  {
    Pt2i spt (pt1.x (), pt1.y ());
    Pt2i ept (pt2.x (), pt2.y ());
    std::vector<Pt2i> scan;
    spt.draw (scan, ept);
    dist /= (int) (scan.size ());
    int pos = 0;
    bool heightToFix = true;
    if (reversed)
    {
      std::vector<Pt2i>::iterator it = scan.end ();
      while (it != scan.begin ())
      {
        it --;
        std::vector<Pt3f> pts;
        for (int j = - subdiv / 2; j <= subdiv / 2; j ++)
          for (int i = - subdiv / 2; i <= subdiv / 2; i ++)
            ptset->collectPoints (pts, subdiv * it->x () + i,
                                       subdiv * it->y () + j);
        if (! pts.empty ())
        {
          float hm = 0.0f;
          std::vector<Pt3f>::iterator pit = pts.begin ();
          while (pit != pts.end ())
          {
            hm += pit->z ();
            pit ++;
          }
          hm /= (int) (pts.size ());   // average height
          if (heightToFix)
          {
            zmin = hm;
            zmax = hm;
            heightToFix = false;
          }
          else
          {
            if (hm < zmin) zmin = hm;
            else if (hm > zmax) zmax = hm;
          }
          profile.push_back (Pt2f (pos * dist, hm));
          index.push_back (pos - ((int) (scan.size ())) / 2);
        }
        pos ++;
      }
    }
    else
    {
      std::vector<Pt2i>::iterator it = scan.begin ();
      while (it != scan.end ())
      {
        std::vector<Pt3f> pts;
        for (int j = - subdiv / 2; j <= subdiv / 2; j ++)
          for (int i = - subdiv / 2; i <= subdiv / 2; i ++)
            ptset->collectPoints (pts, subdiv * it->x () + i,
                                       subdiv * it->y () + j);
        if (! pts.empty ())
        {
          float hm = 0.0f;
          std::vector<Pt3f>::iterator pit = pts.begin ();
          while (pit != pts.end ())
          {
            hm += pit->z ();
            pit ++;
          }
          hm /= (int) (pts.size ());   // average height
          if (heightToFix)
          {
            zmin = hm;
            zmax = hm;
            heightToFix = false;
          }
          else
          {
            if (hm < zmin) zmin = hm;
            else if (hm > zmax) zmax = hm;
          }
          profile.push_back (Pt2f (pos * dist, hm));
          index.push_back (pos - ((int) (scan.size ())) / 2);
        }
        pos ++;
        it ++;
      }
    }
    profile_length = pos * dist;
  }
  if (profile.empty ()) drawable = false;
  else setScale ();
}


void ILSDStripLongProfile::paint (GLWindow* parentWindow)
{
  if (drawable)
  {
    if (! structImage || size () != structImage->getImageResolution ())
    {
      if (structImage) delete structImage;
      structImage = new ASImage (size ());
      update ();
    }
  }
  structImage->Draw (parentWindow);
  if (drawable) paintInfo ();
}


void ILSDStripLongProfile::updateDrawing ()
{
  // Sets display parameters
/*
  float scanx = (float) (p2.x () - p1.x ());
  float scany = (float) (p2.y () - p1.y ());
  p12.set (scanx, scany);
  l12 = (float) sqrt (scanx * scanx + scany * scany);
  if (scanx < 0) scanx = - scanx;
  if (scany < 0) scany = - scany;
  d12 = (scany > scanx ? scany : scanx) / l12;
  sratio = (alti_area_width - 2 * alti_area_margin) * iratio
           / (float) ctrl->straightStripWidth ();
*/

  // Sets height reference
//  href = (ctrl->isStaticHeight () ? href0 : hrefc);
}


void ILSDStripLongProfile::paintStatus ()
{
}


void ILSDStripLongProfile::update ()
{
  structImage->clear (ASColor::WHITE);
  if (drawable)
  {
    updateDrawing ();
    paintProfile ();
  }
}


void ILSDStripLongProfile::save (std::string fname)
{
  structImage->save (fname.c_str ());
}
