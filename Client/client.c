// pass ip and port number of server as command line argument
// how to use -->username@pop-os:~$./client.out 10.2.1.49 8000
#include "common.c"

int main(int argc, char **argv){
	// checking command line args
	if (argc <3){
		printf("Didnt enter server-port/ip/both as command line argument\n");
		exit(1);
	}
	
	//blocking Ctrl +c signal
	sigset_t mask;
	sigemptyset(&mask);            // Initialize an empty signal set
	sigaddset(&mask, SIGINT);      // Add SIGINT to the set

	// Block SIGINT signal
	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		perror("sigprocmask");
		exit(1);
	}
		
	//creating the socket
	int sockfd= socket(AF_INET,SOCK_STREAM,0);
	check_err(sockfd,"Error in opening socket");
	
	// setting server details
	struct sockaddr_in sa;
	sa.sin_family=AF_INET;//setting address family to IPv4
	sa.sin_port=htons(atoi(argv[2]));
	int status = inet_pton(AF_INET,argv[1],&sa.sin_addr);//converting from presentable format to network format and storing in sa.sin_addr
	check_err(status,"Error in client ip conversion format");
	
	// connecting
	status= connect(sockfd,(struct sockaddr *) &sa, sizeof(sa));
	check_err(status,"Error in connecting");


/////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	//variable declare
	char buffer[255];
	char passwd[255];
	char mssg[255];
	long int file_size=0,total_recv=0;
	FILE *fp;
	char path_s[255];// stores path of file on server side
	char path_c[255];// stores path of file on client side
	
	//optional messages	
	printf("\nConnected to Server@%s|%s  ...\n",argv[1],argv[2]);
	printf("\n ------- General Guidelines ------- \n");
	printf("Type after :$$ prompt appears\n");
	printf("Your User Name will be the name of the folder where your files will be kept on server\n");
	printf("List of all feasible operations and their respective commands ->\n\tto upload file from local to server --enter--> u\n\tto download file from server to local --enter--> d\n\tto print contents of your server directory --enter--> p\n\tto view a portion of your server file --enter--> v\n\tto remove a server file --enter--> r\n\tto quit or end session --enter--> q\n\n");
		
	//take username from user as input
	printf("Enter userName :$$ ");
	bzero(buffer,255);
	fgets(buffer,255,stdin);
	buffer[strlen(buffer)-1]='\0';// replace newline by '\0'
	
	// sending user name to server
	status=write(sockfd,buffer,255);
	check_err(status,"Error in writing on client side");
	
	//take password from user as input
	printf("Enter password :$$ ");
	bzero(passwd,255);
	fgets(passwd,255,stdin);
	passwd[strlen(passwd)-1]='\0';// replace newline by '\0'
	
	// sending user name to server
	status=write(sockfd,passwd,255);
	check_err(status,"Error in writing on client side");
	
	//receive user_name  and password validity from server
	bzero(mssg,255);
	status=read(sockfd,mssg,255);
	check_err(status,"Error in reading on client side");
	if(strncmp("Wrong",mssg,5)==0){
		printf("Wrong UserName or password\n\n");
		goto R;
	}
	else if(strncmp("Exist",mssg,5)==0){
		printf("User Name and password validated\n\n");
	}	
	
	//get choice from user	
Q:
	bzero(buffer,255);
	bzero(path_s,255);
	bzero(path_c,255);
	bzero(mssg,255);
	printf("Enter appropriate command for the operation u want to perform :$$ ");	
	fgets(buffer,255,stdin);
	char ch = buffer[0];
	
	
	
	//Menu to upload and download files to server
	switch(ch){
	case 'u': // client wants to upload file
	//sending choice of client to server
	status=write(sockfd,buffer,255);
	check_err(status,"Error in writing on client side");
	bzero(buffer,255);
	
		printf("\n");
		//user input
		bzero(path_c,255);
		printf("Enter path of file(on-client) to upload :$$ ");
		fgets(path_c,255,stdin);
		path_c[strlen(path_c)-1]='\0';//removing newline
				
		//open file
		fp = fopen(path_c,"rb");
		if(fp==NULL){
				// sending satus of path_c to server so that even he can exit
			status=write(sockfd,"Wrong",6);
			check_err(status,"Error in writing on client side");
			printf("NO such %s file exists\n\n",path_c);
			goto Q;//get choice from user
		}
		else{
			status=write(sockfd,"Exist",6);
			check_err(status,"Error in writing on client side");
		}
		
		//user input
		bzero(path_s,255);
		printf("Enter name by which u want to store the file (on-server) :$$ ");
		fgets(path_s,255,stdin);
		path_s[strlen(path_s)-1]='\0';//removing newline
				
		//send path_s to server
		status=write(sockfd,path_s,255);
		check_err(status,"Error in writing on client side");

		//printf("path_s sent to server= #%s#\n",path_s);		
		
		// checking the results returned by server on validity of path_s 
		bzero(mssg,255);
		status=read(sockfd,mssg,255);
		check_err(status,"Error in reading on client side");
		//printf("Mssg=%s# regarding path_s received\n",mssg);
		if(strncmp("Wrong",mssg,5)==0){
			printf("NO such %s file exists\n\n",path_s);
			goto Q;
		}
		
		//getting the size of the file to transfer
		file_size=0;
		fseek(fp,0,SEEK_END);
		file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		printf("FILE SIZE to be transferred is %ldB\n",file_size);
		status=write(sockfd,&file_size,sizeof(file_size));
		check_err(status,"Error in writing on client side");
		
		//reading file word by word
		bzero(buffer,255);//buffer will store word of file
		while ((status=fread(buffer,1,255,fp))>0) {

    			status=write(sockfd,buffer,status);
			check_err(status,"Error in writing on client side");
			bzero(buffer,255);

    		}   		
		printf("Finished Sending file from Client side\n\n");
	    	fclose(fp);
		goto Q;
		
	case 'd': // client wants to download file
	//sending choice of client to server
	status=write(sockfd,buffer,255);
	check_err(status,"Error in writing on client side");
	bzero(buffer,255);
		
		printf("\n");
		//user input
		printf("Enter name of file(on-server) to download :$$ ");
		fgets(path_s,255,stdin);
		path_s[strlen(path_s)-1]='\0';//removing newline
				
		//send path_s to server
		status=write(sockfd,path_s,255);
		check_err(status,"Error in writing on client side");

		// checking the results returned by server on validity of path_s 
		bzero(mssg,255);
		status=read(sockfd,mssg,255);
		check_err(status,"Error in reading on client side");
		if(strncmp("Wrong",mssg,5)==0){
			printf("NO such %s file exists\n\n",path_s);
			goto Q;
		}
		
		//user input		
		printf("Enter path of file(on-client) where u want to store :$$ ");
		fgets(path_c,255,stdin);
		path_c[strlen(path_c)-1]='\0';//removing newline
		
		//open file				
		fp = fopen(path_c,"wb");
		if(fp==NULL){
				// sending satus of path_c to server so that even he can exit
			status=write(sockfd,"Wrong",6);
			check_err(status,"Error in writing on client side");
			printf("NO such %s file exists\n\n",path_c);
			goto Q;
		}
		else{
			status=write(sockfd,"Exist",6);
			check_err(status,"Error in writing on client side");
		}
				//getting file size
		file_size=0;
		status=read(sockfd,&file_size,sizeof(file_size));
		check_err(status,"Error in reading on server side");
		
		//writing words received in server files
		bzero(buffer,255);//buffer will store word of file
		total_recv=0;
		while(total_recv<file_size){
			status=read(sockfd,buffer,255);
			check_err(status,"Error in reading on server side");
			fwrite(buffer,1,status,fp);
			total_recv+=status;
			bzero(buffer,255);
		}
		
		fclose(fp);
		printf("Finished Receiving file from Server side\n\n");		
		
		goto Q;
	
	case 'v':
	//sending choice of client to server
	status=write(sockfd,buffer,255);
	check_err(status,"Error in writing on client side");
	bzero(buffer,255);
	// view 1st 10 words of your mentioned file
		printf("\n");
		//user input
		printf("Enter name of file(on-server) to view top 1024 B :$$ ");
		fgets(path_s,255,stdin);
		path_s[strlen(path_s)-1]='\0';//removing newline
		
		//send path_s to server
		status=write(sockfd,path_s,255);
		check_err(status,"Error in writing on client side");
		
		// checking the results returned by server on validity of path_s 
		bzero(mssg,255);
		status=read(sockfd,mssg,255);
		check_err(status,"Error in reading on client side");
		if(strncmp("Wrong",mssg,5)==0){
			printf("NO such %s file exists\n\n",path_s);
			goto Q;
		}
		
	printf("\t...The 1st 1024 bytes of your mentioned file are\n");
		
		// receiving words from server and printing on client terminal
				//getting file size
		file_size=0;
		status=read(sockfd,&file_size,sizeof(file_size));
		check_err(status,"Error in reading on server side");
		
		//writing words received in server files
		bzero(buffer,255);//buffer will store word of file
		total_recv=0;
		while(total_recv<file_size){
			status=read(sockfd,buffer,255);
			check_err(status,"Error in reading on server side");
			printf("%s ",buffer);
			total_recv+=status;
			bzero(buffer,255);
		}
					
		printf("\n\t...Finished Receiving file from Server side\n\n");				
		goto Q;	
		
	case 'p':
	//sending choice of client to server
	status=write(sockfd,buffer,255);
	check_err(status,"Error in writing on client side");
	bzero(buffer,255);
	//printing contents of dir (received from server)
		printf("\nThe contents of your server folder are\n");
		bzero(buffer,255);//buffer will store word of file
		status=read(sockfd,buffer,255);
		check_err(status,"Error in reading on client side");
		
		while(strncmp("zx25xz",buffer,6)!=0){//"zx25xz" marks the end of sending
			printf("\t%s\n",buffer);
			bzero(buffer,255);
			status=read(sockfd,buffer,255);
			check_err(status,"Error in reading on client side");			
		}
		printf("Thats all ....\n\n");
		
		goto Q;
		
	case 'r':
	//sending choice of client to server
	status=write(sockfd,buffer,255);
	check_err(status,"Error in writing on client side");
	bzero(buffer,255);
	// to remove a particular file from server
		printf("\n");
		//user input
		printf("Enter name of file(on-server) to remove from server :$$ ");
		fgets(path_s,255,stdin);
		path_s[strlen(path_s)-1]='\0';//removing newline
		
		//send path_s to server
		status=write(sockfd,path_s,255);
		check_err(status,"Error in writing on client side");
		
		//read status of removal
		status=read(sockfd,buffer,255);
		check_err(status,"Error in reading on client side");		
		printf("Status of removal : %s\n\n",buffer);
		bzero(buffer,255);		
	
	goto Q;	
			
	case 'q':  
	//sending choice of client to server
	status=write(sockfd,buffer,255);
	check_err(status,"Error in writing on client side");
	bzero(buffer,255);
	printf("\nByeBye..session_ended..\n");	
		 goto R;
		 
	default: 
	//sending choice of client to server
	status=write(sockfd,buffer,255);
	check_err(status,"Error in writing on client side");
	bzero(buffer,255);	
	printf("No such command exists\n\n");
		 goto Q;
	}

	//close
R:	close(sockfd);
	
	
	return 0;
}

