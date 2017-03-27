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
	static const std::string BAR;
	static const std::string EVENT;
	static const std::string DRINK;
	static const std::string COMMENT;
	static const std::string LOCATION;
};

struct sqlFields
{
	struct BAR
	{
		static const std::string ID;
		static const std::string NAME;
		static const std::string RATING;
	};//bars

	struct EVENT
	{
		static const std::string ID;
		static const std::string NAME;
		static const std::string DESCRIPTION;
		static const std::string DRINKID;
		static const std::string PRICE;
		static const std::string SPECIAL;
	};//events/specials

	struct DRINK
	{
		static const std::string ID;
		static const std::string NAME;
		static const std::string DESCRIPTION;
		static const std::string PRICE;
		static const std::string BARID;
	};//drinks

	struct COMMENT
	{
		static const std::string ID;
		static const std::string IP;
		static const std::string MESSAGE;
	};//comments

	struct LOCATION
	{
		static const std::string ID;
		static const std::string BARID;
		static const std::string APTNO;
		static const std::string STREET;
		static const std::string CITY;
		static const std::string STATE;
		static const std::string ZIP;
		static const std::string COUNTRY;
	};//addresses
};

MYSQL* getMainConnection();
bool dbConnect();
void dbClose();
MYSQL* dbUserConnect();
void dbUserClose(MYSQL*);

#endif