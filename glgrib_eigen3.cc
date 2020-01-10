#include "glgrib_eigen3.h"
#include <Eigen/Eigenvalues> 

void glgrib_diag (const glm::dmat2 & A, glm::dmat2 * Q, glm::dvec2 * w)
{
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix2d> es;
  Eigen::Matrix2d A_;

  for (int i = 0; i < 2; i++)
  for (int j = 0; j < 2; j++)
    A_ (i, j) = A[i][j];

  es.compute (A_);

  for (int i = 0; i < 2; i++)
  for (int j = 0; j < 2; j++)
    (*Q)[i][j] = es.eigenvectors ().col (i) (j);

  for (int i = 0; i < 2; i++) 
    (*w)[i] = es.eigenvalues () (i);

#ifdef UNDEF
  for (int i = 0; i < 2; i++)
    {
      Eigen::Vector2d u_ = es.eigenvectors ().col (i);
      Eigen::Vector2d q_ = A_ * u_;
      glm::dvec2 u = (*Q)[i];
      glm::dvec2 q = A * u;
      printf (" ev    = %12.2e\n", (*w)[i]);
      printf (" u_    = %12.2e %12.2e\n", u_ (0), u_ (1));
      printf (" u     = %12.2e %12.2e\n", u  [0], u  [1]);
      printf (" q_    = %12.2e %12.2e\n", q_ (0), q_ (1));
      printf (" q     = %12.2e %12.2e\n", q  [0], q  [1]);
      printf (" q_/u_ = %12.2e %12.2e\n", q_ (0) / u_ (0), q_ (1) / u_ (1));
      printf (" q /u  = %12.2e %12.2e\n", q  [0] / u  [0], q  [1] / u  [1]);
    }
#endif

}
