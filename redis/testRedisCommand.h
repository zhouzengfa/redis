#pragma once

#include "../hiredis/hiredis.h"
#include <iostream>
#include <unordered_map>

using namespace std;

class TestRedisCommand
{
public:
	TestRedisCommand();
	~TestRedisCommand();
	static TestRedisCommand& getInstance();
	bool init(redisContext* rc);

	bool test(redisContext* rc);

private:
	void test_redisCommandArgv(redisContext* rc);


};