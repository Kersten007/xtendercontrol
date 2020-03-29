#include <stdio.h>
#include <stdlib.h>
#include "scom_property.h"
#include "scom_data_link.h"
#include "serial.h"
#include "obj.h"
#include "tool.h"
#include "control.h"

/*
    thread anlegen

*/

void control_init()
{
    ;
}

void control_close()
{

    
}

void control_run()
{
    static int state = 0;

    switch (state)
    {
    case 0:
        printf ("controll_loop\n");
        state++;
        break;
    case 1:
        if(obj_read_short(3049)) //Xtender On/off
            state++;
    break;
    case 2:
        //obj_read_bool();
        if(obj_read_short(3049)) //Xtender On/off
            state++;
    break;    


    case 100:
        SmartBoostRegler();
    break;


    
    default:
        break;
    }

}


void SmartBoostRegler(void)
{
    char buf [2000];
    int offs = 0;
    static float f_Usoll = 13.8;
    static float f_Imem = 0;
    static float f_Omem = -1;
    float p_P = 10.0;
    float p_I = 0.8;
    float f_max = 50.0; //max 50%


    float f_Uist = obj_read_float(3000);
    float f_Uerr = f_Uist - f_Usoll;
    f_Imem = f_Imem + f_Uerr;
    if (f_Imem > (f_max/p_I)) f_Imem = (f_max/p_I); 
    if (f_Imem < 0) f_Imem = 0; //min 0%
    

    float f_out = (p_P * f_Uerr) + (p_I * f_Imem);

    if (f_out > f_max) f_out = f_max;
    if (f_out < 0) f_out = 0;
    


    f_out = (int)(((int)(f_out / 5)) * 5);



    if(f_Omem != f_out)
    {
        f_Omem = f_out;
        //obj_u_write_float(1607 | _D_UNSAVED | _D_NOREAD , f_Omem); // Limitation of the power Boost def=100% 

        //printf("\033[H\033[J");
        system("clear");
        offs = 0;
        offs += sprintf(&buf[offs], "\n\n");
        offs += sprintf(&buf[offs], "out=%.2f\terr=%.2f\tImem=%.2f\tUist=%.2f\n", f_Omem, f_Uerr, f_Imem, f_Uist);
        offs += sprintf(&buf[offs], "\n\n");
        printf(buf);

        //printf ("out=%.2f\terr=%.2f\tImem=%.2f\tUist=%.2f\n", f_Omem, f_Uerr, f_Imem, f_Uist);        
    }
}

