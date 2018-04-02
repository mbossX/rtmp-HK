#ifndef _CACHE_H_
#define _CACHE_H_

#include "public.h"

class Cache
{
  public:
    Cache(int capacity, Link *link)
    {
        this->capacity = capacity;
        this->maxCapacity = 200; // queue max size is 256
        this->q_ = queue<VideoBuffer>();
        this->link_ = link;
        this->tmpTimer = 0;
    };
    ~Cache()
    {
        while (1)
        {
            if(this->empty()){
                break;
            }
            VideoBuffer t = this->pop();
            t.Free();
        }
    };
    void push(VideoBuffer item)
    {
        int size = this->q_.size();
        // full max size
        if (size > this->maxCapacity)
        {
            this->pop();
        }

        this->q_.push(item);
        // this->tmpTimer = 0;
    };
    VideoBuffer pop()
    {
        VideoBuffer t = this->q_.front();
        this->q_.pop();
        return t;
    };
    int size()
    {
        return this->q_.size();
    };
    bool empty()
    {
        return this->q_.empty();
    };

    int capacity;

  private:
    int maxCapacity;
    int tmpTimer;
    queue<VideoBuffer> q_;
    Link *link_;
};

#endif