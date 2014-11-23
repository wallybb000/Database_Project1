#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <map>
using namespace std;

struct name_tableAnditem{
	string table;
	string item;
	int use = 0;
	int count = 0;
};
void InsertForName_tableAnditem(name_tableAnditem& input, string express);



typedef map<string, map<string, vector<string>>> tables;
void OrderBy(tables & allData, name_tableAnditem item, vector<name_tableAnditem> prev_order, string ASCorDESC);
void Tables_Output(tables &input);
typedef map<string, vector<string>> items;
void readTableText(string name, tables & input_Table);
		

bool allItem_ShouldBeShow=false;
bool isDistinct=false;
map<string, string> asforTable;
map<string, name_tableAnditem> asforItem;//TODO
vector<name_tableAnditem> item_Count;
vector<name_tableAnditem> item_ShouldBeShow;
//// ///////////////////////////////////////////////////////////////////////////////////////////
void Select(tables &allData, string str_Select);
tables Process_From(vector<string> input);
void Join(tables &inputA, tables &inputB, string express, bool isInner);
void InnerJoin_Process(vector<string> inputStrings, tables &outputTables);
//// ///////////////////////////////////////////////////////////////////////////////////////////
void getAllSign(vector<string> &allSign, string tempStr);
bool mathCalculate(vector<string> &mathStack, tables &allData);
bool logicCalculate(vector<string> &logicStack);
bool calculate(vector<string> &allSign, tables &allData, int row);



void Select(tables &allData, string str_Input)
{
	
	//處理From
	smatch sm;
	regex e("\\s(\\w+)\\s");
	string allStr_From;
	string allStr_Select;
	string allStr_Where;
	string allStr_Order;
	


	vector <tables> allTables;
	allItem_ShouldBeShow = false;
	regex_search(str_Input, sm, e, regex_constants::match_not_null);
	//todo 分出各種用途之字串
	int location[5] = { 0, str_Input.find("FROM"), str_Input.find("WHERE"), str_Input.find("ORDER BY"), str_Input.find(";") };

	int temp_StrEnd=0;
	for (int i = 1; i < 5; i++)
	if (location[i]>0)
	{
		allStr_Select = str_Input.substr(0, location[i]); 
		break;
	}
	for (int i = 2; i < 5; i++)
	if (location[1]>0&&location[i]>0)
	{
		allStr_From = str_Input.substr(location[1] + 4, location[i] - location[1] - 4);
		break;
	}
	for (int i = 3; i < 5; i++)
	if (location[2]>0 && location[i]>0)
	{
		allStr_Where = str_Input.substr(location[2] + 5, location[i] - location[2] - 5);
		break;
	}
	for (int i = 4; i < 5; i++)
	if (location[3]>0 && location[i]>0)
	{
		allStr_Order = str_Input.substr(location[3] + 8, location[i] - location[3] - 8);
		break;
	}


	////////////////////////////////////////////////////////////////
	//先行新增 item的新名庫
	cout << "先行新增 item的新名庫" << endl;
	int tempStrBegin = allStr_Select.find("SELECT") + 6;
	string tempStr_Select = allStr_Select.substr(tempStrBegin);
	e.assign("([^,]+)");
	while (regex_search(tempStr_Select, sm, e, regex_constants::match_not_null))
	{
		string temp2Str_Select = sm[1];
		vector<string> each_Item;
		smatch sm2;
		regex e2("([^\\(\\)\\s]+)");
		while (regex_search(temp2Str_Select, sm2, e2, regex_constants::match_not_null))
		{
			each_Item.push_back(sm2[1]);
			temp2Str_Select = sm2.suffix().str();
		}

		if (each_Item[0].compare("COUNT") == 0)
		{

			name_tableAnditem temp;

			InsertForName_tableAnditem(temp, each_Item[1]);
			if (each_Item[1].compare("DISTINCT")==0)
			temp.use = 2;
			else
				temp.use = 1;

			item_ShouldBeShow.push_back(temp);
			if (each_Item.size() >3 )
			{
				asforItem.insert(pair<string, name_tableAnditem>(each_Item[each_Item.size()-1], temp)); //加入新名庫
			}
			//TODO 建置count庫
			item_Count.push_back(temp);
			
		}
		else if (each_Item[0].compare("DISTINCT") == 0)
		{
			isDistinct = true;
			name_tableAnditem temp;
			InsertForName_tableAnditem(temp, each_Item[1]);
			temp.use = 3;

			item_ShouldBeShow.push_back(temp);
			if (each_Item.size() > 2)
			{
				asforItem.insert(pair<string, name_tableAnditem>(each_Item[3], temp)); //加入新名庫
			}

		}
		else 
		{

			if (each_Item[0].compare("*") == 0){
				allItem_ShouldBeShow = true;
				name_tableAnditem temp;
				temp.item = "*";
				item_ShouldBeShow.push_back(temp);
			}
			else
			{
				name_tableAnditem temp;
				InsertForName_tableAnditem(temp, each_Item[0]);
				item_ShouldBeShow.push_back(temp);

				if (each_Item.size() > 1 && each_Item[1].compare("AS") == 0)
					asforItem.insert(pair<string, name_tableAnditem>(each_Item[2], temp)); //加入新名庫
			}
		}

		tempStr_Select = sm.suffix().str();
	}
	
	
	////////////////////////////////////////////////////////////////
	//計算FROM字串區域
	cout << "計算FROM字串區域" << endl;
	e.assign("([^,]+)");
	while (regex_search(allStr_From, sm, e, regex_constants::match_not_null))
	{

		string each_Table= sm[1];
		smatch sm2;
		regex e2("([\\(\\)]|INNER JOIN||[\\S]+\\s?=\\s?[\\S]+|[\\S]+)");
		vector<string> str_From;
		while (regex_search(each_Table, sm2, e2, regex_constants::match_not_null))
		{
			str_From.push_back(sm2[1]);
			each_Table = sm2.suffix().str();
		}
		
		if (str_From.size() == 1)
		{
			tables temp;
			readTableText(str_From[0], temp);
			allTables.push_back(temp);

		}
		else if (str_From.size() == 3)
		{
			tables temp;
			readTableText(str_From[0], temp);
			allTables.push_back(temp);

			asforTable.insert(pair<string,string>(str_From[2], str_From[0]));// 將新名加入新名庫
		}
		else if (str_From.size() > 3)
		{
			tables temp;
			InnerJoin_Process(str_From,temp);
			allTables.push_back(temp);

		}
		allStr_From = sm.suffix().str();

	}

	if (allTables.size() != 1)
	{
		string str;
		for (int i = 1; i < allTables.size(); i++)
		{	
			Join(allTables[i], allTables[0], str, false);
		
		}

	}

	allData = allTables[0];
	////////////////////////////////////////////////////////////////
	//計算WHERE字串區域
	cout << "計算WHERE字串區域" << endl;
	if (!allStr_Where.empty())
	{
		tables temp_allData;
		for (tables::iterator it_T = allData.begin(); it_T != allData.end(); it_T++)
		{
			temp_allData.insert(pair<string, items>(it_T->first,items()));
			for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
				temp_allData[it_T->first].insert(pair<string, vector<string>>(it_I->first, vector<string>()));
		}
		
		for (tables::iterator it_T = allData.begin(); it_T != allData.end(); it_T++)
		{
			for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
				temp_allData[it_T->first][it_I->first].push_back( it_I->second[0]);
		}
		
		vector<string> allSign;
		getAllSign(allSign, allStr_Where);
		int rows = allData.begin()->second.begin()->second.size();

		for (int row = 1; row < rows; row++)
		{
			if (calculate(allSign, allData, row))
			{
				for (tables::iterator it_T = temp_allData.begin(); it_T != temp_allData.end(); it_T++)
				for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
				{
					it_I->second.push_back(allData[it_T->first][it_I->first][row]);
					
				}

			};


		}

		allData = temp_allData;
	}
	//Tables_Output(allData);
	////////////////////////////////////////////////////////////////
	//計算ORDER字串區域
	cout << "計算ORDER字串區域" << endl;
	if (!allStr_Order.empty())
	{
		e.assign("([^\\s,]+\\s[^\\s,]+|[^\\s,]+)");
		vector<string> each_order;
		vector<name_tableAnditem> prev_order;
		while (regex_search(allStr_Order, sm, e, regex_constants::match_not_null))
		{
			each_order.push_back(sm[1]);
			allStr_Order = sm.suffix().str();
		}
		for (int i = 0; i < each_order.size(); i++)
		{
			e.assign("([^\\s,]+)");
			vector<string> item_order;
			while (regex_search(each_order[i], sm, e, regex_constants::match_not_null))
			{
				item_order.push_back(sm[1]);
				each_order[i] = sm.suffix().str();
			}

			if (item_order.size() == 1)
			{
				name_tableAnditem temp;
				InsertForName_tableAnditem(temp, item_order[0]);
				OrderBy(allData, temp, prev_order, "ASC");
				prev_order.push_back(temp);
			}
			else 
			{
				name_tableAnditem temp;
				InsertForName_tableAnditem(temp, item_order[0]);
				OrderBy(allData, temp, prev_order, item_order[1]);
				prev_order.push_back(temp);
			}

		}



	}
	////////////////////////////////////////////////////////////////
	//計算COUNT字串區域
	cout << "計算COUNT字串區域" << endl;
	for (int i = 0; i < item_Count.size(); i++)
	{
		int count = 0;


		map<string, string> is_Having;
		int rows = allData.begin()->second.begin()->second.size();
		for (int j = 0; j < rows; j++)
		{
			if (is_Having.find(allData[item_Count[i].table][item_Count[i].item][j]) == is_Having.end())
			{
				count++;
				is_Having.insert(pair<string, string>(allData[item_Count[i].table][item_Count[i].item][j], ""));
			}
			else
			{
				if (item_Count[i].use != 2)
					count++;
			}


		}
		item_Count[i].count = count;
	}
	////////////////////////////////////////////////////////////////
	//SHOH字串
	cout << "SHOH字串" << endl;
	vector<vector<string>> result;

	//標題換新名
	for (int i = 0; i < item_ShouldBeShow.size(); i++)
	{
		string as;
		result.push_back(vector<string>());
		for (map<string, name_tableAnditem>::iterator it = asforItem.begin(); it != asforItem.end(); it++)
		{
			if (it->second.table.compare(item_ShouldBeShow[i].table) == 0 &&
				it->second.item.compare(item_ShouldBeShow[i].item) == 0)
				as = it->first;
		}
		if (as.empty())
		{
			result[i].push_back(item_ShouldBeShow[i].item);
		}
		else
		{
			result[i].push_back(as);
		}
	}


	//填入各列
	if (item_Count.empty ())//若沒count
	{
		if (allItem_ShouldBeShow)
			Tables_Output(allData);
		else
		{

			if (!isDistinct)
			{
				for (int i = 0; i < item_ShouldBeShow.size(); i++)
				{
					int rows = allData.begin()->second.begin()->second.size();
					for (int j = 1; j < rows; j++)
						result[i].push_back(allData[item_ShouldBeShow[i].table][item_ShouldBeShow[i].item][j]);
				}
			}
			else
			{

				int rows = allData.begin()->second.begin()->second.size();

				for (int j = 1; j < rows; j++)
				{

					bool uni = true;
					for (int Bj = 1; Bj < result[0].size(); Bj++)
					{
						int count = 0;
						for (int Bi = 0; Bi < item_ShouldBeShow.size(); Bi++)
						{
							vector<string>	temp = allData[item_ShouldBeShow[Bi].table][item_ShouldBeShow[Bi].item];

							if (temp[j].compare(result[Bi][Bj]) == 0)
								count++;
						}
						if (count == item_ShouldBeShow.size())
							uni = false;

					}

					if (uni)
					for (int i = 0; i < item_ShouldBeShow.size(); i++)
						result[i].push_back(allData[item_ShouldBeShow[i].table][item_ShouldBeShow[i].item][j]);
				}

			}
		}
	}
	else 
	{
		int count = 0;
		for (int i = 0; i < item_ShouldBeShow.size(); i++)
		{
			int rows = allData.begin()->second.begin()->second.size();
			if (item_ShouldBeShow[i].use == 0)
			{
				result[i].push_back(allData[item_ShouldBeShow[i].table][item_ShouldBeShow[i].item][rows]);
			}
			else
			{
				result[i].push_back(to_string(item_Count[count++].count));
			}


		}
	}


	for (int i =0; i < result[0].size(); i++)
	{
		for (int j = 0; j <result.size(); j++)
		{
			cout << result[j][i] << "\t";

		}
		cout << endl;
	}
};

void OrderBy(tables & allData, name_tableAnditem item, vector<name_tableAnditem> prev_order, string ASCorDESC)
{
	
	int ASC_or_DESC = 0;
	if (ASCorDESC.compare("DESC") == 0)ASC_or_DESC = 1;


	int rows = allData.begin()->second.begin()->second.size();

	for (int row1 = 1; row1 < rows-1; row1++)
	for (int row2 = row1+1; row2 < rows; row2++)
	{
		bool shouldSwap = false;

		if (allData[item.table][item.item][0].compare("int")==0)
		{
			if (stoi(allData[item.table][item.item][row1])>stoi(allData[item.table][item.item][row2]))
				shouldSwap = (ASC_or_DESC==0?true:false);
			else
				shouldSwap = (ASC_or_DESC == 1 ? true : false);
		}
		else
		{
			if (allData[item.table][item.item][row1].compare(allData[item.table][item.item][row2])>=0)
				shouldSwap = (ASC_or_DESC == 0 ? true : false);
			else
				shouldSwap = (ASC_or_DESC == 1 ? true : false);
		}

		for (int j = 0; j < prev_order.size();j++)
		if (allData[prev_order[j].table][prev_order[j].item][row1].compare(allData[prev_order[j].table][prev_order[j].item][row2]) != 0)
		{
			shouldSwap = false;
			break;
		}


		if (shouldSwap)
		for (tables::iterator it_T = allData.begin(); it_T != allData.end(); it_T++)
		for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
		{
			string temp;
			temp = it_I->second[row1];
			it_I->second[row1] = it_I->second[row2];
			it_I->second[row2] = temp;
		}
	}

}




void InsertForName_tableAnditem(name_tableAnditem& input, string express)
{
	int pos = express.find(".");
	if (pos != string::npos)
	{
		input.table = express.substr(0, pos);
		input.item = express.substr(pos + 1);
	}
	else 
	{
		input.item = express;
	}

}


//TODO :測試功能  只使用 a inner join b on  xxxx 字串即可

void InnerJoin_Process(vector<string> inputStrings ,tables &outputTables)
{

	int count = 0;
	vector<string> recordStrings;
	int flag_RecordStrings=0;
	tables processingQuene[2];
	int state_ProcessingQuene=0;

	for (int i = 0; i < (int)inputStrings.size(); i++)
	{
		int precount = count;
		if (inputStrings[i].compare("(") == 0)
			count++;
		else if (inputStrings[i].compare(")") == 0)
			count--;


		//////////////////////////////////////////////
		if (count == 1 && precount == 0)
			continue;
		else if (count>0)
		{
			recordStrings.push_back(inputStrings[i]);
			continue;
		}
		else if (count == 0 && precount == 1)
		{
			//將回傳之矩陣存入計算柱列
			flag_RecordStrings = 1;
		}
		//////////////////////////////////////////////

		if (inputStrings[i].compare("ON") == 0)
			state_ProcessingQuene = 4;
		else if (inputStrings[i].compare("INNER JOIN") == 0)
			state_ProcessingQuene = 2;
		else if (inputStrings[i].compare("AS") == 0)
			state_ProcessingQuene += 1;
		else
			switch (state_ProcessingQuene)
			{
				case 0:
					if (flag_RecordStrings==1)
						InnerJoin_Process(recordStrings, processingQuene[0]);
					else
						readTableText(inputStrings[i], processingQuene[0]);
					break;
				case 1:
					processingQuene[0].insert(pair<string, items>(inputStrings[i], processingQuene[0][inputStrings[i - 2]]));//加入新名,並連至原本名字之items(目標table)
					asforTable.insert(pair<string, string>(inputStrings[i - 2], inputStrings[i])); //將新名加入新名庫
					break;
				case 2:
					if (flag_RecordStrings == 1)
						InnerJoin_Process(recordStrings, processingQuene[1]);
					else
						readTableText(inputStrings[i], processingQuene[1]);
					break;
				case 3:
					processingQuene[1].insert(pair<string, items>(inputStrings[i], processingQuene[1][inputStrings[i - 2]]));
					asforTable.insert(pair<string, string>(inputStrings[i - 2], inputStrings[i]));
					break;
				case 4:

					//TODO
					Join(processingQuene[0], processingQuene[1], inputStrings[i],true);

					processingQuene[0] = processingQuene[1];
					break;
			}
		//////////////////////////////////////////////
		
		if (flag_RecordStrings == 1)
		{
			flag_RecordStrings = 0;
			recordStrings.clear();
		}
		
	}
/*	if (processingQuene[1].size()>processingQuene[0].size())
	outputTables = processingQuene[1];
	else*/
		outputTables = processingQuene[0];

}



//inner 與join 皆成功
void Join(tables &inputA, tables &inputB, string express ,bool isInner)
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
						for (tables::iterator it_T = inputA.begin(); it_T != inputA.end(); ++it_T)
						if (it_T->second.find(sm[1]) != it_T->second.end())
							express_Item[count - 1] = it_T->first;
					}
					else
					{
						for (tables::iterator it_T = inputB.begin(); it_T != inputB.end(); ++it_T)
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

	tables tempTable;
	for (tables::iterator it_T = inputB.begin(); it_T != inputB.end(); ++it_T){
		tempTable.insert(pair<string, items>(it_T->first, items()));
		
		for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); ++it_I)
		{
	
			tempTable[it_T->first].insert(pair<string, vector<string>>(it_I->first, vector<string>()));
			tempTable[it_T->first][it_I->first].push_back(it_I->second[0]);//將表格中 第0項(型態)存入

		}
	}
	
	for (tables::iterator it_T = inputA.begin(); it_T != inputA.end(); it_T++){
		tempTable.insert(pair<string, items>(it_T->first, items()));
		for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
		{
			
			tempTable[it_T->first].insert(pair<string, vector<string>>(it_I->first, vector<string>()));
			tempTable[it_T->first][it_I->first].push_back(it_I->second[0]);
		}
	}


	

	for (B_count = 1; B_count < inputB.begin()->second.begin()->second.size(); B_count++)
	for (A_count = 1; A_count < inputA.begin()->second.begin()->second.size(); A_count++)
	{

		string L, R;
		//將input之table 存入L,R 若相反者亦可
		
		if (isInner)
			if (inputA.find(express_Item[0]) != inputA.end())
			{
				L = inputA[express_Item[0]][express_Item[1]][A_count];
				R = inputB[express_Item[2]][express_Item[3]][B_count];
			}
			else if (inputB.find(express_Item[0]) != inputB.end())
			{
				L = inputB[express_Item[0]][express_Item[1]][A_count];
				R = inputA[express_Item[2]][express_Item[3]][B_count];
			}

		//判斷條件是否成立,若是則將A與B整列資料存入temp
	
		if (!isInner || L.compare(R) == 0)
		{
			//插入B之列
			for (tables::iterator it_T = tempTable.begin(); it_T != tempTable.end(); it_T++){
				for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
				{
	
					if (inputB.find(it_T->first)!=inputB.end())
						it_I->second.push_back(inputB[it_T->first][it_I->first][B_count]);
					else if (inputA.find(it_T->first) != inputA.end())
						it_I->second.push_back(inputA[it_T->first][it_I->first][A_count]);
				}
			}
		}
	}

	inputB.clear();
	
	inputB = tempTable;

}


void readTableText(string name, map<string, map<string, vector<string>>> & input_Table)
{
	ifstream inputFile(name + ".txt");
	string inputString;
	smatch sm;
	regex e("([^\\s]+)");

	input_Table.insert(pair<string, items>(name, items()));
	
	int count = 0;
	vector<string> itemName;
	int itemCount = 0;

	while (!getline(inputFile, inputString).eof()){		
		while (regex_search(inputString, sm, e, regex_constants::match_not_null))
		{
			if (count == 0)
			{
				itemName.push_back(sm[1]);
				input_Table[name].insert(pair<string, vector<string>>(sm[1], vector<string>()));
			}
			else
			{
				string temp = sm[1];
				input_Table[name][itemName[itemCount++]].push_back(sm[1]);
			}
			inputString = sm.suffix().str();
			
		}
		itemCount = 0;
		count++;
	}

	




}



void readString(string &str_Select, string &str_Where, string &str_GroupBy, string &str_OrderBy)
{
	ifstream inputFile("input.txt");
	string tempStr;
	
	getline(inputFile, tempStr);
	tempStr = tempStr + "SQLEND";
	string * str_Ptr[4] = { &str_Select, &str_Where, &str_GroupBy, &str_OrderBy };

	
	smatch sm;
	smatch sm2;
	regex e("\\s?(SELECT|WHERE|GROUP BY|ORDER BY|SQLEND)\\s?");



	while (regex_search(tempStr, sm, e, regex_constants::match_not_null))
	{
		int stringType = -1;
		if (sm[1].compare("SELECT") == 0)
			stringType = 0;
		else if (sm[1].compare("WHERE") == 0)
			stringType = 1;
		else if (sm[1].compare("GROUP BY") == 0)
			stringType = 2;
		else if (sm[1].compare("ORDER BY") == 0)
			stringType = 3;
		else if (sm[1].compare("SQLEND") == 0)
			break;
		tempStr = sm.suffix().str();

			
		regex_search(tempStr, sm2, e, regex_constants::match_not_null);
		(*str_Ptr[stringType]) = sm2.prefix().str();
		cout << *str_Ptr[stringType]<<endl;
	}
	

}

void Tables_Output(tables &input)
{


	for (tables::iterator it_T = input.begin(); it_T != input.end(); it_T++)
	{
		for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
			cout << it_I->first << "\t";
		
	}
	cout << endl;
	int rows = input.begin()->second.begin()->second.size();
	for (int i = 0; i < rows; i++){
		for (tables::iterator it_T = input.begin(); it_T != input.end(); it_T++)
		{
			for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
				cout << it_I->second[i] << "\t";

		}
		cout << endl;
	}
}




void getAllSign(vector<string> &allSign, string tempStr)
{
	smatch sm;
	regex e("([\\w.]+|[()]|[<>!=]+)");
	while (regex_search(tempStr, sm, e, regex_constants::match_not_null))
	{
		cout << sm[1];
		allSign.push_back(sm[1]);
		tempStr = sm.suffix().str();

	}
}
bool mathCalculate(vector<string> &mathStack, tables &allData)
{

	if (mathStack[0].compare("int")==0)
	{
		int elementContext = stoi(mathStack[1]);
		int constantContext = stoi(mathStack[3]);

		if (mathStack[2].compare("==") == 0)
			return elementContext == constantContext;
		if (mathStack[2].compare("!=") == 0)
			return elementContext != constantContext;
		if (mathStack[2].compare(">=") == 0)
			return elementContext >= constantContext;
		if (mathStack[2].compare("<=") == 0)
			return elementContext <= constantContext;
		if (mathStack[2].compare("<") == 0)
			return elementContext < constantContext;
		if (mathStack[2].compare(">") == 0)
			return elementContext > constantContext;
	}
	else
	{
	
		if (mathStack[2].compare("==") == 0)
			return mathStack[0].compare(mathStack[2])==0;
		if (mathStack[2].compare("!=") == 0)
			return mathStack[0].compare(mathStack[2]) != 0;

	}


};
bool logicCalculate(vector<string> &logicStack)
{
	bool a_Boolean, b_Boolean;
	
	if (logicStack[0].compare("true") == 0)
		a_Boolean = true;
	else
		a_Boolean = false;
	if (logicStack[2].compare("true") == 0)
		b_Boolean = true;
	else
		b_Boolean = false;

	if (logicStack[1].compare("AND") == 0)
		return a_Boolean && b_Boolean;
	else if (logicStack[1].compare("OR") == 0)
		return a_Boolean || b_Boolean;
	else if (logicStack[1].compare("NOT")==0)
		return  !b_Boolean;

};
bool calculate(vector<string> &allSign, tables &allData, int row)
{
	int count = 0;

	bool flag_endSubSign = false;

	vector<string> sub_allSign;
	vector<string> mathStack;
	vector<string> logicStack;


	for (int i = 0; i < (int)allSign.size(); i++)
	{

		//cout << allSign[i] << endl;

		//判斷是否為子字串符號
		//////////////////////////////////////////////////////////////////////////

		if (allSign[i].compare("(") == 0)
		{
			//遇到( 便將計數值累加
			count++;
		}
		else if (allSign[i].compare(")") == 0)
		{

			bool subResult;
			count--;

			//若該)為最後之)  則將中途紀錄之符號另外計算
			if (count == 0)
			{
				subResult = calculate(sub_allSign, allData, row);
				sub_allSign.clear();

				if (logicStack.size() != 1)
				if (subResult)
					logicStack.push_back("true");
				else
					logicStack.push_back("false");

				flag_endSubSign = true;
			}
		}

		if (count != 0)
		{
			//若count不為0 則該符號在()中, 只需紀錄無須處理
			if (allSign[i].compare("(") != 0 && allSign[i].compare(")") != 0)
				sub_allSign.push_back(allSign[i]);
			continue;
		}


		//獲得字串中的符號,並判斷該符號(如果剛結束子字串符號紀錄 無須判斷)
		//////////////////////////////////////////////////////////////////////////
		if (!flag_endSubSign)
		{

			if (allSign[i][1] == '=' || allSign[i][0] == '<' || allSign[i][0] == '>')
			{
				if (mathStack.size() == 2)mathStack.push_back(allSign[i]);
			}
			else if (allSign[i].compare("AND") == 0 || allSign[i].compare("OR") == 0)
			{
				if (logicStack.size() == 1)logicStack.push_back(allSign[i]);
			}
			else if (allSign[i].compare("NOT") == 0 )
			{
				if (logicStack.size() == 1)
					logicStack.push_back(allSign[i]);
				else if (logicStack.size() == 0)
				{
					logicStack.push_back(allSign[i]);
					logicStack.push_back(allSign[i]);
				}
			}
			else
			{
				name_tableAnditem temp;
				InsertForName_tableAnditem(temp, allSign[i]);
				if (allData.find(temp.table) != allData.end() && allData[temp.table].find(temp.item) != allData[temp.table].end())
				{
					if (mathStack.size() == 0)
					{
						mathStack.push_back(allData[temp.table][temp.item][0]);
						mathStack.push_back(allData[temp.table][temp.item][row]);
					}
					else
					{

						mathStack.push_back(allData[temp.table][temp.item][row]);
						
					}
				}
				else
				{
					if (mathStack.size() == 0)
					{
						cout << "常數不得在左值";
					}
					else
					{

						mathStack.push_back(allSign[i]);
				
					}
				}
			}

			flag_endSubSign = false;

			//////////////////////////////////////////////////////////////////////////
			//若數學堆已滿 ,進行數學運算並將結果放於邏輯堆中的0或2
			if (mathStack.size() == 4 && logicStack.size() != 1)
			{
				

				if (mathCalculate(mathStack, allData))
					logicStack.push_back("true");
				else
					logicStack.push_back("false");
				mathStack.clear();
			}
			//若邏輯堆已滿,進行邏輯運算並將結果放於邏輯堆的1
			if (logicStack.size() == 3)
			{
				bool logicCalculateResult = logicCalculate(logicStack);
				logicStack.clear();
				if (logicCalculateResult)
					logicStack.push_back("true");
				else
					logicStack.push_back("false");

			}

		}

		if (logicStack.size() == 1)
		if (logicStack[0].compare("true") == 0)
			return true;
		else
			return false;

	}
}



/*
void delete(tables& allData, string express)
{

}




*/





void main()
{

	ifstream inputFile("input.txt");
	tables tempa;
	string temp;
	getline(inputFile, temp);
	Select(tempa, temp);

	//TODO:ggrjii

}