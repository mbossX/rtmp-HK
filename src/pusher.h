#ifndef _PUSHER_H_
#define _PUSHER_H_

#include <pthread.h>
#include "cache.h"
#include "srs/srs_librtmp.h"

using namespace std;

class Pusher
{
public:
  Pusher()
  {
    this->tid = -1;
  };
  Pusher(char *url, int fr, Cache *cache, Link *link);
  ~Pusher();
  int start();
  void stop();
  void *tCallback();

private:
  char *url_;
  int fr_; // frame rate
  srs_rtmp_t rtmp;
  Cache *cache_;
  Link *link_;
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

#endif