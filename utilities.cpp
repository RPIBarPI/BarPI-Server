#include "utilities.h"

std::string toUpper(std::string x)
{
       std::string y="";
       for(int i=0;i<x.length();i++)
       {
           char letter=*x.substr(i, 1).c_str();
           if((letter >= 0x61) && (letter <= 0x7A)) letter-=0x20;
           y+=letter;
       }
       return y;
}

std::string toLower(std::string x)
{
       std::string y="";
       for(int i=0;i<x.length();i++)
       {
           char letter=*x.substr(i, 1).c_str();
           if((letter >= 0x41) && (letter <= 0x5A)) letter+=0x20;
           y+=letter;
       }
       return y;
}

std::string trim(std::string x)
{
       while(x.substr(0, 1) == " ") x=x.substr(1);
       while(x.substr(x.length()-1, 1) == " ") x=x.substr(0, x.length()-1);
       return x;
}

std::string intTOstring(int number)
{
   std::stringstream ss;
   ss<<number;
   return ss.str();
}

std::string doubleTOstring(double number)
{
   std::stringstream ss;
   ss<<number;
   return ss.str();
}