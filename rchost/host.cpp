#include "stdafx.h"
#include "host.h"
#include <iostream>
#include <string>
#include <atlstr.h> 
#include <sstream>
#include <iterator>
#include <vector>

void wcharTochar(const wchar_t *wchar, char *chr, int length)
{
	WideCharToMultiByte(CP_ACP, 0, wchar, -1, chr, length, NULL, NULL);
}

HOST_API::HOST_API(int port) :
OpenThreads::Thread()
{
	//Init the mutex with specified mutex type.
	initMutex(new OpenThreads::Mutex());
	//initMutex(new OpenThreads::Mutex(OpenThreads::Mutex::MUTEX_RECURSIVE));
	m_pServer = std::auto_ptr<server>(new server(port));
	loadPathMap();
#if 0 
	for (HOST_MAP_ITER iter = m_mapNamePath.begin(); iter != m_mapNamePath.end(); iter++)
		std::cout << iter->first << " : " << iter->second << std::endl;
	for (HOST_MAP_ITER iter = m_mapNameArgs.begin(); iter != m_mapNameArgs.end(); iter++)
		std::cout << iter->first << " : " << iter->second << std::endl;
#endif
	return;
}


HOST_API::~HOST_API()
{
}

void HOST_API::run()
{
	char buf[_MAX_DATA_SIZE];
	while (m_pServer->isSocketOpen() == true)
	{
		
		std::cout << "Waiting..." << std::endl;
		sockaddr client;
		int msgSize = -1;
		m_pServer->getPacket(client, buf, msgSize, _MAX_DATA_SIZE);
		char feedback[50];
		if (msgSize == sizeof(HOST_MSG))
		{
			memset(feedback, 0, 50);
			HOST_MSG* msg;
			msg = reinterpret_cast<HOST_MSG*>(buf);
			//handle controlling instructions here.
			DWORD error = handle(msg);
			if (error == ERROR_SUCCESS)
				sprintf(feedback, "Operation Success.\n");
			else
				sprintf(feedback, "Operation Failed.ErrorCode: %ld.\n",error);
			m_pServer->sendPacket(client, feedback, strlen(feedback), _MAX_DATA_SIZE);
		}

	}

}
DWORD HOST_API::handleProgram(std::string filename, const char op,bool isCurDirNeeded)
{
	switch (op)
	{
	case _OPEN:
		return createProgram(filename,isCurDirNeeded);
	case _CLOSE:
		return closeProgram(filename);
	default:
		return RPC_X_ENUM_VALUE_OUT_OF_RANGE;
	}

}
DWORD HOST_API::createProgram(std::string filename, std::string path, const char* curDir,std::string args, const int arg)
{
	std::cout << filename << " " << path << " " << args << std::endl;
	HOST_INFO_ITER iter;
	iter = m_vecProgInfo.find(filename);
	//Make sure no duplicated task is to be created.
	if (iter != m_vecProgInfo.end())
	{
		std::cout << "duplicated task is to pending." << std::endl;
		PROCESS_INFORMATION oldInfo = iter->second;
		DWORD dwExitCode = 0;
		GetExitCodeProcess(oldInfo.hProcess, &dwExitCode);
		std::cout << "status of forked process: " << dwExitCode << std::endl;
		if (dwExitCode == STILL_ACTIVE)
			return ERROR_SERVICE_EXISTS;
		else
		{
			CloseHandle(oldInfo.hProcess);
			CloseHandle(oldInfo.hThread);
			m_vecProgInfo.erase(iter);
		}
	}
	STARTUPINFO si = {};
	PROCESS_INFORMATION pi = {};

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	ZeroMemory(&pi, sizeof(pi));

	//Add process access attributes.
#if 0
	PSECURITY_DESCRIPTOR pSD = NULL;
	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);

	if (NULL == pSD)
	{
		return GetLastError();
	}
	if (!InitializeSecurityDescriptor(pSD,
		SECURITY_DESCRIPTOR_REVISION))
	{
		return GetLastError();
	}

	// Add the ACL to the security descriptor. 
	if (!SetSecurityDescriptorDacl(pSD,
		TRUE,     // bDaclPresent flag   
		pACL,
		FALSE))   // not a default DACL 
	{
		return GetLastError();
	}

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof (SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;
#endif

#ifdef UNICODE
	WCHAR* wPath = (WCHAR*)malloc(strlen(path));
	WCHAR* wArgs = (WCHAR*)malloc(strlen(args));

	MultiByteToWideChar(CP_UTF8, 0, path, strlen(path), wPath, strlen(path));
	MultiByteToWideChar(CP_UTF8, 0, args, strlen(args), wArgs, strlen(args));
	if (CreateProcess(_T("C://Windows//System32//calc.exe"), NULL, 0, 0, false, 0, 0, curDir, &si, &pi) != ERROR_SUCCESS)
	{
		return GetLastError();
	}
#else
	//if (!CreateProcess(path.c_str(), const_cast<char*>(args.c_str()), 0, 0, false, 0, 0, const_cast<char*>(curDir.c_str()), &si, &pi))
	if (!CreateProcess(path.c_str(), const_cast<char*>(args.c_str()), 0, 0, false, 0, 0,curDir, &si, &pi))
	{
		return GetLastError();
	}
	m_vecProgInfo[filename.c_str()] = pi;
#if 0
	//now verify the running processes.
	for (HOST_INFO_ITER iter = m_vecProgInfo.begin(); iter != m_vecProgInfo.end(); iter++)
		std::cout << iter->first << " " << iter->second.dwProcessId << std::endl;
#endif
#endif

	return ERROR_SUCCESS;
}
DWORD HOST_API::createProgram(std::string filename,bool isCurDirNeeded)
{

	std::string path;
	HOST_MAP_ITER iter = m_mapNamePath.find(filename);
	if (iter == m_mapNamePath.end())
		return ERROR_NOT_FOUND;
	else
		path = iter->second;
	iter = m_mapNameArgs.find(filename);
	char* curDir = NULL;
	if (isCurDirNeeded)
		curDir = parsePath(path.c_str());

	if (iter != m_mapNameArgs.end())
		return createProgram(filename, path, curDir,iter->second, 1);
	else
		return createProgram(filename, path,curDir,"",0);

}
DWORD HOST_API::closeProgram(std::string filename)
{
	HOST_INFO_ITER iter;
	iter = m_vecProgInfo.find(filename);
	if (iter == m_vecProgInfo.end())
	{
		return ERROR_NOT_FOUND;
	}

	PROCESS_INFORMATION pi = iter->second;

	if (!PostThreadMessage(pi.dwThreadId, WM_QUIT, 0, 0))
		return GetLastError();
	DWORD dwExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);
	if (dwExitCode == STILL_ACTIVE)
	{
		TerminateThread(pi.hThread, dwExitCode);
		TerminateProcess(pi.hProcess, dwExitCode);
	}
	//if (!PostThreadMessage(pi.dwThreadId, WM_KEYDOWN, VK_ESCAPE, 0))
	//	return GetLastError();
	//if (!PostThreadMessage(pi.dwThreadId, WM_KEYUP, VK_ESCAPE, 0))
	//	return GetLastError();
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	m_vecProgInfo.erase(iter);
	return ERROR_SUCCESS;

}

char* HOST_API::parsePath(const char* fullpath)
{
	
	//Parse the full path of the program to be created.
	//Here we suppose the input path is of the following style:
	//				c://path//to//program.exe
	//The current dir shuold be: 
	//				c://path//to
	char* p = const_cast<char*>(fullpath);
	char* start=p;
	char* sub;
	while (p!=NULL && (sub = strstr(p, "//")) != NULL)
		p = sub + 2;
	int sz=p-start-2;
	char* curDir=new char[sz];
	memcpy(curDir,start,sz);
	//std::cout<<curDir<<std::endl;
	return curDir;

}
DWORD HOST_API::loadPathMap()
{

	char strINIPath[MAX_PATH];
	_fullpath(strINIPath, "control.ini", MAX_PATH);
	if (GetFileAttributes(strINIPath) == 0xFFFFFFFF)
	{

		return ERROR_NOT_FOUND;
	}
	CHAR	attriStr[MAX_PATH];
	long hr;
	hr = GetPrivateProfileString("ProgramsList", "Names", "", attriStr, MAX_PATH, strINIPath);
	std::vector<std::string> vecName;
	if (hr)
	{
		std::istringstream buf(attriStr);
		std::istream_iterator<std::string> beg(buf), end;
		std::vector<std::string> tokens(beg, end);
		vecName.assign(tokens.begin(), tokens.end());
	}
	else
	{
		return ERROR_NOT_FOUND;
	}

	memset(attriStr, 0, MAX_PATH);
	for (std::vector<std::string>::iterator iter = vecName.begin(); iter != vecName.end(); iter++)
	{
		hr = GetPrivateProfileString("Path", iter->c_str(), "", attriStr, MAX_PATH, strINIPath);
		if (!hr)
			return ERROR_NOT_FOUND;
		m_mapNamePath[iter->c_str()] = std::string(attriStr);
		memset(attriStr, 0, MAX_PATH);
	}
	for (std::vector<std::string>::iterator iter = vecName.begin(); iter != vecName.end(); iter++)
	{
		hr = GetPrivateProfileString("Args", iter->c_str(), "", attriStr, MAX_PATH, strINIPath);
		if (hr)
		{
			m_mapNameArgs[iter->c_str()] = std::string(attriStr);
			memset(attriStr, 0, MAX_PATH);
		}
	}
	return 0;
}
int HOST_API::start()
{
	m_mutex->lock();
	return OpenThreads::Thread::start();
}
int HOST_API::startThread()
{
	m_mutex->lock();
	return OpenThreads::Thread::startThread();
}

int HOST_API::cancel()
{
	m_mutex->unlock();
	return OpenThreads::Thread::cancel();
}
void HOST_API::initMutex(OpenThreads::Mutex* mutex)
{
	m_mutex = std::auto_ptr<OpenThreads::Mutex>(mutex);
}
const OpenThreads::Mutex* HOST_API::getMutex() const
{
	return m_mutex.get();
}
void HOST_API::lock()
{
	m_mutex->lock();
}
void HOST_API::unlock()
{
	m_mutex->unlock();
}