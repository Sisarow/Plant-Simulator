#pragma once
#ifndef PROCESSSIM_H
#define PROCESSSIM_H
#include <cmath>
#include <vector>
#include <iostream>
#include <random>
#include "FlightSim.h" //processsim contrains flight simulator

class Processsim : public Flightsim
{
public:
	char process;//process type.
	float Co = 0, Pv = 0, Sp = 0, PrevCo = 0, PrevPv = 0, PrevSp = 0;
	float localPv = 0;
	float k = 0, T1 = 0, Td = 0;
	int deadtimesize = 0;
	std::vector<float> oldPv;
	float change = 0;
	bool deadtimechanged = true;

	bool FLIGHTSIM = false; //determines wether to use the flight sim instead. everything in this block is for the air sim.
	double throttle = 1;

	//noise stuffs
	bool Noise = false; //enables or disables noise
	float noisemax_min = 5; //amplitude of noise
	double valuen = 0; //
	float time = 0;
	int valuend = -1;
	float desired_time = 1;
	bool stale = false;

	Processsim()//contructor to initialize process
	{
		bool startup = true;
		printf("please enter a number \n(1 = 1st order(can have deadtime if desired) | 2 = Intergrating(can have deadtime if desired))\n");
		while (startup)
		{
			std::cin >> process; // get input from console
			switch (process)
			{
			case '1':
				k = 1;
				T1 = 1;
				Td = 0;
				startup = false;
				break;
			case '2':
				k = 0;
				T1 = 1;
				Td = 0;
				startup = false;
				break;
			case '3':
				FLIGHTSIM = true;
				startup = false;
				break;
			default:
				printf("Please enter another number.\n");
				break;
			}
		}
	}


	void Update(double Time_step)// updates process and uses set time step such as 1 sec or milisec etc.
	{
		if (FLIGHTSIM) // only run the flight simulator
		{	
			controls[1] = ((Co / 100) * (4)) + -2;
			controls[3] = throttle;
			UpdateSim(Time_step);
			Pv = (-state.z / 20000) * 100;
		}
		else
		{
			if (deadtimechanged) //update the deadtime list which is used to delay the output.
			{
				deadtimesize = (int)(Td * 60 * (1 / Time_step)); //find size of delay list.
				oldPv.clear();
				oldPv.resize(deadtimesize, localPv);//resize to an int and round might be slightly off... could be a problem for accountablity of project.
				deadtimechanged = false;
			}
			Time_step = Time_step / 60.0f;// timestep must be in minutes

			float differance = ((Co * k) - localPv);
			
			if (process == '1')//first order and/or deadtime
			{
				if (k < 0) // for -k values the equation will change
				{
					localPv = (differance + 100) - ((differance + 100) * exp((-Time_step / T1))) + localPv;
				}
				else
				{
					localPv = (differance)-((differance)*exp((-Time_step / T1))) + localPv;
				}
			}
			if (process == '2') // integrating plant and/or deadtime.
			{
				if (Co != PrevCo)
				{
					change = Co - PrevCo;
					PrevCo = Co;
				}
				localPv += (change / T1) * Time_step;
			}


			if (Noise && valuen == 0 && stale)
			{
				std::random_device noise;
				std::mt19937 gen(noise());
				std::uniform_int_distribution<> dis(10000, 50000); //noise detial generated
				int value = dis(gen);
				desired_time = value / 10000; // time between noise is either 1 or 5 seconds
				stale = false;
			}
			if (Noise && time > desired_time)
			{
				std::random_device noise;
				std::mt19937 gen(noise());
				std::uniform_int_distribution<> dis(0, 100000); //noise detial generated
				int value = dis(gen);
				value = value - 50000; // either -50000 or + 500000
				valuen = (double)(value / (double)50000); //normalize value between 0 and 1;
				valuen *= noisemax_min; // scale to max/min noise value
				time = 0;
				stale = true;
			}
			time += Time_step * 60; //revert time to seconds


			if ((oldPv.size()) >= deadtimesize && Td != 0)// do deadtime if its active
			{
				if ((localPv + valuen) > 100)
				{
					oldPv.push_back(100);
				}
				else if ((localPv + valuen) < 0)
				{
					oldPv.push_back(0);
				}
				else
				{
					oldPv.push_back(localPv + valuen);
				}

				Pv = oldPv[0];
				oldPv.erase(oldPv.begin());
			}
			else //default if no deadtime.
			{
				if ((localPv + valuen) > 100)
				{
					Pv = 100;
				}
				else if ((localPv + valuen) < 0)
				{
					Pv = 0;
				}
				else
				{
					Pv = localPv + valuen;
				}

			}

			if (Noise && valuen > 0 && valuend < -1) //handles the duration of the noise created
			{
				std::random_device noise;
				std::mt19937 gen(noise());
				std::uniform_int_distribution<> dis(1000, 20000); //noise detial generated
				valuend = dis(gen) / 100;
			}
			if (valuend > 0) //remove added noise once value is 0
			{
				valuend--;
			}
			else
			{
				valuen = 0; // reset noise signal after duration is over
				valuend--; //used to make sure program does not estimate a new duration.
			}
		}
	}

private:

};
#endif