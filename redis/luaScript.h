#pragma once

#include "../hiredis/hiredis.h"
#include <iostream>
#include <unordered_map>

using namespace std;

class LuaScriptMgr
{
public:
	~LuaScriptMgr();
	static LuaScriptMgr& getInstance();
	bool init(redisContext* rc);

	bool test(redisContext* rc);

private:
	void testUnRegZone(redisContext* rc);
	void testTable(redisContext* rc);
	void testRank(redisContext* rc);
	void testGetPhotList(redisContext* rc);

private:
	LuaScriptMgr();
	bool loadScript(redisContext* rc, string& scriptName, const char* script);

private:
	//redisContext* rc;
	std::unordered_map<string, string> script2Sha;
};