#include "Convert2JS.h"
#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <sstream>
#include "XStrUtil.h"
#include <iostream>
#include <string>
#include "encoding.h"

static int enumNumber = 0;

#define BUFMARK 0xA13B

#define BUFCODE1 0x3ABA

#define BUFCODE2 0x413C

#define quotedToken						'#'
#define comment					        "//"
#define commentbegin				    "/*"
#define commentend				        "*/"
#define LF   '\n'
#define CR	 '\r'
#define CRLF "\r\n"
#define _2TAB	"\t\t"
#define TAB     "\t"


#define const_define	"const"
#define inc_define		"#include"
#define macro_define	"#define"
#define class_define	"class"
#define struct_define	"struct"
#define enum_define		"enum"

#define IMPORT_INC_SOCK "import { SocketHelper } from './SocketHelper'\n"


#define INTERFACE_BEGIG(ss, iclass) ss << "export interface " << iclass << "{ " << endl;

#define CLASS_BEGIN(ss, _class, iclass, _pclass) ss << "export class " << _class; \
												 if ( !_pclass.empty() ) \
												 {\
													ss << " extends " << _pclass;\
												 }\
												 ss << " implements " << iclass << "{" << endl;

#define CTOR_BEGIN(ss, _class, super) ss << TAB << "constructor(obj?:" << _class << "){" << endl;\
									  if ( super )\
									  {\
										  ss << _2TAB << "super()" << endl; \
										  ss << _2TAB << "this.len=%d" << endl;\
										  ss << _2TAB << "this.cmd=%d" << endl;\
									      ss << _2TAB << "this.cmdType=%d" << endl;\
                                          ss << _2TAB << "this.code=(this.len ^ " << BUFCODE1 << ") & " <<  BUFCODE2 << endl;\
                                          ss << _2TAB << "this.mark=" << BUFMARK  << endl;\
									  }\
									  ss << _2TAB << "if (obj){" << endl;


#define TO_OBJ_BEGIN(ss, super, size) ss << TAB << "toObject(buffer: Buffer){\n" ;\
							    ss << _2TAB << "let read = new SocketHelper(); \n" ;\
								if ( super )\
								{\
									ss << _2TAB << "super.toObject(read.getBuffer(buffer, " << size << "));\n";\
								}
								

#define TO_BUFFER_BEGIN(ss) ss << TAB << "toBuffer(): Buffer {" << endl;


#define INTERFACE_APPEND(ss, name, type, comment)  ss << TAB << name << ":" << type  << ";" << comment  << endl;

#define CLASS_APPEND(ss, name, type, comment)  ss << TAB << name << ":" << type  << ";" << comment  << endl;

//#define CTOR_APPEND(ss, name, type) if ( !type.empty() ) \
									{\
										ss << _2TAB << "let tmpObj = new " << type << "(" << "obj." << name<< ");" << endl; \
										ss << _2TAB << "this." << name << "=tmpObj;" << endl;\
									}\
									else\
									{\
										ss << _2TAB << "this." << name << "= obj." << name << endl;\
									}

#define CTOR_APPEND(ss, name, type) ss << TAB << _2TAB << "this." << name << "= obj." << name << endl;\


#define TO_OBJ_APPEND(ss, name, func, count) if ( count > 1 )\
												ss << _2TAB << "this." << name << "=" << "read." << func << "(buffer," << count << ");"  << endl;\
											 else \
												ss << _2TAB << "this." << name << "=" << "read." << func << "(buffer);"  << endl;

#define TO_OBJ_ARRAY_APPEND(ss, name, type, count, size) if ( count > 1)\
												   {\
													   ss << _2TAB << "let tmpArray = new Array<" << type << ">();" << endl;\
													   ss << _2TAB << "for (let i=0; i<" << count << "; i++ ){" << endl;\
													   ss << _2TAB << TAB << "let item = new " << type << "();" << endl;\
													   ss << _2TAB << TAB << "item.toObject(read.getBuffer(buffer, " << size << "));" << endl;\
													   ss << _2TAB << TAB << "tmpArray.push(item)" << endl;\
													   ss << _2TAB << "}" << endl;\
													   ss << _2TAB << "this." <<name << "= tmpArray" << endl;\
												   }\
												  else\
												  {\
													  ss << _2TAB << "let tmpObj = new " << type << "();" << endl;\
													  ss << _2TAB << "tmpObj.toObject(read.getBuffer(buffer, " << size << "));" << endl;\
													  ss << _2TAB << "this." << name << " = " << "tmpObj; " << endl;\
												  }

#define TO_BUFFER_ALLOC(ss, size,super)  ss << _2TAB << "let buffer=Buffer.alloc(" << size << ");" << endl;\
										 ss << _2TAB << "let helper=new SocketHelper();" << endl;\
										 if ( super )\
											 ss << _2TAB << "helper.putBuffer(buffer, super.toBuffer());" << endl;

#define TO_BUFFER_APPEND(ss, name, func, count) if ( count > 1 )\
												ss << _2TAB << "helper." << func << "(buffer, this." << name << "," << count << ");" << endl;\
											 else \
												ss << _2TAB << "helper." << func << "(buffer, this." << name << ");" <<  endl;

#define TO_BUFFER_ARRAY_APPEND(ss, name, func, count)  if ( count > 1 ) \
													   {\
															 ss << _2TAB << "for ( let i=0; i<" << count << "; i++ ){" << endl; \
															 ss << _2TAB << TAB << "helper.putBuffer(buffer, this." << name << "[i].toBuffer())" << endl;\
															 ss << _2TAB << "}" << endl;\
													   }\
													   else\
														{\
															ss << _2TAB << "helper.putBuffer(buffer,this." << name << ".toBuffer());" << endl;\
														}

#define INTERFACE_END(ss) ss <<  "}" << endl;

#define CTOR_END(ss) ss  << _2TAB << "}" << endl;\
					 ss << TAB << "}" << endl;

#define TO_OBJ_END(ss) ss << TAB << "}" << endl;

#define TO_BUFFER_END(ss) ss << _2TAB << "return buffer\n" << TAB << "}" << endl;

#define CLASS_END(ss, ss0, ss1, ss2) ss << TAB << "//ctor1... " << endl; \
									 ss << ss0.rdbuf();\
									 ss << TAB << "//ctor2... " << endl;\
									 ss << ss1.rdbuf();\
									 ss << TAB << "//toBuffer function... " << endl;\
									 ss << ss2.rdbuf();\
									 ss << "}";

#define COMMENT_SS(ss, len, cmd, cmdtype) ss << "/**" << endl;\
										  ss << "	len=" << len << endl;\
										  ss << "	cmd=" << cmd << endl;\
										  ss << "	cmdtype=" << cmdtype << endl;\
										  ss << "**/" << endl;

						

Convert::Convert()
{
	initStuCmd();
	mIsComment = false;
	mKeySet.insert("unsigned");
	mKeySet.insert("char");
	mKeySet.insert("short");
	mKeySet.insert("int");
	mKeySet.insert("long");
	mKeySet.insert("float");
	mKeySet.insert("double");
    mKeySet.insert("CByte");
    mKeySet.insert("CInt");
    mKeySet.insert("CUShort");
    mKeySet.insert("CUInt");
    mKeySet.insert("CLong");
    mKeySet.insert("CULong");

	mTypeMap.insert(make_pair("char", "number"));
	mTypeMap.insert(make_pair("char", "number"));
	mTypeMap.insert(make_pair("char", "number"));
	mTypeMap.insert(make_pair("unsigned char", "number"));
	mTypeMap.insert(make_pair("short", "number"));
	mTypeMap.insert(make_pair("unsigned short", "number"));
	mTypeMap.insert(make_pair("int", "number"));
	mTypeMap.insert(make_pair("unsigned int", "number"));
	mTypeMap.insert(make_pair("long", "number"));
	mTypeMap.insert(make_pair("unsigned long", "number"));
	mTypeMap.insert(make_pair("long long", "number"));
	mTypeMap.insert(make_pair("unsigned long long", "number"));
	mTypeMap.insert(make_pair("float", "number"));
	mTypeMap.insert(make_pair("double", "number"));
	mTypeMap.insert(make_pair("char[]", "string"));
	mTypeMap.insert(make_pair("short[]", "number[]"));
	mTypeMap.insert(make_pair("int[]", "number[]"));
	mTypeMap.insert(make_pair("CLong[]", "number[]"));
	mTypeMap.insert(make_pair("CULong[]", "number[]"));
	mTypeMap.insert(make_pair("CInt[]", "number[]"));
	mTypeMap.insert(make_pair("CUInt[]", "number[]"));
	mTypeMap.insert(make_pair("CByte[]", "string"));
	mTypeMap.insert(make_pair("CByte", "number"));
	mTypeMap.insert(make_pair("CInt", "number"));
	mTypeMap.insert(make_pair("CUShort", "number"));
	mTypeMap.insert(make_pair("CUInt", "number"));
	mTypeMap.insert(make_pair("CLong","number"));
	mTypeMap.insert(make_pair("CULong","number"));

	mTypeSizeMap.insert(make_pair("char", sizeof(char)));
	mTypeSizeMap.insert(make_pair("unsigned char", sizeof(unsigned char)));
	mTypeSizeMap.insert(make_pair("short", sizeof(short)));
	mTypeSizeMap.insert(make_pair("unsigned short", sizeof(unsigned short)));
	mTypeSizeMap.insert(make_pair("int", sizeof(int)));
	mTypeSizeMap.insert(make_pair("unsigned int", sizeof(unsigned int)));
	mTypeSizeMap.insert(make_pair("long", sizeof(long)));
	mTypeSizeMap.insert(make_pair("unsigned long", sizeof(unsigned long)));
	mTypeSizeMap.insert(make_pair("long long", sizeof(long long)));
	mTypeSizeMap.insert(make_pair("unsigned long long", sizeof(unsigned long long)));
	mTypeSizeMap.insert(make_pair("float", sizeof(float)));
	mTypeSizeMap.insert(make_pair("double", sizeof(double)));
    mTypeSizeMap.insert(make_pair("CByte", sizeof(char)));
    mTypeSizeMap.insert(make_pair("CInt", sizeof(int)));
    mTypeSizeMap.insert(make_pair("CUShort", sizeof(unsigned short)));
    mTypeSizeMap.insert(make_pair("CUInt", sizeof(unsigned int)));
    mTypeSizeMap.insert(make_pair("CLong", sizeof(long long)));
    mTypeSizeMap.insert(make_pair("CULong", sizeof(unsigned long long)));
	
	mTypeFuncMap.insert(make_pair("char", make_pair("getChar", "putChar")));
	mTypeFuncMap.insert(make_pair("unsigned char", make_pair("getUChar", "putUChar")));
	mTypeFuncMap.insert(make_pair("short", make_pair("getShort", "putShort")));
	mTypeFuncMap.insert(make_pair("unsigned short", make_pair("getUShort", "putUShort")));
	mTypeFuncMap.insert(make_pair("int", make_pair("getInt","putInt")));
	mTypeFuncMap.insert(make_pair("unsigned int", make_pair("getUInt", "putUInt")));
	mTypeFuncMap.insert(make_pair("long", make_pair("getInt", "putInt")));
	mTypeFuncMap.insert(make_pair("unsigned long", make_pair("getUInt", "putUInt")));
	mTypeFuncMap.insert(make_pair("long long", make_pair("getInt64", "putInt64")));
	mTypeFuncMap.insert(make_pair("unsigned long long", make_pair("getUInt64", "putUInt64")));
	mTypeFuncMap.insert(make_pair("float", make_pair("getFloat","putFloat")));
	mTypeFuncMap.insert(make_pair("double", make_pair("getDouble", "putDouble")));
	mTypeFuncMap.insert(make_pair("char[]", make_pair("getString", "putString")));
	mTypeFuncMap.insert(make_pair("CByte[]", make_pair("getString", "putString")));
	mTypeFuncMap.insert(make_pair("short[]", make_pair("getShortArray", "putShortArray")));
	mTypeFuncMap.insert(make_pair("CUShort[]", make_pair("getUShortArray", "putUShortArray")));
	mTypeFuncMap.insert(make_pair("int[]", make_pair("getIntArray", "putIntArray")));
	mTypeFuncMap.insert(make_pair("CLong[]", make_pair("getInt64Array", "putInt64Array")));
	mTypeFuncMap.insert(make_pair("CULong[]", make_pair("getUInt64Array", "putUInt64Array")));
	mTypeFuncMap.insert(make_pair("CByte", make_pair("getUChar", "putUChar")));
	mTypeFuncMap.insert(make_pair("CInt", make_pair("getInt","putInt")));
	mTypeFuncMap.insert(make_pair("CUShort", make_pair("getUShort", "putUShort")));
	mTypeFuncMap.insert(make_pair("CUInt", make_pair("getUInt", "putUInt")));
	mTypeFuncMap.insert(make_pair("CLong", make_pair("getInt64", "putInt64")));
	mTypeFuncMap.insert(make_pair("CULong", make_pair("getUInt64", "putUInt64")));
}

void Convert::initStuCmd()
{
	fstream reader("./stuMap.txt", ios::in | ios::out);
	if (!reader.is_open())
	{
		return;
	}
	string contentStr;
	vector<string> vItems;
	while (getline(reader, contentStr))
	{
		if (contentStr.empty())
		{
			continue;
		}
		size_t pos = contentStr.find("#");
		if (pos != string::npos)
		{
			continue;
		}
		contentStr = XStrUtil::chop(contentStr, " \t\n\r");
		vItems.clear();
		XStrUtil::split(contentStr, " \t", vItems);
		if (vItems.size() < 3)
		{
			continue;
		}
		pair<int, int> tmpPair;
		int cmd;
		int cmdtype;
		XStrUtil::to_number(vItems[1], cmd);
		XStrUtil::to_number(vItems[2], cmdtype);
		m_stCmdMap.insert(make_pair(vItems[0], make_pair(cmd, cmdtype)));
	}
	reader.close();
}

bool Convert::getStuCmd(const string& stStr, int& cmd, int& cmdtype)
{
	TMapStructCmdIter it = m_stCmdMap.find(stStr);
	if (it != m_stCmdMap.end())
	{
		cmd = it->second.first;
		cmdtype = it->second.second;
		return true;
	}
	else
	{
		return false;
	}
}

Convert::~Convert()
{
    mStrPath.clear();
}

int Convert::toJSFile(string& file, bool bAppend)
{
    int ret = error_ok;
	cout << "======================Convert " << file << " start." << endl;
    if ( !bAppend )
    {
        XStrUtil::getfilePath(file, mStrPath);
    }
    if ( bAppend )
    {
        string tmpStr(mStrPath);
        tmpStr.append("/");
        tmpStr.append(file);
        file = tmpStr;
    }
    cout << "========" << mStrPath << endl;
	string outfilename;
	XStrUtil::getfileName(file, outfilename);
	if ( outfilename.empty() )
	{
		cout << "invalid file ... " << endl;
		return error_invalid_file;
	}
	ifstream inStream;
        inStream.open(file.c_str(), ios::out|ios::in);
	if ( inStream.is_open() == false )
	{
		cout << "open " << file << " failed, error: " << errno << endl;
		return error_file;
	}

	//write file
	ofstream outStream;
        string tmpStr = mStrPath;
        tmpStr.append("\\");
	tmpStr.append(outfilename);
	tmpStr.append(".ts");
	outfilename = tmpStr;
        cout << "========" << outfilename << endl;
        outStream.open(outfilename.c_str(), ios::in|ios::out|ios::trunc);
	if ( outStream.is_open() == false )
	{
		cout << "open file: " << outfilename << " failed, error: " << errno << endl;
		return error_file;
	}
	outStream.write(IMPORT_INC_SOCK, strlen(IMPORT_INC_SOCK));
	//read file
	string contentString;
	char buf[4096] = {0};
	while(inStream.getline(buf, 4095) )
	{
		stringstream ss;
		string tmpString;
		tmpString.append(buf);
		memset(buf, 0x00, 4096);
		if (tmpString.empty())
		{
			continue;
		}
#ifdef WIN32
		tmpString.append(CRLF);
#endif
		size_t pos = 0;
		if (mIsComment)
		{
			pos = tmpString.find(commentend);
			if (pos != string::npos)
			{
				mIsComment = false;
				continue;
			}
		}
		else
		{
			pos = tmpString.find(commentbegin);
			if (pos != string::npos)
			{
				mIsComment = true;
			}
		}
		if (mIsComment == true)
		{
			continue;
		}
		contentString.append(tmpString);
		//cout << contentString;
		if ( contentString.empty() ) 
		{
			contentString.clear();
			continue;
		}
        ret = parseBuffer(contentString, ss);
		if ( ret == error_ok )
		{
			string jsStr = ss.str();
            jsStr = Encoding::ANSIToUTF8(jsStr.c_str()).toStdString();
			outStream.write( jsStr.c_str(), jsStr.length() );
			contentString.clear();
			continue;
		}
		else if ( ret == error_again )
		{
			//TODO	
		}
		else 
		{
			cout  << "file error ! " << endl;
			break;
		}
	}
	inStream.close();
	outStream.close();
	cout << "======================Convert " << file << " finish" << endl; 

        return ret;
}
	
int  Convert::writeClassToJSFile(const string& className, vector<vardef>& rVarVec,stringstream& rOutStream, bool isHerit, string& parentString)
{
	//拼接
	bool isInherit = isHerit;
	string parentClass = parentString;
	stringstream interfaceStream;
	stringstream varStream;
	stringstream ctor1Stream;
	stringstream ctor2Stream;
	stringstream toBufStream;
	stringstream commentStream;
	string IClassName("I");
	IClassName.append(className);

	varStream << "//class " << className << " define" << endl;
	if (rVarVec.size() > 0 && rVarVec[0].isInherit )
	{
		isInherit = true;
		parentClass = rVarVec[0].parentTypeString;
	}

	INTERFACE_BEGIG(interfaceStream, IClassName)
	CLASS_BEGIN(varStream, className, IClassName, parentClass)
	CTOR_BEGIN(ctor1Stream, IClassName, isInherit)
	TO_OBJ_BEGIN(ctor2Stream, isInherit, getTypeSize(parentClass))
	TO_BUFFER_BEGIN(toBufStream)

	int classSize = 0;
	vector<vardef>::iterator vIt = rVarVec.begin();
	for ( ; vIt != rVarVec.end(); ++vIt )
	{
		classSize += (*vIt).nsize * (*vIt).count;
		INTERFACE_APPEND(interfaceStream, (*vIt).nameString, getJsType((*vIt).typeString), (*vIt).commentString)
		CLASS_APPEND(varStream, (*vIt).nameString, getJsType((*vIt).typeString), (*vIt).commentString)
		string jsFuncString = getJsFunc((*vIt).typeString, 1);
		if ( jsFuncString.compare("") == 0 )
		{
			CTOR_APPEND(ctor1Stream, (*vIt).nameString, (*vIt).typeString)
			
			if ((*vIt).typeString.find("[") != string::npos)
			{
				string tstring = (*vIt).typeString;
				tstring = XStrUtil::chop(tstring, "[]");
				TO_OBJ_ARRAY_APPEND(ctor2Stream, (*vIt).nameString, tstring, (*vIt).count, getTypeSize(tstring))
			}
			else
			{
				TO_OBJ_ARRAY_APPEND(ctor2Stream, (*vIt).nameString, (*vIt).typeString, (*vIt).count, getTypeSize((*vIt).typeString))
			}
		}
		else 
		{
			CTOR_APPEND(ctor1Stream, (*vIt).nameString, string(""))
			TO_OBJ_APPEND(ctor2Stream, (*vIt).nameString, jsFuncString, (*vIt).count)
		}
	}
	if (isInherit)
	{
		classSize += getTypeSize(parentClass);
	}
	TO_BUFFER_ALLOC(toBufStream, classSize, isInherit)
	vIt = rVarVec.begin();
	for ( ; vIt != rVarVec.end(); ++vIt )
	{
		string jsFuncString = getJsFunc((*vIt).typeString, 0);
		if ( jsFuncString.compare("") ==0 )
		{
			TO_BUFFER_ARRAY_APPEND(toBufStream, (*vIt).nameString, "", (*vIt).count)
			
		}
		else 
		{
			TO_BUFFER_APPEND(toBufStream, (*vIt).nameString, jsFuncString, (*vIt).count)
		}
	}
	int cmd = 0;
	int type = 0;
	if (isInherit)
	{
		char buf[4094] = { 0 };
		getStuCmd(className, cmd, type);
		sprintf(buf, ctor1Stream.str().c_str(), (short)classSize, (char)cmd, (char)type);
		ctor1Stream.str("");
		string ctorStr(buf);
		ctor1Stream << ctorStr;
		cout << ctor1Stream.str() << endl;
	}
	INTERFACE_END(interfaceStream)
	CTOR_END(ctor1Stream)
	TO_OBJ_END(ctor2Stream)
	TO_BUFFER_END(toBufStream)
	CLASS_END(varStream, ctor1Stream, ctor2Stream, toBufStream)
	COMMENT_SS(commentStream, classSize, cmd, type)
	interfaceStream << commentStream.rdbuf();
	interfaceStream << varStream.rdbuf();
	interfaceStream << endl;

    rOutStream << interfaceStream.rdbuf();

	cout << "=======" << className << " size: " << classSize << endl;
	//add class size
	if ( addTypeSize(className, classSize) == false )
	{
		cout << "redefinition " << className << endl;
		return error_redefine_type; 
	}
	return error_ok; 
}


int Convert::parseEnum(string& str,stringstream& rOutStream)
{
	int ret = error_again;
	str = XStrUtil::chop(str, " ");

	//判断类是否读取完整
	size_t pos = str.find("{");
	if ( pos == string::npos )
	{
		return ret;
	}
	pos = str.find("}");
	if ( pos == string::npos )
	{
		return ret;
	}
	vector<string> vLineItems;
	XStrUtil::split(str, "\r\n", vLineItems);
	string enumNameStr = vLineItems[0];
	enumNameStr = XStrUtil::chop(enumNameStr, " \t\r\n{");
	if ((pos = enumNameStr.find_first_of(" \t")) == string::npos)
	{
		stringstream ss;
		ss << " " << "Tenum" << ++enumNumber;
		enumNameStr.append(ss.str());
	}
	rOutStream << "export " << enumNameStr << " { " << endl;
	for ( int i=1; i<(int)vLineItems.size(); ++i )
	{
		string& tmpStr = vLineItems[i];
		size_t pos = tmpStr.find_first_of("{}");
		if ( pos != string::npos )
		{
			continue;
		}
		rOutStream << tmpStr << endl;
	}
	rOutStream << "}" << endl;
	/*
	vector<string> vItems;
	XStrUtil::split(str, "{}", vItems);
	if ( vItems.size () < 3 )
	{
		return error_ok;
	}
	rOutStream.clear();
	rOutStream << "export " << vItems[0] << "{" << endl;
	rOutStream  << vItems[1] << endl;
	rOutStream  << "}" << endl;
	*/
	
	return error_ok;
}

int Convert::parseClass(string& str,stringstream& rOutStream)
{
	int ret = error_again;
	str = XStrUtil::chop(str, " ");

	//判断类是否读取完整
	size_t pos = str.find("{");
	if ( pos == string::npos )
	{
		return ret;
	}
	pos = str.find("}");
	if ( pos == string::npos )
	{
		return ret;
	}

	//开始解析类
	
	//解析行
	string className;
	vector<string> vLineItems;
	XStrUtil::split(str, "\n\r", vLineItems);
	//获取类名
	string lineString = vLineItems[0];
	vector<string> tmpVec;
	bool isInherit = false;
	string parentTypeString;
    XStrUtil::split(lineString, ":", tmpVec);
	if (tmpVec.size() > 1)
	{
		isInherit = true;
		vector<string> vClass;
		XStrUtil::split(tmpVec[1], " \t", vClass);
		if (vClass.size() >= 2)
		{
			parentTypeString = XStrUtil::chop(vClass[1], " \t{");
		}
	}
	vector<string> vClass;
	XStrUtil::split(tmpVec[0], " \t", vClass);
	if (vClass.size() < 2)
	{
		return error_format;
	}
	className = vClass[1];

	mKeySet.insert(XStrUtil::chop(className, " \t{"));	
	mImportClass.insert(XStrUtil::chop(className, " \t{"));
	//解析变量
	vector<vardef> varDefVec;
	set<string>::iterator it;
	for ( int i=1; i<(int)vLineItems.size(); ++i )
	{
		lineString = vLineItems[i];
		string varString;
		string commentString;
		string typeString;
		string varname;
		size_t pos = lineString.find("/");
		if (pos == string::npos )
		{
			varString = lineString;	
		}
		else 
		{
			varString = lineString.substr(0, pos);
			commentString = lineString.substr(pos);
		}
        if ( varString.size() <= 0 )
        {
            continue;
        }
		
		string arrStr;
		if ((pos = varString.find("[") ) != string::npos)
		{
			arrStr = varString.substr(pos);
			varString = varString.substr(0, pos);
		}
		vector<string> vItems;
		XStrUtil::split(varString, " \t", vItems);
        if ( vItems.size() < 2 )
        {
            continue;
        }
        //变量
        varname = vItems.back();
		varname.append(arrStr);
        vItems.pop_back();
        //类型
		bool isContiune = false;
        for ( int j=0; j < (int)vItems.size(); ++j )
        {
                string& key = vItems[j];
                it = mKeySet.find(key);
                if ( it != mKeySet.end() ) //类型
                {
                        typeString.append(" ");
                        typeString.append(key);
                }
                else
                {
                        //return error_format;
					isContiune = true;
					break;
                }
        }
		if (isContiune)
		{
			continue;
		}
		if ( varname.find_first_of("()") != string::npos ) //函数drop
		{
			continue;
		}
		vardef stVar;
		typeString = XStrUtil::chop(typeString, " ");
		varname    = XStrUtil::chop(varname, "; ");
		int size = 0;
		int cnt  = 1; 
		vector<string> varVec;
		XStrUtil::split(varname, "[]", varVec);
		if ( varVec.size() > 1 ) //array[]
		{
			varname = varVec[0];
			XStrUtil::to_number(varVec[1], cnt);
			if (getArraySize(varVec[1], cnt) == false)
			{
				continue;
			}
			size = getTypeSize(typeString) ;
			typeString.append("[]");
		}
		else 
		{
			size = getTypeSize(typeString);
		}
		stVar.typeString = typeString;
		stVar.nameString = varname;
		stVar.commentString = commentString;
		stVar.count = cnt;
		stVar.nsize = size;
		stVar.isInherit = isInherit;
		stVar.parentTypeString = parentTypeString;
		varDefVec.push_back(stVar);
	}
	//map<string, vector<vardef> > tmpMap;
	//tmpMap.insert(make_pair(parentTypeString, varDefVec));
	//拼接
	ret = writeClassToJSFile(className, varDefVec, rOutStream, isInherit, parentTypeString);
	return ret;
}

int Convert::getTypeSize(const string& cType)
{
	map<string, int>::iterator it = mTypeSizeMap.find(cType);
	if ( it != mTypeSizeMap.end() )
	{
		return it->second;
	}
	return 0;
}

bool Convert::addTypeSize(const string& cType, int size)
{
	map<string, int>::iterator it = mTypeSizeMap.find(cType);
	if ( it != mTypeSizeMap.end() )
	{
		return false;
	}
	mTypeSizeMap.insert(make_pair(cType, size));
	return true;
}

string Convert::getJsFunc(const string& cType, int read)
{
	map<string, pair<string, string> >::iterator it = mTypeFuncMap.find(cType);
	if ( it != mTypeFuncMap.end() )
	{
		pair<string, string>& tmpPair = it->second;
		if ( read == 1 )
		{
			return tmpPair.first;
		}
		else 
		{
			return tmpPair.second;
		}
	}
	return "";
}

int Convert::getTypeValue(string& sType)
{
	for ( int i=0; i<(int)m_keyMap.size(); ++i )
	{
		if ( m_keyMap[i].first.compare(sType) == 0 )
		{
			return m_keyMap[i].second;
		}
	}
	return -1;
}

string Convert::getJsType(const string& cType)
{
	map<string, string>::iterator it = mTypeMap.find(cType);
	if ( it != mTypeMap.end() )
	{
		return it->second;
	}
	else 
	{
		return (string)cType;
	}
}

string Convert::getJsType(int vMacro)
{
	return "number";
}
	
string Convert::getJSFun(int vMacro)
{
	for ( int i=0; i<(int)m_funMap.size(); ++i )
	{
		if ( m_funMap[i].first == vMacro )
		{
			return m_funMap[i].second;
		}
	}
	return "";
}


int Convert::parseConst(string& str, stringstream& rOutStream)
{
	vector<string> vItems;
	string& constString = XStrUtil::chop(str, " \n\r");
	XStrUtil::split(constString, "=", vItems);
	if (vItems.size() < 2)
	{
		return error_ok;
	}
	int value = 0;
	vector<string> vs;
	XStrUtil::split(vItems[1], ";", vs);
	if (vs.size() <= 0)
	{
		return error_ok;
	}
	string valueStr = XStrUtil::chop(vs[0], " \t");
	if (XStrUtil::to_number(valueStr, value) == false)
	{
		return error_ok;
	}
	string name = XStrUtil::chop(vItems[0], " \t");
	vItems.clear();
	XStrUtil::split(name, " \t", vItems);
	if (vItems.size() < 3)
	{
		return error_ok;
	}
	name = vItems[2];
	m_macroMap.insert(make_pair(name, value));
	rOutStream << "const " << name << " = " << value << endl;
	return error_ok;
}

int Convert::parseMacro(string& str,stringstream& rOutStream)
{
	vector<string> vItems;
	string& macroString = XStrUtil::chop(str, " \n\r");
	XStrUtil::split(macroString, " \t", vItems);
	if (vItems.size() < 3)
	{
		return error_ok;
	}
	int value = 0;
	if ( XStrUtil::to_number(vItems[2], value) == false )
	{
		return error_ok; 
	}
	m_macroMap.insert(make_pair(vItems[1], value));
	rOutStream << "const " << vItems[1] << " = " << vItems[2] << endl;
	return error_ok;
}

int Convert::parseInclude(string& str, stringstream& rOutStream)
{
	string contentStr = str;
	contentStr = XStrUtil::chop(contentStr, " ");
	if ( contentStr[0] == '/' )
	{
		//comment 
		return error_ok;
	}
	vector<string> vItems;
	XStrUtil::split(contentStr, " \t", vItems );
	if ( vItems.size() < 2 )
	{
		return error_format;
	}

	string tmpIncStr;
	string incStr = vItems[1];
    incStr = XStrUtil::chop(incStr, " \"\r\n");
	tmpIncStr = incStr;
    if (toJSFile(incStr, true) != error_ok)
	{
		return error_ok;
	}
	
	size_t pos = tmpIncStr.find(".");
	if ( pos != string::npos )
	{
		tmpIncStr = tmpIncStr.substr(0, pos);
	}
	set<string>::iterator it = mImportClass.begin();
	for (; it != mImportClass.end(); ++it)
	{
		rOutStream << "import {" << *it << "} from " << "'./" << tmpIncStr << "';" << endl;
	}
	rOutStream << endl;
	mImportClass.clear();
	return error_ok;
}


int Convert::parseBuffer(string& str, stringstream& rOutStream)
{
	int ret = error_ok;
	size_t pos = str.find(inc_define);
	if ( pos != string::npos )
	{
		ret = parseInclude(str, rOutStream);	
	}
	pos = str.find(const_define);
	if (pos != string::npos)
	{
		ret = parseConst(str, rOutStream);
	}
	pos = str.find(macro_define);		
	if ( pos != string::npos ) //宏
	{
		ret =  parseMacro(str, rOutStream);
	} 
	pos = str.find(class_define);
	if ( pos != string::npos )  //class
	{
		ret = parseClass(str, rOutStream);	
	}
	pos = str.find(struct_define); //struct
	if ( pos != string::npos )
	{
		ret = parseClass(str, rOutStream);
	}
	pos = str.find(enum_define);	
	if ( pos != string::npos )
	{
		ret = parseEnum(str, rOutStream);
	}
	return ret;
}

bool Convert::getArraySize(string& numStr, int& size)
{
	size = 0;
	vector<string> vItems;
	XStrUtil::split(numStr, "+", vItems);
	if (vItems.size() <= 0)
	{
		return false;
	}
	map<string, int>::iterator it;
	for (int i = 0; i < (int)vItems.size(); ++i)
	{
		string tmpStr = XStrUtil::chop(vItems[i], " ");
		if (!XStrUtil::is_number(tmpStr))
		{
			it = m_macroMap.find(tmpStr);
			if (it == m_macroMap.end())
			{
				return false;
			}
			size += it->second;
		}
		else
		{
			int ntmp;
			XStrUtil::to_number(tmpStr, ntmp);
			size += ntmp;
		}
	}
	return true;
}




