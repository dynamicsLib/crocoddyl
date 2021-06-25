///////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (C) 2019-2021, LAAS-CNRS, IRI: CSIC-UPC, University of Edinburgh
// Copyright note valid unless otherwise stated in individual files.
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#ifndef CROCODDYL_CORE_INTEGRATOR_RK4_HPP_
#define CROCODDYL_CORE_INTEGRATOR_RK4_HPP_

#include "crocoddyl/core/fwd.hpp"
#include "crocoddyl/core/integr-action-base.hpp"

namespace crocoddyl {

template <typename _Scalar>
class IntegratedActionModelRK4Tpl : public IntegratedActionModelAbstractTpl<_Scalar> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  typedef _Scalar Scalar;
  typedef MathBaseTpl<Scalar> MathBase;
  typedef IntegratedActionModelAbstractTpl<Scalar> Base;
  typedef IntegratedActionDataRK4Tpl<Scalar> Data;
  typedef ActionDataAbstractTpl<Scalar> ActionDataAbstract;
  typedef DifferentialActionModelAbstractTpl<Scalar> DifferentialActionModelAbstract;
  typedef ControlParametrizationModelAbstractTpl<Scalar> ControlParametrizationModelAbstract;
  typedef typename MathBase::VectorXs VectorXs;
  typedef typename MathBase::MatrixXs MatrixXs;

  IntegratedActionModelRK4Tpl(boost::shared_ptr<DifferentialActionModelAbstract> model,
                              const Scalar time_step = Scalar(1e-3), const bool with_cost_residual = true);
  IntegratedActionModelRK4Tpl(boost::shared_ptr<DifferentialActionModelAbstract> model,
                              boost::shared_ptr<ControlParametrizationModelAbstract> control,
                              const Scalar time_step = Scalar(1e-3), const bool with_cost_residual = true);
  virtual ~IntegratedActionModelRK4Tpl();

  virtual void calc(const boost::shared_ptr<ActionDataAbstract>& data, const Eigen::Ref<const VectorXs>& x,
                    const Eigen::Ref<const VectorXs>& u);
  virtual void calcDiff(const boost::shared_ptr<ActionDataAbstract>& data, const Eigen::Ref<const VectorXs>& x,
                        const Eigen::Ref<const VectorXs>& u);
  virtual boost::shared_ptr<ActionDataAbstract> createData();
  virtual bool checkData(const boost::shared_ptr<ActionDataAbstract>& data);

  virtual void quasiStatic(const boost::shared_ptr<ActionDataAbstract>& data, Eigen::Ref<VectorXs> u,
                           const Eigen::Ref<const VectorXs>& x, const std::size_t maxiter = 100,
                           const Scalar tol = Scalar(1e-9));

  /**
   * @brief Print relevant information of the Runge-Kutta 4 integrator model
   *
   * @param[out] os  Output stream object
   */
  virtual void print(std::ostream& os) const;

protected:
  using Base::has_control_limits_;  //!< Indicates whether any of the control limits are active
  using Base::nr_;                  //!< Dimension of the cost residual
  using Base::nu_;                  //!< Dimension of the control
  using Base::control_;             //!< Control discretization
  using Base::controlData_;         //!< Control discretization data
  using Base::state_;               //!< Model of the state
  using Base::u_lb_;                //!< Lower control limits
  using Base::u_ub_;                //!< Upper control limits
  using Base::unone_;               //!< Neutral state
  using Base::differential_;
  using Base::time_step_;
  using Base::with_cost_residual_;
  using Base::enable_integration_;

  void init();
  
 private:
  std::vector<Scalar> rk4_c_;
};

template <typename _Scalar>
struct IntegratedActionDataRK4Tpl : public IntegratedActionDataAbstractTpl<_Scalar> {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  typedef _Scalar Scalar;
  typedef MathBaseTpl<Scalar> MathBase;
  typedef IntegratedActionDataAbstractTpl<Scalar> Base;
  typedef typename MathBase::VectorXs VectorXs;
  typedef typename MathBase::MatrixXs MatrixXs;

  template <template <typename Scalar> class Model>
  explicit IntegratedActionDataRK4Tpl(Model<Scalar>* const model) : Base(model) {
    const std::size_t ndx = model->get_state()->get_ndx();
    const std::size_t nx = model->get_state()->get_nx();
    const std::size_t nv = model->get_state()->get_nv();
    const std::size_t nu_diff = model->get_nu_diff();
    const std::size_t nu = model->get_nu();

    for (std::size_t i = 0; i < 4; ++i) {
      differential.push_back(
          boost::shared_ptr<DifferentialActionDataAbstractTpl<Scalar> >(model->get_differential()->createData()));
    }

    dx = VectorXs::Zero(ndx);
    u_diff = std::vector<VectorXs>(4, VectorXs::Zero(nu_diff));
    integral = std::vector<Scalar>(4, Scalar(0.));

    ki = std::vector<VectorXs>(4, VectorXs::Zero(ndx));
    y = std::vector<VectorXs>(4, VectorXs::Zero(nx));
    dx_rk4 = std::vector<VectorXs>(4, VectorXs::Zero(ndx));

    dki_dx = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, ndx));
    dki_dudiff = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, nu_diff));
    dki_du = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, nu));
    dfi_du = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, nu));
    dyi_dx = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, ndx));
    dyi_du = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, nu));
    dki_dy = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, ndx));

    dli_dx = std::vector<VectorXs>(4, VectorXs::Zero(ndx));
    dli_dudiff = std::vector<VectorXs>(4, VectorXs::Zero(nu_diff));
    dli_du = std::vector<VectorXs>(4, VectorXs::Zero(nu));
    ddli_ddx  = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, ndx));
    ddli_ddudiff  = std::vector<MatrixXs>(4, MatrixXs::Zero(nu_diff, nu_diff));
    ddli_dudiffdu = std::vector<MatrixXs>(4, MatrixXs::Zero(nu_diff, nu));
    ddli_ddu  = std::vector<MatrixXs>(4, MatrixXs::Zero(nu, nu));
    // std::cout<<"ddli_ddu = "<<nu<<"\n";
    ddli_dxdudiff = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, nu_diff));
    ddli_dxdu = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, nu));
    Luu_partialx = std::vector<MatrixXs>(4, MatrixXs::Zero(nu, nu));
    Lxu_i = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, nu));
    Lxx_partialx = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, ndx));
    Lxx_partialu = std::vector<MatrixXs>(4, MatrixXs::Zero(ndx, nu));

    dyi_dx[0].diagonal().array() = (Scalar)1;
    for (std::size_t i = 0; i < 4; ++i) {
      dki_dy[i].topRightCorner(nv, nv).diagonal().array() = (Scalar)1;
    }
  }
  virtual ~IntegratedActionDataRK4Tpl() {}

  VectorXs dx;
  std::vector<VectorXs> u_diff;
  std::vector<boost::shared_ptr<DifferentialActionDataAbstractTpl<Scalar> > > differential;
  std::vector<Scalar> integral;
  std::vector<VectorXs> ki;
  std::vector<VectorXs> y;
  std::vector<VectorXs> dx_rk4;

  std::vector<MatrixXs> dki_dx;
  std::vector<MatrixXs> dki_dudiff;
  std::vector<MatrixXs> dki_du;
  std::vector<MatrixXs> dfi_du;
  std::vector<MatrixXs> dyi_dx;
  std::vector<MatrixXs> dyi_du;
  std::vector<MatrixXs> dki_dy;

  std::vector<VectorXs> dli_dx;
  std::vector<VectorXs> dli_dudiff; // not used
  std::vector<VectorXs> dli_du;
  std::vector<MatrixXs> ddli_ddx;
  std::vector<MatrixXs> ddli_ddudiff;
  std::vector<MatrixXs> ddli_dudiffdu;
  std::vector<MatrixXs> ddli_ddu;
  std::vector<MatrixXs> ddli_dxdudiff;
  std::vector<MatrixXs> ddli_dxdu;
  std::vector<MatrixXs> Luu_partialx;
  std::vector<MatrixXs> Lxu_i;
  std::vector<MatrixXs> Lxx_partialx;
  std::vector<MatrixXs> Lxx_partialu;

  using Base::cost;
  using Base::Fu;
  using Base::Fx;
  using Base::Lu;
  using Base::Luu;
  using Base::Lx;
  using Base::Lxu;
  using Base::Lxx;
  using Base::r;
  using Base::xnext;
};

}  // namespace crocoddyl

/* --- Details -------------------------------------------------------------- */
/* --- Details -------------------------------------------------------------- */
/* --- Details -------------------------------------------------------------- */
#include "crocoddyl/core/integrator/rk4.hxx"

#endif  // CROCODDYL_CORE_INTEGRATOR_RK4_HPP_
