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

    obj_read(3000, "3000 = %.2f\n", T_FLOAT);

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