#ifndef _RTMP_H
#define _RTMP_H
#include "../public.h"
#include "srs_librtmp.h"

namespace streamPusher
{
class Rtmp
{
  public:
    const char *id_;
    const char *url_;
    Rtmp(){
        this->rtmp = NULL;
        this->url_ = "";
        this->id_ = "1";
    };
    Rtmp(const char *url, const char *id)
    {
        this->url_ = url;
        this->id_ = id;
        this->rtmp = NULL;
    }
    ~Rtmp()
    {
        this->Destroy();
    }

    void Destroy()
    {
        if (this->rtmp != NULL)
        {
            srs_rtmp_destroy(this->rtmp);
#if (defined(_WIN32) || defined(_WIN_WCE)) || defined(__APPLE__)
            WSACleanup();
#endif
        }
		this->rtmp = NULL;
    }

#pragma region init
    int Init()
    {
        this->rtmp = srs_rtmp_create(this->url_);
        if (this->rtmp == NULL)
        {
            cout << "***create rtmp error!" << endl;
            return -1;
        }
#ifdef _WIN32
        srs_rtmp_set_timeout(rtmp, 500, 500);
#endif
        int ret = srs_rtmp_handshake(this->rtmp);
        if (ret != HPR_OK)
        {
            cout << "***rtmp handshake error " << ret << endl;
            return ret;
        }
        ret = srs_rtmp_connect_app(this->rtmp);
        if (ret != HPR_OK)
        {
            cout << "***rtmp connect app error " << ret << endl;
            return ret;
        }
        ret = srs_rtmp_publish_stream(this->rtmp);
        if (ret != 0)
        {
            cout << "***rtmp publish stream error " << ret << endl;
            return ret;
        }
        if (ret != HPR_OK)
        {
            cout << "***init rtmp error " << ret << endl;
            return ret;
        }
        return HPR_OK;
    }
#pragma endregion

    // if return value not equal zero, restart it.
    int send(char *buffer, int size, unsigned long dt)
    {
        if (this->rtmp == NULL)
        {
            return -1;
        }
        // cout<<"send "<<endl;
        if (buffer == NULL || size < 1)
        {
            return 0;
        }
        int ret = srs_h264_write_raw_frames(this->rtmp, buffer, size, dt, dt);
        if (ret != 0)
        {
            if (srs_h264_is_dvbsp_error(ret) || srs_h264_is_duplicated_pps_error(ret) || srs_h264_is_duplicated_sps_error(ret))
            {
                ret = 0;
            }
            else
            {
                cout << "***the " << this->id_ << " rtmp error " << ret << endl;
                if (ret == 3041)
                {
                    ret = 0;
                }
                else
                {
                    cout << "***the " << this->id_ << " rtmp restart!" << endl;
                    this->Destroy();
                    return this->Init();
                }
            }
        }
        return ret;
    }

  private:
    srs_rtmp_t rtmp;
};
}

#endif