#include "glgrib_eigen3.h"
#include <Eigen/Eigenvalues> 

void glgrib_diag (const glm::dmat3 & A, glm::dmat3 * Q, glm::dvec3 * w)
{
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es;
  Eigen::Matrix3d A_;

  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
    A_ (i, j) = A[i][j];

  es.compute (A_);

  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
    (*Q)[i][j] = es.eigenvectors ().col (i) (j);

  for (int i = 0; i < 3; i++) 
    (*w)[i] = es.eigenvalues () (i);

#ifdef UNDEF
  for (int i = 0; i < 3; i++)
    {
      Eigen::Vector3d u_ = es.eigenvectors ().col (i);
      Eigen::Vector3d q_ = A_ * u_;
      glm::dvec3 u = (*Q)[i];
      glm::dvec3 q = A * u;
      printf (" ev    = %12.2e\n", (*w)[i]);
      printf (" u_    = %12.2e %12.2e %12.2e\n", u_ (0), u_ (1), u_ (2));
      printf (" u     = %12.2e %12.2e %12.2e\n", u  [0], u  [1], u  [2]);
      printf (" q_    = %12.2e %12.2e %12.2e\n", q_ (0), q_ (1), q_ (2));
      printf (" q     = %12.2e %12.2e %12.2e\n", q  [0], q  [1], q  [2]);
      printf (" q_/u_ = %12.2e %12.2e %12.2e\n", q_ (0) / u_ (0), q_ (1) / u_ (1), q_ (2) / u_ (2));
      printf (" q /u  = %12.2e %12.2e %12.2e\n", q  [0] / u  [0], q  [1] / u  [1], q  [2] / u  [2]);
    }
#endif

}
