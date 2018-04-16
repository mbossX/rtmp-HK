/*
* Copyright(C) 2010,Hikvision Digital Technology Co., Ltd 
* 
* File   name��GetStream.cpp
* Discription��
* Version    ��1.0
* Author     ��panyd
* Create Date��2010_3_25
* Modification History��
*/

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#endif

#include <stdio.h>
#include <iostream>
#include <fstream>
#include "camera.hpp"
#include "log.h"
using namespace std;
using namespace streamPusher;
int main(int argc, char **args)
{
    // int fr_send = 6;
    // int fr_fetch = 6;
    // int cache_second = 10;
    Config config;
    Camera *cameras = NULL;
    // read config json file
    /*
    {
        "cameras": [
            {
                "id": "C001",
                "ip": "172.16.1.205",
                "port": 12202,
                "user": "mboss",
                "password": "mjl04140906",
                "framerate": 6,
                "rtmp":{
                    "url": "rtmp://172.16.1.90/live/C001",
                    "framerate": 6
                }
            }
        ]
    }
    */
	cout << "place select function:" << "\n\t0  start push" << "\n\t1  get channels" << endl;
    int input;
	cin >> input;
	if (input == 1) {
		string ip;
		cout << "input ip:" << endl;
		cin >> ip;
		int port;
		cout << "input port:" << endl;
		cin >> port;
		string user;
		cout << "input user:" << endl;
		cin >> user;
		string pwd;
		cout << "input password:" << endl;
		cin >> pwd;
		Fetcher::GetChannel(ip.c_str(), port, user.c_str(), pwd.c_str());
	}

#ifdef _WIN32
	HWND hwnd;
	hwnd = FindWindow(L"ConsoleWindowClass", NULL);
	if (hwnd)
	{
		ShowOwnedPopups(hwnd, SW_HIDE);
		ShowWindow(hwnd, SW_HIDE);
	}
#endif

	Log::start();
    json cJson;
    if (argc > 1)
    {
        ifstream infile;
        infile.open(args[1]);
        if (!infile.is_open())
        {
            cout << "***config file " << args[1] << " did not exists!" << endl;
            return 1;
        }
        string data((istreambuf_iterator<char>(infile)),
                    istreambuf_iterator<char>());

        // cout << "***config is" << endl
        //      << data << endl;
        infile.close();
        using json = nlohmann::json;
        cJson = json::parse(data);
        config.from_json(cJson);
        // cout << config.cameras[0].rtmp[0].id_ << endl;

        cameras = new Camera[config.length];
        for (int i = 0; i < config.length; i++)
        {
            cameras[i] = Camera(&config.cameras[i]);
            if (cameras[i].start() != HPR_OK)
            {
                cout << "***camera " << config.cameras[i].id_ << " start fail!" << endl;
            }
            else
            {
                cout << "***start camera " << config.cameras[i].id_ << " success!" << endl;
            }
        }
    }
    else
    {
        cout << "***need config file!" << endl;
        goto cleanup;
    }

    while(true){
#ifdef _WIN32
		Sleep(5 * 1000);
#elif defined(__linux__) || defined(__APPLE__)
		usleep(5 * 1000 * 1000);
#endif
		for (int i = 0; i < config.length; i++)
		{
			if (cameras[i].inited) {
				cameras[i].pingpong();
			}
		}
        /*cin >> input;
        if(input == 'q'){
            break;
        }*/
    }

    // #ifdef _WIN32
    //     system("pause");
    // #elif defined(__linux__) || defined(__APPLE__)
    //     while (1)
    //     {
    //         pause();
    //     }
    // #endif

cleanup:
    if (cameras != NULL)
    {
        /*for (int i = 0; i < config.length; i++)
        {
            cameras[i].Cleanup();
        }*/
        delete[] cameras;
    }
	Log::close();
    // fetcher.Cleanup();
    // pusher.stop();
    return 0;
}