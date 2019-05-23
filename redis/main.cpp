#include "luaScript.h"
#include "../hiredis/hiredis.h"
#include <iostream>

using namespace std;

int main()
{
	redisContext* c = redisConnect("127.0.0.1", 6379);
	if (c->err)
	{
		redisFree(c);
		cout << "Connect to redisServer fail" << endl;
		return 1;
	}
	
	cout << "Connect to redisServer Success" << endl;
	redisReply* r = (redisReply*)redisCommand(c, "select 1");
	cout << r->type << endl;
	cout << r->str << endl;
	freeReplyObject(r);

	r = (redisReply*)redisCommand(c, "set ccc ccc");
	cout << r->type << endl;
	cout << r->str << endl;
	freeReplyObject(r);

	r = (redisReply*)redisCommand(c, "get ccc");
	cout << r->type << endl;
	cout << r->str << endl;
	freeReplyObject(r);

	LuaScriptMgr::getInstance().init(c);
	LuaScriptMgr::getInstance().test(c);

	redisFree(c);
	return 0;
}