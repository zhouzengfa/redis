
#include "testRedisCommand.h"
#include "../hiredis/hiredis.h"
#include <iostream>
#include <memory>
#include <cstring>
#include <vector>

using namespace std;


TestRedisCommand::TestRedisCommand()
{

}

TestRedisCommand::~TestRedisCommand()
{

}

TestRedisCommand& TestRedisCommand::getInstance()
{
	static TestRedisCommand inst;
	return inst;
}

bool TestRedisCommand::init(redisContext* rc)
{
	if (nullptr == rc)
	{
		std::cout << " rc == null";
		return false;
	}

	return true;
}

bool TestRedisCommand::test(redisContext* rc)
{
	if (nullptr == rc)
	{
		cout << " rc == null";
		return false;
	}

	test_redisCommandArgv(rc);

	return true;
}

void mset(redisContext *c, const vector<string> &vtKey, const vector<string> & vtVal)
{
	if (vtKey.size() != vtVal.size())
	{
		throw runtime_error("Redis error");
	}

	vector<const char *> argv(vtKey.size() + vtVal.size() + 2);
	vector<size_t> argvlen(vtKey.size() + vtVal.size() + 2);
	int j = 0;

	static char msetcmd[] = "HMSET";
	argv[j] = msetcmd;
	argvlen[j] = sizeof(msetcmd) - 1;
	++j;

	static char msetcmd1[] = "testCommandArgv";
	argv[j] = msetcmd1;
	argvlen[j] = sizeof(msetcmd1) - 1;
	++j;

	for (int i = 0; i < vtKey.size(); ++i)
	{
		argvlen[j] = vtKey[i].length();
		argv[j] = vtKey[i].c_str();
		j++;

		argvlen[j] = vtVal[i].length();
		argv[j] = vtVal[i].data();
		j++;
	}

	void *r = redisCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]));
	if (c->err)
	{
		std::cout << "redisCommandArgv error.";
	}

	if (!r)
		throw runtime_error("Redis error");

	freeReplyObject(r);
}

void TestRedisCommand::test_redisCommandArgv(redisContext* rc)
{
	vector<string> keys, values;
	keys.push_back("key1");
	keys.push_back("key2");

	values.push_back("value1");
	values.push_back("value2");

	mset(rc, keys, values);


	string test = "abc";
}

