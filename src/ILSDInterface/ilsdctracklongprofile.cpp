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
#include "ilsdctracklongprofile.h"
#include "directionalscanner.h"
#include "asImage.h"
#include "imgui.h"
#include "math.h"
#include "asPainter.h"


ILSDCTrackLongProfile::ILSDCTrackLongProfile (CTrackDetector* detector,
                                              ILSDItemControl *item_ctrl)
                      : ILSDLongProfileItem (item_ctrl)
{
  det = detector;
}


std::string ILSDCTrackLongProfile::profileName () const
{
  return ("plateau");
}


void ILSDCTrackLongProfile::buildProfile (Pt2i pt1, Pt2i pt2)
{
  ILSDLongProfileItem::buildProfile (pt1, pt2);
  profile.clear ();
  index.clear ();
  profile_length = 1.0f;
  if (! drawable) return;

  CarriageTrack *ct = det->getCarriageTrack (ctrl->isInitialDetection ());
  if (ct != NULL && ct->plateau (0) != NULL && ct->plateau(0)->isAccepted ())
  {
    bool nopt = true;
    float length = 0.0f;
    Pt2f cen, oldcen;
    int bleft = ct->getLeftScanCount ();
    int bright = ct->getRightScanCount ();
    for (int i = - bright; i <= bleft; i++)
    {
      if (ct->plateau(i)->isAccepted ())
      {
        float h = ct->plateau(i)->getMinHeight ();
        cen.set (localize (i, ct->plateau(i)->estimatedCenter ()));
        if (nopt)
        {
          zmin = h;
          zmax = h;
          nopt = false;
        }
        else
        {
          if (h < zmin) zmin = h;
          else if (h > zmax) zmax = h;
          length += (float) sqrt (oldcen.vectorTo(cen).norm2 ());
        }
        profile.push_back (Pt2f (length, h));
        index.push_back (i);
        oldcen.set (cen);
      }
    }
    profile_length = length;
    setScale ();
  }
  else drawable = false;
}


void ILSDCTrackLongProfile::paint (GLWindow* parentWindow)
{
  if (! structImage || size () != structImage->getImageResolution ())
  {
    if (structImage) delete structImage;
    structImage = new ASImage (size ());
    update ();
  }
  structImage->Draw (parentWindow);

  if (drawable)
  {
    if (det->getCarriageTrack (ctrl->isInitialDetection ()) != NULL)
      paintInfo ();
    else paintStatus ();
  }
}


void ILSDCTrackLongProfile::updateDrawing ()
{
}


void ILSDCTrackLongProfile::update ()
{
  structImage->clear (ASColor::WHITE);
  if (drawable)
  {
    updateDrawing ();

    if (det->getCarriageTrack (ctrl->isInitialDetection ()) != NULL)
      paintProfile ();
  }
}


/*
void ILSDCTrackLongProfile::paintInfo ()
{
}
*/


void ILSDCTrackLongProfile::paintStatus ()
{
}


void ILSDCTrackLongProfile::save (std::string fname)
{
  structImage->save (fname.c_str ());
}


Pt2f ILSDCTrackLongProfile::localize (int num, float pos) const
{
  CarriageTrack *ct = det->getCarriageTrack (ctrl->isInitialDetection ());
  if (ct == NULL || ct->plateau (0) == NULL) return (Pt2f ());
  std::vector<Pt2i> *scan = ct->getDisplayScan (0);
  bool rev = ct->isScanReversed (0);
  Pt2i p1 (scan->front ());
  Pt2i p2 (scan->back ());
  float p1px = 0.f, p1py = 0.f;
  int p12x = p2.x () - p1.x ();
  int p12y = p2.y () - p1.y ();
  float l12 = (float) sqrt (p12x * p12x + p12y * p12y);
  float q = 0.0f;
  if (p12y > (p12x < 0 ? - p12x : p12x))
  {
    q = num * p12x / (iratio * l12);
    p1px = p12x * (pos + q) / l12 - num / iratio;
    p1py = p12y * (pos + q) / l12;
  }
  else
  {
    if (p12x < 0) num = - num;
    q = num * p12y / (iratio * l12);
    p1px = p12x * (pos - q) / l12;
    p1py = p12y * (pos - q) / l12 + num /iratio;
  }
  return (Pt2f (rev ? p2.x () - p1px : p1.x () + p1px,
                rev ? p2.y () - p1py : p1.y () + p1py));
}
