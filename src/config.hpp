#ifndef _CONFIG_H
#define _CONFIG_H

#include "json/json.hpp"
using nlohmann::json;
namespace streamPusher
{
struct Rtmp_
{
    string url_;
    string id_;
    void from_json(const json &j)
    {
        this->url_ = j.at("url").get<std::string>();
        this->id_ = j.at("id").get<std::string>();
    }
};
struct Camera_
{
    Camera_() { this->rtmp = NULL; }
    ~Camera_()
    {
        if (this->rtmp != NULL)
        {
            delete[] this->rtmp;
        }
    }
	int channel_;
    string id_;
    int cache_;
    string ip_;
    int port_;
    string user_;
    string password_;
	int framerate_;
	int bitrate_;
	int resolution_;
    int fetch_;
    int send_;
    Rtmp_ *rtmp;
    int length;
    void from_json(const json &j)
    {
		this->channel_ = j.at("channel").get<int>();
        this->id_ = j.at("id").get<std::string>();
        this->cache_ = j.at("cache").get<int>();
        this->ip_ = j.at("ip").get<std::string>();
        this->port_ = j.at("port").get<int>();
        this->user_ = j.at("user").get<std::string>();
        this->password_ = j.at("password").get<std::string>();
		this->framerate_ = j.at("framerate").get<int>();
		this->resolution_ = j.at("resolution").get<int>();
		this->bitrate_ = j.at("bitrate").get<int>();
		this->fetch_ = j.at("fetch").get<int>();
        this->send_ = j.at("send").get<int>();
        json rtmp = j.at("rtmp");
        if (rtmp.is_array())
        {
            this->rtmp = new Rtmp_[rtmp.size()];
            this->length = rtmp.size();
            int i = 0;
            for (auto &element : rtmp)
            {
                this->rtmp[i].from_json(element);
                i++;
            }
        }
    }
};

struct Config
{
  public:
    Camera_ *cameras;
    int length;
    Config() { this->cameras = NULL; }
    ~Config()
    {
        if (this->cameras != NULL)
        {
            delete[] this->cameras;
        }
    }
    void from_json(const json &j)
    {
        json cameras = j.at("cameras");
        if (!cameras.is_array())
        {
            return;
        }
        this->cameras = new Camera_[cameras.size()];
        this->length = cameras.size();
        int i = 0;
        // iterate the array
        for (auto &element : cameras)
        {
            this->cameras[i].from_json(element);
            i++;
        }
    }
};
}

#endif