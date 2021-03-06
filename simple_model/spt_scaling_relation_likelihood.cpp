//
//  spt_scaling_relation_likelihood.cpp
//
//  Created by ALLEN PINJIC on 7/12/22.
//

#include <iostream>
#include <random>
#include <cmath>
#include <math.h>
#define Y3_CLUSTER_CPP_EZ_SQ_HH

float theta[9];
float eps = 1e-9;
float M0 = 3e14;
/* What each value in the theta array stands for:
float A_sze = theta[0];
float B_sze = theta[1];
float C_sze = theta[2];
float scatter_sze = theta[3];
float A_lambda = theta[4];
float B_lambda = theta[5];
float C_lambda = theta[6];
float scatter_lambda = theta[7];
float rho = theta[8]; */

namespace y3_cluster {
  class EZ_sq {
  public:
    EZ_sq(double omega_m, double omega_l, double omega_k)
      : _omega_m(omega_m), _omega_l(omega_l), _omega_k(omega_k)
    {}
    double
    operator()(double z) const
    {
      // NOTE: this is valid only for \Lambda CDM cosmology, not wCDM
      double const zplus1 = 1.0 + z;
      return (_omega_m * zplus1 * zplus1 * zplus1 + _omega_k * zplus1 * zplus1 +
              _omega_l);
    }
    float get_omega_m() {
          return _omega_m;
    }
    float get_omega_l() {
            return _omega_l;
    }
    float get_omega_k() {
            return _omega_k;
    }
  private:
      double _omega_m = 0.3153;
      double _omega_l = 0.6847;
      double _omega_k = -0.0096;
  };
}

float ln_zeta_given_M(float M, float z){
    float omega_m = y3_cluster.Ez_sq.get_omega_m();
    float omega_l = get_omega_l();
    float omega_k = get_omega_k();
    return (log(theta[0]) + (theta[1]) * log(M/M0) + (theta[2]) * (log(EZ_sq(omega_m, omega_l, omega_k)/Ez0)));
}
// E(z) function is not available in C++
float ln_lbd_given_M(float M, float z) {
    return (log(theta[4]) + (theta[5]) * log(M/M0) + (theta[6]) * (log(E(z)/Ez0)));
}

float gaussian(float x, float m, float error){
    float mean = m;
    float std_dev = error;
    const float inv_sqrt_2pi = 0.3989422804014327;
    
    return (inv_sqrt_2pi/std_dev) * (exp((-0.5) * pow(((x - mean)/(std_dev)), 2)));
}

float Prob_lbd_hat(float lambda, float lambda_hat, float lambda_error) {
    float res = gaussian(lambda, lambda_hat, lambda_error);
    return res;
}
    
float Prob_xi(float x, float additional, float error) {
    float res = gaussian(x, additional, error);
    return res;
}

/* Cannot yet implement the HMF since the mass_function.massFunction
 is only available in the colossus.cosmology module in Python */
float HMF(float M, float z) {
    // return mass_function.massFunction(M, z, mdef = '500c', model = 'bocquet16');
    return 0;
}

float compute_log_pLbdZeta(float lambda, float zeta,
                           float ln_lbd_pred, float ln_zeta_pred,
                           double eps = 1e-9) {
    // converting std to normal distribution
    float s_zeta = theta[3];
    float s_lambda = theta[7];
    
    // Initialization
    float s_lambda_inv = 0;
    
    if (s_lambda <= eps){
        s_lambda_inv = INFINITY;
    }
    else {
        s_lambda_inv = 1 / s_lambda;
    }
    
    // Initialization
    float s_zeta_inv = 0;
    
    if (s_zeta <= eps){
        s_zeta_inv = INFINITY;
    }
    else {
        s_zeta_inv = 1 / s_zeta;
    }
    
    float rho2 = 1 - pow(theta[8], 2);
    
    // Initialization
    float rho_inv = 0;
    
    if (rho2 <= eps){
        rho_inv = INFINITY;
    }
    else {
        rho_inv = 1/rho2;
    }
    
    float cov2 = pow(s_lambda,2) * pow(s_zeta,2) * rho2;
    float additional_cov = (-0.5) * log(M_PI * cov2);
        
    float lbd_std = (log(lambda) - ln_lbd_pred) * s_lambda_inv;
    float zeta_std = (log(zeta) - ln_zeta_pred) * s_zeta_inv;
    // np.seterr(invalid='ignore')
    // C++ equivalent?

    // lbd_likelihood
    float lp_lbd  = (((-rho_inv) * pow(lbd_std,2)) / 2);

    // zeta likelihood
    float lp_zeta = (((-rho_inv)* pow(zeta_std,2)) / 2);

    // corr likelihod
    float lp_corr = (theta[8]) * (rho_inv) * (lbd_std) * (zeta_std);
    
    // total likelihood
    float lp_total_m = lp_lbd + lp_zeta + lp_corr + additional_cov;
    
    return lp_total_m;
}

float P_lbd_zeta(float mass, float redshift, float lambda, float zeta, float eps) {
    /* Both mass and redshift will only work when/if
     a dataset is loaded into the file */
    float ln_lbd_pred = ln_lbd_given_M(mass, redshift);
    float ln_zeta_pred= ln_zeta_given_M(mass, redshift);
    float res = compute_log_pLbdZeta(lambda, zeta, ln_lbd_pred, ln_zeta_pred, eps);
    return exp(res);
}

float integrand(float lambda, float zeta, float mass, float xi,
                 float lambda_hat, float lambda_error, float redshift) {
    
    float res1 = P_lbd_zeta(mass, redshift, lambda, zeta, eps);
    float res2 = HMF(mass, redshift);
    float res3 = Prob_xi(xi,zeta * zeta + 3, 1);
    float res4 = Prob_lbd_hat(lambda_hat, lambda, lambda_error);
    return (res1) * (res2) * (res3) * (res4);
}


/* float prob_xi(double zeta, double xi) {
    double temp = (xi - sqrt(pow(zeta,2) + 2));
    double res = exp(pow(-temp, 2) / 2) / (sqrt(2 * M_PI));
    return res;
 }
 
 float log_likelihood = integrand(x, lambda, zeta, mass, xi, lambda_hat,
                                    lambda_error, redshift);
 */


int main(int argc, const char * argv[]) {
    // insert code here...
    return 0;
}

// The true values of theta are shown below for reference
// From (Bleem et al. 2019) & (S. Grandis et al. 2021)
float theta_true[9] = {5.24, 1.534, 0.465, 0.161, 76.9, 1.02, 0.29, 0.16, 0.8};


/* float lambda = 0;
float zeta = 0;
float mass = 0;
float xi = 0;
float lambda_hat = 0;
float lambda_error = 0;
float redshift  = 0;
float x = 0; */
