#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void check_err(int status,const char *err_mssg){
	if(status <0){
		perror(err_mssg);
		exit(1);
	}
}

int check_entry(char * userN , char * passwd){
	FILE *fp=fopen("id_passwd.txt","r");
	if(fp==NULL){perror("Error opening id_passwd file"); return 0;}
	char line[2*255+2];// userN + ':' + passwd
	
	char target[2*255+2];
	snprintf(target,sizeof(target),"%s:%s",userN,passwd);
	target[strlen(target)-1]='\0';
	//printf("target is *%s*\n",target);
	while(fscanf(fp,"%s",line)!=EOF){
		//line[strcspn(line,"\n")]='\0';
		//printf("line of file is *%s*\n",line);
		if(strcmp(line,target)==0){fclose(fp);return 1;}	
	}
	
	fclose(fp);
	return 0;
	
}









