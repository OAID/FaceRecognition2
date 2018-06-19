#ifndef _UTIL_H
#define _UTIL_H

#include <string>
#include <sstream>
#include <vector>

#ifdef _WIN32	//Windows
#include <direct.h>
#include <io.h>
#include <windows.h>
#include <WinSock2.h>
#else 			//Linux
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <iconv.h>
#include <arpa/inet.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

#endif

#include <stdint.h>

using namespace std;

class Util
{
public:
	static string Uint2String(unsigned int value);
	static string Int2String(int value);
	static string Ulint2String( unsigned long long value);
	static string Lint2String(long long value);
	static int String2Int(const string& value);
	static unsigned int String2Uint(const string& value);
	static long long String2Lint(const string& value);
	static unsigned long long String2Ulint(const string& value);
	static string Double2String(double value, int length);
	static double String2Double(const string& value);
	static const string Trim(const string& value);
	static vector<string> Split( const string& src, const string& split);

	static bool IsFileExist(const string& strPath);	//判断文件和目录是否存在
	static bool CreateDir(const string& strDirectory);	//创建目录
	static long long GetFileSize(FILE* fStream);	//获取文件大小
	static long long GetFileSize(const char* strFile);	//获取文件大小
	static vector<string> GetFileList(const string& strDir, const string& strExtension );
	static vector<string> GetFileListByName(const string& strDir, const string& strFileName);
	static bool GetCurWorkDir(string& strCurWorkDir);
	static string GetRootDir();
	static string GetFileDir(const string& strFilePath);
	static bool DeleteDir(const string& strDir);
	static bool DeleteFile(const string& strFile);

	static int16_t Net2Host16(int16_t net16);
	static int16_t Host2Net16(int16_t host16);
	static int32_t Net2Host32(int32_t net32);
	static int32_t Host2Net32(int32_t host32);
	static int64_t Net2Host64(int64_t net64);
	static int64_t Host2Net64(int64_t host64);

	static bool KillProcess(int32_t process_id);
	static void Wait(int64_t mill_second);
	static long GetCurrentThreadID();
};

#endif
