#ifndef _SOCKET
#define _SOCKET

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <vector>

const std::string getPort();
bool isEscaped(const size_t, const std::string&);
size_t getNonEscapedDelimiter(const std::string&, const std::string&);
std::string encrypt(const std::string&);
int openConnection(int);
int readConnection(int, std::vector<std::string>&);
int writeConnection(int, std::vector<std::string>&);
void closeConnection(int);

#endif