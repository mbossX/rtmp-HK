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
#include "cache.h"
#include "pusher.h"
#include "fetcher.h"
using namespace std;

const int fr_send = 6;
const int fr_fetch = 6;
const int cache_second = 10;

int main()
{
#ifdef W_IN32
	set_recc
#endif
    Link link;
    Cache cache(cache_second * fr_send, &link);

    Fetcher fetcher("172.16.1.205", 12202, "mboss", "mjl04140906", &cache, &link);
    Pusher pusher("rtmp://172.16.1.90/live/lt", fr_send, &cache, &link);

    int ret = fetcher.Init();
    if (ret != HPR_OK)
    {
        cout << "init fetcher error " << ret << endl;
        goto cleanup;
    }
    cout << "init dvr success" << endl;
#ifdef _WIN32
        Sleep(cache_second * ((float)fr_send / (float)fr_fetch) * 1000);
#elif defined(__linux__) || defined(__APPLE__)
    sleep(cache_second * ((float)fr_send / (float)fr_fetch));
#endif
    ret = pusher.start();

#ifdef _WIN32
        system("pause");
#elif defined(__linux__) || defined(__APPLE__)
    while (1)
    {
        pause();
    }
#endif

cleanup:
    fetcher.Cleanup();
    pusher.stop();
    return 0;
}
