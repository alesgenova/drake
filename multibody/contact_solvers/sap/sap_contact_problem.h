#pragma once

#include <vector>

#include "drake/common/drake_copyable.h"
#include "drake/common/eigen_types.h"

namespace drake {
namespace multibody {
namespace contact_solvers {
namespace internal {

/* In the SAP formulation of contact the state of a mechanical system is
advanced from the previous state x₀ = [q₀,v₀] to the next state x = [q,v]. The
SAP formulation linearizes the discrete time dynamics of the mechanical system
of interest to obtain the SAP momentum equations:
  A(v*)⋅(v−v*) = J(q₀)ᵀ⋅γ                                                   (1)
where v denotes the generalized velocities of the system, of size nv. Matrix A
(of size nv x nv) is a symmetric positive definite (SPD) approximation of the
linearized discrete dynamics, v* (of size nv) are the free-motion generalized
velocities (i.e. the velocities of the system when constraint impulses are
zero), γ (of size nk) are the constraint impulses and J (of size nk x nv) the
constraint Jacobian. SAP's formulation solves a strictly convex optimization
problem such that at optimality:
  1. the linearized momentum equation in Eq. (1) is satisfied and,
  2. impulses γ are constrained to live in a convex set.
Please refer to [Castro et al., 2021] for details.

Therefore a SapContactProblem is described by:
  1. The linear dynamics matrix A,
  2. the free motion velocities v* and,
  3. the set of constraints for the problem (which defines the Jacobian J).

Often times matrix A is block diagonal. This block diagonal structure of A
induces a partition of the nv generalized velocities. A subset in this partition
is referred to as a "clique". The total number of cliques in this partition
equals the number of diagonal blocks in A.

[Castro et al., 2021] Castro A., Permenter F. and Han X., 2021. An Unconstrained
Convex Formulation of Compliant Contact. Available at
https://arxiv.org/abs/2110.10107 */
template <typename T>
class SapContactProblem {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(SapContactProblem);

  /* Constructs a SAP contact problem for a system of equations discretized with
   a given `time_step` provided its linear dynamics matrix A and free motion
   velocities v*. See this class's documentation for details.

   @param[in] time_step
     The time step used to discretize the dynamics in time, it must be
     strictly positive. E.g. the time step used to write an implicit Euler
     scheme of the dynamics.
   @param[in] A
     SPD approximation of the linearized dynamics, [Castro et al., 2021]. This
     matrix is block diagonal with each block corresponding to a "clique". The
     number of cliques is A.size() and the number of DOFs in the c-th clique
     is A[c].rows() (or A[c].cols() since each block is square). The total
     number of generalized velocities of the system is nv = ∑A[c].rows().
     This class does not check for the positive definiteness of each block in
     A, it is the responsability of the calling code to enforce this
     invariant. Ultimately, the SAP solver can fail to converge if this
     requirement is not satisfied.
   @param[in] v_star
     Free-motion velocities, of size nv. DOFs in v_star must match the
     ordering implicitly induced by A.

   @throws if the blocks in A are not square or have zero size.
   @throws if the size of v_star is not nv = ∑A[c].rows(). */
  SapContactProblem(const T& time_step, std::vector<MatrixX<T>> A,
                    VectorX<T> v_star);

  /* TODO(amcastro-tri): consider constructor API taking std::vector<VectorX<T>>
   for v_star. It could be useful for deformables. */

  /* TODO(amcastro-tri): Implement APIs to add constraints in follow up PRs. */

  /* Returns the number of cliques. */
  int num_cliques() const { return A_.size(); }

  /* The total number of generalized velocities for this problem. */
  int num_velocities() const { return nv_; }

  /* The number of generalized velocities for clique with index `clique_index`.
   clique_index must be in the interval [0, num_cliques()). */
  int num_velocities(int clique_index) const {
    DRAKE_DEMAND(0 <= clique_index && clique_index < num_cliques());
    return A_[clique_index].rows();
  }

  const T& time_step() const { return time_step_; }

  /* Returns the block diagonal dynamics matrix A. */
  const std::vector<MatrixX<T>>& dynamics_matrix() const { return A_; }

  /* Returns the full vector of free-motion velocities v*, of size
   num_velocities(). */
  const VectorX<T>& v_star() const { return v_star_; }

 private:
  int nv_{0};                    // Total number of generalized velocities.
  T time_step_{0.0};             // Discrete time step.
  std::vector<MatrixX<T>> A_;    // Linear dynamics matrix.
  VectorX<T> v_star_;            // Free-motion velocities.
};

}  // namespace internal
}  // namespace contact_solvers
}  // namespace multibody
}  // namespace drake
