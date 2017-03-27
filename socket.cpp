#include "socket.h"
#include "error.h"

const std::string ANYADDR="0.0.0.0";
const std::string LOCALHOST="127.0.0.1";
const std::string PORT="42069";

const std::string getPort() { return PORT; }

bool isEscaped(const size_t index, const std::string& text)
{
	int counter=0;
    while((index > 0) && (text[index-counter-1] == '%'))
        ++counter;
    return (counter%2 != 0);
}

size_t getNonEscapedDelimiter(const std::string& text, const std::string& delimiter)
{
	size_t breakPoint=-1;
    do
    {
    	if(breakPoint == -1) breakPoint=text.find(delimiter.c_str());
    	else breakPoint=text.find(delimiter.c_str(), breakPoint+delimiter.length());
    } while(isEscaped(breakPoint, text));
    return breakPoint;
}

std::string encrypt(const std::string& text)
{
    std::string encText="";
    for(int i=0;i<text.length();++i)
        encText+=((unsigned char)text[i])+0x80;
    return encText;
}

int openConnection(int sockfd)
{	
	sockfd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockfd < 0) err("Could not open socket");

	int optval=1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	struct addrinfo *result;
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;

	int status=getaddrinfo(ANYADDR.c_str(), PORT.c_str(), &hints, &result);
	if(status < 0) err("Get addr info fail");

	status=bind(sockfd, result->ai_addr, result->ai_addrlen);
	if(status < 0) err("Could not bind server!");

	listen(sockfd, 5);

	return sockfd;
}

int readConnection(int sockfd, std::vector<std::string> &data)
{
    char buffer[1025];
    std::string text="";

    do
    {
        bzero(buffer, 1025);
        sockfd=read(sockfd, buffer, 1024);

        if(sockfd < 0)
            return -1;
        else if(sockfd > 0)//proper read
            text+=std::string(buffer);

    } while(sockfd == 1024);

	data.clear();

    size_t breakPoint=getNonEscapedDelimiter(text, "|");

    do
    {
        size_t breakPoint2=getNonEscapedDelimiter(text, "\\|");

        if((breakPoint == -1) || (breakPoint2 == -1))
            break;

        if(breakPoint == breakPoint2+1)//reached the end
            data.push_back(text.substr(0, breakPoint2));
        else
        	data.push_back(text.substr(0, breakPoint));

        text=text.substr(breakPoint+1);

	    breakPoint=getNonEscapedDelimiter(text, "|");

    } while(breakPoint > 0);

    for(int i=0;i<data.size();++i)
    {
    	//unescape the characters
    	std::string escText=data[i];
    	for(int j=0;j<escText.length();++j)
    	{
			if(escText[j] == '%')
				escText.erase(j, 1);
    	}
    	if(data[i].length() != escText.length()) data[i]=escText;
    }

	return sockfd;
}

int writeConnection(int sockfd, std::vector<std::string> &data)
{
    const std::string options="%\\|";

    std::string text="";
    for(int i=0;i<data.size();++i)
    {
    	//escape the characters
    	std::string escText=data[i];
    	for(int j=0;j<escText.length();++j)
    	{
    		size_t breakPoint=options.find(escText[j]);
			if(breakPoint != -1)
			{
				escText.insert(j, "%");
				++j;
			}
    	}
    	if(i < data.size()-1) text+=escText+"|";
    	else text+=escText+"\\|";
    }

    //text=encrypt(text);
    sockfd=write(sockfd, text.c_str(), text.length());

    return sockfd;
}

void closeConnection(int sockfd)
{
	close(sockfd);
}