/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    SO3.h
 * @brief   3*3 matrix representation of SO(3)
 * @author  Frank Dellaert
 * @author  Luca Carlone
 * @author  Duy Nguyen Ta
 * @date    December 2014
 */

#pragma once

#include <gtsam/geometry/SOn.h>

#include <gtsam/base/Lie.h>
#include <gtsam/base/Matrix.h>

#include <cmath>
#include <iosfwd>
#include <vector>

namespace gtsam {

using SO3 = SO<3>;

//   /// Static, named constructor that finds SO(3) matrix closest to M in
//   Frobenius norm. static SO3 ClosestTo(const Matrix3& M);

//   /// Static, named constructor that finds chordal mean = argmin_R \sum
//   sqr(|R-R_i|_F). static SO3 ChordalMean(const std::vector<SO3>& rotations);

//   static Matrix3 Hat(const Vector3 &xi); ///< make skew symmetric matrix
//   static Vector3 Vee(const Matrix3 &X);  ///< inverse of Hat

/**
 * Exponential map at identity - create a rotation from canonical coordinates
 * \f$ [R_x,R_y,R_z] \f$ using Rodrigues' formula
 */
template <>
SO3 SO3::Expmap(const Vector3& omega, ChartJacobian H);

// template<>
// Matrix3 SO3::ExpmapDerivative(const Vector3& omega) {
//   return sot::DexpFunctor(omega).dexp();
// }

/// Derivative of Expmap
//   static Matrix3 ExpmapDerivative(const Vector3& omega);

/**
 * Log map at identity - returns the canonical coordinates
 * \f$ [R_x,R_y,R_z] \f$ of this rotation
 */
template <>
Vector3 SO3::Logmap(const SO3& R, ChartJacobian H);

template <>
Matrix3 SO3::AdjointMap() const {
  return matrix_;
}

// Chart at origin for SO3 is *not* Cayley but actual Expmap/Logmap
template <>
SO3 SO3::ChartAtOrigin::Retract(const Vector3& omega, ChartJacobian H) {
  return Expmap(omega, H);
}

template <>
Vector3 SO3::ChartAtOrigin::Local(const SO3& R, ChartJacobian H) {
  return Logmap(R, H);
}

template <>
Vector9 SO3::vec(OptionalJacobian<9, 3> H) const;

//   private:

//     /** Serialization function */
//     friend class boost::serialization::access;
//     template<class ARCHIVE>
//     void serialize(ARCHIVE & ar, const unsigned int /*version*/)
//     {
//        ar & boost::serialization::make_nvp("R11", (*this)(0,0));
//        ar & boost::serialization::make_nvp("R12", (*this)(0,1));
//        ar & boost::serialization::make_nvp("R13", (*this)(0,2));
//        ar & boost::serialization::make_nvp("R21", (*this)(1,0));
//        ar & boost::serialization::make_nvp("R22", (*this)(1,1));
//        ar & boost::serialization::make_nvp("R23", (*this)(1,2));
//        ar & boost::serialization::make_nvp("R31", (*this)(2,0));
//        ar & boost::serialization::make_nvp("R32", (*this)(2,1));
//        ar & boost::serialization::make_nvp("R33", (*this)(2,2));
//     }

namespace sot {

/**
 * Compose general matrix with an SO(3) element.
 * We only provide the 9*9 derivative in the first argument M.
 */
Matrix3 compose(const Matrix3& M, const SO3& R,
                OptionalJacobian<9, 9> H = boost::none);

/// (constant) Jacobian of compose wrpt M
Matrix99 Dcompose(const SO3& R);

// Below are two functors that allow for saving computation when exponential map
// and its derivatives are needed at the same location in so<3>. The second
// functor also implements dedicated methods to apply dexp and/or inv(dexp).

/// Functor implementing Exponential map
class GTSAM_EXPORT ExpmapFunctor {
 protected:
  const double theta2;
  Matrix3 W, K, KK;
  bool nearZero;
  double theta, sin_theta, one_minus_cos;  // only defined if !nearZero

  void init(bool nearZeroApprox = false);

 public:
  /// Constructor with element of Lie algebra so(3)
  explicit ExpmapFunctor(const Vector3& omega, bool nearZeroApprox = false);

  /// Constructor with axis-angle
  ExpmapFunctor(const Vector3& axis, double angle, bool nearZeroApprox = false);

  /// Rodrigues formula
  SO3 expmap() const;
};

/// Functor that implements Exponential map *and* its derivatives
class GTSAM_EXPORT DexpFunctor : public ExpmapFunctor {
  const Vector3 omega;
  double a, b;
  Matrix3 dexp_;

 public:
  /// Constructor with element of Lie algebra so(3)
  explicit DexpFunctor(const Vector3& omega, bool nearZeroApprox = false);

  // NOTE(luca): Right Jacobian for Exponential map in SO(3) - equation
  // (10.86) and following equations in G.S. Chirikjian, "Stochastic Models,
  // Information Theory, and Lie Groups", Volume 2, 2008.
  //   expmap(omega + v) \approx expmap(omega) * expmap(dexp * v)
  // This maps a perturbation v in the tangent space to
  // a perturbation on the manifold Expmap(dexp * v) */
  const Matrix3& dexp() const { return dexp_; }

  /// Multiplies with dexp(), with optional derivatives
  Vector3 applyDexp(const Vector3& v, OptionalJacobian<3, 3> H1 = boost::none,
                    OptionalJacobian<3, 3> H2 = boost::none) const;

  /// Multiplies with dexp().inverse(), with optional derivatives
  Vector3 applyInvDexp(const Vector3& v,
                       OptionalJacobian<3, 3> H1 = boost::none,
                       OptionalJacobian<3, 3> H2 = boost::none) const;
};
}  //  namespace sot

/*
 * Define the traits. internal::LieGroup provides both Lie group and Testable
 */

template <>
struct traits<SO3> : public internal::LieGroup<SO3> {};

template <>
struct traits<const SO3> : public internal::LieGroup<SO3> {};

}  // end namespace gtsam