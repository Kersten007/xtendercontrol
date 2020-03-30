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

bool do_debug = 1;
int control_state = S__WAIT_FOR_ON;
int control_timer = 0;
int control_timeout = 0;
int control_next = S__WAIT_FOR_ON;
float f_U_Bat_ist = 0;

void control_init()
{
    ;
}

void control_close()
{

}

void control_set_state(int state, int timer)
{
    control_next = state;
    if(timer)
    {
        control_state = S__TIMER;
        control_timer = timer;       
    }
    else
    {
        control_state = control_next;
        if(do_debug)
            printf("Set State %d\n", control_state);        
    }
}

void control_run()
{
    switch (control_state)
    {
    case S__DO_NOTHING:
    break;
    case S__TIMER:
        watch_system();
        if(control_timer-- <= 0)
            control_set_state(control_next, 0);
    break;
    case S__WAIT_FOR_ON:
        if(obj_read_short(3049)) //Xtender On/off
            control_set_state(S__WAIT_TO_START, 10);
    break;
    case S__POLL_TO_START:
        watch_system();
        if (f_U_Bat_ist > 14.1)
        {
            control_set_state(S__WAIT_TO_START, 0);
            control_timeout = 2000; //2000*10ms = 20s           
        }
    break;
    case S__WAIT_TO_START:
        watch_system();
        
        if (f_U_Bat_ist < 14.0)
            control_set_state(S__POLL_TO_START, 0);

        if(control_timeout-- < 0)
            control_set_state(S__INIT_LOOP, 0);        
    break;
    case S__INIT_LOOP:
        obj_u_write_bool(1124, 1); //Inverter allowed
        obj_u_write_bool(1125, 1); //Charger allowed
        obj_u_write_bool(1126, 1); //Boost allowed
        obj_u_write_bool(1128, 1); //Transfer allowed
        obj_u_write_bool(1296, 1); //Batteries priority
        obj_u_write_bool(1538, 0); //Prohibits Transfer
        obj_u_write_bool(1578, 0); //Fernsteuereingang Activated by AUX 1 state, 1=on
        obj_u_write_float(1297, 14.2); //Battery prio voltage def=12.9V
        control_set_state(S__RUN_LOOP, 0);
    break;
    case S__RUN_LOOP:
        watch_system();
        SmartBoostRegler();
    break;

    case S__STOP_LOOP:
        obj_u_write_bool(1125, 0); //Charger allowed
        obj_u_write_bool(1126, 0); //Boost allowed
        obj_u_write_bool(1538, 1); //Prohibits Transfer
        obj_u_write_bool(1578, 1); //Fernsteuereingang Activated by AUX 1 state, 1=on
        control_set_state(S__DO_NOTHING, 0);
    break;
    
    default:
        break;
    }

}


void watch_system(void)
{
    f_U_Bat_ist = obj_read_float(3000);

    if (f_U_Bat_ist > 14.1)
        control_set_state(S__INIT_LOOP, 50);

    if (f_U_Bat_ist < 12.5)
        control_set_state(S__STOP_LOOP, 0);
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


    float f_Uerr = f_U_Bat_ist - f_Usoll;
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

