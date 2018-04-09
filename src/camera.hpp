#ifndef _CAMERA_H
#define _CAMERA_H

#include "public.h"
#include "pusher.h"
#include "fetcher.h"
namespace streamPusher
{
class Camera
{
  private:
    Camera_ *config;
    Link *link;
    Cache *cache;
    Fetcher *fetcher;
    Pusher *pusher;

  public:
    Camera()
    {
        config = NULL;
        this->link = NULL;
        this->cache = NULL;
        this->fetcher = NULL;
        this->pusher = NULL;
    }
    Camera(Camera_ *c)
    {
        this->config = c;
        this->link = NULL;
        this->cache = NULL;
        this->fetcher = NULL;
        this->pusher = NULL;
    }
    ~Camera()
    {
        this->Cleanup();
        if(this->fetcher != NULL){
            delete this->fetcher;
        }
        if (this->pusher != NULL)
        {
            delete this->pusher;
        }
        if (this->link != NULL)
        {
            delete this->link;
        }
        if (this->cache != NULL)
        {
            delete this->cache;
        }
    }

    int start()
    {
        // no rtmp config
        if (this->config->length < 1)
        {
            return HPR_ERROR;
        }
        // init cache, link
        this->link = new Link();
        this->link->id_ = this->config->id_;
        this->cache = new Cache(this->config->cache_ * this->config->send_, this->link);
        this->fetcher = new Fetcher(this->config->channel_, this->config->ip_.c_str(), this->config->port_, this->config->user_.c_str(), this->config->password_.c_str(), this->cache, this->link);
        this->pusher = new Pusher(this->config, this->cache, this->link);
        // init dvr
        int ret = fetcher->Init();
        if (ret != HPR_OK)
        {
            cout << "init fetcher error " << ret << endl;
            this->Cleanup();
            return ret;
        }
        cout << "init the " << this->config->id_ << " dvr success!" << endl;

#ifdef _WIN32
        Sleep(this->config->cache_ * ((float)this->config->send_ / (float)this->config->fetch_) * 1000);
#elif defined(__linux__) || defined(__APPLE__)
        sleep(this->config->cache_ * ((float)this->config->send_ / (float)this->config->fetch_));
#endif
        ret = this->pusher->start();
        if (ret != HPR_OK)
        {
            cout << "the " << this->config->id_ << " rtmp init error! " << ret << endl;
            return HPR_ERROR;
        }
        return HPR_OK;
    }

    void Cleanup()
    {
        if (this->fetcher != NULL)
        {
            this->fetcher->Cleanup();
        }
        if (this->pusher != NULL)
        {
            this->pusher->stop();
        }
    }
};
}

#endif