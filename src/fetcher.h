/*
* Copyright(C) 2010,Hikvision Digital Technology Co., Ltd 
* 
* File   name��GetStream.h
* Discription��
* Version    ��1.0
* Author     ��panyd
* Create Date��2010_3_25
* Modification History��
*/
#ifndef _NET_DVR_STREAM_H_
#define _NET_DVR_STREAM_H_

#include "cache.h"

class Fetcher
{
public:
  Fetcher() { this->lUserID = -1; };
  Fetcher(char *ip, int port, char *user, char *pwd, Cache *cache, Link *link);
  ~Fetcher();
  void Cleanup();
  int Init();
  void RealDataCallBack_V40(NET_DVR_PACKET_INFO_EX *pack);
  void RealDataCallBack_V402(DWORD dwDataType, BYTE *pBuffer, DWORD dwBufferSize);
  void exceptionCallBack(DWORD dwType);

private:
  long lRealPlayHandle;
  LONG lUserID;
  char *ip;
  char *user;
  char *pwd;
  int port;
  Cache *cache_;
  Link *link_;

  unsigned long lastTime;
  float avgTime;
  int times;

  int GetStream_V40();
  void getAvgTime();
};
#endif
