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
	*
	* Populate the client
	* POPULATE_ALL|password\| (password is the password, change in main)
	*
	* 5 Star rating for the bar
	* RATE_BAR|barid|rating\|
	*
	* 5 Star rating for an event
	* RATE_EVENT|barid|eventid|rating\|
	*
	* Chat message (eventid=0 is bar chat)
	* NEW_CHAT_MSG|barid|eventid|message\|
	*
	* Request chat messages (eventid=0 is bar chat)
	* REQ_CHAT_MSGS|barid|eventid|timestamp\|
	*/

struct commands
{
	static const int POPULATE_ALL;
	static const int RATE_BAR;
	static const int RATE_EVENT;
	static const int NEW_CHAT_MSG;
	static const int REQ_CHAT_MSGS;
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
	int id;
	std::string ip;
	std::vector<pthread_t*> sThreads;//all the active service threads

	user();
	user(int, int, std::string);
	~user();

	void newService(std::vector<std::string>&);
	static void* launchService(void* y);

private:
	void startService(newServiceArgs*);
	void exitService(newServiceArgs*);
};

#endif