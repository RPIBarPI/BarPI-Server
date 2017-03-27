#include "user.h"
#include "main.h"
#include "socket.h"
#include "sql.h"

const int commands::POPULATEALL=0;

user::user()
{
	sockfd=-1;
	ip="";
}

user::user(int newSockFD, std::string newIP)
{
	sockfd=newSockFD;
	ip=newIP;
}

user::~user()
{
	sockfd=-1;
	ip="";
}

void user::newService(std::vector<std::string> &sockData)
{
	//launch a new service thread

	//set the args to pass in
	newServiceArgs *x=new newServiceArgs[sizeof(newServiceArgs)];
	x->thisUser=this;
	x->sockData=sockData;
	x->sThread=new pthread_t[sizeof(pthread_t)];

	//call the helper
	pthread_create(x->sThread, NULL, &user::launchService, (void*)x);
}

void* user::launchService(void* y)
{
	//helper function for pthread_create
	
	//set the service args
	newServiceArgs *x=(newServiceArgs*)y;
	
	//start the service
	x->thisUser->startService(x);
	
	//exit the service
    x->thisUser->exitService(x);
}

void user::startService(newServiceArgs* x)
{
	printf("Service Start: %s\n", this->ip.c_str());
	//add the thread to the users active thread vector
	this->sThreads.push_back(x->sThread);

	//for the mysql
	MYSQL *userConnection=dbUserConnect();
	x->userConnection=userConnection;
	if(userConnection == NULL) return;

	//get the command
	std::vector<std::string> data=x->sockData;
	int command=atoi(data[0].c_str());
	data.erase(data.begin());

	switch(command)
	{
		case commands::POPULATEALL://populate the client
		{
			if(data.size() < 1) break;

			//get the password
			std::string keyPhrase=data[0];
			if(keyPhrase != "password") break;

			//give the client bar info

			break;
		}
	}
}

void user::exitService(newServiceArgs* x)
{
	printf("Service Exit: %s\n", this->ip.c_str());
	//handle exiting a service

	//for the mysql
	dbUserClose(x->userConnection);
	mysql_thread_end();

	//remove the thread
	for(std::vector<pthread_t*>::iterator itr=this->sThreads.begin();itr!=this->sThreads.end();++itr)
	{
		pthread_t *tempThread=(*itr);
		if(x->sThread == tempThread)
		{
			sThreads.erase(itr);
			delete[] tempThread;
			break;
		}
	}
	delete[] x;
}