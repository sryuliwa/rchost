#pragma once
#include "stdafx.h"
#include "rc_common.h"
#include <assert.h>

#if 0
void writeArgs(_MSG* msg, const char* arg)
{

	char* temp = const_cast<char*>(arg);
	int curSize = strlen(temp)+1;
	assert(msg->_pBuf+_size+ curSize != NULL);
	memcpy(msg->_pBuf +msg->_size, temp, curSize*sizeof(char));
	msg->_size += curSize;
	msg->_vecSize.push_back(msg->_size);

}
void readArgs(_MSG* msg, int i,char* arg)
{
	assert(i <msg-> _vecSize.size());
	int curSize = msg->_vecSize[i + 1] - msg->_vecSize[i];
	assert(arg + curSize != NULL);
	memcpy(arg, msg->_pBuf + msg->_vecSize[i], msg->_vecSize[i + 1] -msg->_vecSize[i]);

}
void clear(_MSG* msg)
{
	memset(msg->_pBuf,0,msg->_size);
	msg->_vecSize.clear();
	msg->_size = 0;
}
#endif

void writeArgs(_MSG* msg, const char* arg)
{
	assert(msg != NULL);
	char* temp = const_cast<char*>(arg);
	int curSize = strlen(temp) + 1;
	assert(msg->_pBuf + msg->_size + curSize != NULL);
	memcpy(msg->_pBuf + msg->_size, temp, curSize*sizeof(char));
	msg->_size += curSize;
	msg->_pSize[++(msg->_argc)] = msg->_size;

}
void readArgs(_MSG* msg, int i, char* arg)
{
	assert(msg != NULL);
	assert(i < msg->_argc);
	int curSize = msg->_pSize[i + 1] - msg->_pSize[i];
	assert(arg + curSize != NULL);
	memcpy(arg, msg->_pBuf + msg->_pSize[i], msg->_pSize[i + 1] - msg->_pSize[i]);

}
void clear(_MSG* msg)
{
	assert(msg != NULL);
	memset(msg->_pBuf, 0, msg->_size);
	memset(msg->_pSize, 0, msg->_argc);
	msg->_size = 0;
	msg->_argc = 0;
}
void parseMsg(char* inMsg,_MSG* outMsg)
{
	char* delimeter = "#";
	char* pd= strstr(inMsg, delimeter);
	if (pd) 
	{
		*pd = '\0';
		sprintf(outMsg->_filename, inMsg);
		if (strcmp(pd + 1, "on") == 0)
			outMsg->_operation = _OPEN;
		else if (strcmp(pd + 1, "off") == 0)
			outMsg->_operation = _CLOSE;
		else if (strcmp(pd + 1, "pause") == 0)
			outMsg->_operation = _PLAY_PAUSE;
		else {
			outMsg->_operation = _OPEN;
			__STD_PRINT("illegal input: %s\n", pd + 1);
		}
	}
}
void parseMsg(char* inMsg,_MSG& outMsg)
{
	char* delimeter = "#";
	char* pd= strstr(inMsg, delimeter);
	if (pd) 
	{
		*pd = '\0';
		sprintf(outMsg._filename, inMsg);
		if (strcmp(pd + 1, "on") == 0)
			outMsg._operation = _OPEN;
		else if (strcmp(pd + 1, "off") == 0)
			outMsg._operation = _CLOSE;
		else if (strcmp(pd + 1, "pause") == 0)
			outMsg._operation = _PLAY_PAUSE;
		else {
			outMsg._operation = _OPEN;
			__STD_PRINT("illegal input: %s\n", pd + 1);
		}
	}
}
