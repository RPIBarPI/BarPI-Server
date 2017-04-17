#ifndef _SQL
#define _SQL

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <mysql/mysql.h>
#include "error.h"
#include "utilities.h"

struct sqlTables
{
	static const std::string BARS;
	static const std::string EVENTS;
	static const std::string DRINKS;
	static const std::string LOCATIONS;
	static const std::string SPECIALINFO;
	static const std::string MESSAGES;
	static const std::string REGUSERS;
	//dont need sessions (web only)
};

struct sqlFields
{
	struct BARS//dont need the username/password for a bar
	{
		static const std::string ID;
		static const std::string NAME;
		static const std::string DESCRIPTION;
		static const std::string RATING;
		static const std::string TIMESRATED;
	};//bars

	struct EVENTS
	{
		static const std::string ID;
		static const std::string BARID;
		static const std::string NAME;
		static const std::string DESCRIPTION;
		static const std::string ISEVENTTODAY;
	};//events/specials

	struct DRINKS
	{
		static const std::string ID;
		static const std::string NAME;
		static const std::string DESCRIPTION;
		static const std::string PRICE;
		static const std::string BARID;
		static const std::string ISONMENUTODAY;
	};//drinks

	struct LOCATIONS
	{
		static const std::string ID;
		static const std::string BARID;
		static const std::string APTNO;
		static const std::string STREET;
		static const std::string CITY;
		static const std::string STATE;
		static const std::string ZIP;
		static const std::string COUNTRY;
		static const std::string LONGITUDE;
		static const std::string LATITUDE;
	};//addresses

	struct SPECIALINFO
	{
		static const std::string ID;
		static const std::string EVENTID;
		static const std::string DRINKID;
		static const std::string PRICE;
	};//info for the specials

	struct MESSAGES
	{
		static const std::string ID;
		static const std::string BARID;
		static const std::string EVENTID;
		static const std::string UID;
		static const std::string TIMESTAMP;
		static const std::string MESSAGE;
	};//messages

	struct REGUSERS
	{
		static const std::string ID;
		static const std::string IP;
	};//regular users
};

//INSERT
int insertRegUser(std::string);
void insertMessage(MYSQL*, int, int, int, int, std::string);

//SELECT
std::vector<std::map<std::string, std::string> > getBars(MYSQL*);
std::vector<std::map<std::string, std::string> > getDrinks(MYSQL*, int);
std::vector<std::map<std::string, std::string> > getEvents(MYSQL*, int);
std::map<std::string, std::string> getBarLocation(MYSQL*, int);
bool chatOpenedToday(MYSQL*, int, int, int, int);
std::vector<std::map<std::string, std::string> >
getChatMessages(MYSQL*, const int, const int, const int, const int);

//OTHER
MYSQL* getMainConnection();
bool dbConnect();
void dbClose();
MYSQL* dbUserConnect();
void dbUserClose(MYSQL*);
bool checkRegID(int, std::string);

#endif