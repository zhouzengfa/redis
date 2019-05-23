
#include "luaScript.h"
#include "../hiredis/hiredis.h"
#include <iostream>

//int main()
//{
//	redisContext* c = redisConnect("127.0.0.1", 6379);
//	redisReply *reply;
//	if (c->err)
//	{
//		redisFree(c);
//		cout << "Connect to redisServer fail" << endl;
//		return 1;
//	}
//	cout << "Connect to redisServer Success" << endl;
//	redisReply* r = (redisReply*)redisCommand(c, "select 1");
//	cout << r->type << endl;
//	cout << r->str << endl;
//
//	r = (redisReply*)redisCommand(c, "set ccc ccc");
//	cout << r->type << endl;
//	cout << r->str << endl;
//	freeReplyObject(r);
//
//	r = (redisReply*)redisCommand(c, "get ccc");
//	cout << r->type << endl;
//	cout << r->str << endl;
//	freeReplyObject(r);
//	return 0;
//}

const char* unRegZone = \
"\
	--[[KEYS[1] zone --]]\
		redis.call('del', 'zone:'..KEYS[1]);\
		redis.call('zrem', 'zones', KEYS[1]);\
		redis.log(redis.LOG_NOTICE, 'unregister zone.', 'zone:'..KEYS[1]);\
		return 1;";

const char* testLuaTable = \
"\
	--[[KEYS[1] zone --]]\
			local t={}\
			table.insert(t,{2,'a',1})\
			table.insert(t,{'b',2})\
	return t;";


const char* testLuaRank = \
"\
	--[[KEYS[1] zone --]]\
				redis.call('hmset','ranktest:1', 'name','zhou1', 'level', 1);\
				redis.call('hmset','ranktest:2', 'name','zhou2', 'level', 2);\
				redis.call('hmset','ranktest:3', 'name','zhou3', 'level', 3);\
				redis.call('hmset','ranktest:4', 'name','zhou4', 'level', 4);\
				redis.call('zadd', 'pvprank', 100, 1);\
				redis.call('zadd', 'pvprank', 100, 2);\
				redis.call('zadd', 'pvprank', 100, 3);\
				redis.call('zadd', 'pvprank', 100, 4);\
				local t = {}\
				local rank = redis.call('zrevrange', 'pvprank', KEYS[1], KEYS[2], 'withscores');\
			for i=1,#(rank),2 do\
				redis.log(redis.LOG_NOTICE, i, ' ' ,rank[i],' ', rank[i+1]);\
				local tmp=redis.call('hmget','ranktest:'..rank[i], 'name', 'level')\
				for k,v in pairs(tmp) do\
					redis.log(redis.LOG_NOTICE, i, k,v);\
				end\
				table.insert(tmp, 1, rank[i])\
				table.insert(tmp, 2, rank[i+1])\
				table.insert(t,tmp)\
			end\
	return t;";


/*
const char* testLuaTable = \
"\
	--[[KEYS[1] zone --]]\
redis.call('hmset','ranktest:1', 'name','zhou1', 'level', 1);\
redis.call('zadd', 'pvprank', 100, 1);\
local t = {}\
local rank = redis.call('zrevrange', 'pvprank', 0, -1, 'withscores');\
table.insert(t,{2,'a',1})\
table.insert(t,{'b',2})\
return t;";
*/
LuaScriptMgr::~LuaScriptMgr()
{

}

LuaScriptMgr& LuaScriptMgr::getInstance()
{
	static LuaScriptMgr inst;
	return inst;
}

bool LuaScriptMgr::init(redisContext* rc)
{
	if (nullptr == rc)
	{
		std::cout << " rc == null";
		return false;
	}

#define LOAD_SCRIPT(script){\
	string scriptName(#script);\
	loadScript(rc, scriptName, script);}
	
	LOAD_SCRIPT(unRegZone);
	LOAD_SCRIPT(testLuaTable);
	LOAD_SCRIPT(testLuaRank);

#undef LOAD_SCRIPT

	return true;
}


bool LuaScriptMgr::test(redisContext* rc)
{
	if (nullptr == rc)
	{
		cout << " rc == null";
		return false;
	}

	testUnRegZone(rc);
	testTable(rc);
	testRank(rc);

	return true;
}

void LuaScriptMgr::testRank(redisContext* rc)
{
	std::cout << "+++++++++++++++" << __FUNCTION__ << "++++++++++++++++++" << endl;
	redisReply* r = (redisReply*)redisCommand(rc, "evalsha %s 2 %d %d", script2Sha["testLuaRank"].c_str(), 0, -1);
	if (nullptr == r)
	{
		std::cout << __FUNCTION__ << " test error." << endl;
		return;
	}
	if (REDIS_REPLY_ERROR == r->type)
	{
		cout << r->str << endl;
		return;
	}
	if (REDIS_REPLY_ARRAY != r->type)
	{
		cout << r->type << endl;
		return;
	}

	for (int index = 0; index < r->elements; ++index)
	{
		cout << index << ":" << endl;
		redisReply* subReply = r->element[index];
		for (int subIndex = 0; subIndex < subReply->elements; subIndex++)
		{
			redisReply* elem = subReply->element[subIndex];
			if (REDIS_REPLY_STRING == elem->type)
				cout << "	" << subIndex << ":" << elem->type << " " << elem->str << endl;
			else if (REDIS_REPLY_INTEGER == elem->type)
				cout << "	" << subIndex << ":" << elem->type << " " << elem->integer << endl;
		}
	}

	freeReplyObject(r);
	std::cout << "+++++++++++++++" << __FUNCTION__ << "++++++++++++++++++" << endl;
}

void LuaScriptMgr::testUnRegZone(redisContext* rc)
{
	std::cout << "+++++++++++++++" << __FUNCTION__ << " start ++++++++++++++++++"<< endl;
	redisReply* r = (redisReply*)redisCommand(rc, "evalsha %s %u %u", script2Sha["unRegZone"].c_str(), 1, 100);
	if (nullptr == r)
	{
		std::cout << __FUNCTION__ << " test error." << endl;
		return;
	}

	cout << r->type << endl;
	//cout << r->str << endl;
	cout << r->integer << endl;

	freeReplyObject(r);
	std::cout << "+++++++++++++++" << __FUNCTION__ << " end ++++++++++++++++++"<< endl;
}


void LuaScriptMgr::testTable(redisContext* rc)
{
	std::cout << "+++++++++++++++" << __FUNCTION__ << "++++++++++++++++++"<< endl;
	redisReply* r = (redisReply*)redisCommand(rc, "evalsha %s 0", script2Sha["testLuaTable"].c_str());
	if (nullptr == r)
	{
		std::cout << __FUNCTION__ << " test error." << endl;
		return;
	}
	if (REDIS_REPLY_ERROR == r->type)
	{
		cout << r->str << endl;
		return;
	}
	if (REDIS_REPLY_ARRAY != r->type)
	{
		cout << r->type << endl;
		return;
	}

	for (int index = 0; index < r->elements; ++index)
	{
		cout << index << ":" << endl;
		redisReply* subReply = r->element[index];
		for (int subIndex = 0; subIndex < subReply->elements; subIndex++)
		{
			redisReply* elem = subReply->element[subIndex];
			if (REDIS_REPLY_STRING == elem->type)
				cout << "	" << subIndex << ":" << elem->type << " " << elem->str << endl;
			else if (REDIS_REPLY_INTEGER == elem->type)
				cout << "	" << subIndex << ":" << elem->type << " " << elem->integer<<endl;
		}
	}

	freeReplyObject(r);
	std::cout << "+++++++++++++++" << __FUNCTION__ << "++++++++++++++++++"<< endl;
}

LuaScriptMgr::LuaScriptMgr()
{

}

bool LuaScriptMgr::loadScript(redisContext* rc, string& scriptName, const char* script)
{
	if (nullptr == rc || nullptr == script)
		return false;

	if (scriptName.size() <= 0)
	{
		std::cout << "scriptName:size = " << scriptName.size();
		return false;
	}

	redisReply* r = (redisReply*)redisCommand(rc, "script load %s", script);
	if (nullptr == r)
	{
		std::cout << "load script error." << script << endl;;
		return false;
	}

	if (r->type != REDIS_REPLY_STRING)
	{
		std::cout << r->type << " error:" << r->str<< endl;
		freeReplyObject(r);
		return false;
	}

	script2Sha[scriptName] = r->str;
	cout << "script:" << scriptName.c_str() << " -> " << script2Sha[scriptName].c_str() << endl;;

	freeReplyObject(r);
	return true;
}
