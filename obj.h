
typedef enum {
    T_SHORT,
    T_FLOAT,
    T_BOOL,
    T_LONG
} typ_t;

typedef struct {
    int obj;
    typ_t type;
    char format[60];
} obj_t;

#define _D_VALUE_QSP 0x5
#define _D_LEVEL_QSP 0x8
#define _D_UNSAVED_VALUE_QSP 0xD

#define _D_UNSAVED 0x10000
#define _D_NOREAD  0x20000
#define _D_LEVEL   0x40000
#define _D_XCOM    0x80000

int obj_find(int obj);
float obj_read(int object_id, char * text, typ_t t);
int obj_write(int object_id, float data, typ_t t);
scom_error_t exchange_frame(scom_frame_t* frame);

int obj_read_bool(int object_id);
int obj_u_read_bool(int object_id);
int obj_read_short(int object_id);
float obj_read_float(int object_id);

int obj_write_bool(int object_id, int data);
int obj_u_write_bool(int object_id, int data); //unsaved
int obj_write_short(int object_id, int data);
int obj_u_write_short(int object_id, int data); //unsaved
int obj_write_long(int object_id, long data);
int obj_u_write_long(int object_id, long data); //unsaved
int obj_write_float(int object_id, float data);
int obj_u_write_float(int object_id, float data); //unsaved

