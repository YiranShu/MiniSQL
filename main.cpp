#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm> 
#include <vector>
#include <direct.h>
#include "GlobalDeclar.h"
#include "CatalogManager.h"
#include "BufferManager.h"
#include "RecordManager.h"
#include "BM_File_iterator.h"
#include "Result.h"

using namespace std;

BufferManager BuffM(BUFNUM);
RecordManager RecM;
CatalogManager CataM;

int create_table(stringstream &strin, Result &res)
{
	int i = 0;
	struct TABLE *t= new (struct TABLE);
	t->primary_key_index = 0;
	t->num_attr = 0;
	string buf;

	strin >> buf;	//table name

	PageID pid;
	pid = CataM.FindTable(buf);	//if found table, can't create table
	if(pid != -1){				//output ERROR
		res.SetError( "ERROR: Table '" + buf + "' already exists" );
		return -1;
	}
	t->table_name = buf;

	strin >> buf;
	int offset = 0;
	if(buf != "("){
		res.SetSyntaxError(buf);
		return -1;
	}
	while(1){
		strin >> buf;

		if(buf == "primary"){	//adding primary key constraint
			strin >> buf;
			if(buf != "key"){	//parsing
				res.SetSyntaxError(buf);
				return -1;
			}
			strin >> buf;
			if(buf != "("){
				res.SetSyntaxError(buf);
				return -1;
			}
			strin >> buf;		//primary key name
			int flag = 0;
			int index;
			for(int j = 1; j <= t->num_attr; j++){	//record the index of primary key
				if(buf == t->attribute[j].attr_name){
					index = j;
					flag = 1;
					break;
				}
			}
			if(flag == 0){
				res.SetSyntaxError(buf);
				return -1;
			}
			strin >> buf;
			if(buf != ")"){
				res.SetSyntaxError(buf);
				return -1;
			}

			t->attribute[index].unique = 1;
			t->attribute[index].index_true = 1;
			t->primary_key_index = index;

			strin >> buf;
			if(buf == ",")
				continue;
			else if(buf == ")")
				break;
			else{
				res.SetSyntaxError(buf);
				return -1;
			}
		}
		else{	//adding attributes
			i++;
			t->attribute[i].attr_name = buf;
			t->num_attr++;
			strin >> buf;		//type
			int flag = 0;
			stringstream ss;
			string s;
			for(int j = 1; j <= 255; j++){
				ss << "char(" << j << ")" << endl;
				ss >> s;
				if(buf == s || buf == s+","){
					flag = j;
					break;
				}
			}	
			if(flag != 0 || buf == "char" || buf == "int" || buf == "float" || buf == "int," || buf == "float,"){
				if(flag != 0 || buf == "char"){	//char() type
					if(buf == "char"){
						strin >> buf;
						buf = "char" + buf;
						for(int j = 1; j <= 255; j++){
							ss << "char(" << j << ")" << endl;
							ss >> s;
							ss.clear();
							if( buf == s || buf == s+","){
								flag = j;
								break;
							}
						}
					}
					t->attribute[i].offset = offset;
					t->attribute[i].size = flag;
					offset += flag;
					t->attribute[i].type = s;
					if(buf == s+",")
						continue;
				}
				else if (buf == "int" || buf == "int,"){	//int type
					t->attribute[i].offset = offset;
					t->attribute[i].size = 11;
					offset += 11;
					t->attribute[i].type = "int";
					if(buf == "int,")
						continue;
				}
				else if (buf == "float" || buf == "float,"){	//float type
					t->attribute[i].offset = offset;
					t->attribute[i].size = 47;
					offset += 47;
					t->attribute[i].type = "float";
					if(buf == "float,")
						continue;
				}
				strin >> buf;
				if(buf == ",")
					continue;
				else if(buf == ")")
					break;
				else if (buf == "unique," || buf == "unique"){	//adding unique constraint
					t->attribute[i].unique = 1;
					if (buf == "unique,")
						continue;
					if (buf == "unique"){
						strin >> buf;
						if (buf == ",")
							continue;
						else if (buf == ")")
							break;
						else{
							res.SetSyntaxError(buf);
							return -1;
						} 
					}
				}
				else{
					res.SetSyntaxError(buf);
					return -1;
				}
			}	
			else if (buf == "char"){
				strin >> buf;


			}
			else{
				res.SetSyntaxError(buf);
				return -1;		
			}
		}
	}

//write table into page
	CataM.WriteTable(t, res);
	if(t->primary_key_index != 0)
		RecM.create("primary_key"+t->table_name, t->table_name, t->attribute[t->primary_key_index].attr_name, res);
	delete t;
	return 0;
}

int drop_table(stringstream &strin, Result &res)
{
	string table_name;
	strin >> table_name;

	RecM.DropIndexOnTable(table_name, res);
	CataM.DeleteTable(table_name, res);	
	return 0;
}

int select(stringstream &strin, Result &res)
{
	std::vector<struct condition> vcond;
	string table_name;
	PageID pid;

	string buf;
	strin >> buf;
	if(buf != "*"){		//parsing
		res.SetSyntaxError(buf);
		return -1;
	}
	strin >> buf;
	if(buf != "from"){
		res.SetSyntaxError(buf);
		return -1;
	}

	strin >> table_name;
	TABLE t;
	pid = CataM.ReadTable(table_name, &t);
	if(pid == -1){
		res.SetError( "ERROR: No such table!" );
		return -1;
	}

	strin >> buf;
	if(strin.eof()){
		RecM.selectRecords(t, vcond, res);
		return 0;
	}
	if(buf != "where"){
		res.SetSyntaxError(buf);
		return -1;
	}
	while(1){		//read in conditions
		struct condition tmp;
		strin >> tmp.attr.attr_name;
		int i = 1;
		for(; i <= t.num_attr; i++){		//find the attribute in the table
			if (tmp.attr.attr_name == t.attribute[i].attr_name){
				tmp.attr = t.attribute[i];
				break;
			}
		}
		if(i > t.num_attr){
			res.SetError( "ERROR: No such attribute!" );
			return -1;
		}

		strin >> tmp.opt;	//read in the operator
		if(tmp.opt != "=" && tmp.opt != "<>" && tmp.opt != "<" && tmp.opt != ">" && tmp.opt != "<=" && tmp.opt != ">="){
			res.SetError( "ERROR: Wrong operator!" );
			return -1;
		}

		strin >> buf;		//read in the operand
		if(buf[0] == '\'' && buf.back() == '\''){ 	//get rid of ' in char type value
			if(tmp.attr.type[0] != 'c'){
				res.SetError( "ERROR: Type of attribute " + tmp.attr.attr_name + " should be " + tmp.attr.type );
				return -1;
			}			
			tmp.opd = buf.substr(1, buf.length()-2);
		}
		else{		//copy the value of int or float type directly 
			for(int j = 0; j < buf.length(); j++){
				if(! (buf[j] >= '0' && buf[j] <= '9' || buf[j] == '.' ) || tmp.attr.type[0] == 'c'){
					res.SetError( "ERROR: Type of attribute " + tmp.attr.attr_name + " should be " + tmp.attr.type );
					return -1;
				}
			}	
			tmp.opd = buf;
		}

 		vcond.push_back(tmp);
 		if(tmp.attr.index_true == 1 && tmp.opt == "=")
 			swap(vcond[0], vcond.back());

		strin >> buf;
		if(strin.eof()){	//check end of conditions
			RecM.selectRecords(t, vcond, res);
			return 0;
		}
		if(buf != "and"){	//still conditions to be read in
			res.SetSyntaxError(buf);
			return -1;
		}		
	}
}

int insert(stringstream &strin, Result &res)
{
	std::vector<struct condition> vcond;
	stringstream ss;
	string s;
	
	string record;
	string table_name;
	string buf;
	strin >> buf;
	if(buf != "into"){		//parsing
		res.SetSyntaxError(buf);
		return -1;
	}

	strin >> table_name;
	TABLE t;
	PageID pid = CataM.ReadTable(table_name, &t);
	if(pid == -1){
		res.SetError( "ERROR: No such table" );
		return -1;
	}

	strin >> buf;
	if(buf != "values" && buf != "value"){		//parsing
		res.SetSyntaxError(buf);
		return -1;
	}

	string type;
	bool unique;
	for(int i = 1; i <= t.num_attr; i++){
		strin >> buf;
		if(i == 1){
			if(buf.front() != '('){
				res.SetSyntaxError(buf);
				return -1;
			}
			buf = buf.substr(1, buf.length()-1);
		}
		if(i == t.num_attr){
			if(buf.back() != ')'){
				res.SetSyntaxError(buf);
				return -1;
			}
		}

		buf = buf.substr(0, buf.length()-1);
		type = t.attribute[i].type;
		unique = t.attribute[i].unique;
		struct condition tmp;

		if( (buf.front() == '\'' && buf.back() == '\'') || (buf.front() == '\"' && buf.back() == '\"') ){
		//char() type
			stringstream ss;
			string s;
			ss << "char(" << buf.length()-2 << ")"; //caculate length
			ss >> s;
			ss.clear();

			if(type != s){
				res.SetError( "ERROR: Type does not fit" );
				return -1;
			}
			buf = buf.substr(1, buf.length()-2);
			tmp.opd = buf;
		}
		else if( buf.find('.') != std::string::npos ){//find '.', must be float type
			tmp.opd = buf;
			if(type != "float"){
				res.SetError( "ERROR: Type does not fit" );
				return -1;
			}
		//add 0s to the end the extend buf to length of 47
			int digit = buf.length();
			buf = buf.insert(digit, 47-digit, '0');
		}
		else {
		//can not find '.', can be int type or float type
			tmp.opd = buf;
			if(type == "int"){
				int digit = buf.length();
				if(buf.front() == '-'){		//add 0s to the beginning to extend buf to length of 11
					buf = buf.insert(1, 11-digit, '0');
				}
				else{
					buf = buf.insert(0, 11-digit, '0');
				}
			}
			else if(type == "float"){
				buf = buf + ".";
				int digit = buf.length();
				buf = buf.insert(digit, 47-digit, '0');	//add 0s to the end to extend buf to length of 47
			}
			else{
				res.SetError( "ERROR: Type does not fit" );
				return -1;
			}
		}

		//contact the values into one string
		record += buf;
		/*
		 *if the attribute is unique, record the condition with operator "="
		 *then select the table with the conditions to ensure no duplicate key
		 */
		if( unique == 1 ){
			tmp.opt = "=";
			tmp.attr = t.attribute[i];
			vcond.push_back(tmp);
			if(tmp.attr.index_true == 1){
				swap(vcond[0], vcond.back());
			}
		}
	}
	std::vector<struct condition>::iterator it;
	for(it = vcond.begin(); it != vcond.end(); it++){
		Result re;
		std::vector<struct condition> singleV;
		singleV.push_back(*it);
		RecM.selectRecords(t, singleV, re);
		if(re.res_rows > 0){
			res.SetError("ERROR: Duplicate key");
			return -1;
		}			
	}
	RecM.insertRecords(t, record, res);
	return 0;
}

int Delete(stringstream &strin, Result &res)
{
	std::vector<struct condition> vcond;
	string table_name;
	RecordID recID;

	string buf;
	strin >> buf;
	if(buf != "from"){
		res.SetSyntaxError(buf);
		return -1;
	}
	strin >> table_name;
	
	TABLE t;
	PageID pid = CataM.ReadTable(table_name, &t);
	if(pid == -1){
		res.SetError( "ERROP: No such table" );
		return -1;
	}

	strin >> buf;
	if(strin.eof()){	//no condition, delete all
		RecM.deleteRecords(t, vcond, res);
		return 0;
	}
	if(buf != "where"){
		res.SetSyntaxError(buf);
		return -1;
	}

	while(1){	//read in conditions
		struct condition tmp;
		strin >> tmp.attr.attr_name;
		int i = 1;
		for(; i <= t.num_attr; i++){	//find the attribute in the table
			if (tmp.attr.attr_name == t.attribute[i].attr_name){	
				tmp.attr = t.attribute[i];
				break;
			}
		}

		if(i > t.num_attr){
			res.SetError( "ERROR: No such attribute!" );
			return -1;
		}

		strin >> tmp.opt;	//read in the operator
		if(tmp.opt != "=" && tmp.opt != "<>" && tmp.opt != "<" && tmp.opt != ">" && tmp.opt != "<=" && tmp.opt != ">="){
			res.SetError( "ERROR: Wrong operator!" );
			return -1;
		}

		strin >> buf;		//read in the operand
		if(buf[0] == '\'' && buf.back() == '\''){ 	//get rid of ' in char type value
			if(tmp.attr.type[0] != 'c'){
				res.SetError( "ERROR: Type of attribute " + tmp.attr.attr_name + " should be " + tmp.attr.type );
				return -1;
			}			
			tmp.opd = buf.substr(1, buf.length()-2);
		}
		else{		//copy the value of int or float type directly 
			for(int j = 0; j < buf.length(); j++){
				if(! (buf[j] >= '0' && buf[j] <= '9' || buf[j] == '.' ) || tmp.attr.type[0] == 'c'){
					res.SetError( "ERROR: Type of attribute " + tmp.attr.attr_name + " should be " + tmp.attr.type );
					return -1;
				}
			}	
			tmp.opd = buf;
		}

		vcond.push_back(tmp);
 		if(tmp.attr.index_true == 1 && tmp.opt == "=")
 			swap(vcond[0], vcond.back());

		strin >> buf;	//check end of conditions
		if(strin.eof()){
			RecM.deleteRecords(t, vcond, res);
			return 0;
		}
		if(buf != "and"){	
			res.SetSyntaxError(buf);
			return -1;
		}	//if there is still other conditions to be read, continue the loop
	}
}

int create_index(stringstream &strin, Result &res)
{
	string buf;
	string index_name;
	string table_name;
	string attr_name;

	strin >> index_name;

	strin >> buf;
	if(buf != "on"){
		res.SetSyntaxError(buf);
		return -1;
	}

	strin >> table_name;

	strin >> buf;
	if(buf != "("){
		res.SetSyntaxError(buf);
		return -1;
	}

	strin >> attr_name;

	strin >> buf;
	if(buf != ")"){
		res.SetSyntaxError(buf);
		return -1;
	}
	
	PageID pid = CataM.FindTable(table_name);
	if(pid == -1){
		res.SetError( "ERROR: Unknown table '" + table_name + "'" );
		return -1;
	}

	RecM.create(index_name, table_name, attr_name, res);
	return 0;
}

int drop_index(stringstream &strin, Result &res)
{
	string index_name;
	strin >> index_name;

	RecM.drop(index_name, res);
	return 0;
}

int execfile(char* filename, Result &res);

int interpreter(stringstream &strin)
{
	Result res;		//record the result information to be printed
	string buf;
	strin >> buf;
	if(buf == "quit")
		return -2;
	else if(buf == "create"){	//parsing
		strin >> buf;
		if(buf == "table")
			create_table(strin, res);
		else if(buf == "index")
			create_index(strin, res);
		else{
			res.SetSyntaxError(buf);
		}
	}
	else if(buf == "drop"){
		strin >> buf;
		if(buf == "table")
			drop_table(strin, res);
		else if(buf == "index")
			drop_index(strin, res);
		else{
			res.SetSyntaxError(buf);
		}
	}
	else if(buf == "select")
		select(strin, res);
	else if(buf == "insert")
		insert(strin, res);
	else if(buf == "delete")
		Delete(strin, res);
	else if(buf == "execfile"){
		char filename[100];
		strin >> filename;
		strin >> buf;
		if(!strin.eof()){
			res.SetSyntaxError(buf);
		}
		execfile(filename, res);
	}
	else{
		res.SetSyntaxError(buf);
	}
	res.PrintResult();	//print the result
}

int execfile(char* filename, Result &res)
{
	stringstream strin;
	ifstream fin(filename);
	if (! fin.is_open()){
		res.SetError( "ERROR: No such file" );
		return -1; 
	}
	while(1){
		char stmt[1000];
		fin.getline(stmt, 1000, ';'); //read a statement back with ';'
		if(fin.eof())
			break;
		strin.clear();
		strin.str(stmt);	//initialize stringstream
		interpreter(strin);	
	}
	res.SetError( "execfile complete" );	//not real error, just to show this information
	return 0;
}

int main()
{
	stringstream strin;
	mkdir("catalog");
	mkdir("index");
	mkdir("record");
	CataM.OpenFile("Catalog.db");
	while(1){
		char stmt[1000];
		cin.getline(stmt, 1000, ';'); //read a statement back with ';'
		if(cin.eof())
			break;
		strin.clear();
		strin.str(stmt);	//initialize stringstream
		if(interpreter(strin) == -2)	//quit
			break;
	}
	CataM.CloseFile();
	return 0;
}
