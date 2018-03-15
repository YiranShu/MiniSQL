#include <iostream>

using namespace std;

int main()
{
	freopen("gtest.sql", "w", stdout);
	cout << "drop table pt;"<<endl;
	cout << "create table pt ( \nid int, \nname char(5), \nincome float, \nprimary key ( id )\n);"<<endl;
	for(int i = 1; i <= 10000; i ++){
		cout <<"insert into pt value ("<< i <<", 'name"<< i%10 <<"', "<< i*100.4/7 <<");"<< endl;
	}
//	cout << "select * from pt where income > 1000 and id = 9939; "<<endl;
	return 0;
}