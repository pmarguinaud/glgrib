#pragma once

#include <string>
#include <vector>
#include <limits>
#include <cmath>
#include <cstdio>

namespace glGrib
{

template <int N> 
class KdTree
{
public:
  static int Id;

  class Point
  {
  public:
    Point () { for (int i = 0; i < N; i++) { x[i] = 0.; } }
    Point (const float _x[]) { for (int i = 0; i < N; i++) { x[i] = _x[i]; } }
    Point (const std::vector<float> & _x) { for (int i = 0; i < N; i++) { x[i] = _x[i]; } }
    Point (const float _x) { for (int i = 0; i < N; i++) { x[i] = _x; } }
    float x[N];
    bool operator== (const Point & p) const
    {
      bool b = true;
      for (int i = 0; i < N; i++)
        b = b && (x[i] == p.x[i]);
      return b;
    }
    float distance (const Point & p) const
    {
      float dist = 0.;
      for (int i = 0; i < N; i++)
        dist = std::max (dist, std::abs (p.x[i] - x[i]));
      return dist;
    }
  };


  class Result;

  class KdNode
  {
  public:
    KdNode (const std::vector<Point> * _points) : points (_points), id (Id++)
    {
    }

    ~KdNode ()
    {
      if (prev)
        delete prev;
      prev = nullptr;
      if (next)
        delete next;
      next = nullptr;
    }

    void searchDown (const Point &, Result *) const;
    void searchFull (const Point &, Result *, int, const KdNode *, int = 0) const;
    Result search (const Point &, int = std::numeric_limits<int>::max ()) const;
    Result searchDumb (const Point &) const;
    void display (bool all = false, const int depth = 0) const;
    void displayPath (const Point &, const Point &, FILE * = stdout) const;

    const std::vector<Point> & getPoints () const
    {
      return *points;
    }

    float mindist (const Point &) const;

  private:
   
    void build (int, int = 10);
   
    const KdNode * prev = nullptr, * next = nullptr, * up = nullptr;
   
    const std::vector<Point> * points = nullptr;
    const int id = -1;
   
    int mid = -1;
    int dir = -1;
   
    std::vector<int> list;

    friend class KdTree;
  };

  class Result
  {
  public:
    float dist = +std::numeric_limits<float>::max ();
    int rank = -1;
    const KdNode * node = nullptr;
    int count = 0;
  };

  KdTree () : root (&points)
  {
  }

  void searchDown (const Point & p, Result * r) const
  {
    root.searchDown (p, r);
  }
  Result search (const Point & p, int maxCount = std::numeric_limits<int>::max ()) const
  {
    return root.search (p, maxCount);
  }
  Result searchDumb (const Point & p) const
  {
    return root.searchDumb (p);
  }
  void display (bool all = false, const int depth = 0) const
  {
    root.display (all, depth);
  }
  void displayPath (const Point & p, const Point & q, FILE * fp = stdout) const
  {
    root.displayPath (p, q, fp);
  }

  void build ();

  std::vector<Point> & getPoints ()
  {
    return points;
  }
  
  const std::vector<Point> & getPoints () const
  {
    return points;
  }
  

private:

  KdNode root;
  std::vector<Point> points;


};

}
