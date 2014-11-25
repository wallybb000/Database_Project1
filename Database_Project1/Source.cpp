#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <map>
#include "tables.h"
using namespace std;


typedef map<string, vector<string>> items;


bool allItem_ShouldBeShow = false;
bool isDistinct = false;
map<string, string> asforTable;
map<string, itemAttribute> asforItem;//TODO
vector<itemAttribute> item_Count;
vector<itemAttribute> item_ShouldBeShow;
//// ///////////////////////////////////////////////////////////////////////////////////////////
void Select(tables &allData, string str_Select);
tables Process_From(vector<string> input);
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
	//分出各種用途之字串
	int location[5] = { 0, str_Input.find("FROM"), str_Input.find("WHERE"), str_Input.find("ORDER BY"), str_Input.find(";") };

	int temp_StrEnd = 0;
	for (int i = 1; i < 5; i++)
	if (location[i]>0)
	{
		allStr_Select = str_Input.substr(0, location[i]);
		break;
	}
	for (int i = 2; i < 5; i++)
	if (location[1]>0 && location[i] > 0)
	{
		allStr_From = str_Input.substr(location[1] + 4, location[i] - location[1] - 4);
		break;
	}
	for (int i = 3; i < 5; i++)
	if (location[2]>0 && location[i] > 0)
	{
		allStr_Where = str_Input.substr(location[2] + 5, location[i] - location[2] - 5);
		break;
	}
	for (int i = 4; i < 5; i++)
	if (location[3]>0 && location[i] > 0)
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

			itemAttribute temp(each_Item[1]);
			if (each_Item[1].compare("DISTINCT") == 0)
				temp.use = 2;
			else
				temp.use = 1;

			item_ShouldBeShow.push_back(temp);
			if (each_Item.size() > 3)
			{
				asforItem.insert(pair<string, itemAttribute>(each_Item[each_Item.size() - 1], temp)); //加入新名庫
			}
			//TODO 建置count庫
			item_Count.push_back(temp);

		}
		else if (each_Item[0].compare("DISTINCT") == 0)
		{
			isDistinct = true;
			itemAttribute temp(each_Item[1]);
			temp.use = 3;

			item_ShouldBeShow.push_back(temp);
			if (each_Item.size() > 2)
			{
				asforItem.insert(pair<string, itemAttribute>(each_Item[3], temp)); //加入新名庫
			}

		}
		else
		{

			if (each_Item[0].compare("*") == 0){
				allItem_ShouldBeShow = true;
				itemAttribute temp;
				temp.item = "*";
				item_ShouldBeShow.push_back(temp);
			}
			else
			{
				itemAttribute temp(each_Item[0]);
				item_ShouldBeShow.push_back(temp);

				if (each_Item.size() > 1 && each_Item[1].compare("AS") == 0)
					asforItem.insert(pair<string, itemAttribute>(each_Item[2], temp)); //加入新名庫
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

		string each_Table = sm[1];
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
			temp.readTableText(str_From[0]);
			allTables.push_back(temp);

		}
		else if (str_From.size() == 3)
		{
			tables temp;
			temp.readTableText(str_From[0]);
			allTables.push_back(temp);

			asforTable.insert(pair<string, string>(str_From[2], str_From[0]));// 將新名加入新名庫
		}
		else if (str_From.size() > 3)
		{
			tables temp;
			InnerJoin_Process(str_From, temp);
			allTables.push_back(temp);

		}
		allStr_From = sm.suffix().str();

	}

	if (allTables.size() != 1)
	{
		string str;
		for (int i = 1; i < allTables.size(); i++)
		{
			allTables[0].join(allTables[i], str, false);

		}

	}

	allData = allTables[0];
	////////////////////////////////////////////////////////////////

	//計算WHERE字串區域
	cout << "計算WHERE字串區域" << endl;
	if (!allStr_Where.empty())
	{
		tables temp_allData;
		for (tableSet::iterator it_T = allData.getData().begin(); it_T != allData.getData().end(); it_T++)
		{
			temp_allData.insert(it_T->first, items());
			for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
				temp_allData[it_T->first].insert(pair<string, vector<string>>(it_I->first, vector<string>()));
		}

		for (tableSet::iterator it_T = allData.getData().begin(); it_T != allData.getData().end(); it_T++)
		{
			for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
				temp_allData[it_T->first][it_I->first].push_back(it_I->second[0]);
		}

		vector<string> allSign;
		getAllSign(allSign, allStr_Where);
		int rows = allData.getData().begin()->second.begin()->second.size();

		for (int row = 1; row < rows; row++)
		{
			if (calculate(allSign, allData, row))
			{
				for (tableSet::iterator it_T = temp_allData.getData().begin(); it_T != temp_allData.getData().end(); it_T++)
				for (items::iterator it_I = it_T->second.begin(); it_I != it_T->second.end(); it_I++)
				{
					it_I->second.push_back(allData[it_T->first][it_I->first][row]);

				}

			};


		}

		allData.getData() = temp_allData.getData();
	}
	//Tables_Output(allData);

	////////////////////////////////////////////////////////////////

	//計算ORDER字串區域
	cout << "計算ORDER字串區域" << endl;
	if (!allStr_Order.empty())
	{
		e.assign("([^\\s,]+\\s[^\\s,]+|[^\\s,]+)");
		vector<string> each_order;
		vector<itemAttribute> prev_order;
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
				itemAttribute temp(item_order[0]);
				allData.OrderBy(temp, prev_order, "ASC");
				prev_order.push_back(temp);
			}
			else
			{
				itemAttribute temp(item_order[0]);
				allData.OrderBy(temp, prev_order, "ASC");
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
		int rows = allData.getData().begin()->second.begin()->second.size();
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
		for (map<string, itemAttribute>::iterator it = asforItem.begin(); it != asforItem.end(); it++)
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
	if (item_Count.empty())//若沒count
	{
		if (allItem_ShouldBeShow)
			allData.Tables_Output();
		else
		{

			if (!isDistinct)
			{
				for (int i = 0; i < item_ShouldBeShow.size(); i++)
				{
					int rows = allData.getData().begin()->second.begin()->second.size();
					for (int j = 1; j < rows; j++)
						result[i].push_back(allData[item_ShouldBeShow[i].table][item_ShouldBeShow[i].item][j]);
				}
			}
			else
			{

				int rows = allData.getData().begin()->second.begin()->second.size();

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
			int rows = allData.getData().begin()->second.begin()->second.size();
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


	for (int i = 0; i < result[0].size(); i++)
	{
		for (int j = 0; j < result.size(); j++)
		{
			cout << result[j][i] << "\t";

		}
		cout << endl;
	}
};



//TODO :測試功能  只使用 a inner join b on  xxxx 字串即可

void InnerJoin_Process(vector<string> inputStrings, tables &outputTables)
{

	int count = 0;
	vector<string> recordStrings;
	int flag_RecordStrings = 0;
	tables processingQuene[2];
	int state_ProcessingQuene = 0;

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
		else if (count > 0)
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
				if (flag_RecordStrings == 1)
					InnerJoin_Process(recordStrings, processingQuene[0]);
				else
					processingQuene[0].readTableText(inputStrings[i]);
				break;
			case 1:
				processingQuene[0].insert(inputStrings[i], processingQuene[0][inputStrings[i - 2]]);//加入新名,並連至原本名字之items(目標table)
				asforTable.insert(pair<string, string>(inputStrings[i - 2], inputStrings[i])); //將新名加入新名庫
				break;
			case 2:
				if (flag_RecordStrings == 1)
					InnerJoin_Process(recordStrings, processingQuene[1]);
				else
					processingQuene[1].readTableText(inputStrings[i]);
				break;
			case 3:
				processingQuene[1].insert(inputStrings[i], processingQuene[1][inputStrings[i - 2]]);
				asforTable.insert(pair<string, string>(inputStrings[i - 2], inputStrings[i]));
				break;
			case 4:
				processingQuene[1].join(processingQuene[0], inputStrings[i], true);
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

	outputTables = processingQuene[0];

}



//inner 與join 皆成功
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
		cout << *str_Ptr[stringType] << endl;
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

	if (mathStack[0].compare("int") == 0)
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
			return mathStack[0].compare(mathStack[2]) == 0;
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
	else if (logicStack[1].compare("NOT") == 0)
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
			else if (allSign[i].compare("NOT") == 0)
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
				itemAttribute temp(allSign[i]);
				if (allData.getData().find(temp.table) != allData.getData().end() && allData[temp.table].find(temp.item) != allData[temp.table].end())
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

		if (logicStack.size() == 1) if (logicStack[0].compare("true") == 0)
			return true;
		else
			return false;

	}
}



void Delete(string str_input)
{
	ofstream output;
	string allstr_From;
	string allstr_Where;
	vector<string> allSign;
	vector<int> vaildArray;
	tables allData;
	int indexOfFrom = str_input.find("FROM");
	int indexOfWhere = str_input.find("WHERE");
	regex e;
	allstr_From = str_input.substr(indexOfFrom +5 , indexOfWhere - indexOfFrom - 6);
	allstr_Where = str_input.substr(indexOfWhere + 5);

	
	
	allData.readTableText(allstr_From);
	
	getAllSign(allSign, allstr_Where);
	int length = allData.getData().begin()->second.begin()->second.size();
	vaildArray.assign(length, 1);
	for (int index = 1; index < length; index++)
	{
		if (calculate(allSign, allData, index))//條件對了就取消輸出
			vaildArray[index] = 0;

	}
	//先列出	每一項目名
	tableSet data = allData.getData();
	vector<itemAttribute> attri = allData.getAttribute();

	output.open(allstr_From + ".txt", ostream::out | ostream::trunc);
	for (int index =0 ; index < attri.size(); index++)
		output << attri.at(index).item << " ";
	output << endl;
	for (int indexRow = 0; indexRow < length; indexRow++)
	{
		if (vaildArray[indexRow] == 0)continue;

		for (int indexCol = 0; indexCol < attri.size(); indexCol++)
			output << data[attri[indexCol].table][attri[indexCol].item][indexRow] << " ";
		output << endl;
	}

	output.flush();
	output.close();


}









void main()
{

	ifstream inputFile("input2.txt");
	tables tempa;
	string temp;
	getline(inputFile, temp);
	//Select(tempa, temp);
	Delete(temp);
	//TODO:ggrjii

}