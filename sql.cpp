#include "sql.h"

//tables
const std::string sqlTables::BAR="bars";
const std::string sqlTables::EVENT="events";
const std::string sqlTables::DRINK="drinks";
const std::string sqlTables::COMMENT="comments";
const std::string sqlTables::LOCATION="locations";

//bar
const std::string sqlFields::BAR::ID="id";
const std::string sqlFields::BAR::NAME="name";
const std::string sqlFields::BAR::RATING="rating";

//event
const std::string sqlFields::EVENT::ID="id";
const std::string sqlFields::EVENT::NAME="name";
const std::string sqlFields::EVENT::DESCRIPTION="description";
const std::string sqlFields::EVENT::DRINKID="drinkid";
const std::string sqlFields::EVENT::PRICE="price";
const std::string sqlFields::EVENT::SPECIAL="special";

//drink
const std::string sqlFields::DRINK::ID="id";
const std::string sqlFields::DRINK::NAME="name";
const std::string sqlFields::DRINK::DESCRIPTION="description";
const std::string sqlFields::DRINK::PRICE="price";
const std::string sqlFields::DRINK::BARID="barid";

//comment
const std::string sqlFields::COMMENT::ID="id";
const std::string sqlFields::COMMENT::IP="ip";
const std::string sqlFields::COMMENT::MESSAGE="message";

//location
const std::string sqlFields::LOCATION::ID="id";
const std::string sqlFields::LOCATION::BARID="barid";
const std::string sqlFields::LOCATION::APTNO="aptno";
const std::string sqlFields::LOCATION::STREET="street";
const std::string sqlFields::LOCATION::CITY="city";
const std::string sqlFields::LOCATION::STATE="state";
const std::string sqlFields::LOCATION::ZIP="zip";
const std::string sqlFields::LOCATION::COUNTRY="country";

MYSQL *mainConnection, mainMysql;

//OTHER

MYSQL* getMainConnection()
{
	return mainConnection;
}

bool dbConnect()
{
	//connect
	mysql_library_init(0, NULL, NULL);
	mysql_init(&mainMysql);
	my_bool reconnect=1;
	mysql_options(&mainMysql, MYSQL_OPT_RECONNECT, &reconnect);
	mainConnection =
	mysql_real_connect(&mainMysql, "seanwaclawik.com", "barpi",
		"MySQL146", "medius_barpi", 5941, 0, 0);
	if(mainConnection == NULL)
	{
		err(mysql_error(&mainMysql));
		return false;
	}
	else return true;
}

void dbClose()
{
     //close the db and clean it up
	mysql_close(mainConnection);
	mysql_library_end();
}

MYSQL* dbUserConnect()
{
    //connect a thread
    MYSQL *userConnection=new MYSQL[sizeof(MYSQL)];

	mysql_init(userConnection);
	my_bool reconnect=1;
	mysql_options(userConnection, MYSQL_OPT_RECONNECT, &reconnect);
	userConnection =
	mysql_real_connect(userConnection, "robot-universe.com", "medius_appointed",
		"X.IB@wKmJ4#U", "medius_appointed", 0, 0, 0);
	return userConnection;
}

void dbUserClose(MYSQL *userConnection)
{
     //close the connection
	mysql_close(userConnection);
	delete userConnection;
}