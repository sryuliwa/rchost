// rchost.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "rc_common.h"
#include "host.h"
int main(int argc, char* argv[])
{
	std::cout
		<< "Usage: " << std::endl
		<< "host dWindowFlag port(optional) " << std::endl;
	if (argc < 2)
	{
		std::cout << "e.g:\n host 0 or host 0 6000" << std::endl;
		return 0;
	}
	int port = 20715;
	int quiet = atoi(argv[1]);
	if (quiet)
		ShowWindow(GetConsoleWindow(), SW_HIDE);
	if (argc > 2)
		port = atoi(argv[2]);

	HostOperator::instance()->loadConfig("control.ini");
	std::unique_ptr<host> host(new host(port));
	host->start();
	host->join();
	host.release();
	return 0;
}

