#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>

#define CONNECT_PORT 8000 //Port used for the next connection
#define BACKLOG      10   //Limit of pending connections.
#define MAXSOCKETS   10   //Limit of sockets.
#define TIMETOSLEEP  3   //Time in seconds that pthread "SendMessages" will sleep.
#define pwm_1 25U

//#define TRUE 1
//#define FALSE 0

    int fd;
    int8_t bus=0;
//Global variables used by ptheads
int listSocket[MAXSOCKETS]={0};
int socketIndex;
pthread_mutex_t socketMutex = PTHREAD_MUTEX_INITIALIZER;
int newSocketMember = 0;
static int time_high = 0;
static int cycle = 200U;
typedef struct{
    int8_t SOF;
    int8_t SEN;
    int8_t DT;
    int8_t CS;

}bufferIn;
typedef struct{
    int8_t SOF;
    int8_t SEN;
    int8_t DT;
    int8_t CS;
}bufferOut;
int8_t valacel=0;
char buff[]="AA";
    bufferIn *pclientePetision, clientePetision; /*buffer de los datos recividos desde el cliente apuntador y estructura*/
    bufferOut *pservidorRespuesta, servidorRespuesta;/*Buffer de datos enviados hacia el cliente*/
void *SendMessages (void *ptr);
void clientes(void);
void i2ctransfer (void);
void pwm(int8_t valacel);
int main (int argc, char *argv[])
  {
    
	servidorRespuesta.SOF=0XAA;
	servidorRespuesta.SEN=0X01;
	servidorRespuesta.DT=bus;
        servidorRespuesta.CS=~(servidorRespuesta.SOF+servidorRespuesta.SEN+servidorRespuesta.DT);
	wiringPiSetup(); 
        pinMode(pwm_1, SOFT_PWM_OUTPUT);
        softPwmCreate(pwm_1,time_high,cycle);
   
  while(1){
	
 	
	i2ctransfer();
	printf("Pedí el valor del acelerometro\n");
	servidorRespuesta.DT=bus;
	//servidorRespuesta.DT=bus;
        
	servidorRespuesta.CS=~(servidorRespuesta.SOF+servidorRespuesta.SEN+servidorRespuesta.DT); 
        clientes();
	
	printf("Mandé el dato al cliente\n");
	printf("%d", bus);
      }


    return 0;
}
void pwm(int8_t valacel){

        valacel=(valacel*17)/100;
        if(valacel < 7){
        valacel=7;
        }
        softPwmWrite(pwm_1, valacel);
}

void i2ctransfer (void)
{
    
   /*Inicializamos los pines de salida y entrada SDA y SCL*/
    fd = wiringPiI2CSetup(0x20);/*Nos conectamos a la dirección del esclavo la 0x20*/
    printf("I2C Init");
    
        wiringPiI2CWrite(fd,0xc);  /*Mandamos una C al buffer*/
	delay(15);
	bus=wiringPiI2CRead(fd);  /*Leemos el buffer*/ 
	printf("\n%d \n", bus);
	
  
}


void clientes(void){



    //Variable declaration
    //bufferIn *pclientePetision, clientePetision; /*buffer de los datos recividos desde el cliente apuntador y estructura*/
    //bufferOut *pservidorRespuesta, servidorRespuesta;/*Buffer de datos enviados hacia el cliente*/

    int sockFdListen;
    int newScoketConnectionFD;
    struct sockaddr_in myAddress;
    struct sockaddr_in theirAddress;
    socklen_t sinSize;
    int iPthRc;
    pthread_t pthSendMessages;
    int iWriteHistRc;
    

	//char *message="Hello from server";

  pclientePetision = &clientePetision;
  pservidorRespuesta = &servidorRespuesta;
 	clientePetision.SOF = 0xAA;
 	servidorRespuesta.SOF = 0xAA;
       	
    

    //This will create the socket that will listen for new connections

    if ((sockFdListen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	  printf("There was an error trying to get a socket, errno:%d\n",errno);
	  exit(1);
	}

    //Now, it is necessary to fill the myAddress with tne necessary
    //information for the connection.
    myAddress.sin_family = AF_INET; //host by order
    myAddress.sin_port = htons(CONNECT_PORT); //short, network byre order
    myAddress.sin_addr.s_addr = INADDR_ANY; //auto-fill with my IP
    memset(&(myAddress.sin_zero), 8, 1); //it is necessary to clean the rest of the structure.
    
    //Lets now bind the socket to assign an address to it.
    if (bind(sockFdListen, (struct sockaddr *)&myAddress, sizeof(struct sockaddr)) < 0)
      {
        printf("There was an error when trying to bind the listening socket, errno:%d\n",errno);
        exit(1);
      }
    
    //Now, the socket must be wait and listen for new connections.
    if ((listen(sockFdListen, BACKLOG)) < 0)
      {
        printf("There was an error traying to listen in the socket, errno:%d\n",errno);
        exit(1);
      }
    
    //It is necessary to create the pthread that will send the messages
    //to the connected sockets every 5 seconds.
    iPthRc = pthread_create(&pthSendMessages, NULL, &SendMessages, NULL);
    if (iPthRc < 0)
	{
		printf("There was an error trying to create the pthread, iPthRc:%d\n", iPthRc);
		exit(1);
	}
    
    sinSize = sizeof(struct sockaddr_in);
	socketIndex=0;
    //From here we have to handle every next connection.
    while (TRUE)
	{
        //This will make the thread to sleep until a new connection
        //comes and then we will send the conection to a new thread
        //to be handle.
        printf("I am the principal deamon listening at port: %d\n", CONNECT_PORT);

        newScoketConnectionFD = accept(sockFdListen, 
                                       (struct sockaddr *)&theirAddress,
                                       &sinSize);

        //Validate that the accept was success, otherwise we will try again
    if (newScoketConnectionFD < 0)
		{
			printf("There was an error traying to accept in the socket, errno:%d\n", errno);
			continue;
		}

        //Add the socket to the list of sockets in an available pleace.
        pthread_mutex_lock(&socketMutex);

    for (socketIndex = 0; socketIndex < 10; socketIndex++)
		{
      if (listSocket[socketIndex] == 0)
			{
        listSocket[socketIndex] = newScoketConnectionFD;
      }
      else
      {
        continue;
      }
			printf("adding new connection listSocket[%d] = %d\n", socketIndex, listSocket[socketIndex]);
			
			//iWriteHistRc = send(listSocket[socketIndex], (void *)message, strlen(message), MSG_NOSIGNAL);
			 iWriteHistRc = send(listSocket[socketIndex], pservidorRespuesta , sizeof(pservidorRespuesta), 0);
			//If there was an error writing to the socket then
			//we shutdown the socket to let other the connection.
			if (iWriteHistRc <= 0)
			{
				printf("Shuting down a socket of index: %d\n", socketIndex);
				shutdown(listSocket[socketIndex], SHUT_RDWR);
				listSocket[socketIndex] = 0;
			}
			sleep(1);  
			break;
    }
        pthread_mutex_unlock(&socketMutex);
	}
   // printf("%X\n", pservidorRespuesta->SOF);
    //printf("%X\n", pclientePetision->SOF);


}

void *SendMessages (void *ptr)
  {
    //Variable declaration.
    int iWriteRc;
   // char kk[10]="hola";
    //int iMsgCounter;
    //char cMsg[20];
   
    while (TRUE)
    {
        pthread_mutex_lock(&socketMutex);    
        //Send a messages to all the members connected.
        for (socketIndex = 0; socketIndex < 10; socketIndex++)
        {
          if (listSocket[socketIndex] != 0)
          {
          //Write the message in the socket.
          printf("Writing to: %d\n",listSocket[socketIndex]);
  	  printf("%X\n", pservidorRespuesta->SOF);
	  //servidorRespuesta.CS=~(servidorRespuesta.SOF+servidorRespuesta.SEN+servidorRespuesta.DS+servidorRespuesta.DT);
	  printf("%X CSvalue:  \n", pservidorRespuesta->CS);
		
	i2ctransfer();
        servidorRespuesta.DT=bus; 
	servidorRespuesta.CS=~(servidorRespuesta.SOF+servidorRespuesta.SEN+servidorRespuesta.DT); 
	 pwm(bus);
	iWriteRc = send(listSocket[socketIndex], pservidorRespuesta , sizeof(pservidorRespuesta), 0);
          //If there was an error writing to the socket then
          //we shutdown the socket to let other the connection.
          if (iWriteRc <= 0)
            {
            printf("Shuting down a socket of index: %d\n", socketIndex);
            shutdown(listSocket[socketIndex], SHUT_RDWR);
            listSocket[socketIndex] = 0;
            }
          }
        }
        pthread_mutex_unlock(&socketMutex);
          
        //This pthread must sleep for 10 seconds.
        sleep(TIMETOSLEEP);      
        }
    return 0;
  }


