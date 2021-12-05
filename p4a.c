//gcc -Wall v3wiring.c -o v3wiring.out -lwiringPi

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>


int main (void)
{
    int fd;
    int8_t bus=0;
    
    wiringPiSetup();/*Inicializamos los pines de salida y entrada SDA y SCL*/
    fd = wiringPiI2CSetup(0x20);/*Nos conectamos a la dirección del esclavo la 0x20*/
    printf("I2C Init");
    
        wiringPiI2CWrite(fd,0xc);  /*Mandamos una C al buffer*/
	delay(15);
	bus=wiringPiI2CRead(fd);  /*Leemos el buffer*/ 
	printf("\n%d \n", bus);
	
    return 0;
}

