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
void tables::readTableText(string name)
{
	ifstream inputFile(name + ".txt");
	string inputString;
	smatch sm;
	regex e("([^\\s]+)");

	this->_data.insert(pair<string, itemSet>(name, itemSet()));
	
	int count = 0;
	vector<string> itemName;
	int itemCount = 0;

	while (!getline(inputFile, inputString).eof()){		
		while (regex_search(inputString, sm, e, regex_constants::match_not_null))
		{
			if (count == 0)
			{
				itemName.push_back(sm[1]);
				this->_data[name].insert(pair<string, vector<string>>(sm[1], vector<string>()));
			}
			else
			{
				string temp = sm[1];
				this->_data[name][itemName[itemCount++]].push_back(sm[1]);
			}
			inputString = sm.suffix().str();
			
		}
		itemCount = 0;
		count++;
	}

}

void tables::OrderBy(itemAttribute item, vector<itemAttribute> prev_order, string ASCorDESC)
{

	
	int ASC_or_DESC = 0;
	if (ASCorDESC.compare("DESC") == 0)ASC_or_DESC = 1;


	int rows = this->_data.begin()->second.begin()->second.size();

	for (int row1 = 1; row1 < rows-1; row1++)
	for (int row2 = row1+1; row2 < rows; row2++)
	{
		bool shouldSwap = false;

		if (this->_data[item.table][item.item][0].compare("int")==0)
		{
			if (stoi(this->_data[item.table][item.item][row1])>stoi(this->_data[item.table][item.item][row2]))
				shouldSwap = (ASC_or_DESC == 0 ? true : false);
			else
				shouldSwap = (ASC_or_DESC == 1 ? true : false);
		}
		else
		{
			if (this->_data[item.table][item.item][row1].compare(this->_data[item.table][item.item][row2]) >= 0)
				shouldSwap = (ASC_or_DESC == 0 ? true : false);
			else
				shouldSwap = (ASC_or_DESC == 1 ? true : false);
		}

		for (int j = 0; j < prev_order.size();j++)
		if (this->_data[prev_order[j].table][prev_order[j].item][row1].compare(this->_data[prev_order[j].table][prev_order[j].item][row2]) != 0)
		{
			shouldSwap = false;
			break;
		}


		if (shouldSwap)
		for (tableSet::iterator it_T = this->_data.begin(); it_T != this->_data.end(); it_T++)
		for (itemSet::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
		{
			string temp;
			temp = it_I->second[row1];
			it_I->second[row1] = it_I->second[row2];
			it_I->second[row2] = temp;
		}
	}

}
void tables::insert(string name, itemSet itemS)
{
	_data.insert(pair<string, itemSet>(name, itemS));
}
