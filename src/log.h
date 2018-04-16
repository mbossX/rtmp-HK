#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;

namespace streamPusher
{
	class Log {
	public:
		static ofstream logfile;
		static void start() {
			if (!Log::logfile.is_open()) {
				Log::logfile.open("./log.txt");
				if (!Log::logfile.is_open())
				{
					cout << "***log file open failed!" << endl;
					return;
				}
			}
			cout.rdbuf(Log::logfile.rdbuf());
		}
		static void close() {
			if (Log::logfile.is_open()) {
				Log::logfile.close();
			}
		}
	};
}
#endif