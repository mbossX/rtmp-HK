
#include "pusher.h"
#include <signal.h>

// callback and re callback

#ifdef _WIN32
DWORD WINAPI g_tCallback(LPVOID ptr)
{
    if (ptr != NULL)
    {
        return (DWORD)((Pusher *)ptr)->tCallback();
    }
	return 0;
}
#elif defined(__linux__) || defined(__APPLE__)
void *g_tCallback(void *ptr)
{
    if (ptr != NULL)
    {
        return ((Pusher *)ptr)->tCallback();
    }
}
#endif

int Pusher::init()
{
    //init rtmp
    this->rtmp = srs_rtmp_create(this->url_);
	srs_rtmp_set_timeout(rtmp, 500, 500);
    int ret = srs_rtmp_handshake(this->rtmp);
    if (ret != 0)
    {
        cout << "rtmp handshake error " << ret << endl;
        return ret;
    }
    ret = srs_rtmp_connect_app(this->rtmp);
    if (ret != 0)
    {
        cout << "rtmp connect app error " << ret << endl;
        return ret;
    }
    ret = srs_rtmp_publish_stream(this->rtmp);
    if (ret != 0)
    {
        cout << "rtmp publish stream error " << ret << endl;
        return ret;
    }
    if (ret != HPR_OK)
    {
        cout << "init rtmp error " << ret << endl;
        return ret;
    }

        // init thread
#ifdef _WIN32
    this->tid = CreateThread(NULL, 0, g_tCallback, this, 0, NULL);
    if (this->tid == FALSE)
    {
        cout << "init thread error " << ret << endl;
        return HPR_ERROR;
    }
#elif defined(__linux__) || defined(__APPLE__)
    ret = pthread_create(&this->tid, NULL, g_tCallback, this);
    if (0 != ret)
    {
        cout << "init thread error " << ret << endl;
        return HPR_ERROR;
    }
#endif

    return HPR_OK;
}

#pragma region start
int Pusher::start()
{
    this->stop();
    int ret = this->init();
    if (HPR_OK != ret)
    {
        this->stop();
        return ret;
    }
    cout << "init rtmp success" << endl;
#ifdef _WIN32
    DWORD retW = WaitForSingleObject(this->tid, INFINITE);
    if (ret != 0)
    {
        this->tid = NULL;
        // restart
        return this->start();
    }
#elif defined(__linux__) || defined(__APPLE__)
    void *retT;
    pthread_join(this->tid, &retT);
    if (retT != NULL)
    {
        this->tid = NULL;
        // restart
        return this->start();
    }
#endif
    return 0;
}
#pragma endregion

#pragma region stop
void Pusher::stop()
{
    if (this->rtmp != NULL)
    {
        srs_rtmp_destroy(this->rtmp);
#if (defined(_WIN32) || defined(_WIN_WCE)) || defined(__APPLE__)
        WSACleanup();
#endif
    }

#ifdef _WIN32
    if (this->tid != NULL)
    {
        CloseHandle(this->tid);
    }
#elif defined(__linux__) || defined(__APPLE__)
    if (this->tid > -1)
    {
        pthread_exit(NULL);
        this->tid = -1;
    };
#endif
}
#pragma endregion

Pusher::Pusher(char *url, int fr, Cache *cache, Link *link)
{
    this->url_ = url;
    this->fr_ = fr;
    this->cache_ = cache;
    this->link_ = link;
    this->link_->frRTMP = fr;
    this->dt = 0;
    this->rtmp = NULL;
    this->times = 0;
#ifdef _WIN32
    this->tid = NULL;
#elif defined(__linux__) || defined(__APPLE__)
    this->tid = -1;
#endif
}

Pusher::~Pusher()
{
    this->stop();
}

int Pusher::send()
{
    if (this->cache_->empty())
    {
        return 0;
    }
    VideoBuffer vb = this->cache_->pop();
    int ret = srs_h264_write_raw_frames(this->rtmp, vb.buffer, vb.size, (unsigned int)this->dt, (unsigned int)this->dt);
    vb.Free();
    if (ret != 0)
    {
        if (srs_h264_is_dvbsp_error(ret) || srs_h264_is_duplicated_pps_error(ret) || srs_h264_is_duplicated_sps_error(ret))
        {
            ret = 0;
        }
        else
        {
            cout << "rtmp error " << ret << endl;
            cout << "restart rtmp " << endl;
            if (ret == 3041)
            {
                ret = 0;
            }
        }
    }
    if (this->times++ % (6 * 300) == 0)
    {
        cout << "avg: " << vb.avgTime << "  qsize: " << this->cache_->size() << "   " << this->fr_ << endl;
    }
    this->dt += 1000 / this->fr_;
    return ret;
}

#pragma region sleep
int Pusher::doSleep(float st)
{
    if (st < 0.1)
    {
        return 0;
    }
#ifdef _WIN32
    Sleep(st);
#elif defined(__linux__) || defined(__APPLE__)
    usleep(1000 * st);
#endif
    return 0;
}
#pragma endregion

void *Pusher::tCallback()
{
    while (running)
    {
        this->link_->frRTMP = this->link_->frDVR;
        this->fr_ = this->link_->frDVR;
        if (0 != this->send())
        {
            return (void *)1;
        }
        float st = 1000 / this->fr_;
        st *= ((float)this->cache_->capacity / this->cache_->size());
        this->doSleep(st);
    }
    return NULL;
}
