#include <netcdf>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>

#include "glGrib/KdTree.h"

template <int N>
void glGrib::KdTree<N>::display (bool all, const int depth) const
{
  const KdTree<N> * tree = this;

  auto spc = [](int i) { while (i > 0) { printf (" "); i--; } };

  for (int i = 0; i < N; i++)
    {
      spc (depth*2); 
      printf (" x[%d] = %12.4e .. %12.4e | %12.4e\n", 
               i, tree->min.x[i], tree->max.x[i], tree->max.x[i]-tree->min.x[i]);
    }

  if (tree->prev)
    {
      spc (depth*2); printf ("-PREV\n");
      tree->prev->display (all, depth+1);
    }
  if (tree->next)
    {
      spc (depth*2); printf ("-NEXT\n");
      tree->next->display (all, depth+1);
    }


  if (tree->list.size ())
    {
      spc (depth*2); printf (" COUNT = %ld\n", tree->list.size ());
    }

  const std::vector<Point> & points = *(tree->points);

  if (all && tree->list.size ())
    {
      spc (depth*2); printf (" LIST\n");
      
      for (const auto & p : tree->list)
        {
          spc (depth*2); 
	  printf ("(");
	  for (int i = 0; i < N; i++)
            {
	      printf ("%12.4e", points[p].x[i]);
              if (i > 0)
                printf (",");
	    }
	  printf (")\n");
	}
    }

}

template <int N>
void glGrib::KdTree<N>::_build (int depth)
{
  KdTree<N> * tree = this;
  const int n = tree->list.size ();
  const std::vector<Point> & points = *(tree->points);

  for (int i = 0; i < n; i++)
  for (int j = 0; j < N; j++)
    {
      tree->min.x[j] = std::min (tree->min.x[j], points[tree->list[i]].x[j]); 
      tree->max.x[j] = std::max (tree->max.x[j], points[tree->list[i]].x[j]);
    }

  if (n < 10)
    return;

  if (depth > 20)
    return;

  int rank[n];
  for (int i = 0; i < n; i++)
    rank[i] = i;

  float dx[N];

  for (int j = 0; j < N; j++)
    dx[j] = tree->max.x[j] - tree->min.x[j];

  float * max = std::max_element (&dx[0], &dx[0] + N);
  int jmax = max - &dx[0];

  if (*max > 0.)
    {
      std::sort (&rank[0], &rank[0]+n, [tree,jmax,&points](int i, int j) 
        { return points[tree->list[i]].x[jmax] < points[tree->list[j]].x[jmax]; });

      int i;
     
      i = n/2;

      while (i > 0)
        {
          if (points[tree->list[rank[i-1]]].x[jmax] < points[tree->list[rank[i]]].x[jmax])
            break;
	  i--;
	}

      if (i == 0)
        {
          i = n/2+1;
          while (i < n-1)
            {
              if (points[tree->list[rank[i-1]]].x[jmax] < points[tree->list[rank[i]]].x[jmax])
                break;
              i++;
            }
        }

      float xa = points[tree->list[rank[i-1]]].x[jmax],
            xb = points[tree->list[rank[i+0]]].x[jmax];

      tree->prev = new KdTree<N> (&points); 
      tree->prev->min = tree->min;
      tree->prev->max = tree->max;
      tree->prev->max.x[jmax] = xa;
      tree->prev->list.reserve (i);

      for (int j = 0; j < i; j++)
        tree->prev->list.push_back (tree->list[rank[j]]);

      tree->next = new KdTree<N> (&points); 
      tree->next->min = tree->min;
      tree->next->max = tree->max;
      tree->next->min.x[jmax] = xb;
      tree->next->list.reserve (tree->list.size () - i + 1);

      for (int j = i; j < n; j++)
        tree->next->list.push_back (tree->list[rank[j]]);

      tree->list.resize (0);
#pragma omp task 
      {
        tree->prev->_build (depth+1);
      }
#pragma omp task 
      {
        tree->next->_build (depth+1);
      }

    }
  if (depth == 0)
    {
#pragma omp taskwait
    }
}

template <int N>
int glGrib::KdTree<N>::search (const Point & p) const
{
  const KdTree<N> * tree = this;
  const KdTree<N> * next = tree->next;
  const KdTree<N> * prev = tree->prev;

  if (! tree->contains (p))
    return -1;

  if (next || prev)
    {
      if (next && prev)
        {
          if (next->contains (p) && prev->contains (p))
            abort ();
	}
      if (next && next->contains (p))
        return next->search (p);
      if (prev && prev->contains (p))
        return prev->search (p);
      return -1;
    }
  
  const std::vector<Point> & points = *(tree->points);
  const int n = tree->list.size ();
  for (int i = 0; i < n; i++)
    {
      const Point & q = points[tree->list[i]];
      float d = 0.;
      for (int j = 0; j < N; j++)
        d += (p.x[j] - q.x[j]) * (p.x[j] - q.x[j]);
      d = sqrt (d);
      if (d == 0.)
        return tree->list[i];
    }

  return -1;
}

template <int N>
void glGrib::KdTree<N>::build ()
{
  const int n = points->size ();
  list.resize (n);
  for (int i = 0; i < n; i++)
    list[i] = i;
  _build (0);
}

template class glGrib::KdTree<3>;



