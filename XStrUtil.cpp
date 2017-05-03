#include "XStrUtil.h"

string& XStrUtil::chop(string& sSrc, const char* delm)
{

	size_t pos = sSrc.find_first_not_of(delm);
	sSrc= sSrc.erase(0, pos);
	pos = sSrc.find_last_not_of(delm);
	if ( pos == string::npos )
	{
		return sSrc;	
	}
	sSrc.erase(++pos);
	return sSrc;
}

bool XStrUtil::is_number(string& sSrc)
{
	int index = 0;
	for (; index<(int)sSrc.size(); ++index)
	{
		if (sSrc[index] < '0' || sSrc[index] > '9')
		{
			return false;
		}
	}
	return true;
}

bool XStrUtil::to_number(string& sSrc, int& rOutV)
{
	int sum = 0;
	int sign = 1;
	int index = 0;
	sSrc = chop(sSrc, "");
	if ( sSrc[0] == '-')
	{
		sign = -1;
		index = 1;
	}
	for ( ; index<(int)sSrc.size(); ++index )
	{
		if ( sSrc[index] < '0' || sSrc[index] > '9' )
		{
			return false;
		}
		char& ch = sSrc[index];
		sum = sum * 10 + (ch - '0');
	}
	sum = sum*sign;
	rOutV = sum;
	return true;
}

void XStrUtil::split(string& sSrc, const char* sep, vector<string>& rOut)
{
	sSrc = chop(sSrc, sep);
	size_t beginPos = 0;
	size_t endPos   = 0;
	string subStr;
	bool isbreak = false;
	for (;;)
	{
		endPos = sSrc.find_first_of(sep, beginPos);
		if ( endPos == string::npos )
		{
			subStr = sSrc.substr(beginPos);
			isbreak = true;
		}
		else 
		{
			subStr = sSrc.substr(beginPos, endPos-beginPos);
		}
		subStr = chop(subStr, sep);
		if ( subStr.empty() == false )
		{
			rOut.push_back(subStr);
		}
		if ( isbreak )	
		{
			break;
		}
		beginPos = endPos + 1;
	}
}
void XStrUtil::getfileName(const string& sPath, string& filename)
{
	if (sPath.size()<=0 )
	{
		filename = "";
		return;
	}
	size_t pos = sPath.find_last_of("/\\");
	if ( pos == string::npos)
	{
		filename = sPath;	
	}
	else 
	{
		filename = sPath.substr(pos+1);
	}
	//drop suffix
	pos = filename.find(".");
	if ( pos == string::npos)
	{
		return;
	}
	filename = filename.substr(0, pos);
}

void XStrUtil::getfilePath(const string& sPath, string& path)
{
	if ( sPath.size() <=0 )
	{
		path = "";
		return;
	}
	size_t pos = sPath.find_last_of("/\\");
	if ( pos == string::npos )
	{
		path = "./";
	}
	else 
	{
		path = sPath.substr(0, pos);
	}
	return;
}

