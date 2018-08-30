#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
     
bool getDeviceLink(char *path, char *dev);
float getTemp(char *devPath);
     
int main (void) {
	
	//VARIABLES DEFINITION
	char dev[16];		//Device ID
	char devPath[128];	//Path to Device
	char path[] = "/sys/bus/w1/devices";
	int timeout=-1;
	int threadPID=-1;
	int pipe1[2],pipe2[2];
	float temp;
	char tempString[6];
	bool exit=false;
	
	
	printf("Ogni quanti secondi eseguire una misurazione\n>> ");
	scanf("%d",&timeout);
	
	
	if(getDeviceLink(path,dev)){
		sprintf(devPath, "%s/%s/W1_slave", path,dev);
		
		pipe(pipe1);
		pipe(pipe2);
		threadPID = fork();
		
		while(!exit){
			
			
			if(threadPID==0){
					//Thread Padre
					close(pipe1[0]);	//Chiusura ingresso pipe1
					close(pipe2[1]);	//Chiusura uscita pipe2
					int nbytes;
					char readBuffer[128];
					nbytes = read(pipe1[1], readBuffer, sizeof(readBuffer));
					if(nbytes>0){
						printf("Temperature: %s",readBuffer);
					}
			}
			else{
					//Thread Figlio
					close(pipe1[1]);	//Chiusura ucita pipe1
					close(pipe2[0]);	//Chiusura ingresso pipe2
					temp = getTemp(devPath);
					sprintf(tempString,"%f",temp);
					write (pipe1[0], tempString, (strlen(tempString)+1));
			}
			sleep(timeout*1000);
		}
	}
	else{
		return(EXIT_FAILURE);
	}

return(EXIT_SUCCESS);
}

bool getDeviceLink(char *path, char *dev){
	DIR *dir;
	struct dirent *dirent;
		
	dir = opendir(path);
	if(dir != NULL){
		while((dirent = readdir(dir))){
			if(dirent->d_type == DT_LNK && strstr(dirent->d_name, "28-") != NULL){
				strcpy (dev, dirent->d_name);
				printf("\nDevice: %s\n", dev);
			}
		}
		closedir (dir);
	}
	else{
		perror ("Couldn't open the w1 device");
		return false;
	}
	return true;
}

float getTemp(char *devPath){
	float tempC;
	char buf[256];		//Buffer for data read from device
	char tmpData[6];	//Temp C° * 1000 reported by device
	ssize_t numRead;
	
	int fd = open(devPath, O_RDONLY);
	if(fd == -1){
		perror("Couldn't open the w1 device");
		return -1;
	}
	else{
		while((numRead = read(fd,buf,256))>0){
			strncpy(tmpData, strstr(buf,"t=") + 2, 5);
			tempC = strtof(tmpData, NULL);
			
		}
	}
	close(fd);
	return tempC/1000;
}


