#include <stdbool.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>  
#include <stdio.h> 
#include <signal.h>  
#include <stdlib.h> 

 
void* get_in_addr(struct sockaddr *sa) {
	if(AF_INET == sa->sa_family) 
		return &(((struct sockaddr_in*)sa)->sin_addr); 
	else
		return &(((struct sockaddr_in6*)sa)->sin6_addr); 
}

bool  write_str(int fd, const char* buf, int len)  {
	if(write(fd, buf, len) < 0) {
		return false;   
	}
	return true; 
}

#define FILE_NAME "/var/tmp/aesdsocketdata"  
int init_file_writer() {   
	int fd;
	fd = open(FILE_NAME, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if(fd < 0)  {
		printf("%d %s %d\n", fd, __FUNCTION__, __LINE__);  
		return -1;
	} 
	return fd; 
}

bool read_file_to_buf(char* buf, unsigned int tot_bytes_recv) {
	int fd; 

	fd = open(FILE_NAME, O_RDONLY, 0644);    
	if(fd < 0) {
		return false;
	}
	read(fd, buf, tot_bytes_recv); 
	close(fd);
	return true;  
}

bool close_file(int fd) {
	if(close(fd) <0)  {
		return false;		
	}
	return true; 
}

//bool is_running = false;
bool is_running = true ;
void sigint_handler(int sig) {
	syslog(LOG_INFO, "Caught signal,exiting");
	is_running = false; 
}

bool init_sigaction() {
	struct sigaction sa;
	sa.sa_handler = sigint_handler; 
	sigemptyset(&sa.sa_mask) ;
	sa.sa_flags = 0;

	if(sigaction(SIGINT, &sa, NULL) < 0) {
		printf("%s %d\n", __FUNCTION__, __LINE__); 
		return false; 
	}
	if(sigaction(SIGTERM, &sa, NULL) < 0) {
		printf("%s %d\n", __FUNCTION__, __LINE__); 
		return false ; 
	}
	return true;
}

//#define BUF_LENGTH  200
#define BUF_LENGTH  200000
#define PORT_NUM "9000" 
int main(int argc, char** argv) {
	bool flag_daemon = false;
	if(2 == argc)  {   
		flag_daemon = (!strcmp(argv[1], "-d"))?true:false; 	
	}
	printf("%s %d\n", __FUNCTION__, __LINE__); 
	if(!init_sigaction())  {  
		printf("%s %d\n", __FUNCTION__, __LINE__); 
		return -1;
	} 
	//char buf[BUF_LENGTH]; 
	unsigned int buf_length = BUF_LENGTH; 
	char* buf = calloc(buf_length, sizeof(char));    

	int fd = init_file_writer(); 
	if(fd < 0)  {
		printf("%s %d\n", __FUNCTION__, __LINE__); 
		return -1;  
	} 
 
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 

	unsigned int total_bytes_recv = 0;
	//unsigned int buf_length = BUF_LENGTH; 

	struct addrinfo *ptr_result = NULL;  
	if(getaddrinfo(NULL, PORT_NUM, &hints, &ptr_result))  { 
		printf("%s %d\n", __FUNCTION__, __LINE__); 
		return -1; 
	}
	int server_fd = 0;
	for(struct addrinfo* p = ptr_result; p != NULL; p=p->ai_next) {
			
		server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(-1 == server_fd) {
			printf("%s %d\n", __FUNCTION__, __LINE__); 
			continue; 
		}   
		int opt = 0;	
		if(-1 == setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int))) { 
			printf("%s %d\n", __FUNCTION__, __LINE__); 
			return -1;
		}
		if(-1 == bind(server_fd, p->ai_addr, p->ai_addrlen))  {
			printf("%s %d\n", __FUNCTION__, __LINE__); 
			continue;
		}
		else {  
			printf("%s %d\n", __FUNCTION__, __LINE__); 
			break;
		} 
	}	
	freeaddrinfo(ptr_result);  

	if(-1 == listen(server_fd, 10)) {
		printf("%s %d\n", __FUNCTION__, __LINE__); 
		return -1;
	}
	if(flag_daemon) {
		pid_t pid = fork();
		if(0 < pid) {
			return 0 ;
		}
		else if(0 > pid) {
			return -1; 
		}
	}
 
	struct sockaddr_storage new_conn_addr ;
	socklen_t new_conn_addr_size = 0;  
	while(is_running)  {
		int new_fd = accept(server_fd, (struct sockaddr *)&new_conn_addr, &new_conn_addr_size) ;
		if(-1 == new_fd) {
			printf("%s %d\n", __FUNCTION__, __LINE__); 
			return 0; 
		}
		char s[INET6_ADDRSTRLEN];
		inet_ntop(new_conn_addr.ss_family, 
			get_in_addr((struct sockaddr *)&new_conn_addr), 
			s, sizeof(s));

		syslog(LOG_INFO, "Accepted connection from %s\n", s);
		//printf("Accepted connection from %s\n", s);

		unsigned int total_bytes_packet = 0;
		unsigned int num_bytes_recv = 0;
		while(is_running) {
			num_bytes_recv = recv(new_fd, buf + total_bytes_packet, 
				BUF_LENGTH - total_bytes_packet -1, 0); 
			if(-1 == num_bytes_recv) {
				printf("%s %d\n", __FUNCTION__, __LINE__); 
				return -1;
			}
			else if(0 == num_bytes_recv)  { 
				printf("%s %d\n", __FUNCTION__, __LINE__); 
				break; 
			} 
			else {
				printf("%08X/%08X %s %d\n", num_bytes_recv, buf_length, __FUNCTION__, __LINE__); 
				
			} 
			total_bytes_packet += num_bytes_recv;
			buf[total_bytes_packet] = '\0';
			const char NEW_LINE_CHAR = '\n'; 	
			if(strchr(buf, NEW_LINE_CHAR )) { 
				printf("NEWLINE(%s) %s %d\n", buf, __FUNCTION__, __LINE__); 
//				printf("(%s) %s %d\n", buf, __FUNCTION__, __LINE__); 
				break;
			} 
		}  
		total_bytes_recv += total_bytes_packet;
		if(!write_str(fd, buf, total_bytes_packet)) 
			return -1 ; 
 		if(!read_file_to_buf(buf, total_bytes_recv)) 
			return -1;
		send(new_fd, buf, total_bytes_recv, 0);

		syslog(LOG_INFO, "Closed connection from %s\n", s);   
	}
	if(!close_file(fd)) 
		return -1;
	remove(FILE_NAME);
	return 0;
	
}
