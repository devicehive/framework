#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <map>
#include <algorithm>
#include <syslog.h>
#include <unistd.h>
#include <mongcpp.h>

#include "gpiopin.hpp"
#include "wireless.hpp"
#include "server.hpp"

bool sigusr1 = false;

void sig_handler(int sig)
{
    sigusr1 = true;
};


int main()
{
    pid_t pid, sid;
        
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);
 
	sid = setsid();
    if (sid < 0) {
        return 1;
    }
        
    if ((chdir("/")) < 0) {
        return 1;
    }
        
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

	struct sigaction sig_struct;
	memset (&sig_struct, '\0', sizeof(sig_struct));
	sig_struct.sa_handler = sig_handler;
	sig_struct.sa_flags = 0;
	sigset_t   set; 
	sigemptyset(&set);                                                             
	sigaddset(&set, SIGUSR1); 
	sig_struct.sa_mask = set;
	if (sigaction(SIGUSR1, &sig_struct, NULL) == -1){
		return 1;
	}

	GPIOServer server;
    server.setOption("document_root", "html");
    server.setOption("listening_ports", "8080");
    server.setOption("num_threads", "5");
    server.start();

	GPIOPin gpio3(3);
	int oldv = -1;
	while (true)
	{
		if (sigusr1)
			break;

		int newv = gpio3.read();
		if (newv != oldv)
		{
			oldv = newv;
			switch (newv)
			{
			case 0:
				setAP();
				break;
			case 1:
				setCli();
				break;
			default:
				break;
			}
		}

		sleep(2);
	}

	server.stop();
	return 0;
}
