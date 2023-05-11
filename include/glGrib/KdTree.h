#pragma once

#include <string>
#include <vector>
#include <limits>

namespace glGrib
{

template <int N> 
class KdTree
{
public:


  class Point
  {
  public:
    Point () { for (int i = 0; i < N; i++) { x[i] = 0.; } }
    Point (const float _x[]) { for (int i = 0; i < N; i++) { x[i] = _x[i]; } }
    Point (const std::vector<float> & _x) { for (int i = 0; i < N; i++) { x[i] = _x[i]; } }
    Point (const float _x) { for (int i = 0; i < N; i++) { x[i] = _x; } }
    float x[N];
  };

  KdTree (const std::vector<Point> * _points) : points (_points)
  {
  }

  ~KdTree ()
  {
    if (prev)
      delete prev;
    prev = nullptr;
    if (next)
      delete next;
    next = nullptr;
  }

  bool contains (const Point & p) const
  {
    bool b = true;
    for (int i = 0; i < N; i++)
      b = b && (min.x[i] <= p.x[i]) && (p.x[i] <= max.x[i]);
    return b;
  }

  int search (const Point & p) const;

  void display (bool all = false, const int depth = 0) const;

  void build ();

private:

  void _build (int);

  KdTree<N> * prev = nullptr, * next = nullptr;

  const std::vector<Point> * points = nullptr;

  std::vector<int> list;

  Point min = Point (+std::numeric_limits<float>::max ());
  Point max = Point (-std::numeric_limits<float>::max ());

};

}
