
#include "pusher.h"

// callback and re callback
namespace streamPusher
{
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
    cout<<21<<endl;
    return NULL;
}
#endif

int Pusher::init()
{
    //init rtmp
    int ret = -1;
    for (int i = 0; i < this->length; i++)
    {
        ret = this->rtmp[i]->Init();
        if (ret != HPR_OK)
        {
            cout << "init the " << this->rtmp[i]->id_ << " rtmp error " << ret << endl;
            return ret;
        }
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
	this->running = true;

    return HPR_OK;
}

#pragma region start
int Pusher::start()
{
    this->stop();
    int ret = this->init();
    if (HPR_OK != ret)
    {
        return ret;
    }
    cout << "init rtmp success" << endl;
#ifdef _WIN32
    DWORD retW = WaitForSingleObject(this->tid, INFINITE);
    if (retW != 0)
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
        this->tid = 0;
        // restart
        cout << "restart pusher!" << endl;
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
        for (int i = 0; i < this->length; i++)
        {
			if (this->rtmp[i] != NULL) {
				this->rtmp[i]->Destroy();
			}
        }
    }

#ifdef _WIN32
    if (this->tid != NULL)
    {
        CloseHandle(this->tid);
    }
#elif defined(__linux__) || defined(__APPLE__)
    if (this->tid > 0)
    {
        pthread_exit(NULL);
        this->tid = 0;
    };
#endif
}
#pragma endregion

Pusher::Pusher()
{
    this->length = 0;
    this->rtmp = NULL;
#ifdef _WIN32
    this->tid = NULL;
#elif defined(__linux__) || defined(__APPLE__)
    this->tid = 0;
#endif
}

Pusher::Pusher(Camera_ *c, Cache *cache, Link *link)
{
    this->id_ = c->id_.c_str();
    // this->url_ = url;
    this->fr_ = c->send_;
    this->cache_ = cache;
    this->link_ = link;
    this->link_->frRTMP = c->fetch_;
#ifdef _WIN32
    this->tid = NULL;
#elif defined(__linux__) || defined(__APPLE__)
    this->tid = 0;
#endif
	this->times = 0;
	this->times_ = 0;
	this->avgTime = 0;
	this->lastTimes = 0;
    this->dt = 0;
    this->length = c->length;
    this->running = false;
    this->rtmp = new Rtmp*[this->length];
    for (int i = 0; i < this->length; i++)
    {
        this->rtmp[i] = new Rtmp(c->rtmp[i].url_.c_str(), c->rtmp[i].id_.c_str());
    }
}

Pusher::~Pusher()
{
    this->stop();
    if (this->rtmp != NULL)
    {
        cout << "deq " << 888 << endl;
		for (int i = 0; i < this->length; i++)
		{
			delete this->rtmp[i];
		}
        delete[] this->rtmp;
    }
}

int Pusher::send()
{
    if (this->cache_->empty())
    {
        return 0;
    }
    VideoBuffer vb = this->cache_->pop();
    int ret;
    for (int i = 0; i < this->length; i++)
    {
        ret = this->rtmp[i]->send(vb.buffer, vb.size, this->dt);
        if (ret != 0)
        {
            goto end;
        }
    }

end:
    vb.Free();
    if (this->times++ % (this->fr_ * 60 * 1) == 0)
    {
        cout << "avgf: " << vb.avgTime << "	 avgp: " << this->avgTime << "  qsize: " << this->cache_->size() << "   " << this->fr_ << endl;
    }
    this->dt += 1000.0 / this->fr_;
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
		unsigned long time_ = srs_utils_time_ms();
        this->link_->frRTMP = this->link_->frDVR;
        this->fr_ = this->link_->frDVR;
        if (0 != this->send())
        {
            cout << "send fail and restart!" << endl;
            return (void *)1;
        }

        float st = 1000.0 / this->fr_;
        st *= ((float)this->cache_->capacity / this->cache_->size());
        this->doSleep(st - (time_ - this->lastTimes));
		this->getAvgTime(time_);
		this->lastTimes = time_;
    }
    return NULL;
}

void Pusher::getAvgTime(unsigned long now)
{
	if (this->times_ > 2)
	{
		this->avgTime = (this->avgTime + (float)(now - this->lastTimes) / (float)(this->times_ - 1)) * ((float)(this->times_ - 1) / (float)this->times_);
	}
	else if (this->times_ == 2)
	{
		this->avgTime = (float)(now - this->lastTimes);
	}
	this->times_++;
	if (this->times_ > 25)
	{
		this->times_ = 5;
	}
}

}