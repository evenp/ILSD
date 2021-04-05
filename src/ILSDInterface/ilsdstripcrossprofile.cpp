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
#include "ilsdstripcrossprofile.h"
#include "directionalscanner.h"
#include "asImage.h"
#include "math.h"
#include "asPainter.h"


ILSDStripCrossProfile::ILSDStripCrossProfile (int subdiv,
                                              ILSDItemControl *item_ctrl)
                      : ILSDCrossProfileItem (item_ctrl)
{
  this->subdiv = subdiv;
  href0 = 0.0f;
  hrefc = 0.0f;
}


std::string ILSDStripCrossProfile::profileName () const
{
  return ("cprofile");
}


void ILSDStripCrossProfile::setData (ASImage* image, IPtTileSet* pdata)
{
  ILSDCrossProfileItem::setData (image, pdata);
  scanp.setSize (subdiv * imageWidth, subdiv * imageHeight);
  scani.setSize (imageWidth, imageHeight);
}


void ILSDStripCrossProfile::buildScans (Pt2i pt1, Pt2i pt2)
{
  // Checks drawability
  ILSDCrossProfileItem::buildScans (pt1, pt2);
  if (! drawable) return;

  // Resets the profiles
  rightscan.clear ();
  leftscan.clear ();
  rightiscan.clear ();
  leftiscan.clear ();
  ctrl->resetScan ();

  // Gets a point cloud scan iterator
  Vr2i scandir = p1.vectorTo(p2).orthog ();
  if ((p1.x () > p2.x () && p1.y () > p2.y ())
      || (p1.x () < p2.x () && p1.y () < p2.y ())) scandir.invert ();
  DirectionalScanner* ds = scanp.getScanner (
    Pt2i (((p1.x () + p2.x ()) / 2) * subdiv + subdiv / 2,
          ((p1.y () + p2.y ()) / 2) * subdiv + subdiv / 2),
    scandir, ctrl->straightStripWidth () * subdiv);
  reversed = scanp.isLastScanReversed ();

  // Extracts central scan
  std::vector<Pt2i> pix;
  if (ds->first(pix) < MIN_SCAN) { drawable = false; delete ds; return; }
  leftscan.push_back (pix);
  bool scanOn = true;
  while (scanOn)
  {
    std::vector<Pt2i> scan;
    if (ds->nextOnLeft (scan) < MIN_SCAN) scanOn = false;
    else
      if (reversed) rightscan.push_back (scan);
      else leftscan.push_back (scan);
  }

  // Extracts right scans
  scanOn = true;
  while (scanOn)
  {
    std::vector<Pt2i> scan;
    if (ds->nextOnRight (scan) < MIN_SCAN) scanOn = false;
    else
      if (reversed) leftscan.push_back (scan);
      else rightscan.push_back (scan);
  }
  delete ds;

  // Gets a image scan iterator
  ds = scanp.getScanner (
    Pt2i ((p1.x () + p2.x ()) / 2, (p1.y () + p2.y ()) / 2),
    p1.vectorTo(p2).orthog (), ctrl->straightStripWidth ());

  // Extracts central and left scans
  std::vector<Pt2i> pix2;
  if (ds->first (pix2) < MIN_SCAN) { drawable = false; delete ds; return; }
  leftiscan.push_back (pix2);
  int minscan = 0;
  int maxscan = 1;
  scanOn = true;
  while (scanOn)
  {
    std::vector<Pt2i> scan;
    if (ds->nextOnLeft (scan) < MIN_SCAN) scanOn = false;
    else
    {
      if (reversed)
      {
        rightiscan.push_back (scan);
        minscan --;
      }
      else
      {
        leftiscan.push_back (scan);
        maxscan ++;
      }
    }
  }

  // Extracts right scans
  scanOn = true;
  while (scanOn)
  {
    std::vector<Pt2i> scan;
    if (ds->nextOnRight (scan) < MIN_SCAN) scanOn = false;
    else
    {
      if (reversed)
      {
        leftiscan.push_back (scan);
        maxscan ++;
      }
      else
      {
        rightiscan.push_back (scan);
        minscan --;
      }
    }
  }
  delete ds;
  ctrl->setMinScan (minscan);
  ctrl->setMaxScan (maxscan);

  // Updates local position reference
  double lsz = ctrl->straightStripWidth () /
               (2 * sqrt (pt1.vectorTo(pt2).norm2 ()));
  np1.set ((int) (pt1.x () - lsz * (pt1.y () - pt2.y ()) + 0.5),
           (int) (pt1.y () - lsz * (pt2.x () - pt1.x ()) + 0.5));

  // Sets absolute and local height references
  updateProfile ();
  href0 = hrefc;
}


void ILSDStripCrossProfile::rebuildScans ()
{
  int cur_scan = ctrl->scan ();
  buildScans (p1, p2);
  if (cur_scan >= ctrl->minScan () && cur_scan <= ctrl->maxScan ())
  {
    ctrl->setScan (cur_scan);
    updateProfile ();
  }
}


bool ILSDStripCrossProfile::isStripAdaptable () const
{
  return true;
}


std::vector<Pt2i> *ILSDStripCrossProfile::getCurrentScan ()
{
  return (leftiscan.empty () ? NULL :  getDisplayScan (ctrl->scan ()));
}


void ILSDStripCrossProfile::updateProfile ()
{
  if (drawable)
  {
    current_points.clear ();
    float minz = 0.0f, maxz = 0.0f;
    float scanx = (float) (p1.y () - p2.y ());
    float scany = (float) (p2.x () - p1.x ());
    float scanl = (float) sqrt (scanx * scanx + scany * scany);
    scanx /= scanl;
    scany /= scanl;

    bool initialized = false;
    int lastscan = (ctrl->scan () + 1) * subdiv - subdiv / 2;
    for (int curscan = lastscan - subdiv;
         curscan < lastscan; curscan ++)
    {
      std::vector<Pt2i> scan;
      if (curscan >= 0) scan = leftscan.at (curscan);
      else scan = rightscan.at (- curscan - 1);

      std::vector<Pt2i>::iterator it = scan.begin ();
      while (it != scan.end ())
      {
        std::vector<Pt3f> pts;
        ptset->collectPoints (pts, it->x (), it->y ());
        std::vector<Pt3f>::iterator pit = pts.begin ();
        while (pit != pts.end ())
        {
          if (initialized)
          {
            if (pit->z () < minz) minz = pit->z ();
            if (pit->z () > maxz) maxz = pit->z ();
          }
          else
          {
            minz = pit->z ();
            maxz = pit->z ();
            initialized = true;
          }
          double vx = pit->x () * iratio - np1.x () - 0.5;
          double vy = pit->y () * iratio - np1.y () - 0.5;
          current_points.push_back (
            Pt2f ((float) ((vx * scanx + vy * scany) / iratio), pit->z ()));
          pit ++;
        }
        it ++;
      }
    }
    hrefc = (minz + maxz) / 2;
  }
}


std::vector<Pt2i> *ILSDStripCrossProfile::getDisplayScan (int num)
{	
  if (num < 0) return (&(rightiscan.at (- num - 1)));
  else return (&(leftiscan.at (num)));
}


std::vector<Pt2f> *ILSDStripCrossProfile::getProfile (int num)
{
  return &current_points;
}


void ILSDStripCrossProfile::paint (GLWindow* parentWindow)
{
  if (drawable)
  {
    if (!structImage || size () != structImage->getImageResolution ())
    {
      if (structImage) delete structImage;
      structImage = new ASImage (size());
      update ();
    }
  }
  structImage->Draw (parentWindow);
  if (drawable) paintInfo ();
}


void ILSDStripCrossProfile::updateDrawing ()
{
  // Sets display parameters
  float scanx = (float) (p2.x () - p1.x ());
  float scany = (float) (p2.y () - p1.y ());
  p12.set (scanx, scany);
  l12 = (float) sqrt (scanx * scanx + scany * scany);
  if (scanx < 0) scanx = - scanx;
  if (scany < 0) scany = - scany;
  d12 = (scany > scanx ? scany : scanx) / l12;
  sratio = (alti_area_width - 2 * alti_area_margin) * iratio
           / (float) ctrl->straightStripWidth ();

  // Sets scan range
  ctrl->setMinScan (- 1 - (int) (rightiscan.size ()));
  ctrl->setMaxScan ((int) (leftiscan.size ()));

  // Sets height reference
  href = (ctrl->isStaticHeight () ? href0 : hrefc);
}


void ILSDStripCrossProfile::paintScans ()
{
  paintAlignedScans ();
}


void ILSDStripCrossProfile::paintStatus ()
{
}


void ILSDStripCrossProfile::update ()
{
  structImage->clear (ASColor::WHITE);
  if (drawable)
  {
    updateDrawing ();
    if (ctrl->isAligned ()) paintAlignedScans ();
    else paintScans ();
    paintProfile ();
  }
}


void ILSDStripCrossProfile::save (std::string fname)
{
  structImage->save (fname.c_str ());
}
