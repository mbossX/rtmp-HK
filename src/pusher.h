#ifndef _PUSHER_H_
#define _PUSHER_H_
#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#endif
#include "cache.h"
#include "srs/rtmp.hpp"
#include "config.hpp"

using namespace std;
namespace streamPusher
{
class Pusher
{
public:
  Pusher();
  Pusher(Camera_ *c, Cache *cache, Link *link);
  ~Pusher();
  int start();
  void stop();
  void *tCallback();

private:
  Rtmp *rtmp;
  int length;
  Cache *cache_;
  Link *link_;
  const char *url_;
  int fr_; // frame rate
  const char* id_;
  bool running;
  double dt;
  unsigned long times;
#ifdef _WIN32
  HANDLE tid;
#elif defined(__linux__) || defined(__APPLE__)
  pthread_t tid;
#endif

  int init();
  int doSleep(float st);
  int send();
};
}
#endif