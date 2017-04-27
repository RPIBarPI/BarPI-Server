#include "user.h"
#include "main.h"
#include "socket.h"
#include "sql.h"

const int commands::POPULATE_ALL=0;
const int commands::RATE_BAR=1;
const int commands::RATE_EVENT=2;
const int commands::NEW_CHAT_MSG=3;
const int commands::REQ_CHAT_MSGS=4;
const int commands::REQ_BAR_DRINKS=5;
const int commands::REQ_BAR_EVENTS=6;
const int commands::REQ_BAR_SPECIALS=7;
const int commands::REQ_BAR_FULL=8;

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

			std::vector<std::string> wData;

			//tell the user their id
			wData.push_back("I");
			wData.push_back(intTOstring(this->id));

			//tell the client wats gucci
			writeConnection(this->sockfd, wData);
			wData.clear();

			//give the client bar info (eventually by coordinates and proximity)
			std::vector<std::map<std::string, std::string> > barInfo=
				getBars(userConnection);

			if(barInfo.size() > 0)
			{
				for(int i=0;i<barInfo.size();++i)
				{
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
					wData.push_back(sqlFields::BARS::TIMESRATED);
					wData.push_back(barInfo[i][sqlFields::BARS::TIMESRATED]);

					//the location...
					std::map<std::string, std::string> barLocation=
						getBarLocation(userConnection, atoi(barInfo[i][sqlFields::BARS::ID].c_str()));

					if(barLocation.size() > 0)
					{
						if(barLocation[sqlFields::LOCATIONS::APTNO] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::APTNO);
							wData.push_back(barLocation[sqlFields::LOCATIONS::APTNO]);
						}
						if(barLocation[sqlFields::LOCATIONS::STREET] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::STREET);
							wData.push_back(barLocation[sqlFields::LOCATIONS::STREET]);
						}
						if(barLocation[sqlFields::LOCATIONS::CITY] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::CITY);
							wData.push_back(barLocation[sqlFields::LOCATIONS::CITY]);
						}
						if(barLocation[sqlFields::LOCATIONS::STATE] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::STATE);
							wData.push_back(barLocation[sqlFields::LOCATIONS::STATE]);
						}
						if(barLocation[sqlFields::LOCATIONS::ZIP] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::ZIP);
							wData.push_back(barLocation[sqlFields::LOCATIONS::ZIP]);
						}
						if(barLocation[sqlFields::LOCATIONS::COUNTRY] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::COUNTRY);
							wData.push_back(barLocation[sqlFields::LOCATIONS::COUNTRY]);
						}
						if(barLocation[sqlFields::LOCATIONS::LONGITUDE] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::LONGITUDE);
							wData.push_back(barLocation[sqlFields::LOCATIONS::LONGITUDE]);
						}
						if(barLocation[sqlFields::LOCATIONS::LATITUDE] != "")
						{
							wData.push_back(sqlFields::LOCATIONS::LATITUDE);
							wData.push_back(barLocation[sqlFields::LOCATIONS::LATITUDE]);
						}
					}

					//tell the client wats gucci
					writeConnection(this->sockfd, wData);
					wData.clear();
				}
			}

			break;
		}
		case commands::RATE_BAR://rate a bar
		{
			if(data.size() < 4) break;

			//get the barid
			int barid=atoi(data[0].c_str());

			//get the eventid (0=bar)
			int eventid=atoi(data[1].c_str());

			//get the rating
			float rating=atof(data[2].c_str());

			//has the user already rated the bar?
			int ratedAlready=atoi(data[3].c_str());

			if((ratedAlready < 0) || (ratedAlready > 1))
				ratedAlready=0;

			if(eventid > 0)//event rating
			{
				//update the new event rating
				updateEventRating(userConnection, eventid, rating, !ratedAlready);

				//get the new rating of a bar
				std::map<std::string, std::string> eventInfo=
					getEventRating(userConnection, eventid);

				//populate wData
				std::vector<std::string> wData;
				wData.push_back("H");
				wData.push_back(eventInfo[sqlFields::BARS::ID]);
				wData.push_back(sqlFields::BARS::RATING);
				wData.push_back(eventInfo[sqlFields::BARS::RATING]);
				wData.push_back(sqlFields::BARS::TIMESRATED);
				wData.push_back(eventInfo[sqlFields::BARS::TIMESRATED]);

				//tell the client wats gucci
				writeConnection(this->sockfd, wData);
			}
			else
			{
				//update the new bar rating
				updateBarRating(userConnection, barid, rating, !ratedAlready);

				//get the new rating of a bar
				std::map<std::string, std::string> barInfo=
					getBarRating(userConnection, barid);

				//populate wData
				std::vector<std::string> wData;
				wData.push_back("B");
				wData.push_back(barInfo[sqlFields::BARS::ID]);
				wData.push_back(sqlFields::BARS::RATING);
				wData.push_back(barInfo[sqlFields::BARS::RATING]);
				wData.push_back(sqlFields::BARS::TIMESRATED);
				wData.push_back(barInfo[sqlFields::BARS::TIMESRATED]);

				//tell the client wats gucci
				writeConnection(this->sockfd, wData);
			}

			break;
		}
		case commands::RATE_EVENT://rate an event or special at a bar
		{
			//unneeded
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
			//???
			timeSinceFive-=(24*60*60);
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
		case commands::REQ_BAR_DRINKS://request the drinks that a bar is serving
		{
			if(data.size() < 1) break;

			int barid=atoi(data[0].c_str());

			std::vector<std::string> wData;

			//give the client the bar's menu (drinks)
			std::vector<std::map<std::string, std::string> > drinkList=
				getDrinks(userConnection, barid);

			if(drinkList.size() > 0)
			{
				for(int i=0;i<drinkList.size();++i)
				{
					wData.push_back("D");
					wData.push_back(data[0].c_str());//bar id
					wData.push_back(drinkList[i][sqlFields::DRINKS::ID]);

					if(drinkList[i][sqlFields::DRINKS::NAME] != "")
					{
						wData.push_back(sqlFields::DRINKS::NAME);
						wData.push_back(drinkList[i][sqlFields::DRINKS::NAME]);
					}

					if(drinkList[i][sqlFields::DRINKS::DESCRIPTION] != "")
					{
						wData.push_back(sqlFields::DRINKS::DESCRIPTION);
						wData.push_back(drinkList[i][sqlFields::DRINKS::DESCRIPTION]);
					}

					if(drinkList[i][sqlFields::DRINKS::PRICE] != "")
					{
						wData.push_back(sqlFields::DRINKS::PRICE);
						wData.push_back(drinkList[i][sqlFields::DRINKS::PRICE]);
					}

					//tell the client wats gucci
					writeConnection(this->sockfd, wData);
					wData.clear();
				}
			}

			break;
		}
		case commands::REQ_BAR_EVENTS://request the events of a bar
		{
			if(data.size() < 1) break;

			int barid=atoi(data[0].c_str());

			std::vector<std::string> wData;

			//give the client the bar's events
			std::vector<std::map<std::string, std::string> > eventList=
				getEvents(userConnection, barid);

			if(eventList.size() > 0)
			{
				for(int i=0;i<eventList.size();++i)
				{
					wData.push_back("H");
					wData.push_back(data[0].c_str());//bar id
					wData.push_back(eventList[i][sqlFields::EVENTS::ID]);

					if(eventList[i][sqlFields::EVENTS::NAME] != "")
					{
						wData.push_back(sqlFields::EVENTS::NAME);
						wData.push_back(eventList[i][sqlFields::EVENTS::NAME]);
					}

					if(eventList[i][sqlFields::EVENTS::DESCRIPTION] != "")
					{
						wData.push_back(sqlFields::EVENTS::DESCRIPTION);
						wData.push_back(eventList[i][sqlFields::EVENTS::DESCRIPTION]);
					}

					wData.push_back(sqlFields::EVENTS::RATING);
					wData.push_back(eventList[i][sqlFields::EVENTS::RATING]);
					wData.push_back(sqlFields::EVENTS::TIMESRATED);
					wData.push_back(eventList[i][sqlFields::EVENTS::TIMESRATED]);

					//tell the client wats gucci
					writeConnection(this->sockfd, wData);
					wData.clear();
				}
			}

			break;
		}
		case commands::REQ_BAR_SPECIALS://request the specials of a bar
		{
			if(data.size() < 1) break;

			int barid=atoi(data[0].c_str());

			std::vector<std::string> wData;

			//give the client the bar's specials
			std::vector<std::map<std::string, std::string> > specialList=
				getSpecials(userConnection, barid);

			if(specialList.size() > 0)
			{
				for(int i=0;i<specialList.size();++i)
				{
					wData.push_back("S");
					wData.push_back(data[0].c_str());//bar id
					wData.push_back(specialList[i][sqlFields::SPECIALINFO::ID]);

					if(specialList[i][sqlFields::SPECIALINFO::EVENTID] != "")
					{
						wData.push_back(sqlFields::SPECIALINFO::EVENTID);
						wData.push_back(specialList[i][sqlFields::SPECIALINFO::EVENTID]);
					}

					if(specialList[i][sqlFields::SPECIALINFO::DRINKID] != "")
					{
						wData.push_back(sqlFields::SPECIALINFO::DRINKID);
						wData.push_back(specialList[i][sqlFields::SPECIALINFO::DRINKID]);
					}

					if(specialList[i][sqlFields::SPECIALINFO::PRICE] != "")
					{
						wData.push_back(sqlFields::SPECIALINFO::PRICE);
						wData.push_back(specialList[i][sqlFields::SPECIALINFO::PRICE]);
					}

					//tell the client wats gucci
					writeConnection(this->sockfd, wData);
					wData.clear();
				}
			}

			break;
		}
		case commands::REQ_BAR_FULL:
		{
			if(data.size() < 1) break;

			int barid=atoi(data[0].c_str());

			std::vector<std::string> wData;

			//give the client the bar's menu (drinks)
			std::vector<std::map<std::string, std::string> > drinkList=
				getDrinks(userConnection, barid);

			if(drinkList.size() > 0)
			{
				for(int i=0;i<drinkList.size();++i)
				{
					wData.push_back("D");
					wData.push_back(data[0].c_str());//bar id
					wData.push_back(drinkList[i][sqlFields::DRINKS::ID]);

					if(drinkList[i][sqlFields::DRINKS::NAME] != "")
					{
						wData.push_back(sqlFields::DRINKS::NAME);
						wData.push_back(drinkList[i][sqlFields::DRINKS::NAME]);
					}

					if(drinkList[i][sqlFields::DRINKS::DESCRIPTION] != "")
					{
						wData.push_back(sqlFields::DRINKS::DESCRIPTION);
						wData.push_back(drinkList[i][sqlFields::DRINKS::DESCRIPTION]);
					}

					if(drinkList[i][sqlFields::DRINKS::PRICE] != "")
					{
						wData.push_back(sqlFields::DRINKS::PRICE);
						wData.push_back(drinkList[i][sqlFields::DRINKS::PRICE]);
					}

					//tell the client wats gucci
					writeConnection(this->sockfd, wData);
					wData.clear();
				}
			}

			//give the client the bar's events
			std::vector<std::map<std::string, std::string> > eventList=
				getEvents(userConnection, barid);

			if(eventList.size() > 0)
			{
				for(int i=0;i<eventList.size();++i)
				{
					wData.push_back("H");
					wData.push_back(data[0].c_str());//bar id
					wData.push_back(eventList[i][sqlFields::EVENTS::ID]);

					if(eventList[i][sqlFields::EVENTS::NAME] != "")
					{
						wData.push_back(sqlFields::EVENTS::NAME);
						wData.push_back(eventList[i][sqlFields::EVENTS::NAME]);
					}

					if(eventList[i][sqlFields::EVENTS::DESCRIPTION] != "")
					{
						wData.push_back(sqlFields::EVENTS::DESCRIPTION);
						wData.push_back(eventList[i][sqlFields::EVENTS::DESCRIPTION]);
					}

					wData.push_back(sqlFields::EVENTS::RATING);
					wData.push_back(eventList[i][sqlFields::EVENTS::RATING]);
					wData.push_back(sqlFields::EVENTS::TIMESRATED);
					wData.push_back(eventList[i][sqlFields::EVENTS::TIMESRATED]);

					//tell the client wats gucci
					writeConnection(this->sockfd, wData);
					wData.clear();
				}
			}

			//give the client the bar's specials
			std::vector<std::map<std::string, std::string> > specialList=
				getSpecials(userConnection, barid);

			if(specialList.size() > 0)
			{
				for(int i=0;i<specialList.size();++i)
				{
					wData.push_back("S");
					wData.push_back(data[0].c_str());//bar id
					wData.push_back(specialList[i][sqlFields::SPECIALINFO::ID]);

					if(specialList[i][sqlFields::SPECIALINFO::EVENTID] != "")
					{
						wData.push_back(sqlFields::SPECIALINFO::EVENTID);
						wData.push_back(specialList[i][sqlFields::SPECIALINFO::EVENTID]);
					}

					if(specialList[i][sqlFields::SPECIALINFO::DRINKID] != "")
					{
						wData.push_back(sqlFields::SPECIALINFO::DRINKID);
						wData.push_back(specialList[i][sqlFields::SPECIALINFO::DRINKID]);
					}

					if(specialList[i][sqlFields::SPECIALINFO::PRICE] != "")
					{
						wData.push_back(sqlFields::SPECIALINFO::PRICE);
						wData.push_back(specialList[i][sqlFields::SPECIALINFO::PRICE]);
					}

					//tell the client wats gucci
					writeConnection(this->sockfd, wData);
					wData.clear();
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