#include <string>
#include <vector>
using namespace std;


namespace common
{
class XStrUtil
{
public:
	static string& chop(string& sSrc, const char* delm);
	static bool to_number(string& sSrc, int& rOutV);
	static bool is_number(string& sSrc);
	static void split(string& sSrc, const char* sep, vector<string>& rOut);
	static void getfileName(const string& sPath, string& filename); 
	static void getfilePath(const string& sPath, string& path);
};
}


using namespace common;
