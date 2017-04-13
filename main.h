#ifndef _MAIN
#define _MAIN

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <map>
#include "socket.h"
#include "error.h"
#include "user.h"
#include "sql.h"
#include "version.h"

std::string getVersion();
std::map<int, user*> getUserList();
void stop();
int newUser(std::vector<std::string>&, const int&, const std::string&, int&);
void logout(int);
void executeCommand(std::string);
void* getCommands(void*);

#endif