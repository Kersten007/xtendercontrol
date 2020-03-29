CC := gcc
CFLAGS := 
LDFLAGS := -lncurses

xtendercontrol: main.o control.o serial.o tool.o obj.o scom_property.o scom_data_link.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^ 

.PHONY: clean
clean: 
	rm *.o xtendercontrol
