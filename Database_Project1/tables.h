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
	itemAttribute()
	{
	}
	itemAttribute(string express)
	{
		int pos = express.find(".");
		if (pos != string::npos)
		{
			this->table = express.substr(0, pos);
			this->item = express.substr(pos + 1);
		}
		else 
		{
			this->item = express;
		}

	}
};

typedef map<string, map<string, vector<string>>>tableSet;
typedef map<string, vector<string>> itemSet;
class tables
{
private:

tableSet _data;

public:
	itemSet& operator[] (const string & tableName)
	{
		return _data.at(tableName);
	}
	inline size_t size()
	{
		return _data.size();
	}
	void OrderBy(itemAttribute item, vector<itemAttribute> prev_order, string ASCorDESC);
	tableSet& getData()
	{
		return _data;
	}
	void readTableText(string name);
	void Tables_Output();
	void insert(string name, itemSet itemS);
};
