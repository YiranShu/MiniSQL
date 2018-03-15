#ifndef RESULT_H
#define RESULT_H 
#include <string>
#include "GlobalDeclar.h"
class Result
{
public:
//res_rows is set to -1 by default and affected_rows to 0 and iserror to 0
//
//if there are results coming from select
//res should contain the string to be printed and
//res_rows should be set to rows selected which is obviously >= 0
//
//if there is an error
//store the information in error
//iserror should be set to 1
	Result(): res_rows(-1), affected_rows(0), iserror(0) { };
	~Result(){}
/*
	Set error = "ERROR: Syntax error near '" + s + "'"; 
	Set iserror = 1;
 */
	void SetSyntaxError(string s);
/*
	Set error = s;
	Set iserror = 1;
 */
	void SetError(string s);
	
	void PrintResult();

	string res;
	int res_rows;
	string error;
	bool iserror;
	int affected_rows;
};

#endif

