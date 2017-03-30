#include "sql.h"

//tables
const std::string sqlTables::BARS="bars";
const std::string sqlTables::EVENTS="events";
const std::string sqlTables::DRINKS="drinks";
const std::string sqlTables::LOCATIONS="locations";
const std::string sqlTables::REGUSERS="regusers";
const std::string sqlTables::MESSAGES="messages";

//bar
const std::string sqlFields::BARS::ID="id";
const std::string sqlFields::BARS::NAME="name";
const std::string sqlFields::BARS::DESCRIPTION="description";
const std::string sqlFields::BARS::RATING="rating";
const std::string sqlFields::BARS::TIMESRATED="timesrated";

//event
const std::string sqlFields::EVENTS::ID="id";
const std::string sqlFields::EVENTS::NAME="name";
const std::string sqlFields::EVENTS::DESCRIPTION="description";
const std::string sqlFields::EVENTS::DRINKID="drinkid";
const std::string sqlFields::EVENTS::PRICE="price";
const std::string sqlFields::EVENTS::SPECIAL="special";

//drink
const std::string sqlFields::DRINKS::ID="id";
const std::string sqlFields::DRINKS::NAME="name";
const std::string sqlFields::DRINKS::DESCRIPTION="description";
const std::string sqlFields::DRINKS::PRICE="price";
const std::string sqlFields::DRINKS::BARID="barid";

//location
const std::string sqlFields::LOCATIONS::ID="id";
const std::string sqlFields::LOCATIONS::BARID="barid";
const std::string sqlFields::LOCATIONS::APTNO="aptno";
const std::string sqlFields::LOCATIONS::STREET="street";
const std::string sqlFields::LOCATIONS::CITY="city";
const std::string sqlFields::LOCATIONS::STATE="state";
const std::string sqlFields::LOCATIONS::ZIP="zip";
const std::string sqlFields::LOCATIONS::COUNTRY="country";

//messages
const std::string sqlFields::MESSAGES::ID="id";
const std::string sqlFields::MESSAGES::BARID="barid";
const std::string sqlFields::MESSAGES::EVENTID="eventid";
const std::string sqlFields::MESSAGES::UID="uid";
const std::string sqlFields::MESSAGES::TIMESTAMP="timestamp";
const std::string sqlFields::MESSAGES::MESSAGE="message";

//regular users
const std::string sqlFields::REGUSERS::ID="id";
const std::string sqlFields::REGUSERS::IP="ip";

MYSQL *mainConnection, mainMysql;

//INSERTS
void insertMessage(MYSQL *userConnection, int barid, int eventid, int uid, int timestamp, std::string message)
{
	std::string buffer;
    //escape sequence for the message
	char *escMsg=new char[message.length()*2+1];
	mysql_real_escape_string(userConnection, escMsg, message.c_str(), message.length());
	message.assign(escMsg);
	delete[] escMsg;
	buffer="INSERT INTO "+sqlTables::MESSAGES+"("+sqlFields::MESSAGES::BARID+", ";
	buffer+=sqlFields::MESSAGES::EVENTID+", "+sqlFields::MESSAGES::UID+", ";
	buffer+=sqlFields::MESSAGES::TIMESTAMP+", "+sqlFields::MESSAGES::MESSAGE+") ";
	buffer+="VALUES("+intTOstring(barid)+", "+intTOstring(eventid)+", ";
	buffer+=intTOstring(uid)+", "+intTOstring(timestamp)+", '"+message+"')";
	mysql_query(userConnection, buffer.c_str());
}

//SELECT
std::vector<std::map<std::string, std::string> >
getBars(MYSQL *userConnection)
{
	std::string buffer;
	int state;
	MYSQL_RES *result;
	MYSQL_FIELD *field;
	MYSQL_ROW row;
	buffer="SELECT * FROM "+sqlTables::BARS;
	state = mysql_query(userConnection, buffer.c_str());

	std::vector<std::map<std::string, std::string> > rows;
	if(state == 0)
	{
		result = mysql_store_result(userConnection);

		int num_fields=mysql_num_fields(result);
		char* fields[num_fields];
		for(int i=0;(field=mysql_fetch_field(result));++i)
			fields[i]=field->name;

		while(row=mysql_fetch_row(result))
		{
			std::map<std::string, std::string> newRow;
			for(int i=0;i<num_fields;++i)
				newRow.insert(std::pair<std::string, std::string>(fields[i], row[i]));
			rows.push_back(newRow);
		}

		mysql_free_result(result);
	}

	return rows;
}

std::map<std::string, std::string>
getBarLocation(MYSQL *userConnection, int barid)
{
	std::string buffer;
	int state;
	MYSQL_RES *result;
	MYSQL_FIELD *field;
	MYSQL_ROW row;
	buffer="SELECT * FROM "+sqlTables::LOCATIONS+" WHERE ";
	buffer+=sqlFields::LOCATIONS::BARID+"="+intTOstring(barid);
	state = mysql_query(userConnection, buffer.c_str());

	std::map<std::string, std::string> newRow;
	if(state == 0)
	{
		result = mysql_store_result(userConnection);

		int num_fields=mysql_num_fields(result);
		char* fields[num_fields];
		for(int i=0;(field=mysql_fetch_field(result));++i)
			fields[i]=field->name;

		while(row=mysql_fetch_row(result))
		{
			for(int i=0;i<num_fields;++i)
				newRow.insert(std::pair<std::string, std::string>(fields[i], row[i]));
			return newRow;
		}

		mysql_free_result(result);
	}

	return newRow;
}

bool chatOpenedToday(MYSQL *userConnection, int userid, int barid, int eventid, int timestamp)
{
	std::string buffer;
	int state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	buffer="SELECT COUNT(*) FROM "+sqlTables::MESSAGES+" WHERE ";
	buffer+=sqlFields::MESSAGES::UID+"="+intTOstring(userid)+" AND ";
	buffer+=sqlFields::MESSAGES::TIMESTAMP+">="+intTOstring(timestamp);
	state = mysql_query(userConnection, buffer.c_str());
	if(state == 0)
	{
		result = mysql_store_result(userConnection);
		row=mysql_fetch_row(result);
		mysql_free_result(result);
		if(row != NULL)
		{
			int users=atoi(row[0]);
			return (users > 0);
		}
		else return false;
	}
	return false;
}

std::vector<std::map<std::string, std::string> >
getChatMessages(MYSQL *userConnection, const int barid, const int eventid,
	const int fromTime, const int timeSinceFive)
{
	std::string buffer;
	int state;
	MYSQL_RES *result;
	MYSQL_FIELD *field;
	MYSQL_ROW row;
	buffer="SELECT * FROM "+sqlTables::MESSAGES+" JOIN "+sqlTables::REGUSERS+" ON ";
	buffer+=sqlTables::MESSAGES+"."+sqlFields::MESSAGES::UID+"=";
	buffer+=sqlTables::REGUSERS+"."+sqlFields::REGUSERS::ID+" WHERE ";
	buffer+=sqlTables::MESSAGES+"."+sqlFields::MESSAGES::BARID+"="+intTOstring(barid)+" AND ";
	buffer+=sqlTables::MESSAGES+"."+sqlFields::MESSAGES::EVENTID+"="+intTOstring(eventid)+" AND ";
	buffer+=sqlTables::MESSAGES+"."+sqlFields::MESSAGES::TIMESTAMP+">="+intTOstring(fromTime);
	if(eventid > 0)//bar chat
	{
		int timestamp2=0;
		buffer+="AND "+sqlTables::MESSAGES+"."+sqlFields::MESSAGES::TIMESTAMP+">="+intTOstring(timeSinceFive);
	}
	state = mysql_query(userConnection, buffer.c_str());

	std::vector<std::map<std::string, std::string> > rows;
	if(state == 0)
	{
		result = mysql_store_result(userConnection);

		int num_fields=mysql_num_fields(result);
		char* fields[num_fields];
		for(int i=0;(field=mysql_fetch_field(result));++i)
			fields[i]=field->name;

		while(row=mysql_fetch_row(result))
		{
			std::map<std::string, std::string> newRow;
			for(int i=0;i<num_fields;++i)
				newRow.insert(std::pair<std::string, std::string>(fields[i], row[i]));
			rows.push_back(newRow);
		}

		mysql_free_result(result);
	}

	return rows;
}

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
	mysql_real_connect(&mainMysql, "127.0.0.1", "barpi",
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
	mysql_real_connect(userConnection, "127.0.0.1", "barpi",
		"MySQL146", "medius_barpi", 5941, 0, 0);
	return userConnection;
}

void dbUserClose(MYSQL *userConnection)
{
     //close the connection
	mysql_close(userConnection);
	delete userConnection;
}