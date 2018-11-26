#include<stdio.h>

int main(){

int R1 = 1816, R2 = 750, R = 1000, Rmax = 0, i = 0;  
float i1, i2, Vd_anterior = 0, Vx, Vy, tensao = 5, Vot1, Vot2;


while(i<1200 && R < 2000)
{

	i1 = tensao/(R1 + R);

	i2 = tensao/(R2 + R);

	Vx = i1*R1;

	Vy = i2*R2;

	if((Vx - Vy) > Vd_anterior)
	{
		Vd_anterior = Vx - Vy;
		Vot1 = Vx;
		Vot2 = Vy;
		Rmax = R;
	}
	
	printf("Vmax: %f Vatual: %f Rmax: %d V1: %f V2: %f R: %d Vx: %f Vy: %f \n", Vd_anterior, (Vx-Vy), Rmax, Vot1, Vot2, R, Vx, Vy);

	R++;
	i++;

}

	return 0;
}

