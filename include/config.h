#ifndef _CONFIG_H
#define _CONFIG_H

#include <string>
#include <fstream>
#include <map>
#include <iostream>
#include "lock.h"
using namespace std;

class Config
{
private:
  	Config(void);
  	~Config(void);

  	static Config* _instance;

public:
	static Config* Instance();
	static void Release();

	bool LoadConfig(const string& conf_path);
	string GetValue(const string& key);
	bool SetValue(const string& key, const string& value);

	void PrintAll();

private:
	string _conf_path;
  	map<string, string> conf_kvp;
  	Locker _mutex;

};

#endif
