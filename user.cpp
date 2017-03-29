#include "user.h"
#include "main.h"
#include "socket.h"
#include "sql.h"

const int commands::POPULATE_ALL=0;
const int commands::RATE_BAR=1;
const int commands::RATE_EVENT=2;
const int commands::NEW_CHAT_MSG=3;
const int commands::REQ_CHAT_MSGS=4;

user::user()
{
	sockfd=-1;
	id=-1;
	ip="";
}

user::user(int newSockFD, int newID, std::string newIP)
{
	sockfd=newSockFD;
	id=newID;
	ip=newIP;
}

user::~user()
{
	sockfd=-1;
	id=-1;
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
	mysql_thread_init();
	MYSQL *userConnection=dbUserConnect();
	x->userConnection=userConnection;
	if(userConnection == NULL) return;

	//get the command
	std::vector<std::string> data=x->sockData;
	int command=atoi(data[0].c_str());
	data.erase(data.begin());

	switch(command)
	{
		case commands::POPULATE_ALL://populate the client
		{
			if(data.size() < 1) break;

			//get the password
			std::string keyPhrase=data[0];
			if(keyPhrase != "password") break;

			//give the client bar info (eventually by coordinates and proximity)
			std::vector<std::map<std::string, std::string> > barInfo=
				getBars(userConnection);

			if(barInfo.size() > 0)
			{
				for(int i=0;i<barInfo.size();++i)
				{
					std::vector<std::string> wData;
					wData.push_back("B");
					wData.push_back(barInfo[i][sqlFields::BARS::ID]);

					wData.push_back(sqlFields::BARS::NAME);
					if(barInfo[i][sqlFields::BARS::NAME] != "")
						wData.push_back(barInfo[i][sqlFields::BARS::NAME]);
					else
						wData.push_back("Bar");

					if(barInfo[i][sqlFields::BARS::DESCRIPTION] != "")
					{
						wData.push_back(sqlFields::BARS::DESCRIPTION);
						wData.push_back(barInfo[i][sqlFields::BARS::DESCRIPTION]);
					}

					wData.push_back(sqlFields::BARS::RATING);
					wData.push_back(barInfo[i][sqlFields::BARS::RATING]);

					//the location...
					std::map<std::string, std::string> barLocation=
						getBarLocation(userConnection, atoi(barInfo[i][sqlFields::BARS::ID].c_str()));

					if(barLocation.size() > 0)
					{
						if(barInfo[i][sqlFields::LOCATIONS::APTNO] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::APTNO);
							wData.push_back(barInfo[i][sqlFields::LOCATIONS::APTNO]);
						}
						if(barInfo[i][sqlFields::LOCATIONS::STREET] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::STREET);
							wData.push_back(barInfo[i][sqlFields::LOCATIONS::STREET]);
						}
						if(barInfo[i][sqlFields::LOCATIONS::CITY] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::CITY);
							wData.push_back(barInfo[i][sqlFields::LOCATIONS::CITY]);
						}
						if(barInfo[i][sqlFields::LOCATIONS::STATE] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::STATE);
							wData.push_back(barInfo[i][sqlFields::LOCATIONS::STATE]);
						}
						if(barInfo[i][sqlFields::LOCATIONS::ZIP] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::ZIP);
							wData.push_back(barInfo[i][sqlFields::LOCATIONS::ZIP]);
						}
						if(barInfo[i][sqlFields::LOCATIONS::COUNTRY] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::COUNTRY);
							wData.push_back(barInfo[i][sqlFields::LOCATIONS::COUNTRY]);
						}
					}

					//tell the client wats gucci
					writeConnection(this->sockfd, wData);
				}
			}

			break;
		}
		case commands::RATE_BAR://rate a bar
		{
			break;
		}
		case commands::RATE_EVENT://rate an event or special at a bar
		{
			break;
		}
		case commands::NEW_CHAT_MSG://chat message
		{
			if(data.size() < 3) break;

			//get the barid
			int barid=atoi(data[0].c_str());
			//get the eventid
			int eventid=atoi(data[1].c_str());
			//get the message
			std::string message=data[2];
			//current time
			int timestamp=time(NULL);
			//time since midnight
			int timeSinceFive=time(NULL)-(time(NULL)%(24*60*60));
			//time since 5am (after all bars are legally required to close, except in nevada)
			timeSinceFive+=(5*60*60);

			insertMessage(userConnection, barid, eventid, this->id, timestamp, message);

			//get the users
			std::map<int, user*> userList=getUserList();

			//now loop through all the users and send the message to them
			for(std::map<int, user*>::iterator itr=userList.begin();itr!=userList.end();++itr)
			{
				if(chatOpenedToday(userConnection, (itr->second)->id, barid, eventid, timeSinceFive))
				{
					std::vector<std::string> wData;
					wData.push_back("C");
					wData.push_back(intTOstring(barid));
					wData.push_back(intTOstring(eventid));
					wData.push_back(intTOstring(this->id));
					wData.push_back(this->ip);
					wData.push_back(intTOstring(timestamp));
					wData.push_back(message);

					//tell the client wats gucci
					writeConnection((itr->second)->sockfd, wData);
				}
			}

			break;
		}
		case commands::REQ_CHAT_MSGS://request new chat messages
		{
			if(data.size() < 3) break;

			//get the barid
			int barid=atoi(data[0].c_str());
			//get the eventid
			int eventid=atoi(data[1].c_str());
			//get the time to get from
			int fromTime=atoi(data[2].c_str());
			//time since midnight
			int timeSinceFive=time(NULL)-(time(NULL)%(24*60*60));
			//time since 5am (after all bars are legally required to close, except in nevada)
			timeSinceFive+=(5*60*60);

			std::vector<std::map<std::string, std::string> > newMessages=
				getChatMessages(userConnection, barid, eventid, fromTime, timeSinceFive);

			if(newMessages.size() > 0)
			{
				for(int i=0;i<newMessages.size();++i)
				{
					std::vector<std::string> wData;
					wData.push_back("C");
					wData.push_back(intTOstring(barid));
					wData.push_back(intTOstring(eventid));
					wData.push_back(newMessages[i][sqlFields::MESSAGES::UID]);
					wData.push_back(newMessages[i][sqlFields::REGUSERS::IP]);
					wData.push_back(newMessages[i][sqlFields::MESSAGES::TIMESTAMP]);
					wData.push_back(newMessages[i][sqlFields::MESSAGES::MESSAGE]);

					//tell the client wats gucci
					writeConnection(this->sockfd, wData);
				}
			}

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