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

#include "pt2i.h"


Pt2i::Pt2i ()
{
  xp = 0;
  yp = 0;
}


Pt2i::Pt2i (int x, int y)
{
  xp = x;
  yp = y;
}


Pt2i::Pt2i (const Pt2i &p)
{
  xp = p.xp;
  yp = p.yp;
}


bool Pt2i::inTriangle (Pt2i p1, Pt2i p2, Pt2i p3) const
{
  int x1 = p1.xp, y1 = p1.yp;
  int x2 = p2.xp, y2 = p2.yp;
  int x3 = p3.xp, y3 = p3.yp;

  // Checks bounding rectangle
  int xmin = x1, ymin = y1, xmax = x1, ymax = y1;
  if (x2 < xmin) xmin = x2;
  if (x3 < xmin) xmin = x3;
  if (xp < xmin) return false;
  if (x2 > xmax) xmax = x2;
  if (x3 > xmax) xmax = x3;
  if (xp > xmax) return false;
  if (y2 < ymin) ymin = y2;
  if (y3 < ymin) ymin = y3;
  if (yp < ymin) return false;
  if (y2 > ymax) ymax = y2;
  if (y3 > ymax) ymax = y3;
  if (yp > ymax) return false;

  // Case of aligned triangle vertices
  if ((x2 - x1) * (y3 - y1) == (x3 - x1) * (y2 - y1))
    return ((x2 - x1) * (yp - y1) == (xp - x1) * (y2 - y1));

  // Checks orientation to triangle edges
  int pv1 = (xp - x1) * (y2 - y1) - (yp - y1) * (x2 - x1);
  int pv2 = (xp - x2) * (y3 - y2) - (yp - y2) * (x3 - x2);
  int pv3 = (xp - x3) * (y1 - y3) - (yp - y3) * (x1 - x3);
  return ((pv1 >= 0 && pv2 >= 0 && pv3 >= 0)
          || (pv1 <= 0 && pv2 <= 0 && pv3 <= 0));
}


Vr2i Pt2i::vectorTo (Pt2i p) const
{
  return (Vr2i (p.xp - xp, p.yp - yp));
}


Pt2i *Pt2i::drawing (const Pt2i p, int *n) const
{
  int x1, y1, x2, y2;
  if (xp > p.xp)
  {
    x1 = p.xp;
    x2 = xp;
    y1 = p.yp;
    y2 = yp;
  }
  else
  {
    x1 = xp;
    x2 = p.xp;
    y1 = yp;
    y2 = p.yp;
  }
  int dx = x2 - x1;
  int dy = y2 - y1;
  int e, i = 0;
  Pt2i *pts;

  if (dy > 0)
  {
    // Octant 1
    if (dx >= dy)
    {
      *n = dx + 1;
      pts = new Pt2i[dx + 1];
      e = dx - 1; // middle point lies below the line
      dx *= 2;
      dy *= 2;
      while (x1 < x2)
      {
        pts[i++].set (x1, y1);
        x1 ++;
        e -= dy;
        if (e < 0)
        {
          y1 ++;
          e += dx;
        }
      }
      pts[i].set (x1, y1);
    }

    // Octant 2
    else
    {
      *n = dy + 1;
      pts = new Pt2i[dy + 1];
      e = dy; // middle point lies to the right of the line
      dx *= 2;
      dy *= 2;
      while (y1 < y2)
      {
        pts[i++].set (x1, y1);
        y1 ++;
        e -= dx;
        if (e < 0)
        {
          x1 ++;
          e += dy;
        }
      }
      pts[i].set (x1, y1);
    }
  }

  else
  {
    // Octant 8
    if (dx >= -dy)
    {
      *n = 1 + dx;
      pts = new Pt2i[dx + 1];
      e = dx - 1; // middle point lies below the line
      dx *= 2;
      dy *= 2;
      while (x1 < x2)
      {
        pts[i++].set (x1, y1);
        x1 ++;
        e += dy;
        if (e < 0)
        {
          y1 --;
          e += dx;
        }
      }
      pts[i].set (x1, y1);
    }

    // Octant 7
    else
    {
      *n = 1 - dy;
      pts = new Pt2i[1 - dy];
      e = - dy; // middle point lies to the left of the line
      dx *= 2;
      dy *= 2;
      while (y1 > y2)
      {
        pts[i++].set (x1, y1);
        y1 --;
        e -= dx;
        if (e < 0)
        {
          x1 ++;
          e -= dy;
        }
      }
      pts[i].set (x1, y1);
    }
  }
  return (pts);
}


Pt2i *Pt2i::clipLine (const Pt2i p, int left, int low, int right, int up,
                      int *n) const
{
  if (right < left) { int tmp = left; left = right; right = tmp; }
  if (up < low) { int tmp = low; low = up; up = tmp; }
  int x1, y1, x2, y2;
  if (xp > p.xp)
  {
    x1 = p.xp;
    x2 = xp;
    y1 = p.yp;
    y2 = yp;
  }
  else
  {
    x1 = xp;
    x2 = p.xp;
    y1 = yp;
    y2 = p.yp;
  }
  int dx = x2 - x1;
  int dy = y2 - y1;
  int e, i = 0;
  Pt2i *pts;

  if (dy > 0)
  {
    // Octant 1
    if (dx >= dy)
    {
      *n = dx + 1;
      pts = new Pt2i[dx + 1];
      if (x2 >= left && y2 >= low)
      {
        e = dx - 1; // middle point lies below the line
        dx *= 2;
        dy *= 2;
        while (x1 < x2 && x1 <= right && y1 <= up)
        {
          if (x1 >= left && y1 >= low) pts[i++].set (x1, y1);
          x1 ++;
          e -= dy;
          if (e < 0)
          {
            y1 ++;
            e += dx;
          }
        }
        if (x2 <= right && y2 <= up) pts[i++].set (x2, y2);
      }
    }

    // Octant 2
    else
    {
      *n = dy + 1;
      pts = new Pt2i[dy + 1];
      if (x2 >= left && y2 >= low)
      {
        e = dy; // middle point lies to the right of the line
        dx *= 2;
        dy *= 2;
        while (y1 < y2 && x1 <= right && y1 <= up)
        {
          if (x1 >= left && y1 >= low) pts[i++].set (x1, y1);
          y1 ++;
          e -= dx;
          if (e < 0)
          {
            x1 ++;
            e += dy;
          }
        }
        if (x2 <= right && y2 <= up) pts[i++].set (x2, y2);
      }
    }
  }

  else
  {
    // Octant 8
    if (dx >= -dy)
    {
      *n = 1 + dx;
      pts = new Pt2i[dx + 1];
      if (x2 >= left && y2 <= up)
      {
        e = dx - 1; // middle point lies below the line
        dx *= 2;
        dy *= 2;
        while (x1 < x2 && x1 <= right && y1 >= low)
        {
          if (x1 >= left && y1 <= up) pts[i++].set (x1, y1);
          x1 ++;
          e += dy;
          if (e < 0)
          {
            y1 --;
            e += dx;
          }
        }
        if (x2 <= right && y2 >= low) pts[i++].set (x2, y2);
      }
    }

    // Octant 7
    else
    {
      *n = 1 - dy;
      pts = new Pt2i[1 - dy];
      if (x2 >= left && y2 <= up)
      {
        e = - dy; // middle point lies to the left of the line
        dx *= 2;
        dy *= 2;
        while (y1 > y2 && x1 <= right && y1 >= low)
        {
          if (x1 >= left && y1 <= up) pts[i++].set (x1, y1);
          y1 --;
          e -= dx;
          if (e < 0)
          {
            x1 ++;
            e -= dy;
          }
        }
        if (x2 <= right && y2 >= low) pts[i].set (x2, y2);
      }
    }
  }
  *n = i;
  return (pts);
}


void Pt2i::draw (std::vector<Pt2i> &line, Pt2i p) const
{
  int x1, y1, x2, y2;
  if (xp > p.xp)
  {
    x1 = p.xp;
    x2 = xp;
    y1 = p.yp;
    y2 = yp;
  }
  else
  {
    x1 = xp;
    x2 = p.xp;
    y1 = yp;
    y2 = p.yp;
  }
  int dx = x2 - x1;
  int dy = y2 - y1;
  int e;

  if (dy > 0)
  {
    // Octant 1
    if (dx >= dy)
    {
      e = dx - 1; // middle point lies below the line
      dx *= 2;
      dy *= 2;
      while (x1 < x2)
      {
        line.push_back (Pt2i (x1, y1));
        x1 ++;
        e -= dy;
        if (e < 0)
        {
          y1 ++;
          e += dx;
        }
      }
      line.push_back (Pt2i (x1, y1));
    }

    // Octant 2
    else
    {
      e = dy; // middle point lies to the right of the line
      dx *= 2;
      dy *= 2;
      while (y1 < y2)
      {
        line.push_back (Pt2i (x1, y1));
        y1 ++;
        e -= dx;
        if (e < 0)
        {
          x1 ++;
          e += dy;
        }
      }
      line.push_back (Pt2i (x1, y1));
    }
  }

  else
  {
    // Octant 8
    if (dx >= -dy)
    {
      e = dx - 1; // middle point lies below the line
      dx *= 2;
      dy *= 2;
      while (x1 < x2)
      {
        line.push_back (Pt2i (x1, y1));
        x1 ++;
        e += dy;
        if (e < 0)
        {
          y1 --;
          e += dx;
        }
      }
      line.push_back (Pt2i (x1, y1));
    }

    // Octant 7
    else
    {
      e = - dy; // middle point lies to the left of the line
      dx *= 2;
      dy *= 2;
      while (y1 > y2)
      {
        line.push_back (Pt2i (x1, y1));
        y1 --;
        e -= dx;
        if (e < 0)
        {
          x1 ++;
          e -= dy;
        }
      }
      line.push_back (Pt2i (x1, y1));
    }
  }
}


Pt2i *Pt2i::pathTo (Pt2i p, int *n) const
{
  int x1, y1, x2, y2, delta;
  if (xp > p.xp)
  {
    x1 = p.xp;
    x2 = xp;
    y1 = p.yp;
    y2 = yp;
    delta = -1;
  }
  else
  {
    x1 = xp;
    x2 = p.xp;
    y1 = yp;
    y2 = p.yp;
    delta = 1;
  }
  int dx = x2 - x1;
  int dy = y2 - y1;
  int e, i = 0;
  Pt2i *pts;

  if (dy > 0)
  {
    // Octant 1
    if (dx >= dy)
    {
      *n = dx;
      pts = new Pt2i[dx];
      e = dx - 1; // middle point lies below the line
      if (delta < 0) e++; // ... above
      dx *= 2;
      dy *= 2;
      while (x1 < x2)
      {
        x1 ++;
        e -= dy;
        if (e < 0)
        {
          y1 ++;
          e += dx;
          pts[i++].set (delta, delta);
        }
        else pts[i++].set (delta, 0);
      }
    }

    // Octant 2
    else
    {
      *n = dy;
      pts = new Pt2i[dy];
      e = dy; // middle point lies to the right of the line
      if (delta < 0) e--; // ... to the left
      dx *= 2;
      dy *= 2;
      while (y1 < y2)
      {
        y1 ++;
        e -= dx;
        if (e < 0)
        {
          x1 ++;
          e += dy;
          pts[i++].set (delta, delta);
        }
        else pts[i++].set (0, delta);
      }
    }
  }

  else
  {
    // Octant 8
    if (dx >= -dy)
    {
      *n = dx;
      pts = new Pt2i[dx];
      e = dx - 1; // middle point lies below the line
      if (delta < 0) e++; // ... above
      dx *= 2;
      dy *= 2;
      while (x1 < x2)
      {
        x1 ++;
        e += dy;
        if (e < 0)
        {
          y1 --;
          e += dx;
          pts[i++].set (delta, -delta);
        }
        else pts[i++].set (delta, 0);
      }
    }
    // Octant 7
    else
    {
      *n = -dy;
      pts = new Pt2i[-dy];
      e = - dy; // middle point lies to the left of the line
      if (delta < 0) e--; // ... to the right
      dx *= 2;
      dy *= 2;
      while (y1 > y2)
      {
        y1 --;
        e -= dx;
        if (e < 0)
        {
          x1 ++;
          e -= dy;
          pts[i++].set (delta, -delta);
        }
        else pts[i++].set (0, -delta);
      }
    }
  }
  return (pts);
}


bool *Pt2i::stepsTo (Pt2i p, int *n) const
{
  bool negx = p.xp < xp;
  bool negy = p.yp < yp;
  int x2 = (xp > p.xp) ? xp - p.xp : p.xp - xp;
  int y2 = (yp > p.yp) ? yp - p.yp : p.yp - yp;
  int dx = x2, dy = y2, e = 0;
  if (y2 > x2)
  {
    dx = y2;
    dy = x2;
    x2 = y2;
    if (negx == negy) e++;
  }
  else if (negx != negy) e++;
  e += dx - 1;
  dx *= 2;
  dy *= 2;

  int x = 0;
  *n = x2;
  bool *paliers = new bool[x2];
  while (x < x2)
  {
    e -= dy;
    if (e < 0)
    {
      e += dx;
      paliers[x++] = true;
    }
    else paliers[x++] = false;
  }
  return (paliers); 
}


std::vector<Pt2i> Pt2i::drawOrtho (const Pt2i p2, int offset) const
{
  std::vector<Pt2i> pts;

  int x1 = xp;
  int y1 = yp;
  int x2 = p2.x ();
  int y2 = p2.y ();
  int dx = x2 - x1;
  int dy = y2 - y1;
  int e;

  int num = dx * dy;
  if (num < 0) num = - num;
  int den = dx * dx + dy * dy;
  int nabs = (offset < 0 ? -offset : offset);
  int steps = (offset * num) / den;
  if ((nabs * num) % den >= den / 2) steps += (offset < 0 ? -1 : 1);
  int floors;

  if (dx > 0 && dy > 0)   // Quadrant 1
  {
    if (dx >= dy) // Octant 1
    {
      e = dx - 1;
      dx = dx * 2;
      dy = dy * 2;
      if (offset < 0) floors = (steps * dy - e) / dx;
      else floors = (e - 1 + steps * dy) / dx;
      e += floors * dx - steps * dy;
      x1 += steps;
      y1 -= offset - floors;
      x2 += steps;
      // y2 -= offset - floors;   // useless
      while (x1 < x2)
      {
        pts.push_back (Pt2i (x1, y1));
        x1 ++;
        e = e - dy;
        if (e < 0)
        {
          y1 ++;
          e = e + dx;
        }
      }
      pts.push_back (Pt2i (x1, y1));
    }
    else if (dx < dy) // Octant 2
    {
      e = dy;
      dx = dx * 2;
      dy = dy * 2;
      if (offset < 0) floors = (1 - e + steps * dx) / dy;
      else floors = (e + steps * dx) / dy;
      e -= floors * dy - steps * dx;
      x1 += offset - floors;
      y1 -= steps;
      // x2 += offset - floors;  // useless
      y2 -= steps;  // useless
      while (y1 < y2)
      {
        pts.push_back (Pt2i (x1, y1));
        y1 ++;
        e -= dx;
        if (e < 0)
        {
          x1 ++;
          e += dy;
        }
      }
      pts.push_back (Pt2i (x1, y1));
    }
  }

  else if (dx > 0 && dy < 0) // Quadrant 4
  {
    if (dx >= -dy) // Octant 8
    {
      e = dx - 1;
      dx = dx * 2;
      dy = dy * 2;
      if (offset < 0) floors = (e - 1 + steps * dy) / dx;
      else floors = (steps * dy - e) / dx;   // < 0
      e += floors * dx - steps * dy;
      x1 -= steps;
      y1 -= offset + floors;
      x2 -= steps;
      // y2 -= offset + floors;  // useless
      while (x1 < x2)
      {
        pts.push_back (Pt2i (x1, y1));
        x1 ++;
        e += dy;
        if (e < 0)
        {
          y1 --;
          e += dx;
        }
      }
      pts.push_back (Pt2i (x1, y1));
    }
    else // Octant 7
    {
      e = dy;
      dx = dx * 2;
      dy = dy * 2;
      if (offset < 0) floors = - (e + steps * dx) / dy;
      else floors = (e + 1 - steps * dx) / dy;
      e += floors * dy + steps * dx;
      x1 -= offset - floors;
      y1 -= steps;
      // x2 -= offset - floors;  // useless
      y2 -= steps;
      while (y1 > y2)
      {
        pts.push_back (Pt2i (x1, y1));
        y1 --;
        e += dx;
        if (e > 0)
        {
          x1 ++;
          e += dy;
        }
      }
      pts.push_back (Pt2i (x1, y1));
    }
  }
  else if (dy == 0 && dx > 0)
    while (x1 <= x2) pts.push_back (Pt2i (x1++, y1 - offset));

  if (dx < 0 && dy > 0)    // Quadrant 2
  {
    if (-dx >= dy) // Octant 4
    {
      e = dx - 1;
      dx = dx * 2;
      dy = dy * 2;
      if (offset < 0) floors = - (steps * dy + e) / dx;
      else floors = (e + 1 - steps * dy) / dx;  // > 0
      e -= steps * dy + floors * dx;
      x1 += steps;
      y1 += offset - floors;
      x2 += steps;
      // y2 += offset - floors;   // useless
      while (x1 > x2)
      {
        pts.push_back (Pt2i (x1, y1));
        x1 --;
        e += dy;
        if (e >= 0)
        {
          y1 ++;
          e += dx;
        }
      }
      pts.push_back (Pt2i (x1, y1));
    }
    else if (-dx < dy) // Octant 3
    {
      e = dy;
      dx = dx * 2;
      dy = dy * 2;
      if (offset < 0) floors = (1 - e - steps * dx) / dy;
      else floors = (e - steps * dx) / dy;
      e += floors * dy + steps * dx;
      x1 += offset - floors;
      y1 += steps;
      // x2 += offset - floors;   // useless
      y2 += steps;
      while (y1 < y2)
      {
        pts.push_back (Pt2i (x1, y1));
        y1 ++;
        e += dx;
        if (e <= 0)
        {
          x1 --;
          e += dy;
        }
      }
      pts.push_back (Pt2i (x1, y1));
    }
  }

  else if (dx < 0 && dy < 0)    // Quadrant 3
  {
    if (dx <= dy) // Octant 5
    {
      e = dx - 1;
      dx = dx * 2;
      dy = dy * 2;
      if (offset < 0) floors = (steps * dy - e - 1) / dx;
      else floors = (steps * dy + e) / dx;
      e += floors * dx - steps * dy;
      x1 -= steps;
      y1 += offset - floors;
      x2 -= steps;
      // y2 += offset - floors;  // useless
      while (x1 > x2)
      {
        pts.push_back (Pt2i (x1, y1));
        x1 --;
        e -= dy;
        if (e >= 0)
        {
          y1 --;
          e += dx;
        }
      }
      pts.push_back (Pt2i (x1, y1));
    }
    else // Octant 6
    {
      e = dy;
      dx = dx * 2;
      dy = dy * 2;
      if (offset < 0) floors = (e - steps * dx) / dy;  // > 0
      else floors = - (steps * dx + e + 1) / dy;
      e += floors * dy + steps * dx;
      x1 -= offset + floors;
      y1 += steps;
      y2 += steps;
     // x2 -= offset - floors;  // useless
      while (y1 > y2)
      {
        pts.push_back (Pt2i (x1, y1));
        y1 --;
        e -= dx;
        if (e >= 0)
        {
          x1 --;
          e += dy;
        }
      }
      pts.push_back (Pt2i (x1, y1));
    }
  }
  else if (dy == 0 && dx < 0)
    while (x1 >= x2) pts.push_back (Pt2i (x1--, y1 + offset));
  else if (dx == 0 && dy > 0)
    while (y1 <= y2) pts.push_back (Pt2i (x1 + offset, y1++));
  else if (dx == 0 && dy < 0)
    while(y1 >= y2) pts.push_back (Pt2i (x1 - offset, y1--));
  return pts;
}
