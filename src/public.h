/*
 * Copyright(C) 2010,Hikvision Digital Technology Co., Ltd 
 * 
 * �ļ����ƣ�public.h
 * ��    ����
 * ��ǰ�汾��1.0
 * ��    �ߣ����Ƕ�
 * �������ڣ�2010��3��25��
 * �޸ļ�¼��
 */

#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#define HPR_OK 0
#define HPR_ERROR -1

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/time.h>
#endif

#include <iostream>
#include <queue>
#include <time.h>
#include <string.h>
using namespace std;

#include "../include/HCNetSDK.h"
namespace streamPusher
{
class VideoBuffer
{
    public:
    char *buffer;
    int size;
    int timestamp;
    int framerate;
    float avgTime;
    int frameType;
    VideoBuffer(char* b, int s, int t, int f, float avgt){
        size = s;
        timestamp = t;
        framerate = f;
        avgTime = avgt;
        buffer = new char[size];
        memcpy(buffer, b, size);
        frameType = 0;
    };
    void Free(){
        if(buffer != NULL){
            delete[] buffer;
        }
    }
};

class Link
{
  public:
    bool stateDVR;
    bool stateRTMP;
    float avgFetchTime;
    int frDVR;
    int frRTMP;
    string id_;
    Link()
    {
        this->stateDVR = true;
        this->stateRTMP = true;
        this->avgFetchTime = 0.0;
        this->frDVR = 25;
        this->frRTMP = 6;
    }
};
}
#endif
