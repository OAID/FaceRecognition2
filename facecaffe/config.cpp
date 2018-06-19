#include "config.h"
#include "util.h"

Config* Config::_instance = NULL;

Config::Config(void)
{

}

Config::~Config(void)
{

}

Config* Config::Instance()
{
	if(NULL == _instance)
	{
		_instance = new Config();
	}
	return _instance;
}

void Config::Release()
{
	delete _instance;
	_instance = NULL;
}

bool Config::LoadConfig(const string& conf_path)
{
	ScopeLocker locker(&_mutex);
	
	this->_conf_path = conf_path;

	ifstream fin(conf_path.c_str());
	if (!fin)
	{
		return false;
	}
	string line = "";
	while (getline(fin, line))
	{
		if (line.empty())
		{
			continue;
		}
		if ('#' == line[0])
		{
			continue;
		}
		string::size_type pos = line.find("=");
		if (string::npos != pos)
		{
			string key = line.substr(0, pos);
			string value = "";
			if (line.size() > pos)
			{
				 value = line.substr(pos + 1, line.size() - pos);
			}

		  	key = Util::Trim(key);
		  	value = Util::Trim(value);
		  	this->conf_kvp[key] = value;
		}
	}
	fin.close();	

	return true;
}

void Config::PrintAll()
{
	map<string, string>::iterator it = this->conf_kvp.begin();
	for(; it != this->conf_kvp.end(); ++it)
	{
	  cout<<"key:"<<it->first<<" value:"<<it->second<<endl;
	}
}

string Config::GetValue(const string& key)
{
	ScopeLocker locker(&_mutex);

	map<string, string>::iterator it = this->conf_kvp.find(key);
	if (this->conf_kvp.end() != it)
	{
		return it->second;
	}
	return "";
}

bool Config::SetValue(const string& key, const string& value)
{
	ScopeLocker locker(&_mutex);

	this->conf_kvp[key] = value;

	ofstream fout;
	fout.open(this->_conf_path.c_str(), ios::out | ios::binary | ios::trunc);

	map<string, string>::iterator it = this->conf_kvp.begin();
	for(; it != this->conf_kvp.end(); ++it)
	{
		fout<<it->first<<"="<<it->second<<endl;
	}

	fout.close();

	return true;
}
