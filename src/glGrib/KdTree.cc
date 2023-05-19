#include <netcdf>
#include <string>
#include <vector>
#include <algorithm>
#include <execution>
#include <limits>
#include <cmath>
#include <omp.h>

#include "glGrib/KdTree.h"

template <int N>
void glGrib::KdTree<N>::KdNode::display (bool all, const int depth) const
{
  const KdNode * node = this;
  const auto & points = *(node->points);

  auto spc = [](int i) { while (i > 0) { printf (" "); i--; } };

  for (int i = 0; i < N; i++)
    {
      spc (depth*2); 
      printf (" x[%d] = %12.4e %d\n", i, points[node->mid].x[i], node->dir);
    }

  if (node->prev)
    {
      spc (depth*2); printf ("-PREV\n");
      node->prev->display (all, depth+1);
    }
  if (node->next)
    {
      spc (depth*2); printf ("-NEXT\n");
      node->next->display (all, depth+1);
    }


  if (node->list.size ())
    {
      spc (depth*2); printf (" COUNT = %ld\n", node->list.size ());
    }


  if (all && node->list.size ())
    {
      spc (depth*2); printf (" LIST\n");
      
      for (const auto & p : node->list)
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
void glGrib::KdTree<N>::KdNode::build (int depth, int nPoint)
{
  KdNode * node = this;
  const int n = node->list.size ();
  const auto & points = *(node->points);

  int rank[n];
  for (int i = 0; i < n; i++)
    rank[i] = i;

  float dx[N];

  Point min;
  Point max;

  for (int j = 0; j < N; j++)
    {
      float xmin = +std::numeric_limits<float>::max ();
      float xmax = -std::numeric_limits<float>::max ();
#pragma omp parallel for reduction (min: xmin) reduction(max: xmax) if (n > 128)
      for (int i = 0; i < n; i++)
        {
          xmin = std::min (xmin, points[node->list[i]].x[j]);
          xmax = std::max (xmax, points[node->list[i]].x[j]);
        }
      min.x[j] = xmin;
      max.x[j] = xmax;
    }

  for (int j = 0; j < N; j++)
    dx[j] = max.x[j] - min.x[j];

  float * dmax = std::max_element (&dx[0], &dx[0] + N);
  int jmax = dmax - &dx[0];

  auto cmp = [node,jmax,&points](int i, int j) 
    { return points[node->list[i]].x[jmax] < points[node->list[j]].x[jmax]; };

#if __cplusplus >= 201703L
  if (n > 128)
    std::sort (std::execution::par, &rank[0], &rank[0]+n, cmp);
  else
#endif
    std::sort (&rank[0], &rank[0]+n, cmp);

  int i = n/2;

  node->mid = node->list[rank[i]]; 

  if (n < nPoint)
    return;

  if (*dmax > 0.)
    {
      node->dir = jmax;

      KdNode * prev = new KdNode (&points); prev->up = node;
      KdNode * next = new KdNode (&points); next->up = node;

      node->prev = prev;
      node->next = next;

      prev->list.resize (i);

#pragma omp parallel for
      for (int j = 0; j < i; j++)
        prev->list[j] = node->list[rank[j]];

      next->list.resize (n - i);

#pragma omp parallel for
      for (int j = i; j < n; j++)
        next->list[j-i] = node->list[rank[j]];

      node->list.resize (0);
#pragma omp task 
      {
        prev->build (depth+1, nPoint);
      }
#pragma omp task 
      {
        next->build (depth+1, nPoint);
      }

    }
  if (depth == 0)
    {
#pragma omp taskwait
    }
}

template <int N>
typename glGrib::KdTree<N>::Result glGrib::KdTree<N>::KdNode::searchDumb (const Point & p) const
{
  Result r;
  const auto & points = *(this->points);
  const int n = points.size ();

  for (int i = 0; i < n; i++)
    {
      float dd = p.distance (points[i]);
      if (dd < r.dist)
        {
          r.rank = i;
	  r.dist = dd;
	  r.count++;
	}
    }

  return r;
}

template <int N>
void glGrib::KdTree<N>::KdNode::displayPath (const Point & p, const Point & q, FILE * fp) const
{
  const auto & points = *(this->points);
  Result r;

  searchDown (p, &r);

  const KdNode * node = r.node;

  fprintf (fp, "(");
  for (int i = 0; i < N; i++)
    fprintf (fp, "%12.4e%s", p.x[i], i < N ? ", " : ")");
  fprintf (fp, " %6d [%6d] %6d %f\n", 
           node->prev ? node->prev->id : -1, node->id, node->next ? node->next->id : -1,
	   p.distance (q));
  fprintf (fp, "\n");

  for (node = node->up; node; node = node->up)
    {
      const Point p = points[node->mid];
      fprintf (fp, "(");
      for (int i = 0; i < N; i++)
        fprintf (fp, "%12.4e%s", p.x[i], i < N ? ", " : ")");
      fprintf (fp, " %6d [%6d] %6d %f\n", 
               node->prev ? node->prev->id : -1, node->id, node->next ? node->next->id : -1,
	       p.distance (q));
      fprintf (fp, " ");
      for (int i = 0; i < N; i++)
        fprintf (fp, "%12s%s", node->dir == i ? "------------" : "", i < N ? "  " : " \n");
    }
}

template <int N>
float glGrib::KdTree<N>::KdNode::mindist (const Point & p) const
{
  const auto * node = this;
  const auto & points = node->getPoints ();
  float dist = p.distance (points[node->mid]);

  if (node->prev)
    dist = std::min (dist, node->prev->mindist (p));

  if (node->next)
    dist = std::min (dist, node->next->mindist (p));

  const int n = node->list.size ();
  for (int i = 0; i < n; i++)
    dist = std::min (dist, p.distance (points[node->list[i]]));

  return dist;
}

template <int N>
typename glGrib::KdTree<N>::Result glGrib::KdTree<N>::KdNode::search (const Point & p, int maxCount) const
{
  Result r;
  searchFull (p, &r, maxCount, nullptr);
  return r;
}

template <int N>
void glGrib::KdTree<N>::KdNode::searchFull 
(const Point & p, Result * r, int maxCount, const KdNode * stop, int depth) const
{
  const std::string indent (depth * 2, ' ');

  const auto & points = *(this->points);

  searchDown (p, r);

  for (const KdNode * node = r->node; node != stop; )
    {
      const KdNode * down = node;     // current node is down
      node = node->up;                // node above current node

      if (node == stop)
        break;

      // Distance to plan spanned by node above
      const int dir = node->dir, mid = node->mid;
      float dplan = std::abs (points[mid].x[dir] - p.x[dir]);
      float dnode = p.distance (points[node->mid]);  // distance to node above

      if (dnode < r->dist)
        {
          r->node = node; // node above is better, it is now the best node
	  r->rank = node->mid;
	  r->dist = dnode;
        }
      
      if ((dplan < r->dist) && (r->count < maxCount))
        {
	  Result r1;

          if ((down == node->prev) && node->next)
            {
              node->next->searchFull (p, &r1, maxCount, node, depth+1);
            }
          else 
          if ((down == node->next) && node->prev)
            {
              node->prev->searchFull (p, &r1, maxCount, node, depth+1); 
            }
      
          if (r1.dist < r->dist)
            {
	      r1.count += r->count;
	      *r = r1;
            }
        }

    }

}

template <int N>
void glGrib::KdTree<N>::KdNode::searchDown 
(const Point & p, Result * r) const
{
  const KdNode * node = this;
  const KdNode * next = node->next;
  const KdNode * prev = node->prev;
  const auto & points = *(node->points);
  const Point & m = points[node->mid];

  r->count++;

  if (points[node->mid] == p)
    {
      r->rank = node->mid;
      r->node = this;
      r->dist = 0.;
      return;
    }

  if (next || prev)
    {
      if (next && (p.x[node->dir] >= points[node->mid].x[node->dir]))
        {
          next->searchDown (p, r);
	}
      else 
      if (prev && (p.x[node->dir] <= points[node->mid].x[node->dir]))
        {
          prev->searchDown (p, r);
	}
    }
  else
    {
      const int n = node->list.size ();
      r->node = node;

      for (int i = 0; i < n; i++)
        {
          const Point & q = points[node->list[i]];
          float dpq = p.distance (q);
	  if (dpq <= r->dist)
            {
              r->dist = dpq;
	      r->rank = node->list[i];
              r->count++;
	    }
        }
    }
}

template <int N>
void glGrib::KdTree<N>::build ()
{
  const int n = root.points->size ();
  root.list.resize (n);
#pragma omp parallel for
  for (int i = 0; i < n; i++)
    root.list[i] = i;
  root.build (0);
}

template class glGrib::KdTree<3>;
template<> int glGrib::KdTree<3>::Id = 0;



