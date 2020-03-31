typedef enum {
    S__DO_NOTHING,
    S__WAIT_FOR_ON,
    S__INIT_LOOP,
    S__ON,
    S__RUN_LOOP,
    S__STOP_LOOP,
    S__WAIT_TO_START,
    S__POLL_TO_START,
    S__TIMER,
    S__WAIT_FOR_BOOST,
} t_control_state;


//public
void control_init();
void control_close();
void control_run(void);
void SmartBoostRegler(void);
void watch_system(void);


