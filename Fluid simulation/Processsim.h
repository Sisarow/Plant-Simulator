#pragma once
#ifndef PROCESSSIM_H
#define PROCESSSIM_H
#include <cmath>
#include <vector>
#include <iostream>

class Processsim
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
			default:
				printf("Please enter another number.\n");
				break;
			}
		}
	}


	void Update(double Time_step)// updates process and uses set time step such as 1 sec or milisec etc.
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
			localPv = (differance) - (differance) * exp((-Time_step/T1)) + localPv;
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


		if ((oldPv.size()) >= deadtimesize && Td != 0)// do deadtime if its active
		{
			oldPv.push_back(localPv);
			Pv = oldPv[0];
			oldPv.erase(oldPv.begin());
		}
		else //default if no deadtime.
		{
			Pv = localPv;
		}
		
	}

private:

};
#endif