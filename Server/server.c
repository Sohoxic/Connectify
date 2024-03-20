// pass port number of server as command line argument
// how to use -->username@pop-os:~$./server.out 8000
#include "common.c"
int sockfd,newsockfd;
void signalHandler(int signo) {

    if (signo == SIGINT) {
    	
	printf("CTRL+C pressed to end server\n");
        //close
        printf("Closing sockets\n");
	close(newsockfd);
	close(sockfd);
	       
        exit(0);  	
    }
}
int main(int argc, char **argv){
	// checking command line args
	if (argc <2){
		printf("Didnt enter server-port number as command line argument\n");
		exit(1);
	}
	
	//attaching signal handler
	signal(SIGINT, signalHandler);
	
	//creating the socket
	sockfd= socket(AF_INET,SOCK_STREAM,0);
	check_err(sockfd,"Error in opening socket");
	
	// setting server details
	struct sockaddr_in sa;
	sa.sin_family=AF_INET;//setting address family to IPv4
	sa.sin_port=htons(atoi(argv[1]));
	sa.sin_addr.s_addr=INADDR_ANY;
	
	// binding
	int status= bind(sockfd,(struct sockaddr *) &sa, sizeof(sa));
	check_err(status,"Error in binding");
	
	//listening
	status= listen(sockfd,5);
	check_err(status,"Error in listening");

	//optional messages
	printf("\nServer started waiting for client ...\n");
		
F:	//accepting F is a label for parent to return to
	struct sockaddr_in c_addr;//stores client address who connects in network format
	socklen_t c_addr_len = sizeof(c_addr);//length of c_addr
	char client_addr[100];//stores client address who connects in presentable format
	newsockfd= accept(sockfd,(struct sockaddr *) & c_addr,&c_addr_len);//Q1 is label
	check_err(newsockfd,"Error in accepting client");
	inet_ntop(AF_INET,&c_addr,client_addr,100);//converting the format and store in client_addr
	
	// Handling multiple clients
	pid_t pid = fork();
	check_err(pid,"Error while forking cant handle new clients");
	
/////////////////////////////////////////////////////////////////////////////////////////////////	
	if(pid == 0){// child Move to File transfer
	
	// variable declaration
	char buffer[255]; char buff[1024];
	long int file_size=0,total_recv=0;
	FILE *fp;
	char path_s[255];// stores the path of file where data is written to
	char userN [255];// stores user name of client
	char passwd[255];// stores password of client
	char temp  [255];
		
	//receive userName
	bzero(userN,255);
	status=read(newsockfd,userN,255);
	check_err(status,"Error in reading on server side");
	
	userN[strlen(userN)]='/'; //done for manupulating path_s later in code

	//receive password
	bzero(passwd,255);
	status=read(newsockfd,passwd,255);
	check_err(status,"Error in reading on server side");
	
	passwd[strlen(passwd)]='/'; //done for manupulating path_s later in code
	
      //send userName and password validation message (their must be a corresponding entry in id_passwd.txt)
	if(check_entry(userN,passwd)==0){//entry doesnt exist
		status=write(newsockfd,"Wrong",6);
		check_err(status,"Error in writing on server side");
		exit(1);
	}
	else{//entry exits
		status=write(newsockfd,"Exist",6);
		check_err(status,"Error in writing on server side");		
	}
	
	
	printf("Connected to Client_%s@%s|%d \n",userN,client_addr,c_addr.sin_port);
		
Q:	// receive choice
	bzero(buffer,255);
	bzero(path_s,255);
	bzero(temp,255);
	status=read(newsockfd,buffer,255);
	check_err(status,"Error in reading on server side");
	char ch = buffer[0];

	
	// Menu driven
	switch(ch){
	case 'u': 
		// handle client upload request
		
		// reading the status of path_c(local to client)
		//printf("Wait for path_c status from client\n");
		bzero(buffer,255);
		status=read(newsockfd,buffer,255);
		check_err(status,"Error in reading on client side");
		//printf("Received path_c status from client\n");
		if(strncmp("Wrong",buffer,5)==0){
			printf("Error in local file for Client_%s@%s|%d \n",userN,client_addr,c_addr.sin_port);
			goto Q;
		}
		
		// receive name by which u want to store the file (on-server) from client
		//printf("Wait for path_s from client\n");
		bzero(path_s,255);
		status=read(newsockfd,path_s,255);
		check_err(status,"Error in reading on server side");
		
		//printf("Received path_s from client\n");
				
		//setting up the path on the server
		bzero(temp,255);
		strcpy(temp,userN);
		strcat(temp,path_s);
		strcpy(path_s,temp);
		bzero(temp,255);
		
		
		fp = fopen(path_s,"wb");
		//printf("Write path_s stats to client\n");
		//sending status for opening file(check if file exists or not)
		if(fp==NULL){		
			status=write(newsockfd,"Wrong",6);
			check_err(status,"Error in writing on server side");
			printf("No such %s file exists for Client_%s@%s|%d \n",path_s,userN,client_addr,c_addr.sin_port);
			goto Q;
			
		}
		else{//entry exits
			status=write(newsockfd,"Exist",6);
			check_err(status,"Error in writing on server side");		
		}
		
		//getting file size
		file_size=0;
		status=read(newsockfd,&file_size,sizeof(file_size));
		check_err(status,"Error in reading on server side");
		printf("File Size %ldB for %s file exists for Client_%s@%s|%d \n",file_size,path_s,userN,client_addr,c_addr.sin_port);
		
		//writing words received in server files
		bzero(buffer,255);//buffer will store word of file
		total_recv=0;
		while(total_recv<file_size){
			status=read(newsockfd,buffer,255);
			check_err(status,"Error in reading on server side");
			fwrite(buffer,1,status,fp);
			total_recv+=status;
			bzero(buffer,255);
		}
		
		fclose(fp);
      printf("Finished Receiving %s file from Client_%s@%s|%d \n",path_s,userN,client_addr,c_addr.sin_port);
				
		goto Q;
		
	case 'd': 
		//handle client download request
		
		//Reading name of the file to download from server
		status=read(newsockfd,path_s,255);
		check_err(status,"Error in reading on server side");
		
		//setting up the path on the server
		bzero(temp,255);
		strcpy(temp,userN);
		strcat(temp,path_s);
		strcpy(path_s,temp);
		bzero(temp,255);
		
		fp = fopen(path_s,"rb");
		
		//check if file exist and return apt message to client
		if(fp==NULL){		
			status=write(newsockfd,"Wrong",6);
			check_err(status,"Error in writing on server side");
			printf("No such %s file exists for Client_%s@%s|%d \n",path_s,userN,client_addr,c_addr.sin_port);
			goto Q;
			
		}
		else{//entry exits
			status=write(newsockfd,"Exist",6);
			check_err(status,"Error in writing on server side");		
		}
		
		// reading the status of path_c(local to client)
		bzero(buffer,255);
		status=read(newsockfd,buffer,255);
		check_err(status,"Error in reading on client side");
		if(strncmp("Wrong",buffer,5)==0){
			printf("Error in local file for Client_%s@%s|%d \n",userN,client_addr,c_addr.sin_port);
			goto Q;
		}
				    		
    				//getting the size of the file to transfer
		file_size=0;
		fseek(fp,0,SEEK_END);
		file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		status=write(newsockfd,&file_size,sizeof(file_size));
		check_err(status,"Error in writing on client side");
		
		//reading file word by word
		bzero(buffer,255);//buffer will store word of file
		while ((status=fread(buffer,1,255,fp))>0) {

    			status=write(newsockfd,buffer,status);
			check_err(status,"Error in writing on client side");
			bzero(buffer,255);

    		}   		
		fclose(fp);    		
	printf("Finished Sending %s file of size %ldB to Client_%s@%s|%d \n",path_s,file_size,userN,client_addr,c_addr.sin_port);
						
		goto Q;
		
	case 'v':
		//handle client preview request
		
		//Reading name of the file to view 1st 1024 bytes from server
		status=read(newsockfd,path_s,255);
		check_err(status,"Error in reading on server side");
		
		//setting up the path on the server
		bzero(temp,255);
		strcpy(temp,userN);
		strcat(temp,path_s);
		strcpy(path_s,temp);
		bzero(temp,255);
		
		fp = fopen(path_s,"rb");
		
		//check if file exist and return apt message to client
		if(fp==NULL){		
			status=write(newsockfd,"Wrong",6);
			check_err(status,"Error in writing on server side");
			printf("No such %s file exists for Client_%s@%s|%d \n",path_s,userN,client_addr,c_addr.sin_port);
			goto Q;
			
		}
		else{//entry exits
			status=write(newsockfd,"Exist",6);
			check_err(status,"Error in writing on server side");		
		}
		

    		//getting the size of the file to transfer
		file_size=0;
		fseek(fp,0,SEEK_END);
		file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		if(file_size >1024)file_size=1024;
		status=write(newsockfd,&file_size,sizeof(file_size));
		check_err(status,"Error in writing on client side");
		
		//reading file word by word
		bzero(buff,1024);//buffer will store word of file
		if ((status=fread(buff,1,file_size,fp))>0) {

    			status=write(newsockfd,buff,status);
			check_err(status,"Error in writing on client side");
    		}   		
		fclose(fp);    		
     printf("Finished Sending 1st %ld B of %s file to Client_%s@%s|%d \n",file_size,path_s,userN,client_addr,c_addr.sin_port);
						
		goto Q;	
		
	case 'p':
	// gathering the contents of folder of name userN
		 
	 DIR * dict = opendir(userN);
	 
	 if(dict != NULL){	 	
	 	struct dirent * entry;
	 	while((entry=readdir(dict))!=NULL){
	 		if(entry->d_type == DT_REG)	 			
	 			status=write(newsockfd,entry->d_name,255);
				check_err(status,"Error in writing on server side");
	 	}
	 	closedir(dict);	 	
	 }	 
    		//send zx25xz to mark end of sending
		status=write(newsockfd,"zx25xz",7);
		check_err(status,"Error in writing on server side");    		
		printf("Finished Sending conetnts of %s folder to Client_%s@%s|%d \n",userN,userN,client_addr,c_addr.sin_port);
	 
		goto Q;	
		
	case 'r':
		//Reading name of the file to download from server
		status=read(newsockfd,path_s,255);
		check_err(status,"Error in reading on server side");
		
		//setting up the path on the server
		bzero(temp,255);
		strcpy(temp,userN);
		strcat(temp,path_s);
		strcpy(path_s,temp);
		bzero(temp,255);
		
		if(remove(path_s)==0){
			status=write(newsockfd,"Removal Success",16);
			check_err(status,"Error in writing on server side");
			printf("Removed %s file from Client_%s@%s|%d \n",path_s,userN,client_addr,c_addr.sin_port);		
		}
		else{
			status=write(newsockfd,"Removal Failure",16);
			check_err(status,"Error in writing on server side");
			printf("Failed to Remove %s file from Client_%s@%s|%d \n",path_s,userN,client_addr,c_addr.sin_port);			
		}	
	
		goto Q;
		
	case 'q':
    printf("Session terminated for Client_%s@%s|%d \n",userN,client_addr,c_addr.sin_port);

		 goto R;		
	default: 
    printf("Received invalid command from Client_%s@%s|%d \n",userN,client_addr,c_addr.sin_port);

		 goto Q;
	}
	

	//close
R:	close(newsockfd);
	close(sockfd);
	exit(0);
	}// end of child process specific code
	else{// parent again wait for new connection
		
		goto F;
	}
	
	return 0;
}
