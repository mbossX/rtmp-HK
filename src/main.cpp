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
    json cJson;
    if (argc > 1)
    {
        ifstream infile;
        infile.open(args[1]);
        if (!infile.is_open())
        {
            cout << "config file " << args[1] << " did not exists!" << endl;
            return 1;
        }
        string data((istreambuf_iterator<char>(infile)),
                    istreambuf_iterator<char>());

        // cout << "config is" << endl
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
                cout << "camera " << config.cameras[i].id_ << " start fail!" << endl;
            }
            else
            {
                cout << "start camera " << config.cameras[i].id_ << " success!" << endl;
            }
        }
    }
    else
    {
        cout << "need config file!" << endl;
        goto cleanup;
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
    // fetcher.Cleanup();
    // pusher.stop();
    return 0;
}