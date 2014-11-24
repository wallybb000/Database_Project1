#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <map>
#include "tables.h"
using namespace std;
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
void tables::Tables_Output()
{
	for (tableSet::iterator it_T = this->_data.begin(); it_T != this->_data.end(); it_T++)
	{
		for (itemSet::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
			cout << it_I->first << "\t";

	}
	cout << endl;
	int rows = this->_data.begin()->second.begin()->second.size();
	for (int i = 0; i < rows; i++){
		for (tableSet::iterator it_T = this->_data.begin(); it_T != this->_data.end(); it_T++)
		{
			for (itemSet::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
				cout << it_I->second[i] << "\t";

		}
		cout << endl;
	}

}
