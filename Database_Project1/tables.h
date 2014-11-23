#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <map>
using namespace std;

class itemAttribute
{
public:
	string table;
	string item;
	int use = 0;
	int count = 0;
	
itemAttribute(string express);
};


class tables
{
private:

 map<string, map<string, vector<string>>> tables;

public:

void OrderBy( itemAttribute item, vector<itemAttribute> prev_order, string ASCorDESC);


};