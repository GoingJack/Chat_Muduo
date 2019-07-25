#pragma once

#include <string>
using namespace std;

// AllGroup表的领域模型
class GroupDO
{
public:
	GroupDO() {}
	void setID(int id) { _id = id; }
	void setName(string name) { _groupname = name; }
	void setDesc(string desc) { _groupdesc = desc; }

	int getID()const { return _id; }
	string getName()const { return _groupname; }
	string getDesc()const { return _groupdesc; }
private:
	int _id;
	string _groupname;
	string _groupdesc;
};