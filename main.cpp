#include "main.h"

#define MAX_CONNECTIONS 100

struct sqlTables;
struct sqlFields;

std::map<int, user*> userList;//reguserid, user pointer

bool running=true;
pthread_mutex_t *mutex;
std::string version="0.02";

std::string getVersion()
{
	return version;
}

std::map<int, user*> getUserList()
{
	return userList;
}

void stop()
{
	running=false;
}

int newUser(std::vector<std::string> &data, const int& sockfd, const std::string& fromIP, int& regUserID)
{
	if(data.size() < 3) return 2;

	std::string clientVersion=data[0];
	if(strcmp(getVersion().c_str(), clientVersion.c_str()) != 0) return 1;

	//get the login mode, CONNECT to connect, PING for ping
	std::string loginMode=data[1];

	if(loginMode == "CONNECT")//connect
	{
		//userid (0 if has none)
		regUserID=atoi(data[2].c_str());

		if(regUserID == 0)
		{
			//assign them one and send it back
			regUserID=insertRegUser(fromIP);
		}
		else if(regUserID > 0)
		{
			//if its a different id than in the database, give them a new id
			if(!checkRegID(regUserID, fromIP))
			{
				regUserID=insertRegUser(fromIP);
			}
		}
		else return 4;

		//set the user and add it to the data structure
		pthread_mutex_lock(mutex);
		user *tempUser=new user(sockfd, regUserID, fromIP);
		userList.insert(std::pair<int, user*>(regUserID, tempUser));
		pthread_mutex_unlock(mutex);
	}
	else if(loginMode == "PING")//ping
	{
		return 3;//pong
	}

	return 0;
}

void logout(int regUserID)
{
	//does the user exist in the map?
	if(userList.find(regUserID) != userList.end())
	{
		//close the connection
		close(userList[regUserID]->sockfd);

		//save the user locally
		user *tempUser=userList[regUserID];

		//delete it from the map
		pthread_mutex_lock(mutex);
		userList.erase(userList.find(regUserID));
		pthread_mutex_unlock(mutex);

		//delete the user
		delete tempUser;
	}
}

void executeCommand(std::string command)
{
     //list of commands
     std::string commandList[]={//ALL COMMANDS ARE LOWERCASE
     "shutdown",
     "close",
     "end",
     "exit",
     "forceclose",
     "kill",
     "help",
     "dbreconnect",
     "q",
     "quit"
     };
     //get the command and execute it
     size_t breakPoint=command.find(" ");
     std::string commandArgs;
     std::vector<std::string> cArgList;//for the arg list
     if(breakPoint != -1)//you specified some args
     {
      int lastArgIndex;
      commandArgs=command.substr(breakPoint+1);
      command=command.substr(0, breakPoint);
      breakPoint=commandArgs.find(",");
      //get all the args
      while(breakPoint != -1)//another arg
      {
       cArgList.push_back(commandArgs.substr(0, breakPoint));
       lastArgIndex=cArgList.size()-1;
       cArgList[lastArgIndex]=trim(cArgList[lastArgIndex]);
       commandArgs=commandArgs.substr(breakPoint+1);
       breakPoint=commandArgs.find(",");
      }
      cArgList.push_back(commandArgs);//add the last arg
      lastArgIndex=cArgList.size()-1;
      cArgList[lastArgIndex]=trim(cArgList[lastArgIndex]);
     }
     command=toLower(command);
     int commandArgsNo=cArgList.size();//amount of commands
     int arrSize=sizeof(commandList)/sizeof(commandList[0]);
     bool commandExecuted=false;
     int i;
     for(i=0;i<arrSize;i++)
     {
      if(command != commandList[i]) continue;//not this one
      commandExecuted=true;
      switch(i)
      {
       //which command?
       case 0: case 1: case 2: case 3: case 8: case 9://shutdown safely
            printf("Shutting down...\n");
            running=false;
            break;
       case 4: case 5:
            exit(2);//force close and kill
            break;
       case 6://help
            printf("Separate parameters by commas\n");
            printf("The command list:\n");
            int e;
            for(e=0;e<arrSize;e++)
            {
             //display all the commands
             printf("%d. %s\n", e, commandList[e].c_str());
            }
            break;
       case 7:
            if(dbConnect()) printf("Reconnected to the database\n");
            else printf("Reconnection failed\n");
            break;
      }
     }
     if(commandExecuted == false) printf("Invalid command\n");
}

void* getCommands(void*)
{
     //wait for commands
     std::string command;
     while(running)
     {
      printf("Enter a command: ");
      getline(std::cin, command);
      executeCommand(command);
     }
}

int main(int argc, char *argv[])
{
	//version
	printf("%s\n", header().c_str());

	//database
	if(!dbConnect()) err("Could not connect to the database\n");
	else printf("Connected to the database\n");

	//mutex
	mutex=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));

	//command line
	pthread_t commandThread;
	pthread_create(&commandThread, NULL, getCommands, NULL);

	int sockfd;
	int max_sock=0;
	
	sockfd=openConnection(sockfd);
	if(sockfd < 0) err("Could not create socket\n");
	else printf("Listening on port %s\n", getPort().c_str());

	while(running)//the engine
	{
		fd_set fdarr;
		struct timeval tv;
		tv.tv_sec=10;

		FD_ZERO(&fdarr);
		FD_SET(sockfd, &fdarr);
		max_sock=sockfd;

		//set the max sock
		for(std::map<int, user*>::iterator itr=userList.begin();itr!=userList.end();++itr)
		{
			FD_SET((itr->second)->sockfd, &fdarr);
			if((itr->second)->sockfd > max_sock) max_sock=(itr->second)->sockfd;
		}

		int status=select(max_sock+1, &fdarr, NULL, NULL, &tv);
		if(status < 0) err("Socket select call error\n");
		else if(status == 0) continue;

		if(FD_ISSET(sockfd, &fdarr))
		{
			struct sockaddr_in from;
			socklen_t clientLength=sizeof(from);

			int sockfd2=accept(sockfd, (struct sockaddr*)&from, &clientLength);
			if(sockfd2 < 0) err("Could not accept connection\n");

			if(max_sock < MAX_CONNECTIONS)
			{
				//read the init info
				std::vector<std::string> data;
				int bytesRead=readConnection(sockfd2, data);
				if(bytesRead == 0) continue;//invalid, ignore and listen for the next connection

				//text: "CONNECT|0.01|123\|"

				//get the ip
				char fromIP[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &from, fromIP, INET_ADDRSTRLEN);
				std::string tempFromIP=fromIP;

				//the user id
				int regUserID=0;

				//login the new user
				int loginStatus=newUser(data, sockfd2, tempFromIP, regUserID);

				/* loginStatus info
				* 0: success
				* 1: Bad version
				* 2: badly formatted packet
				* 3: Ping
				* 4: Invalid user id
				*/

				if(loginStatus == 0)
				{
					printf("Login: %s\n", tempFromIP.c_str());

					//tell the client the good news
					std::vector<std::string> data;
					data.push_back("S");//S for success
					writeConnection(sockfd2, data);

					//populate the client
					data.clear();
					data.push_back("0");
					data.push_back("password");

					//launch in a new service so the server can keep listening
					userList[regUserID]->newService(data);
				}
				else//send back the error message
				{
					std::vector<std::string> data;
					if(loginStatus != 3)
					{
						printf("Login Failed: %s (%d)\n", tempFromIP.c_str(), loginStatus);
						data.push_back("E");//E for error
						data.push_back(intTOstring(loginStatus));
					}
					else
					{
						printf("Ping: %s\n", tempFromIP.c_str());
						data.push_back("P");//P for ping
						data.push_back(intTOstring(time(NULL)));//P for ping
					}
					writeConnection(sockfd2, data);
				}
			}
		}
		else//not a new connection
		{
			for(std::map<int, user*>::iterator itr=userList.begin();itr!=userList.end();++itr)
			{
				if(FD_ISSET((itr->second)->sockfd, &fdarr))
				{
					std::vector<std::string> data;
					int bytesRead=readConnection((itr->second)->sockfd, data);
					if(bytesRead <= 0)
					{
						printf("Logout: %s\n", (itr->second)->ip.c_str());
						logout((itr->second)->id);
					}
					else if(bytesRead > 0)
					{
						//launch the service with the command
						(itr->second)->newService(data);
					}
				}
			}
		}
	}
	running=false;
	close(sockfd);
	//join the threads
	pthread_join(commandThread, NULL);

	for(std::map<int, user*>::iterator itr=userList.begin();itr!=userList.end();++itr)
	{
		for(int i=0;i<(itr->second)->sThreads.size();++i)
			pthread_join(*(itr->second)->sThreads[i], NULL);
	}

	free(mutex);

	dbClose();

    return 0;
}