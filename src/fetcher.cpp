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

#include "fetcher.h"
namespace streamPusher
{
#ifdef _WIN32
int gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
    return (0);
}
#endif
#ifndef TIMESTAMP
#define srs_max_(a, b) (((a) < (b)) ? (b) : (a))
#define SYS_TIME_RESOLUTION_US 300 * 1000
unsigned long _srs_system_time_us_cache__ = 0;
unsigned long _srs_system_time_startup_time__ = 0;
unsigned long timestamp()
{
    timeval now;

    if (gettimeofday(&now, NULL) < 0)
    {
        return -1;
    }
    unsigned long now_us = ((unsigned long)now.tv_sec) * 1000 * 1000 + (unsigned long)now.tv_usec;

    if (_srs_system_time_us_cache__ <= 0)
    {
        _srs_system_time_startup_time__ = _srs_system_time_us_cache__ = now_us;
        return _srs_system_time_us_cache__ / 1000;
    }

    // use relative time.
    unsigned long diff = now_us - _srs_system_time_us_cache__;
    diff = srs_max_(0, diff);
    if (diff < 0 || diff > 1000 * SYS_TIME_RESOLUTION_US)
    {
        _srs_system_time_startup_time__ += diff;
    }

    _srs_system_time_us_cache__ = now_us;

    return _srs_system_time_us_cache__ / 1000;
}
#define TIMESTAMP timestamp()
#endif

// callback and recallback
void CALLBACK g_RealplayCallback(LONG lRealHandle, NET_DVR_PACKET_INFO_EX *pack, void *dwUser)
{
    if (dwUser != NULL)
    {
        ((Fetcher *)dwUser)->RealDataCallBack_V40(pack);
    }
}

void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    if (pUser != NULL)
    {
        ((Fetcher *)pUser)->exceptionCallBack(dwType);
    }
}

int Fetcher::Init()
{
    this->Cleanup();
    NET_DVR_Init();
    NET_DVR_SetConnectTime(2000, 2);
    NET_DVR_SetReconnect(10000, true);
    NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);
    //Login device
    NET_DVR_DEVICEINFO_V40 struDeviceInfo = {0};
    NET_DVR_USER_LOGIN_INFO info = {0};
    info.bUseAsynLogin = 0;
    strcpy(info.sDeviceAddress, this->ip);
    info.wPort = this->port;
    strcpy(info.sUserName, this->user);
    strcpy(info.sPassword, this->pwd);

    this->lUserID = NET_DVR_Login_V40(&info, &struDeviceInfo);
    if (this->lUserID < 0)
    {
        cout << "pyd---Login error " << NET_DVR_GetLastError() << endl;
        return NET_DVR_GetLastError();
    }

    if (this->GetStream_V40() != HPR_OK)
    {
        cout << "pyd---GetStream error " << NET_DVR_GetLastError() << endl;
        return NET_DVR_GetLastError();
    }
    return HPR_OK;
}

void Fetcher::RealDataCallBack_V40(NET_DVR_PACKET_INFO_EX *pack)
{
    this->getAvgTime();
    switch (pack->dwPacketType)
    {
    case 0: // head
        //cout << "head" << endl;
        break;
    case 2: // B
        //cout << "B Frame" << endl;
        // break;
    case 1: // I
        // cout << "I Frame" << endl;
        // for (int i = 0; i < 30; i++)
        // {
        //     cout << (int)pack->pPacketBuffer[i];
        // }
        // cout << endl;
    case 3: // P
        // cout << "P Frame" << endl;
        VideoBuffer vb((char *)pack->pPacketBuffer, pack->dwPacketSize, pack->dwTimeStamp, pack->dwFrameRate, this->avgTime);
        vb.frameType = pack->dwPacketType;
        this->cache_->push(vb);

        this->link_->frDVR = pack->dwFrameRate;
        this->link_->avgFetchTime = this->avgTime;
        this->link_->stateDVR = true;
        break;
    }
}

/*******************************************************************
      Function:   Demo_GetStream_V30
   Description:   preview(_V30)
     Parameter:   (IN)   none  
        Return:   0--successful��-1--fail��   
**********************************************************************/
#pragma region GetStream_V40
int Fetcher::GetStream_V40()
{
    NET_DVR_PREVIEWINFO preInfo = {0};
#if (defined(_WIN32) || defined(_WIN_WCE)) || defined(__APPLE__)
    preInfo.hPlayWnd = NULL;
#elif defined(__linux__)
    preInfo.hPlayWnd = 0;
#endif
    preInfo.lChannel = this->channel;
    preInfo.dwStreamType = 0;
    preInfo.dwLinkMode = 0;
    preInfo.bBlocked = 1;

    this->lRealPlayHandle = NET_DVR_RealPlay_V40(this->lUserID, &preInfo, NULL, NULL);
    if (this->lRealPlayHandle < 0)
    {
        return HPR_ERROR;
    }

    //Set H264 stream callback function
    int ret = NET_DVR_SetESRealPlayCallBack(this->lRealPlayHandle, g_RealplayCallback, this);
    if (ret < 0)
    {
        cout << "set stream callback error " << ret << endl;
        return HPR_ERROR;
    }

    return HPR_OK;
}
#pragma endregion

void Fetcher::exceptionCallBack(DWORD dwType)
{
    switch (dwType)
    {
    case EXCEPTION_RECONNECT: //reconnect
        cout << "pyd----------reconnect-------- " << time(NULL) << endl;
        this->link_->stateDVR = false;
        this->Init();
        break;
    default:
        break;
    }
}

Fetcher::Fetcher()
{
    this->lUserID = -1;
    this->link_ = NULL;
}

Fetcher::Fetcher(int channel, const char *ip, int port, const char *user, const char *pwd, Cache *cache, Link *link)
{
	this->channel = channel;
    this->ip = ip;
    this->user = user;
    this->pwd = pwd;
    this->port = port;
    this->cache_ = cache;
    this->link_ = link;
    this->lastTime = 0;
    this->times = 0;
}

Fetcher::~Fetcher()
{
    this->Cleanup();
}

void Fetcher::Cleanup()
{
    if (this->lUserID >= 0)
    {
        NET_DVR_StopRealPlay(this->lRealPlayHandle);
        NET_DVR_Logout_V30(this->lUserID);
        NET_DVR_Cleanup();
    }
    this->lUserID = -1;
    if (this->link_ != NULL)
    {
        this->link_->stateDVR = false;
    }
}

void Fetcher::getAvgTime()
{
    unsigned long now = TIMESTAMP;
    if (this->times > 2)
    {
        this->avgTime = (this->avgTime + (float)(now - this->lastTime) / (float)(this->times - 1)) * ((float)(this->times - 1) / (float)this->times);
    }
    else if (this->times == 2)
    {
        this->avgTime = (float)(now - this->lastTime);
    }
    this->lastTime = now;
    this->times++;
    if (this->times > 25)
    {
        this->times = 5;
    }
}
}