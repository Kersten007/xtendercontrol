

#include "scom_property.h"
#include "scom_data_link.h"
#include "tool.h"
#include "serial.h"
#include "obj.h"

#include <stdio.h>
#include <string.h>


bool obj_do_not_write = 0;

#define OBJEKTE_SIZE  37
const obj_t objekte[OBJEKTE_SIZE] =
{
    1107, T_FLOAT,"Maximum AC current \t1107 = %.2f A\n",
    1108, T_FLOAT,"Battery undervoltage  \t1108 = %.2f V\n",
    1109, T_FLOAT,"Battery undervo load  \t1109 = %.2f V\n",   
    1110, T_FLOAT,"Restart undervoltage  \t1110 = %.2f V\n",      
    1124, T_BOOL,"Inverter allowed   \t1124 = %x\n",
    1125, T_BOOL,"Charger allowed    \t1125 = %x\n", 
    1126, T_BOOL,"Boost allowed       \t1126 = %x\n", 
    1138, T_FLOAT,"Battery charge current \t1138 = %.2f A\n", 
            
    1140, T_FLOAT,"Floating Voltage  \t1140 = %.2f V\n",
    1143, T_FLOAT,"Voltage level 1   \t1143 = %.2f V\n",   
    1144, T_FLOAT,"Dauer Unterspannung \t1144 = %.2f min\n",  
    1155, T_BOOL,"Absorbtion allowed \t1155 = %x\n",
    1156, T_FLOAT,"Absorption Voltage  \t1156 = %.2f V\n",          
    1164, T_FLOAT,"Egalisierungs- Voltage \t1164 = %.2f V\n",        
        
    1187, T_FLOAT,"Standby level     \t1187 = %.2f %\n",  
    1296, T_BOOL,"Batteries priority \t1296 = %x\n",       
    1297, T_FLOAT,"Battery prio voltage \t1297 = %.2f V\n",
    1436, T_BOOL,"Overrun AC current limit \t1436 = %x\n", 

    1607, T_FLOAT, " Limitation of the power Boost \t1607 = %.2f %\n",

    3000, T_FLOAT,"Battery voltage    \t3000 = %.2f V\n",
    3005, T_FLOAT,"Charge current       \t3005 = %.1f A\n",    
    3011, T_FLOAT,"Input voltage AC \t3011 = %.f V\n",
    3012, T_FLOAT,"Input current AC \t3012 = %.2f A\n",
    3017, T_FLOAT,"Input Limit      \t3017 = %.2f A\n",
    3019, T_SHORT,"Boost active    \t3019 = %x\n",
    3020, T_SHORT,"State of transfer  \t3020 = %x\n",

    3021, T_FLOAT,"Output voltage AC \t3021 = %.f V\n",
    3022, T_FLOAT,"Output current AC \t3022 = %.2f A\n",  
    3030, T_SHORT,"State of output    \t3030 = %x\n",
    3031, T_SHORT,"State of auxiliary 1 \t3031 = %x\n",
    3049, T_SHORT,"Xtender On/off  \t3049 = %x\n",

    3136, T_FLOAT,"Output Power AC   \t3136 = %.f W\n",
    3137, T_FLOAT,"Input Power AC   \t3137 = %.f W\n",    
    3142, T_FLOAT,"System state machine \t3142 = %.1f\n",    
    3145, T_FLOAT,"Number overloads     \t3145 = %.1f\n",
    3147, T_FLOAT,"Number bat overvoltage \t3147 = %.1f\n",
    
    3168, T_SHORT,"Over temperature     \t3168 = %d\n",

};

int obj_find(int obj)
{
    for(int i=0; i<OBJEKTE_SIZE; i++)
    {
        if (objekte[i].obj == obj)
            return i;
    }
    return 0;
}

int obj_read_bool(int object_id)
{
    return (int)obj_read(object_id, "", T_BOOL);
}

int obj_u_read_bool(int object_id)
{
    return (int)obj_read(object_id  | _D_UNSAVED, "", T_BOOL);
}


int obj_read_short(int object_id)
{
    return (int)obj_read(object_id, "", T_SHORT);
}

float obj_read_float(int object_id)
{
    return obj_read(object_id, "", T_FLOAT);
}



float obj_read(int object_id, char * text, typ_t t)
{
    int obj = object_id & 0xFFFF;

    scom_frame_t frame;
    scom_property_t property;
    char buffer[1024];

    /* initialize the structures */
    scom_initialize_frame(&frame, buffer, sizeof(buffer));
    scom_initialize_property(&property, &frame);

    frame.src_addr = 1;     /* my address, could be anything */
    frame.dst_addr = 101;   /* the first inverter */
    property.object_id = obj;

    if((property.object_id >= 3000) && (property.object_id<=3200))
    {
        property.object_type = SCOM_USER_INFO_OBJECT_TYPE;
        property.property_id = 2;
    }
    else
    {
        property.object_type = SCOM_PARAMETER_OBJECT_TYPE; 
        if (object_id & _D_UNSAVED)
            property.property_id = _D_UNSAVED_VALUE_QSP;
        else
            property.property_id = _D_VALUE_QSP;         
    }

    scom_encode_read_property(&property);

    if(frame.last_error != SCOM_ERROR_NO_ERROR) {
        printf("obj_read property frame encoding failed with error 0x%x\n", (int) frame.last_error);
        return 9999.0;
    }

    /* do the exchange of frames */
    if(exchange_frame(&frame)!= SCOM_ERROR_NO_ERROR) {
        return 9999.0;
    }

    /* reuse the structure to save space */
    scom_initialize_property(&property, &frame);

    /* decode the read property service part */
    scom_decode_read_property(&property);
    if(frame.last_error != SCOM_ERROR_NO_ERROR) {
        printf("obj_read property decoding failed with error 0x%x\n", (int) frame.last_error);
        return 9999.0;
    }

    /* check the the size */
    int value_length = 0;
    if ((t == T_FLOAT) || (t == T_LONG))
    {
        value_length = 4;
    }
    else if (t == T_SHORT)
    {
        value_length = 2;
    }     
    else if (t == T_BOOL)
    {
        value_length = 1;
    }     
    if(property.value_length != value_length) {
        printf("obj_read invalid property data response size %d != %d \n", property.value_length, value_length);
        return 9999.0;
    }

    /* print the result */
    if (t == T_FLOAT)
    {
        float f = scom_read_le_float(property.value_buffer);
        printf(text, f);
        return f;        
    }
    else if (t == T_LONG)
    {
        long r = scom_read_le32(property.value_buffer) & 0xFFFFFFFF;
        printf(text, r);
        return r;
    }     
    else if (t == T_SHORT)
    {
        int r = scom_read_le16(property.value_buffer) & 0xFFFF;
        printf(text, r);
        return r;
    }     
    else if (t == T_BOOL)
    {
        int r = scom_read_le16(property.value_buffer) & 0x00FF;
        printf(text, r );
        return r;
    }   
    
    return 9999.0;
}


int obj_write_bool(int object_id, int data)
{
    return obj_write(object_id, data, T_BOOL);
}
int obj_u_write_bool(int object_id, int data)
{
    return obj_write(object_id | _D_UNSAVED, data, T_BOOL);
}

int obj_write_short(int object_id, int data)
{
    return obj_write(object_id, data, T_SHORT);
}
int obj_u_write_short(int object_id, int data)
{
    return obj_write(object_id | _D_UNSAVED, data, T_SHORT);
}

int obj_write_long(int object_id, long data)
{
    return obj_write(object_id, data, T_LONG);
}
int obj_u_write_long(int object_id, long data)
{
    return obj_write(object_id | _D_UNSAVED, data, T_LONG);
}

int obj_write_float(int object_id, float data)
{
    return obj_write(object_id, data, T_FLOAT);
}
int obj_u_write_float(int object_id, float data)
{
    return obj_write(object_id | _D_UNSAVED, data, T_FLOAT);
}


int obj_write(int object_id, float data, typ_t t)
{
    int obj = object_id & 0xFFFF;

    scom_frame_t frame;
    scom_property_t property;
    char buffer[1024];

    if(obj_do_not_write)
        return 0;

    /* initialize the structures */
    scom_initialize_frame(&frame, buffer, sizeof(buffer));
    scom_initialize_property(&property, &frame);

    frame.src_addr = 1;     /* my address, could be anything */
    frame.dst_addr = 101;   /* the first inverter */
    property.object_id = obj;

    if((property.object_id >= 3000) && (property.object_id<=3200))
    {
        property.object_type = SCOM_USER_INFO_OBJECT_TYPE;
        property.property_id = 2;
    }
    else
    {
        property.object_type = SCOM_PARAMETER_OBJECT_TYPE;
        if (object_id & _D_UNSAVED)
            property.property_id = _D_UNSAVED_VALUE_QSP;
        else
            property.property_id = _D_VALUE_QSP;  
    }

    if (t == T_FLOAT)
    {
        scom_write_le_float(property.value_buffer, data);        
        property.value_length = 4;
    }
    else if (t == T_LONG)
    {
        scom_write_le32(property.value_buffer, (long)data);
        property.value_length = 2;
    }      
    else if (t == T_SHORT)
    {
        int d = data;
        scom_write_le16(property.value_buffer, d);
        property.value_length = 2;
    }     
    else if (t == T_BOOL)
    {
        int d = data;
        scom_write_le16(property.value_buffer, d & 0xFF);
        property.value_length = 1;
    } 

    scom_encode_write_property(&property);

    if(frame.last_error != SCOM_ERROR_NO_ERROR) {
        printf("write property frame encoding failed with error 0x%x\n", (int) frame.last_error);
        return -1;
    }

    /* do the exchange of frames */
    if(exchange_frame(&frame)!= SCOM_ERROR_NO_ERROR) {
        printf("exchange Error\n");
        return -1;
    }


    //printf ("o:%d w:%d, r:%d\n",obj , data , obj_read_bool(obj));
    //printf ("o:%d w:%.2f, r:%.2f\n",obj , data , obj_read_float(obj));

    return 0;
}

scom_error_t exchange_frame(scom_frame_t* frame)
{
    size_t byte_count;

    serial_clear();

    scom_encode_request_frame(frame);

    if(frame->last_error != SCOM_ERROR_NO_ERROR) {
        printf("data link frame encoding failed with error 0x%x\n", (int) frame->last_error);
        return frame->last_error;
    }

    /* send the request on the com port */
    byte_count = serial_write(frame->buffer, scom_frame_length(frame));
    if(byte_count != scom_frame_length(frame)) {
        printf("error when writing to the com port\n");
        return SCOM_ERROR_STACK_PORT_WRITE_FAILED;
    }

    /* reuse the structure to save space */
    scom_initialize_frame(frame, frame->buffer, frame->buffer_size);

    /* clear the buffer for debug purpose */
    memset(frame->buffer,0, frame->buffer_size);

    /* Read the fixed size header.
       A platform specific handling of a timeout mechanism should be
       provided in case of the possible lack of response */

    byte_count = serial_read(&frame->buffer[0], SCOM_FRAME_HEADER_SIZE);

    if(byte_count != SCOM_FRAME_HEADER_SIZE) {
        printf("error when reading the header from the com port: Bytecount=0x%x\n", byte_count);
        return SCOM_ERROR_STACK_PORT_READ_FAILED;
    }

    /* decode the header */
    scom_decode_frame_header(frame);
    if(frame->last_error != SCOM_ERROR_NO_ERROR) {
        printf("data link header decoding failed with error %d\n", (int) frame->last_error);
        return frame->last_error;
    }

    /* read the data part */
    byte_count = serial_read(&frame->buffer[SCOM_FRAME_HEADER_SIZE],  scom_frame_length(frame) - SCOM_FRAME_HEADER_SIZE);
    if(byte_count != (scom_frame_length(frame) - SCOM_FRAME_HEADER_SIZE)) {
        printf("error when reading the data from the com port\n");
        return SCOM_ERROR_STACK_PORT_READ_FAILED;
    }

    /* decode the data */
    scom_decode_frame_data(frame);
    if(frame->last_error != SCOM_ERROR_NO_ERROR) {
        printf("data link data decoding failed with error %d\n", (int) frame->last_error);
        return frame->last_error;
    }

    return SCOM_ERROR_NO_ERROR;

}