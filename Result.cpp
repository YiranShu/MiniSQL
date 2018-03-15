#include <iostream>
#include <string>
#include "Result.h"
using namespace std;	

void Result::SetSyntaxError(string s)
{
	error = "ERROR: Syntax error near '" + s + "'"; 
	iserror = 1;
}

void Result::SetError(string s)
{
	error = s;
	iserror = 1;
}

void Result::PrintResult(){
	if(res_rows == 0){	//select returned empty result
		cout << "Empty set" << endl;
	}
	else if(res_rows > 0){	//select returned non-empty result
		cout << res <<endl;
		cout << res_rows;
		if(res_rows == 1) 
			cout<< " row in set" <<endl;
		else
			cout<< " rows in set" <<endl;
	}
	else{	//no select result
		if(iserror == 0){
			cout << "Query OK, " << affected_rows << " rows affected" <<endl;
		}
		else{	//affected_rows < 0 represents error
			cout << error <<endl;
		}
	}
	cout <<endl;
}