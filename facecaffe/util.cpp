#include "util.h"

string Util::Uint2String(unsigned int value)
{
	std::ostringstream os;
	os<<value;
	string rlt = os.str();
	os.clear();
	return rlt;
}

string Util::Int2String(int value)
{
	std::ostringstream os;
	os<<value;
	string rlt = os.str();
	os.clear();
	return rlt;
}

string Util::Ulint2String( unsigned long long value)
{
	std::ostringstream os;
	os<<value;
	string rlt = os.str();
	os.clear();
	return rlt;
}

string Util::Lint2String(long long value)
{
	std::ostringstream os;
	os<<value;
	string rlt = os.str();
	os.clear();
	return rlt;
}

int Util::String2Int(const string& value)
{
	int rlt = 0;
	istringstream is(value);
	is>>rlt;
	is.clear();
	return rlt;
}

unsigned int Util::String2Uint(const string& value)
{
	unsigned int rlt = 0;
	istringstream is(value);
	is>>rlt;
	is.clear();
	return rlt;
}

long long Util::String2Lint(const string& value)
{
	long long rlt = 0;
	istringstream is(value);
	is>>rlt;
	is.clear();
	return rlt;
}

unsigned long long Util::String2Ulint(const string& value)
{
	unsigned long long rlt = 0;
	istringstream is(value);
	is>>rlt;
	is.clear();
	return rlt;
}

string	Util::Double2String(double value, int length)
{
	double Val = value;
	string	Result = "";
	if (Val < 0)
	{
		Result.append("-");
		Val *= -1;
	}
	if (Val > 1.0)
	{
		int IntVal = (int)Val;
		string StrVal = Int2String(IntVal);
		Result += StrVal;
		Val -= IntVal;
	}
	else
	{
		Result.append("0");
	}
	Result.append(".");
	for (int i=0; i<length; i++)
	{
		Val *= 10;
		if (Val > 1.0)
		{
			int IntVal = (int)Val;
			string StrVal = Int2String(IntVal);
			Result += StrVal;
			Val -= IntVal;
		}
		else
		{
			Result.append("0");
		}
	}
	printf("input value: %1.8f, %s\n", value, Result.c_str());
	return Result;
}

double Util::String2Double( const string& value )
{
	return atof(value.c_str());
}

const string Util::Trim( const string& value )
{
	std::string::size_type first = value.find_first_not_of(" \n\t\r\0xb");
	if (first == std::string::npos) {
		return std::string();
	}
	else {
		std::string::size_type last = value.find_last_not_of(" \n\t\r\0xb");
		return value.substr( first, last - first + 1);
	}
}

vector<string> Util::Split( const string& src, const string& split)
{
	vector<string> dest;
	string tmp = src;
	while(true)
	{
		string::size_type pos = tmp.find(split);
		if (string::npos == pos)
		{
			dest.push_back(tmp);
			break;
		}

		dest.push_back(tmp.substr(0, pos));
		tmp = tmp.substr(pos + split.size());
	}

	return dest;
}

bool Util::IsFileExist(const string& strPath)
{
    #ifdef _WIN32   //Windows
	if (0 == _access(strPath.c_str(), 0))
	{
		return true;
	}
	#else   //Linux
    if (0 == access(strPath.c_str(), F_OK))
	{
		return true;
	}
	#endif
	return false;
}

bool Util::CreateDir( const string& strDirectory )
{
    #ifdef _WIN32   //Windows
    if (0 == _mkdir(strDirectory.c_str()))
	{
		return true;
	}

    #else   //Linux
    if (0 == mkdir(strDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
	{
		return true;
	}
    #endif

	return false;
}

long long Util::GetFileSize( FILE* fStream )
{
 	if (NULL == fStream)
	{
		return 0;
	}
	long long lCurPos(0), lLength(0);
	lCurPos = ftell(fStream);
	fseek(fStream, 0, SEEK_END);
	lLength = ftell(fStream);
	fseek(fStream, lCurPos, SEEK_SET);
	return lLength;
}

long long Util::GetFileSize( const char* strFile )
{
    FILE* fStream = NULL;
     #ifdef _WIN32   //Windows
	if (0 != fopen_s(&fStream, strFile, "r"))
	{
		return 0;
	}
	#else   //Linux
	fStream = fopen(strFile, "r");
	#endif

	if (NULL == fStream)
	{
		return 0;
	}
	long long lCurPos(0), lLength(0);
	lCurPos = ftell(fStream);
	fseek(fStream, 0, SEEK_END);
	lLength = ftell(fStream);
	fseek(fStream, lCurPos, SEEK_SET);

	fclose(fStream);
	return lLength;
}

vector<string> Util::GetFileList(const string& strDir, const string& strExtension )
{
    #ifdef _WIN32
	//获取分隔符
	string PATH_SPLIT_STRING = "\\";
	if (string::npos == strDir.find(PATH_SPLIT_STRING))
	{
		PATH_SPLIT_STRING = "/";
	}


	string strDirectory(strDir);
	if(0 != strcmp(&strDirectory.at(strDirectory.size() - 1), PATH_SPLIT_STRING.c_str()))
	{
		strDirectory += PATH_SPLIT_STRING;
	}

	string strPattern;
	strPattern += strDirectory;

	if (strExtension.empty())
	{
		strPattern = strPattern + "*.*";
	}else{
		strPattern = strPattern + "*." + strExtension;
	}

	vector<string> vecFileList;
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFileA(strPattern.c_str(), &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		return vecFileList;
	}

	do
	{
		if (0 != strcmp(FindFileData.cFileName, "..") && 0 != strcmp(FindFileData.cFileName, "."))
		{
			vecFileList.push_back(strDir + PATH_SPLIT_STRING + FindFileData.cFileName);
		}
	} while (FindNextFileA(hFind, &FindFileData));


	FindClose(hFind);

	return vecFileList;

	#else //for linux

	vector<string> vecFileList;

	DIR *dp;
    struct dirent *dirp;
    if(NULL == (dp=opendir(strDir.c_str())))
    {
        return vecFileList;
    }

    while(NULL != (dirp=readdir(dp)))
    {
        if(0 == (strcmp(dirp->d_name,"."))||0 == (strcmp(dirp->d_name,"..")))
        {
            continue;
        }

        if(!strExtension.empty())
        {
            string strTmpName(dirp->d_name);
            size_t pos = strTmpName.rfind(".");
            if(string::npos != pos)
            {
                string strExt = strTmpName.substr(pos + 1);
                if(0 == strExt.compare(strExtension))
                {
                    string strFilePath = strDir + string("/") + string(dirp->d_name);
                    vecFileList.push_back(strFilePath);
                }
            }

        }else{
            string strFilePath = strDir + string("/") + string(dirp->d_name);
            vecFileList.push_back(strFilePath);
        }
    }

	return vecFileList;

	#endif
}

vector<string> Util::GetFileListByName( const string& strDir, const string& strFileName )
{
    #ifdef _WIN32
	//获取分隔符
	string PATH_SPLIT_STRING = "\\";
	if (string::npos == strDir.find(PATH_SPLIT_STRING))
	{
		PATH_SPLIT_STRING = "/";
	}
	string strDirectory(strDir);
	if(0 != strcmp(&strDirectory.at(strDirectory.size() - 1), PATH_SPLIT_STRING.c_str()))
	{
		strDirectory += PATH_SPLIT_STRING;
	}

	string strFilePath = strDirectory + "*.*";

	vector<string> vecFileList;
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFileA(strFilePath.c_str(), &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		return vecFileList;
	}

	do
	{
		if (0 != strcmp(FindFileData.cFileName, "..") && 0 != strcmp(FindFileData.cFileName, "."))
		{
			string strSubPath = strDir + PATH_SPLIT_STRING + FindFileData.cFileName;
			//如果是目录，进入目录查找
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				vector<string> vecFiles = GetFileListByName(strSubPath, strFileName);
				vecFileList.insert(vecFileList.end(), vecFiles.begin(), vecFiles.end());
			}else{
				//如果是文件，判断是否是所查找的文件
				if (0 == strFileName.compare(string(FindFileData.cFileName)))
				{
					vecFileList.push_back(strSubPath);
				}
			}
		}
	} while (FindNextFileA(hFind, &FindFileData));

	FindClose(hFind);

	return vecFileList;

	#else //for linux

	vector<string> vecFileList;

	DIR *dp;
    struct dirent *dirp;
    if(NULL == (dp=opendir(strDir.c_str())))
    {
        return vecFileList;
    }

    while(NULL != (dirp=readdir(dp)))
    {
        if(0 == (strcmp(dirp->d_name,"."))||0 == (strcmp(dirp->d_name,"..")))
        {
            continue;
        }

        if(DT_DIR == dirp->d_type)
        {
            vector<string> vecFiles = GetFileListByName(strDir + string("/") + string(dirp->d_name), strFileName);
            vecFileList.insert(vecFileList.end(), vecFiles.begin(), vecFiles.end());
        }

        if(0 == (strcmp(dirp->d_name, strFileName.c_str())))
        {
            string strFilePath = strDir + string("/") + string(dirp->d_name);
            vecFileList.push_back(strFilePath);
        }
    }

	return vecFileList;

	#endif
}


#define MAX_FILEPATH 256

bool Util::GetCurWorkDir( string& strCurWorkDir )
{
    char buf[MAX_FILEPATH] = {0};
    #ifdef _WIN32   //Windows
	if (NULL ==  _getcwd(buf, MAX_FILEPATH))
	{
		return false;
	}
	#else   //Linux
	if (NULL ==  getcwd(buf, MAX_FILEPATH))
	{
		return false;
	}

	strCurWorkDir = buf;
	return true;
	#endif
	return true;
}

std::string Util::GetRootDir()
{
    string strRootDir = "";
    #ifdef _WIN32
	char buf[MAX_PATH];
	if (::GetModuleFileName(IMP_NULL, buf, MAX_PATH) > 0)
	{
		strRootDir = buf;
		size_t pos = strRootDir.find_last_of("\\");
		if (string::npos != pos)
		{
			strRootDir = strRootDir.substr(0, pos);
		}

		vector<string> vec;
		StringSplit(strRootDir, "\\", vec);

		strRootDir = "";

		for (vector<string>::iterator it = vec.begin(); it != vec.end(); ++it)
		{
			if (strRootDir.empty())
			{
				strRootDir = *it;
			}else{
				strRootDir = strRootDir + "/" + *it;
			}


		}
	}

	#else // for linux
	char link[MAX_FILEPATH], path[MAX_FILEPATH];
    sprintf(link, "/proc/%d/exe", getpid());
    memset(path, 0, sizeof(path));
    ssize_t rlt = readlink(link, path, sizeof(path));
    if(rlt < 0)
    {
        return "";
    }
    std::string strPath(path);
    size_t pos = strPath.rfind("/");
    if(string::npos == pos)
    {
        return strRootDir;
    }
    strRootDir = strPath.substr(0, pos);
	#endif
	return strRootDir;
}

std::string Util::GetFileDir( const string& strFilePath )
{
	string strFileDir = "";

	size_t pos = strFilePath.find_last_of("/");
	if (string::npos == pos )
	{
		pos = strFilePath.find_last_of("\\");
	}

	if (string::npos != pos )
	{
		strFileDir = strFilePath.substr(0, pos);
	}
	return strFileDir;
}


bool Util::DeleteDir( const string& strDir )
{
	string strCMD = "";
#ifdef _WIN32
	strCMD = string("rmdir /S /Q ") + strDir;
#else //for linux
	strCMD = string("rm -rf ") + strDir;
#endif

	if (system(strCMD.c_str()) < 0)
	{
		return false;
	}

	return true;
}

bool Util::DeleteFile( const string& strFile )
{
	string strCMD = "";
#ifdef _WIN32
	strCMD = string("del /Q ") + strFile;
#else //for linux
	strCMD = string("rm -f ") + strFile;
#endif

	if (system(strCMD.c_str()) < 0)
	{
		return false;
	}

	return false;
}

int16_t Util::Net2Host16(int16_t net16)
{
    return ntohs(net16);
}

int16_t Util::Host2Net16(int16_t host16)
{
    return htons(host16);
}

int32_t Util::Net2Host32(int32_t net32)
{
    return ntohl(net32);
}

int32_t Util::Host2Net32(int32_t host32)
{
    return htonl(host32);
}

int64_t Util::Net2Host64(int64_t net64)
{
    if (__BYTE_ORDER == __LITTLE_ENDIAN)
    {
        return (((int64_t )htonl((int32_t)((net64 << 32) >> 32))) << 32) | (uint32_t)htonl((int32_t)(net64 >> 32));
    }
    else if (__BYTE_ORDER == __BIG_ENDIAN)
    {
        return net64;
    }
}

int64_t Util::Host2Net64(int64_t host64)
{
    if (__BYTE_ORDER == __LITTLE_ENDIAN)
    {
        return (((int64_t )htonl((int32_t)((host64 << 32) >> 32))) << 32) | (uint32_t)htonl((int32_t)(host64 >> 32));
    }
    else if (__BYTE_ORDER == __BIG_ENDIAN)
    {
        return host64;
    }
}

bool Util::KillProcess(int32_t process_id)
{
	string sys_cmd = "";
#ifdef _WIN32
	sys_cmd = string("taskkill /F /T /PID ") + Int2String(process_id);
#else //for linux
	sys_cmd = string("kill -9 ") + Int2String(process_id);
#endif

	if (system(sys_cmd.c_str()) < 0)
	{
		return false;
	}

	return true;
}

void Util::Wait(int64_t mill_second)
{
#ifdef _WIN32
	::Sleep(mill_second);
#else //for linux
	int64_t micro_second = mill_second*1000;
	usleep(micro_second);
#endif
}

long Util::GetCurrentThreadID()
{
#ifdef _WIN32
	return ::GetCurrentThreadId();
#else //for linux
		return pthread_self();
#endif
}
