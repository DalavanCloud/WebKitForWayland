#ifndef TUNER_H
#define TUNER_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <map>

#include <refsw/nexus_config.h>
#include <refsw/nxclient.h>
#include <refsw/nexus_platform.h>
#include <refsw/nexus_frontend.h>
#include <refsw/nexus_parser_band.h>
#include <refsw/nexus_simple_video_decoder.h>
#include <refsw/nexus_simple_audio_decoder.h>
#include <refsw/nexus_message.h>

#define MAX_MSG 64
#define MAX_PID_CHANNELS MAX_MSG + 1

typedef std::vector<std::pair<int,int>> DVB_PAT;            // progNum,pmtPid

class ES_Info {
    public:
    ES_Info()
        : streamType(0)
        , pid(0)
    {
    }
    ES_Info(int st, int p)
        : streamType(st)
        , pid(p)
    {
    }
    int streamType;
    int pid;
};

typedef std::vector<ES_Info>            ElementaryStrems;   
typedef std::map<int,ElementaryStrems>  DVB_PMT;

class DVB_TS {
    public:
    DVB_TS() 
        : modulation(0)
        , symbolRate(0) 
    {
    }
    int modulation;
    int symbolRate;
    DVB_PAT pat;
    DVB_PMT pmt;
};
typedef std::map<int,DVB_TS> TsArray;

class DVB_NETWORK {
    public:
    TsArray aTs;
};

class Channel {
    public:
    Channel() : type(Normal) {}

    enum Type { Normal, Radio, Data };
    const char *typeStr[3] = { "Normal", "Radio", "Data" };
    int frequency;
    int programNumber;
    int modulation;
    int symbolRate;
    Type type;
};
typedef std::vector<Channel> ChannelMap;

class NexusClient {
    public:
    NexusClient();
    ~NexusClient();
};

class BcmTuner {
    public:
    BcmTuner();
    ~BcmTuner();
    int tune(int freq);
    void setParserBand(NEXUS_ParserBand *pb) { pParserBand = pb; }
    
    private:
    NEXUS_FrontendHandle frontend;
    NEXUS_ParserBand *pParserBand;
    BKNI_EventHandle statusEvent;
    bool bDvb;
};

class BcmVideoDecoder {
    public:
    BcmVideoDecoder();
    ~BcmVideoDecoder();
    unsigned getId();
    int start(const ES_Info &ESVideo);
    int stop();
    void setStcChannel(NEXUS_SimpleStcChannelHandle *pStc) { pStcChannel = pStc; }        
    void setParserBand(NEXUS_ParserBand *pb) { pParserBand = pb; }

    private:
    NEXUS_PidChannelHandle m_pidChannel;
    NEXUS_SimpleVideoDecoderHandle m_videoDecoder;
    NEXUS_SimpleStcChannelHandle *pStcChannel;
    NEXUS_ParserBand *pParserBand;
    NxClient_AllocResults m_allocResults;
};

class BcmAudioDecoder {
    public:
    BcmAudioDecoder();
    ~BcmAudioDecoder();
    unsigned getId();
    int start(const ES_Info &ESAudio);
    int stop();
    void setStcChannel(NEXUS_SimpleStcChannelHandle *pStc) { pStcChannel = pStc; }        
    void setParserBand(NEXUS_ParserBand *pb) { pParserBand = pb; }
        
    private:
    NEXUS_PidChannelHandle m_pidChannel;
    NEXUS_SimpleAudioDecoderHandle m_audioDecoder;
    NEXUS_SimpleStcChannelHandle *pStcChannel;
    NEXUS_ParserBand *pParserBand;
    NxClient_AllocResults m_allocResults;
};

class BcmTVManager {
    public:
    BcmTVManager();
    ~BcmTVManager();
    int init();
    int deinit();
    int tune(int freq);
    int tune(int freq, int progNum);
    int scan(int startFreq, int endFreq = 0);
    int startFilter(int pid, const uint8_t **pBuffer, size_t *pSize);
    int stopFilter();
    int decode(const ES_Info &ESVideo, const ES_Info &ESAudio);
    int stop();
    int disconnect();
    int getChannelMap(ChannelMap &chanMap);

    NEXUS_MessageHandle hMsg[MAX_PID_CHANNELS];
    NEXUS_PidChannelHandle hPidChannel[MAX_PID_CHANNELS];

    private:
    int parsePAT(const uint8_t *buffer, unsigned size, DVB_PAT &pat);
    int parsePMT(const uint8_t *buffer, unsigned size, ElementaryStrems &streams);
    bool getStreamInfo(int freq, int progNum, ES_Info &ESVideo, ES_Info &ESAudio);

    private:
    NexusClient m_nexus;
    unsigned m_connectId;
    int m_pidChanCount;
    NEXUS_ParserBand m_parserBand;
    NEXUS_SimpleStcChannelHandle m_stcChannel;
    BcmTuner        m_tuner;            // XXX: make it array and manage
    BcmVideoDecoder m_videoDecoder;
    BcmAudioDecoder m_audioDecoder;
    NEXUS_PidChannelHandle m_pcrPidChannel;
    BKNI_EventHandle m_msgEvent;
    DVB_NETWORK m_network;
    //std::mutex mtx;
    //std::condition_variable cv;
};


#endif
