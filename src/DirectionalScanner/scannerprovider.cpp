/*  Copyright 2021 Philippe Even and Phuc Ngo,
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

#include "scannerprovider.h"
#include "directionalscannero2.h"
#include "directionalscannero7.h"
#include "directionalscannero1.h"
#include "directionalscannero8.h"
#include "vhscannero2.h"
#include "vhscannero7.h"
#include "vhscannero1.h"
#include "vhscannero8.h"


DirectionalScanner *ScannerProvider::getScanner (Pt2i p1, Pt2i p2,
                                                 bool adaptive)
{
  // Enforces P1 to be lower than P2
  // or to left of P2 in case of equality
  last_scan_reversed = (p1.y () > p2.y ())
                       || ((p1.y () == p2.y ()) && (p1.x () > p2.x ()));
  if (last_scan_reversed)
  {
    Pt2i tmp (p1);
    p1.set (p2);
    p2.set (tmp);
  }

  // Computes the steps position array
  int nbs = 0;
  bool *steps = p1.stepsTo (p2, &nbs);

  // Equation of the strip support lines : ax + by = c
  int a = p2.x () - p1.x ();
  int b = p2.y () - p1.y ();
  if (a < 0 || (a == 0 && b < 0)) // Enforces a >= 0, then b > 0
  {
    a = -a;
    b = -b;
  }
  int c2 = a * p2.x () + b * p2.y ();

  // Builds and returns the appropriate scanner
  if (b < 0)
    if (-b > a)
    {
      if (isOrtho)
      {
        int repx = (p1.x () + p2.x ()) / 2;    // central scan start
        int repy = p1.y () - (int) ((p1.x () - repx) * (p1.x () - p2.x ())
                                    / (p2.y () - p1.y ()));
        return (new VHScannerO1 (xmin, ymin, xmax, ymax,
                                 a, b, c2, nbs, steps, repx, repy));
      }
      else return (adaptive ?
                   (DirectionalScanner *)
                   new AdaptiveScannerO1 (xmin, ymin, xmax, ymax,
                          a, b, c2, nbs, steps, p1.x (), p1.y ()) :
                   new DirectionalScannerO1 (xmin, ymin, xmax, ymax,
                          a, b, c2, nbs, steps, p1.x (), p1.y ()));
    }
    else
    {
      if (isOrtho)
      {
        int repy = (p1.y () + p2.y ()) / 2;    // central scan start
        int repx = p1.x () + (int) ((repy - p1.y ()) * (p2.y () - p1.y ())
                                    / (p1.x () - p2.x ()));
        return (new VHScannerO2 (xmin, ymin, xmax, ymax,
                                 a, b, c2, nbs, steps, repx, repy));
      }
      else return (adaptive ?
                   (DirectionalScanner *)
                   new AdaptiveScannerO2 (xmin, ymin, xmax, ymax,
                          a, b, c2, nbs, steps, p1.x (), p1.y ()) :
                   new DirectionalScannerO2 (xmin, ymin, xmax, ymax,
                          a, b, c2, nbs, steps, p1.x (), p1.y ()));
    }
  else
    if (b > a)
    {
      if (isOrtho)
      {
        int repx = (p1.x () + p2.x ()) / 2;    // central scan start
        int repy = p1.y () - (int) ((repx - p1.x ()) * (p2.x () - p1.x ())
                                    / (p2.y () - p1.y ()));
        return (new VHScannerO8 (xmin, ymin, xmax, ymax,
                                 a, b, c2, nbs, steps, repx, repy));
      }
      else return (adaptive ?
                   (DirectionalScanner *)
                   new AdaptiveScannerO8 (xmin, ymin, xmax, ymax,
                          a, b, c2, nbs, steps, p1.x (), p1.y ()) :
                   new DirectionalScannerO8 (xmin, ymin, xmax, ymax,
                          a, b, c2, nbs, steps, p1.x (), p1.y ()));
    }
    else
    {
      if (isOrtho)
      {
        int repy = (p1.y () + p2.y ()) / 2;    // central scan start
        int repx = p1.x () - (int) ((repy - p1.y ()) * (p2.y () - p1.y ())
                                    / (p2.x () - p1.x ()));
        return (new VHScannerO7 (xmin, ymin, xmax, ymax,
                                 a, b, c2, nbs, steps, repx, repy));
      }
      else return (adaptive ?
                   (DirectionalScanner *)
                   new AdaptiveScannerO7 (xmin, ymin, xmax, ymax,
                          a, b, c2, nbs, steps, p1.x (), p1.y ()) :
                   new DirectionalScannerO7 (xmin, ymin, xmax, ymax,
                          a, b, c2, nbs, steps, p1.x (), p1.y ()));
    }
}


DirectionalScanner *ScannerProvider::getScanner (Pt2i centre, Vr2i normal,
                                                 int length, bool adaptive)
{
  // Gets the steps position array
  int nbs = 0;
  bool *steps = centre.stepsTo (Pt2i (centre.x () + normal.x (),
                                      centre.y () + normal.y ()), &nbs);

  // Orients rightwards
  int a = normal.x ();
  int b = normal.y ();  // as equation is (ax + by = c)
  last_scan_reversed = (b < 0 || (b == 0 && a < 0));
  if (a < 0 || (a == 0 && b < 0))
  {
    a = -a;
    b = -b;
  }

  // Builds and returns the appropriate scanner
  if (b < 0)
    if (-b > a)
      return (adaptive ?
              (isOrtho ?
               (DirectionalScanner *)
               new VHScannerO1 (xmin, ymin, xmax, ymax,
                                a, b, nbs, steps,
                                centre.x (), centre.y (), length) :
               (DirectionalScanner *)
               new AdaptiveScannerO1 (xmin, ymin, xmax, ymax,
                                      a, b, nbs, steps,
                                      centre.x (), centre.y (), length)) :
              (DirectionalScanner *)
              new DirectionalScannerO1 (xmin, ymin, xmax, ymax,
                                        a, b, nbs, steps,
                                        centre.x (), centre.y (), length));
    else
      return (adaptive ?
              (isOrtho ?
               (DirectionalScanner *)
               new VHScannerO2 (xmin, ymin, xmax, ymax,
                                a, b, nbs, steps,
                                centre.x (), centre.y (), length) :
               (DirectionalScanner *)
               new AdaptiveScannerO2 (xmin, ymin, xmax, ymax,
                                      a, b, nbs, steps,
                                      centre.x (), centre.y (), length)) :
              (DirectionalScanner *)
              new DirectionalScannerO2 (xmin, ymin, xmax, ymax,
                                        a, b, nbs, steps,
                                        centre.x (), centre.y (), length));
  else
    if (b > a)
      return (adaptive ?
              (isOrtho ?
               (DirectionalScanner *)
               new VHScannerO8 (xmin, ymin, xmax, ymax,
                                a, b, nbs, steps,
                                centre.x (), centre.y (), length) :
               (DirectionalScanner *)
               new AdaptiveScannerO8 (xmin, ymin, xmax, ymax,
                                      a, b, nbs, steps,
                                      centre.x (), centre.y (), length)) :
              (DirectionalScanner *)
              new DirectionalScannerO8 (xmin, ymin, xmax, ymax,
                                        a, b, nbs, steps,
                                        centre.x (), centre.y (), length));
    else
      return (adaptive ?
              (isOrtho ?
               (DirectionalScanner *)
               new VHScannerO7 (xmin, ymin, xmax, ymax,
                                a, b, nbs, steps,
                                centre.x (), centre.y (), length) :
               (DirectionalScanner *)
               new AdaptiveScannerO7 (xmin, ymin, xmax, ymax,
                                      a, b, nbs, steps,
                                      centre.x (), centre.y (), length)) :
              (DirectionalScanner *)
              new DirectionalScannerO7 (xmin, ymin, xmax, ymax,
                                        a, b, nbs, steps,
                                        centre.x (), centre.y (), length));
}


bool ScannerProvider::isReversed (const Vr2i &vec)
{
  return (vec.y () < 0 || (vec.y () == 0 && vec.x () < 0));
}
