#ifndef __convert_h__
#define __convert_h__
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <map>
using namespace std;

enum error
{
	error_ok     = 0,
	error_file   = 1,
	error_format = 2, 
	error_file_empty = 3,
	error_again = 4,
	error_file_not_open = 5,
	error_invalid_file = 6,
	error_redefine_type = 7,
};

#define BUFFER_SIZE 4096
struct vardef
{
	string typeString;
	string nameString;
	string commentString;
	int    nsize;
	int    count;
	bool   isInherit;
	string parentTypeString;
};

class Convert
{
	typedef map<string, string> TClassTypeVar;
	typedef map<string, string>::iterator TClassTypeVarIter;
public:
	Convert();
	~Convert();

        int toJSFile(string& file,bool bAppend=false);

private:
	int parseBuffer(string& str,stringstream& rOutStream);
	int parseConst(string& str, stringstream& rOutStream);
	int parseMacro(string& str,stringstream& rOutStream);
	int parseClass(string& str,stringstream& rOutStream);
	int parseEnum(string& str,stringstream& rOutStream);
	int parseInclude(string& str, stringstream& rOutStream);
	int writeClassToJSFile(const string& className, vector<vardef>& rVarVec,stringstream& rOutStream, bool isHerit, string& parentString);
	int getTypeValue(string& sType);
	string getJSFun(int vMacro);
	string getJsType(int vMacro);
	string getJsType(const string& cType);
	int    getTypeSize(const string& cType);
	string getJsFunc(const string& cType, int read);
	bool addTypeSize(const string& cType, int size);
	bool getArraySize(string& numStr, int& size);
private:
	map<string, int> m_macroMap;

	map<string, int> m_keyTypeMap;
	vector<pair<string, int> > m_keyMap;
	vector<pair<int, string> > m_funMap;	
	set<string> mKeySet;
	set<string> mImportClass;
	map<string, int> mTypeSizeMap;
	map<string, pair<string,string> > mTypeFuncMap;
	map<string, string> mTypeMap;
    string mStrPath;
	bool   mIsComment;
};

#endif // __convert_h__
