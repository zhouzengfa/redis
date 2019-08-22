
#include "luaScript.h"
#include "../hiredis/hiredis.h"
#include "json.h"
#include <iostream>
#include <memory>
#include <cstring>

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
#if _NOT_USE_JSON
const char* testGetPhoto = \
"\
\
--[[KEYS[1] start pos\
--KEYS[2] num\
--KEYS[3] 照片表名\
--KEYS[4] 已看过的照片表名\
--]]\
\
local index = tonumber(KEYS[1]) \
local num = tonumber(KEYS[2]) \
local table_name = KEYS[3]; \
local totalNum = redis.call('zcard', table_name) \
local t = {} \
local newPosition = index \
redis.log(redis.LOG_NOTICE, 'test', index, totalNum, table_name); \
for index = tonumber(KEYS[1]), totalNum, 10 do \
	local photolist = redis.call('zrange', table_name, index, index + 10, 'withscores'); \
	redis.log(redis.LOG_NOTICE, #(photolist), 'index', index) \
	for i = 1, #(photolist), 2 do \
		redis.log(redis.LOG_NOTICE, i, ' ', photolist[i], ' ', photolist[i + 1]); \
		if (redis.call('exists', KEYS[4]) == 0 or redis.call('getbit', KEYS[4], tonumber(photolist[i + 1])) == 0) \
		then \
			redis.log(redis.LOG_NOTICE, 'insert', photolist[i + 1], ' ', photolist[i]); \
			local tmp = {} \
			table.insert(tmp, 1, photolist[i]) \
			table.insert(tmp, 2, photolist[i + 1]) \
			table.insert(t, tmp) \
			if (table.getn(t) >= num) \
			then \
				return{ newPosition, t } \
			end \
		end \
	end \
	newPosition = newPosition + 10 \
end \
if (newPosition > totalNum) \
then \
	newPosition = totalNum \
end \
return{ newPosition, t }";
#else
const char* testGetPhoto = \
"\
\
--[[KEYS[1] start pos\
--KEYS[2] num\
--KEYS[3] 照片表名\
--KEYS[4] 已看过的照片表名\
--]]\
\
local index = tonumber(KEYS[1]) \
local num = tonumber(KEYS[2]) \
local table_name = KEYS[3]; \
local totalNum = redis.call('zcard', table_name) \
local t = {} \
local newPosition = index \
redis.log(redis.LOG_NOTICE, 'test', index, totalNum, table_name); \
for index = tonumber(KEYS[1]), totalNum, 10 do \
	local photolist = redis.call('zrange', table_name, index, index + 10, 'withscores'); \
	redis.log(redis.LOG_NOTICE, #(photolist), 'index', index) \
	for i = 1, #(photolist), 2 do \
		redis.log(redis.LOG_NOTICE, i, ' ', photolist[i], ' ', photolist[i + 1]); \
		if (redis.call('exists', KEYS[4]) == 0 or redis.call('getbit', KEYS[4], tonumber(photolist[i + 1])) == 0) \
		then \
			redis.log(redis.LOG_NOTICE, 'insert', photolist[i + 1], ' ', photolist[i]); \
			local tmp = {} \
			table.insert(tmp, 1, photolist[i]) \
			table.insert(tmp, 2, photolist[i + 1]) \
			table.insert(t, tmp) \
			if (table.getn(t) >= num) \
			then \
				local ret={} \
				ret[\"index\"]=newPosition \
				ret[\"data\"] = t \
				return cjson.encode(ret) \
			end \
		end \
	end \
	newPosition = newPosition + 10 \
end \
if (newPosition > totalNum) \
then \
	newPosition = totalNum \
end \
local ret={} \
ret[\"index\"]=newPosition \
ret[\"data\"] = t \
return cjson.encode(ret) ";

#endif


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
	LOAD_SCRIPT(testGetPhoto);

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
	testGetPhotList(rc);

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

#if _NOT_USE_JSON
void LuaScriptMgr::testGetPhotList(redisContext* rc)
{
	std::cout << "+++++++++++++++" << __FUNCTION__ << " start ++++++++++++++++++" << endl;
	char photo[] = "photo";
	char testbit[] = "testbit";
	redisReply* r = (redisReply*)redisCommand(rc, "evalsha %s %d %d %d %s %s", script2Sha["testGetPhoto"].c_str(), 4, 1, 100, "photo", "testbit");
	//redisReply* r = (redisReply*)redisCommand(rc, "evalsha 1f579a6c4f04c78fd00dc4ed72361635b824ec05 4 1 100 photo testbit");
	if (nullptr == r)
	{
		std::cout << __FUNCTION__ << " test error." << endl;
		return;
	}

	cout << "type:" << r->type << endl;
	//cout << "str:" << r->str << endl;
	if (REDIS_REPLY_ERROR == r->type)
	{
		cout << r->str << endl;
		return;
	}
	if (REDIS_REPLY_ARRAY != r->type)
	{
		cout << " type isn't array." << r->type;
		return;
	}

	if (2 != r->elements)
	{
		cout << " ele num != 2 " << r->elements;
		return;
	}

	if (REDIS_REPLY_INTEGER != r->element[0]->type)
	{
		cout << "ele[0] type isn't int" << r->element[0]->type;
		return;
	}

	int index = r->element[0]->integer;
	cout << "index:" << index << endl;
	auto & ele1 = r->element[1];
	if (ele1->type != REDIS_REPLY_ARRAY)
	{
		cout << "ele1 is not array." << ele1->type;
		return;
	}

	for (int i = 0; i < ele1->elements; )
	{
		auto tm1 = ele1->element[i]->str;
		auto tm2 = ele1->element[i+1]->str;
		cout << "tmp1:" << tm1 << " tmp2:" << tm2;
		i += 2;
	}
	//cout << r->str << endl;
	//cout << r->integer << endl;

	freeReplyObject(r);
	std::cout << "+++++++++++++++" << __FUNCTION__ << " end ++++++++++++++++++" << endl;
}
#else
void LuaScriptMgr::testGetPhotList(redisContext* rc)
{
	std::cout << "+++++++++++++++" << __FUNCTION__ << " start ++++++++++++++++++" << endl;

	{
		redisReply* r = (redisReply*)redisCommand(rc, "setbit testbit 2 1");
		freeReplyObject(r);
	}
	redisReply* r = (redisReply*)redisCommand(rc, "evalsha %s %d %d %d %s %s", script2Sha["testGetPhoto"].c_str(), 4, 1, 100, "photo", "testbit");
	if (nullptr == r)
	{
		std::cout << __FUNCTION__ << " test error." << endl;
		return;
	}

	cout << "type:" << r->type << endl;
	if (REDIS_REPLY_STRING != r->type)
	{
		cout << r->str << endl;
		return;
	}
	cout << "str:" << r->str << endl;

	Json::Value root;
	JSONCPP_STRING errs;
	Json::CharReaderBuilder readerBuilder;
	std::unique_ptr<Json::CharReader> const reader(readerBuilder.newCharReader());
	if (!reader->parse(r->str, r->str + strlen(r->str), &root, &errs) || !errs.empty())
	{
		cout << "parse error." << errs << endl;
		return;
	}

	if (root["index"].isNull())
	{
		cout << "not find index field." << endl;
		return;
	}

	cout << "index:" << root["index"].asInt() << endl;

	if (root["data"].isNull())
	{
		cout << "not find data field." << endl;
		return;
	}

	Json::Value& data = root["data"];
	if (!data.isArray())
	{
		cout << "data is not array." << endl;
		return;
	}

	for (auto item : data)
	{
		cout << item[0] << "," << item[1] <<endl;
	}
	//cout << "index:" << root["data"].

	freeReplyObject(r);
	std::cout << "+++++++++++++++" << __FUNCTION__ << " end ++++++++++++++++++" << endl;
}
#endif
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
