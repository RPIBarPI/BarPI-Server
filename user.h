#ifndef _USER
#define _USER

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <time.h>
#include <vector>
#include <pthread.h>
#include "sql.h"

/*
	* THE ENGINE
	* Populate the client
	* POPULATEALL|password\| (password is the password, change in main)
	*/

struct commands
{
	static const int POPULATEALL;
};

struct user;

struct newServiceArgs
{
    user *thisUser;
    std::vector<std::string> sockData;
    pthread_t *sThread;
    MYSQL* userConnection;
};

struct user
{
	int sockfd;
	std::string ip;
	std::vector<pthread_t*> sThreads;//all the active service threads

	user();
	user(int, std::string);
	~user();

	void newService(std::vector<std::string>&);
	static void* launchService(void* y);

private:
	void startService(newServiceArgs*);
	void exitService(newServiceArgs*);
};

#endif