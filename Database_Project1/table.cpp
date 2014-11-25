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
				this->_itemNameArray.push_back(itemAttribute(name,sm[1]));
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
void tables::insert(const string &name, itemSet itemS)
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
void tables::join( tables &input, string express ,bool isInner)
{
	string express_Item[4];
	bool flag=false;
	int count = 0;
	if (isInner)
	{
		smatch sm;
		regex e("(\\.|=|\\w+)");
		while (regex_search(express, sm, e, regex_constants::match_not_null))
		{
			if (sm[1].compare(".") == 0)
			{
				if (!flag)
				{
					if (count < 2)
					{
						for (tableSet::iterator it_T = input.getData().begin(); it_T != input.getData().end(); ++it_T)
						if (it_T->second.find(sm[1]) != it_T->second.end())
							express_Item[count - 1] = it_T->first;
					}
					else
					{
						for (tableSet::iterator it_T = this->_data.begin(); it_T != this->_data.end(); ++it_T)
						if (it_T->second.find(sm[1]) != it_T->second.end())
							express_Item[count - 1] = it_T->first;
					}

				}
				else
					flag = false;
			}
			else if (sm[1].compare("=") == 0)
			{
				count = 2;
			}
			else
			{
				express_Item[count++] = sm[1];
				flag = true;
			}

			express = sm.suffix().str();
		}
	}
	//////////////////////////////////////////////////

	count = 0;
	int A_count = 1;//避開0 第0項要存放型態
	int B_count = 1;

	tableSet tempTable;
	for (tableSet::iterator it_T = this->_data.begin(); it_T != this->_data.end(); ++it_T){
		tempTable.insert(pair<string,itemSet>(it_T->first, itemSet()));
		
		for (itemSet::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); ++it_I)
		{
	
			tempTable[it_T->first].insert(pair<string, vector<string>>(it_I->first, vector<string>()));
			tempTable[it_T->first][it_I->first].push_back(it_I->second[0]);//將表格中 第0項(型態)存入

		}
	}
	
	for (tableSet::iterator it_T = input.getData().begin(); it_T != input.getData().end(); it_T++){
		tempTable.insert(pair<string,itemSet>(it_T->first, itemSet()));
		for (itemSet::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
		{
			
			tempTable[it_T->first].insert(pair<string, vector<string>>(it_I->first, vector<string>()));
			tempTable[it_T->first][it_I->first].push_back(it_I->second[0]);
		}
	}


	

	for (B_count = 1; B_count < this->_data.begin()->second.begin()->second.size(); B_count++)
	for (A_count = 1; A_count < input.getData().begin()->second.begin()->second.size(); A_count++)
	{

		string L, R;
		//將input之table 存入L,R 若相反者亦可
		
		if (isInner)
			if (input.getData().find(express_Item[0]) != input.getData().end())
			{
				L = input[express_Item[0]][express_Item[1]][A_count];
				R = this->_data[express_Item[2]][express_Item[3]][B_count];
			}
			else if (this->_data.find(express_Item[0]) != this->_data.end())
			{
				L = this->_data[express_Item[0]][express_Item[1]][A_count];
				R = input[express_Item[2]][express_Item[3]][B_count];
			}

		//判斷條件是否成立,若是則將A與B整列資料存入temp
	
		if (!isInner || L.compare(R) == 0)
		{
			//插入B之列
			for (tableSet::iterator it_T = tempTable.begin(); it_T != tempTable.end(); it_T++){
				for (itemSet::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
				{
	
					if (this->_data.find(it_T->first)!=this->_data.end())
						it_I->second.push_back(this->_data[it_T->first][it_I->first][B_count]);
					else if (input.getData().find(it_T->first) != input.getData().end())
						it_I->second.push_back(input[it_T->first][it_I->first][A_count]);
				}
			}
		}
	}
	for (vector<itemAttribute>::iterator it_v = input._itemNameArray.begin(); it_v != input._itemNameArray.end(); it_v++)
		this->_itemNameArray.push_back(*it_v);
	this->_data.clear();
	this->_data= tempTable;

}

