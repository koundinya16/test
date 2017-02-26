
/* -----------------------------------------------------emailclient.c----------------------------------------------------
 Author : Koundinya AE13B010
 ------------------------------------------------------------------------------------------------------------------------
 A TCP based email client that gets commands from the user and transmits appropriate 
 requests to the emailserver, and prints the data from server
 
 RUN : ./eclient <hostname/ip> <port number>

 Available commands :
 ------------------

 -Listusers
 -Adduser <userid>
 -SetUser <userid>
 	->Read
 	->Delete
 	->Send <receiver id>
 	->Done
 -Quit

 ------------------------------------------------------------------------------------------------------------------------
 Acknowledgement : Parts of source code(for resolving both IPv4 and IPv6 addresses) taken from  | http://beej.us/guide |
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define MSGBUFSIZE 102400
#define DATABUFSIZE 102400
#define INPUTBUFSIZE 102400


// Error handler
void error(char *msg) 
{
    perror(msg);
    exit(0);
}


int main(int argc, char **argv) 
{
    char *hostname, *port_num;

    int socket_fd;

    int error_num_ga,error_num_io;

    int num_data_recvd,num_data_sent;
    
    struct addrinfo server_sockspecs, *server_addrinfo;
    
    void *ipaddr;

    char server_ipaddress[INET6_ADDRSTRLEN];
    char *server_ipaddress_version;

    char transmit_buff[MSGBUFSIZE],receive_buff[DATABUFSIZE],input_buff[INPUTBUFSIZE];
    char command[20],*userId;
    int invalidcommand_flag=0,quit_flag=0,send_flag=0,loggedin_flag=0;
    int num_user_arg=0;
    

    // Check command line arguments
    if (argc != 3)
    {
       fprintf(stderr,"Usage: %s <hostname> <port>\n", argv[0]);
       return 1;
    }

   
    // Get the port number and hostname of Server for establshing connection  
    hostname = argv[1];
    port_num = argv[2];

    
    // Initialize network read/write buffers and user input buffer 
    memset(transmit_buff,0,MSGBUFSIZE);
    memset(receive_buff,0,DATABUFSIZE);
    memset(input_buff,0,INPUTBUFSIZE);

    // Initialize the addrinfo struct : server_sockspecs 
    memset(&server_sockspecs,0,sizeof(server_sockspecs));
    
    // Filter for the Server socket specifications 
    server_sockspecs.ai_family   = AF_UNSPEC;     // IPv4 or IPv6
    server_sockspecs.ai_socktype = SOCK_STREAM;   // TCP
    server_sockspecs.ai_protocol = 0;             // Default


   
    // Get the addrinfo(structs) of all the server sockets which match the given specs 
    error_num_ga = getaddrinfo(hostname,port_num,&server_sockspecs,&server_addrinfo);
    if(error_num_ga!=0)
    {
        fprintf(stderr,"ERROR - Unable to get host address info : %s\n",gai_strerror(error_num_ga));
        return 1;
    }

    // Open socket on the client side
    socket_fd = socket(server_addrinfo->ai_family, server_addrinfo->ai_socktype, 0);
    if (socket_fd < 0) 
        error("ERROR - Unable to open socket");
    
 

    // Establish connection with Server (the first socket from the list of available server sockets is selected)    
    if (connect(socket_fd, server_addrinfo->ai_addr,server_addrinfo->ai_addrlen) < 0) 
      error("ERROR - could not establish connection");
    
    // Determine server IP address
    if(server_addrinfo->ai_family==AF_INET)
    {
        struct sockaddr_in *ipv4= (struct sockaddr_in *)server_addrinfo->ai_addr;
        ipaddr = &(ipv4->sin_addr);
        server_ipaddress_version="IPv4";

    }
    else
    {
       struct sockaddr_in6 *ipv6= (struct sockaddr_in *)server_addrinfo->ai_addr;
       ipaddr = &(ipv6->sin6_addr);
       server_ipaddress_version="IPv6";
    }

    inet_ntop(server_addrinfo->ai_family,ipaddr,server_ipaddress,INET6_ADDRSTRLEN);
    printf("\n\n ------ Connection Established with : %s (%s address) ------\n\n",server_ipaddress,server_ipaddress_version);   

    while(1)
    {
    	invalidcommand_flag=0;
       // Take User Input
    	memset(input_buff,0,INPUTBUFSIZE);
    	if(loggedin_flag==0)
    	    printf("$$ > ");
        else if(send_flag!=1)
        	printf("\t$$ > ");


   		fgets(input_buff, INPUTBUFSIZE, stdin);

        memset(transmit_buff,0,MSGBUFSIZE);

   		// Parse the input and get user command
        userId = (char *)malloc(strlen(input_buff)+1);
        memset(command,0,20);
        if(send_flag==0)
         num_user_arg=sscanf(input_buff,"%s %s",command,userId);
        else
         num_user_arg=2;
        if(num_user_arg==2)
        {
        	if (strcmp(command,"Adduser")==0 || strcmp(command,"adduser")==0)
        	{
              strcpy(transmit_buff,"ADDU ");
              strcat(transmit_buff,userId);
        	}

        	else if (strcmp(command,"Setuser")==0 || strcmp(command,"setuser")==0)
        	{
        	  strcpy(transmit_buff,"USER ");
              strcat(transmit_buff,userId);
              loggedin_flag=1;
        	}

        	else if (strcmp(command,"Send")==0 || strcmp(command,"send")==0)
        	{
        	  strcpy(transmit_buff,"SEND ");
              strcat(transmit_buff,userId);
              send_flag=1;	
        	}
        	else 
            { 
            	if(send_flag==1)
            	{
                  strcpy(transmit_buff,input_buff);
                  send_flag=0;
            	}

        	    else
        	    {
                 printf("\rInvalid command\n");
        	     invalidcommand_flag=1;        	    	
        	    }
            }
        }
        else
        {

  	      	if (strcmp(command,"Listusers")==0 || strcmp(command,"listusers")==0)
    	    {
        	  strcpy(transmit_buff,"LSTU");	
        	}
        	else if (strcmp(command,"Read")==0 || strcmp(command,"read")==0)
        	{
              strcpy(transmit_buff,"READM");
        	}

        	else if (strcmp(command,"Delete")==0 || strcmp(command,"delete")==0)
        	{
        		strcpy(transmit_buff,"DELM");
        	}
        	else if (strcmp(command,"Done")==0 || strcmp(command,"done")==0)
        	{
        		strcpy(transmit_buff,"DONEU");
        		loggedin_flag=0;
        	}
        	else if (strcmp(command,"Quit")==0 || strcmp(command,"quit")==0)
        	{
                strcpy(transmit_buff,"QUIT");
                quit_flag=1;
        	}
        	else
            { 
        	printf("\rInvalid command\n");
        	invalidcommand_flag=1;
            }
        }
        

        if(invalidcommand_flag!=1)
        {
       		// Send Message to the Server
    		error_num_io = write(socket_fd, transmit_buff, strlen(transmit_buff));
   			if (error_num_io < 0) 
       	 	 error("ERROR - writing to socket");
     
        	// Read Message from the Server
        	memset(receive_buff,0,DATABUFSIZE);

    		error_num_io = read(socket_fd, receive_buff,DATABUFSIZE);
    		if (error_num_io < 0) 
      	 	 error("ERROR - reading from socket");

      	 	if(strcmp(receive_buff,"Error sending Mail : User doensn't exist")==0 || strcmp(receive_buff,"Error Sending Mail,login to continue")==0)
      	 			send_flag=0;

      	 	if(strcmp(receive_buff,"User does not exit")==0)
      	 		loggedin_flag=0;
            
            if(loggedin_flag==1)
            	printf("\t%s \n",receive_buff);
            else
                printf("%s \n",receive_buff);
            

    		memset(receive_buff,0,DATABUFSIZE);
        }
        if(quit_flag==1)
        {
        	quit_flag=0;
        	break;
        }

    }
    printf("-------Disconnected-----\n");
    close(socket_fd);
    
    return 0;
}
