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
namespace streamPusher
{
class Fetcher
{
public:
  Fetcher();
  Fetcher(int channel, const char *ip, int port, const char *user, const char *pwd, Cache *cache, Link *link);
  ~Fetcher();
  void Cleanup();
  int Init();
  void RealDataCallBack_V40(NET_DVR_PACKET_INFO_EX *pack);
  void exceptionCallBack(DWORD dwType);

private:
  static bool inited;
  long lRealPlayHandle;
  LONG lUserID;
  int channel;
  const char *ip;
  const char *user;
  const char *pwd;
  int port;
  Cache *cache_;
  Link *link_;

  unsigned long lastTime;
  float avgTime;
  int times;

  int GetStream_V40();
  void getAvgTime();
};
}
#endif
