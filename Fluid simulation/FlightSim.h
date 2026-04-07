#pragma once
#ifndef FLIGHTSIM_H
#define FLIGHTSIM_H
#include <cmath>
#include <vector>
#include <iostream>
#include "eigen-3.4.1/Eigen/Dense"

struct Mq_9Coefficents
{
	//same speed same atlitude
	//angle of attack degrees		 	-4		0		4		8		12		16		20
	std::vector<double> ClAA	 = {-0.0323, 0.8888, 1.7661, 2.3706, 2.1060, 1.9047, 1.8456};
	std::vector<double> CdAA	 = {0.0633, 0.0679, 0.1209, 0.2142, 0.3745, 0.5187, 0.6968};
	//same altitude same angle of attack
	//mach num                   	0.27	0.3		0.33	0.36	0.39	0.42
	std::vector<double> ClSP	 = {0.9446, 0.8888, 0.8476, 0.8178, 0.7971, 0.7844};
	//

};
struct States
{
	double u,v,w; // Body velocities (m/s)
	double p,q,r; // Body angular rates (rad/s)
	double phi, theta, psi; // Euler angles (rad)
	double x,y,z; // earth positions (m) -z indicates higher alititude
};

class Flightsim
{
public:
	States state;// [u, v, w] [p, q, r] [phi, theta, psi] [x, y, z]
	double pi = 3.1415926535897932384626433832795;
	Eigen::Vector4d controls; //plane control states alerion, tail, rudder, throttle.
	double mass = 3090; //plane starting mass (may make this change as fuel is used up. Maybe) in KG
	double S = 20.9; //wing area in M^2
	double St = 8; // tail area M^2
	double C_bar = 1.476; //mean aerodynamic chord M
	double rho = 0.4135; //air density at 0KM Kg/M^3
	double g = 9.80665; // gravity on earth
	double xcg = 0;
	double ycg = 0 * C_bar;
	double zcg = 0.109146;
	double xac = -0.12 * C_bar;
	double yac = 0.0 * C_bar;
	double zac = 0.0 * C_bar;
	double zliftAOA = -3*pi/180;
	double depsda = 0.25;
	double xapt1 = -5,yapt1 = 0, zapt1 = -0.2; // in m

    float wind_bias = 0;

    double phi1=0, theta1=0, psi1=0;

	Eigen::Matrix3d J;          // Inertia Tensor Matrix
	Eigen::Matrix3d J_inv;      // Pre-calculated inverse of J for speed

	Flightsim()
	{
		// Initialize States (Cruise speed, level flight)
		state.u = 90.000;
		state.v = 0;
		state.w = 1.390;
		state.p = 0;
		state.q = 0;
		state.r = 0;
		state.phi = 0;
		state.theta = 0.885 * (pi/180.0);
		state.psi = 0;
		state.x = 0;
		state.y = 0;
		state.z = -10000;
		controls << 0, 0, 0, 0; //controls which are all 0 including thrust(might change this later),

		// Values are estimates for a 3400lb GA aircraft
		J << 2000.0, 0.0, -100.0,
			0.0, 3000.0, 0.0,
			-100.0, 0.0, 4000.0;
		J_inv = J.inverse();
	}
	void UpdateSim(double dt)
	{
		limit_checks();

        // Pack the struct into a 12x1 Eigen Vector for clean RK4 math
        Eigen::Matrix<double, 12, 1> StateVec;
        StateVec << state.u, state.v, state.w,
            state.p, state.q, state.r,
            state.phi, state.theta, state.psi,
            state.x, state.y, state.z;

        // Perform 4th-Order Runge-Kutta Integration
        Eigen::Matrix<double, 12, 1> k1 = get_derivatives(StateVec, controls);
        Eigen::Matrix<double, 12, 1> k2 = get_derivatives(StateVec + (dt / 2.0) * k1, controls);
        Eigen::Matrix<double, 12, 1> k3 = get_derivatives(StateVec + (dt / 2.0) * k2, controls);
        Eigen::Matrix<double, 12, 1> k4 = get_derivatives(StateVec + dt * k3, controls);

        StateVec += (dt / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);// 4. Wrap Euler angles between -PI and PI so they don't count infinitely
        

        // Unpack back to the struct so UI/Graphing tools can read it better
        state.u = StateVec(0); state.v = StateVec(1); state.w = StateVec(2);
        state.p = StateVec(3); state.q = StateVec(4); state.r = StateVec(5);
        state.phi = wrap_angle_pi(StateVec(6));

        state.theta = StateVec(7);
        if (state.theta > pi/2) state.theta = pi/2 - 0.001;
        if (state.theta < -pi/2) state.theta = -pi/2 + 0.001;

        state.psi = wrap_angle_pi(StateVec(8));
        state.x = StateVec(9); state.y = StateVec(10); state.z = StateVec(11);
       

	}

private:
    double wrap_angle_pi(double angle)
    {
        double wrapped = std::fmod(angle + pi, 2.0 * pi);
        if (wrapped < 0.0)
        {
            wrapped += 2.0 * pi;
        }
        return wrapped - pi;
    }

	void limit_checks()
	{
		double elavator_limit = 15 * pi / 180;
		if (controls[1] > elavator_limit)
		{
			controls[1] = elavator_limit;
		}
		else
		{
			if (controls[1] < -elavator_limit)
			{
				controls[1] = -elavator_limit;
			}
		}
		double aleron_roll_limit = 20 * pi / 180;
		if (controls[0] > aleron_roll_limit)
		{
			controls[0] = aleron_roll_limit;
		}
		else
		{
			if (controls[0] < -aleron_roll_limit)
			{
				controls[0] = -aleron_roll_limit;
			}
		}
		double yaw_roll_limit = 10 * pi / 180;
		if (controls[2] > yaw_roll_limit)
		{
			controls[2] = yaw_roll_limit;
		}
		else
		{
			if (controls[2] < -yaw_roll_limit)
			{
				controls[2] = -yaw_roll_limit;
			}
		}
		double thrust_limit = 5;
		if (controls[3] > thrust_limit)
		{
			controls[3] = thrust_limit;
		}
		else
		{
			if (controls[3] < 0)
			{
				controls[3] = 0;
			}
		}

	}
    Eigen::Matrix<double, 12, 1> get_derivatives(Eigen::Matrix<double, 12, 1> statevec, Eigen::Vector4d ctrl)
    {
        double airspeed = sqrt(statevec[0] * statevec[0] + statevec[1] * statevec[1] + statevec[2] * statevec[2]);
        if (airspeed < 0.1) airspeed = 0.1; // Division by zero protection

        double alpha = atan2(statevec[2], statevec[0]) + wind_bias;
        double beta = asin(statevec[1] / airspeed);
        double Q = 0.5 * rho * airspeed * airspeed;

        double effective_alpha = alpha;
        //if (effective_alpha > (20.0 * pi / 180.0)) effective_alpha = 20.0 * pi / 180.0;
        //if (effective_alpha < -(15.0 * pi / 180.0)) effective_alpha = -(15.0 * pi / 180.0);

        double Cl_wb;
        Eigen::Vector3d v_b;
        v_b << statevec[0], statevec[1], statevec[2];

        //Use effective_alpha for all aerodynamic forces
        if (effective_alpha <= (25 * pi / 180)) {
            Cl_wb = 12.88009 * (effective_alpha - zliftAOA);
        }
        else {
            Cl_wb = (-(effective_alpha * effective_alpha * effective_alpha) + (effective_alpha * effective_alpha) + effective_alpha + 4.402697);
        }

        double epsilon = depsda * (alpha - zliftAOA);
        double alpha_t = alpha - epsilon + ctrl[1] + (1.3 * statevec[4] * 6.2 / airspeed);
        double CL_t = 3.1 * (St / S) * alpha_t;
        double CL = Cl_wb + CL_t;

        //Quadratic drag polar prevents negative drag
        double CD = 0.063 + 2.06228 * (alpha * alpha);
        if (CD < 0.063) CD = 0.063;

        double Cy = -1.6 * beta + 0.24 * ctrl[2];

        Eigen::Vector3d Fa_s;
        Fa_s << -CD * Q * S, Cy* Q* S, -CL * Q * S;

        Eigen::Matrix3d C_bs;
        C_bs << cos(alpha), 0, -sin(alpha),
            0, 1, 0,
            sin(alpha), 0, cos(alpha);

        Eigen::Vector3d Fa_b = C_bs * Fa_s;

        double eta1l = -1.4 * beta;
        double eta2l = 0.05 - (3.1 * (St * 6.2) / (S * C_bar)) * (alpha - epsilon);
        double eta3l = (1 - alpha * (180 / (15 * pi))) * beta;

        Eigen::Vector3d eta;
        eta << eta1l, eta2l, eta3l;

        Eigen::Matrix3d num;
        num << -11, 0, 5,
            0, (-4.03 * (St * 6.2 * 6.2) / (S * C_bar * C_bar)), 0,
            1.7, 0, -11.5;

        Eigen::Matrix3d dcMdx = (C_bar / airspeed) * num;

        Eigen::Matrix3d dcMdu;
        dcMdu << -0.6, 0, 0.22,
            0, (-3.1 * (St * 6.2) / (S * C_bar)), 0,
            0, 0, -0.63;

        Eigen::Vector3d num2;
        num2 << ctrl[0], ctrl[1], ctrl[2];

        Eigen::Vector3d wbe_b;
        wbe_b << statevec[3], statevec[4], statevec[5];

        Eigen::Vector3d Cmac_b = eta + dcMdx * wbe_b + dcMdu * num2;
        Eigen::Vector3d maac_b = Cmac_b * S * Q * C_bar;

        Eigen::Vector3d rcgb;
        rcgb << xcg, ycg, zcg;

        Eigen::Vector3d racb;
        racb << xac, yac, zac;

        Eigen::Vector3d macg_b = maac_b + Fa_b.cross(rcgb - racb);

        double thrust = ctrl[3] * 2234;
        Eigen::Vector3d Fe_b;
        Fe_b << thrust, 0, 0;

        Eigen::Vector3d mew1;
        mew1 << (xcg - xapt1), (yapt1 - yac), (zcg - zapt1);

        Eigen::Vector3d mecg1_b = mew1.cross(Fe_b);

        Eigen::Vector3d g_b;
        // Z-axis gravity must use cos(phi)
        g_b << (-g * sin(statevec[7])),
            (g * cos(statevec[7]) * sin(statevec[6])),
            (g * cos(statevec[7]) * cos(statevec[6]));

        Eigen::Vector3d Fg_b = mass * g_b;

        Eigen::Vector3d F_b = Fg_b + Fe_b + Fa_b;
        Eigen::Vector3d x1to3dot = ((1 / mass) * F_b) - (wbe_b.cross(v_b));

        Eigen::Vector3d Mcg_b = macg_b + mecg1_b;
        Eigen::Vector3d x4to6dot = J_inv * (Mcg_b - (wbe_b.cross(J * wbe_b)));

        double safe_theta = statevec[7];
        if (safe_theta > 1.56) safe_theta = 1.56;
        if (safe_theta < -1.56) safe_theta = -1.56;

        Eigen::Matrix3d H_phi;

        // Bottom row denominators must be cos(safe_theta)
        H_phi << 1.0, sin(statevec[6])* tan(safe_theta), cos(statevec[6])* tan(safe_theta),
            0.0, cos(statevec[6]), -sin(statevec[6]),
            0.0, sin(statevec[6]) / cos(safe_theta), cos(statevec[6]) / cos(safe_theta);

        Eigen::Vector3d x7to9dot = H_phi * wbe_b;

        // Navigation Integration
        Eigen::Matrix3d DCM;
        DCM << cos(statevec[7]) * cos(statevec[8]), sin(statevec[6])* sin(statevec[7])* cos(statevec[8]) - cos(statevec[6]) * sin(statevec[8]), cos(statevec[6])* sin(statevec[7])* cos(statevec[8]) + sin(statevec[6]) * sin(statevec[8]),
            cos(statevec[7])* sin(statevec[8]), sin(statevec[6])* sin(statevec[7])* sin(statevec[8]) + cos(statevec[6]) * cos(statevec[8]), cos(statevec[6])* sin(statevec[7])* sin(statevec[8]) - sin(statevec[6]) * cos(statevec[8]),
            -sin(statevec[7]), sin(statevec[6])* cos(statevec[7]), cos(statevec[6])* cos(statevec[7]);

        Eigen::Vector3d pos_dot = DCM * v_b;

        Eigen::Matrix<double, 12, 1> newvals;
        newvals << x1to3dot[0], x1to3dot[1], x1to3dot[2],
            x4to6dot[0], x4to6dot[1], x4to6dot[2],
            x7to9dot[0], x7to9dot[1], x7to9dot[2],
            pos_dot[0], pos_dot[1], pos_dot[2];

        return newvals;
    }
};


#endif