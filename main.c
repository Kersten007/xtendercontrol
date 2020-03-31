#include <stdio.h>
#include <string.h>
//#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

#include "scom_property.h"
#include "scom_data_link.h"
#include "serial.h"
#include "main.h"
#include "control.h"
#include "tool.h"
#include "obj.h"


int main()
{
    intit_all();
    sleep(1);


    //obj_u_write_bool(1124, 1); //Inverter allowed
    //printf ("1124 = %d\n", obj_read_bool(1124));
    //printf ("u 1124 = %.f\n", obj_u_read_bool(1124));


    //obj_u_write_bool(1125, 0); //Charger allowed
    //printf ("1125 = %d\n", obj_u_read_bool(1125));    

    //printf ("3049 = %d\n", obj_read_short(3049)); 
    

    //return 0;

    while (1) //main loop
    {
        tool_timer();
        control_run();
    }

    close_all();
    
    return 0;
}


void intit_all()
{

    serial_init();
    control_init();
}

void close_all()
{

    serial_close();
    control_close();
}