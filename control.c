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
int control_delay = 0;
int control_next = S__WAIT_FOR_ON;
float f_U_Bat_ist = 0;

void control_init()
{
    ;
}

void control_close()
{

}

void control_set_state(int state, int delay, char * text)
{
    control_next = state;
    if(delay)
    {
        control_state = S__TIMER;
        control_delay = delay;
        if(do_debug)
            printf("Set State %d - %s - delay: %d\n", control_state, text, delay);  
    }
    else
    {
        control_state = control_next;
        if(do_debug)
            printf("Set State %d - %s\n", control_state, text);        
    }
}

void control_debug(char * text)
{
    if(do_debug)
        printf(text);
}

void control_run()
{
    static int control_timeout = 0;

    switch (control_state)
    {
    case S__DO_NOTHING:
    break;
    case S__TIMER:
        watch_system();
        if(control_delay-- <= 0)
            control_set_state(control_next, 0, "");
    break;
    case S__WAIT_FOR_ON:
        if(obj_read_short(3049)) //Xtender On/off
        {
            obj_read(3000, "Bat Voltage is = %.2f\n", T_FLOAT);
            obj_u_write_float(1607 | _D_UNSAVED | _D_NOREAD , 5.0); // Limitation of the power Boost def=100%          
            control_set_state(S__WAIT_TO_START, 50, "Xtender is ON");
        }
    break;
    case S__POLL_TO_START:
        watch_system();
        if (f_U_Bat_ist > 14.1)
        {
            printf("f_U_Bat_ist = %.2f", f_U_Bat_ist);
            control_set_state(S__WAIT_TO_START, 0, "S__WAIT_TO_START");
            control_timeout = 2000; //2000*10ms = 20s           
        }
    break;
    case S__WAIT_TO_START:
        watch_system();
        
        if (f_U_Bat_ist < 13.9)
        {
            printf("f_U_Bat_ist = %.2f", f_U_Bat_ist);
            control_set_state(S__POLL_TO_START, 0, "S__POLL_TO_START");            
        }

        if(control_timeout-- <= 0)
            control_set_state(S__INIT_LOOP, 0, "S__INIT_LOOP");        
    break;
    case S__INIT_LOOP:
        //obj_u_write_bool(1124, 1); //Inverter allowed
        //obj_u_write_bool(1125, 1); //Charger allowed
        obj_u_write_bool(1126, 1); //Boost allowed
        //obj_u_write_bool(1128, 1); //Transfer allowed
        //obj_u_write_bool(1296, 1); //Batteries priority
        obj_u_write_bool(1538, 0); //Prohibits Transfer
        //obj_u_write_bool(1578, 0); //Fernsteuereingang Activated by AUX 1 state, 1=on
        //obj_u_write_float(1297, 14.2); //Battery prio voltage def=12.9V
        control_set_state(S__WAIT_FOR_BOOST, 0, "S__WAIT_FOR_BOOST");
        control_timeout = 2000;
    break;
    case S__WAIT_FOR_BOOST:
        if(obj_read_short(3019)) //Boost active 
        {
            control_set_state(S__RUN_LOOP, 0, "S__RUN_LOOP");
        }
        if(control_timeout-- <= 0)
        {
            control_set_state(S__STOP_LOOP, 0, "S__STOP_LOOP - Timeout Boost active");            
        }
    break;
    case S__RUN_LOOP:
        watch_system();
        SmartBoostRegler();
    break;

    case S__STOP_LOOP:
        //obj_u_write_bool(1125, 0); //Charger allowed
        obj_u_write_bool(1126, 0); //Boost allowed
        obj_u_write_bool(1538, 1); //Prohibits Transfer
        //obj_u_write_bool(1578, 1); //Fernsteuereingang Activated by AUX 1 state, 1=on

        //obj_u_write_bool(1395, 1); //flash reload
        control_set_state(S__DO_NOTHING, 0, "S__DO_NOTHING");
    break;
    
    default:
        break;
    }

}


void watch_system(void)
{
    static int counter = 50;

    f_U_Bat_ist = obj_read_float(3000);

    if (f_U_Bat_ist < 12.2)
        control_set_state(S__STOP_LOOP, 0, "S__STOP_LOOP - VBat < 12.2 V");

    if(counter-- < 0)
    {
        counter = 50;
        if(obj_read_short(3030) == 0) //State of Output
        {
            control_set_state(S__STOP_LOOP, 0, "S__STOP_LOOP - State Output = 0");
        }
    }
}

void SmartBoostRegler(void)
{
    static int counter = 10;
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
        obj_u_write_float(1607 | _D_UNSAVED | _D_NOREAD , f_Omem); // Limitation of the power Boost def=100% 
    }

    offs = 0;
    offs += sprintf(&buf[offs], "\n\n");
    offs += sprintf(&buf[offs], "out=%.2f\terr=%.2f\tImem=%.2f\tUist=%.2f\n", f_Omem, f_Uerr, f_Imem, f_U_Bat_ist);
    
    static float f_3137 = 0, f_3136 = 0, f_3005 = 0, f_3017 = 0;
    if(counter-- <=0)
    {
        counter = 10;
        f_3137 = obj_read_float(3137);
        f_3136 = obj_read_float(3136);
    }
    else if (counter == 5)
    {
        f_3005 = obj_read_float(3005);
        f_3017 = obj_read_float(3017);         
    }

    offs += sprintf(&buf[offs], "Input Power AC   \t3137 = %.f W\n", 1000 * f_3137);
    offs += sprintf(&buf[offs], "Output Power AC  \t3136 = %.f W\n" , 1000 * f_3136);
    offs += sprintf(&buf[offs], "Charge current   \t3005 = %.1f A\n", f_3005);
    offs += sprintf(&buf[offs], "Input Limit      \t3017 = %.2f A\n", f_3017);   
    offs += sprintf(&buf[offs], "\n\n");

    system("clear");        
    printf(buf);

        //printf ("out=%.2f\terr=%.2f\tImem=%.2f\tUist=%.2f\n", f_Omem, f_Uerr, f_Imem, f_Uist);        

}

