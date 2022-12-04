#include <stdio.h>   
#include <syslog.h>  
#include <errno.h>
#include <string.h>

int main(int argc, char* argv[]) {

	if(3 !=  argc) {  
	       printf("need 2 arguments\n");  
	      return 1; 
	}	       
	char* writefile = argv[1];
	char* writestr = argv[2];
	openlog(NULL, LOG_CONS, LOG_USER); 
 	syslog(LOG_DEBUG, "Writing %s to %s\n", writestr, writefile); 
	printf("writefile is %s\n", writefile);   
	FILE* fp = fopen(writefile, "a");  
	if(NULL == fp) {  
		syslog(LOG_ERR, "%s\n", strerror(errno));  
		closelog();
		return 1;
	}
	int ret = fwrite(writestr, 1, strlen(writestr), fp) ;   
	printf("writestr is %s %ld\n", writestr, strlen(writestr)) ;
	if(0 > ret) {  
		syslog(LOG_ERR, "%s\n", strerror(errno));  
		closelog();
		return 1;
	}

	closelog(); 	
	return 0;
}
