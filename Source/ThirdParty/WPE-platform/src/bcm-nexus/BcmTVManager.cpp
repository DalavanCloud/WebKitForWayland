#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <iostream>
#include "BcmTVManager.h"
#include <refsw/ts_psi.h>

#define myprintf(...)   printf(__VA_ARGS__)
#define err_msg         myprintf
#define dbg_msg         myprintf
#define log_msg         //myprintf
#define mytrace()       log_msg("%s: %s:%d\n", __FUNCTION__, __FILE__, __LINE__)

#define TUNE_TIMEOUT 2000
#define DATA_TIMEOUT 5000
static const unsigned J83A_SYMBOLRATE_UPPER   = 7200000;
static const unsigned J83A_SYMBOLRATE_LOWER   = 4000000;
static const unsigned J83A_SYMBOLRATE_DEFAULT = 6952000;
static const unsigned J83B_SYMBOLRATE_QAM64   = 5056941;
static const unsigned J83B_SYMBOLRATE_QAM256  = 5360537;

using namespace std;

long long getMS()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long long ms = (long long) tp.tv_sec * 1000L + tp.tv_usec / 1000;
    // c++11
    //using namespace std::chrono;
    //milliseconds ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
    return ms;
}


// XXX : Replace PAT/PMT with libdvbpsi
// PAT -------------------------------------------------------------------------
#define TS_READ_16( buf ) ((uint16_t)((buf)[0]<<8|(buf)[1]))

#define TS_PSI_TABLE_ID_OFFSET				0
#define TS_PSI_SECTION_LENGTH_OFFSET		1
#define TS_PSI_TABLE_ID_EXT_OFFSET			3
#define TS_PSI_CNI_OFFSET					5
#define TS_PSI_SECTION_NUMBER_OFFSET		6
#define TS_PSI_LAST_SECTION_NUMBER_OFFSET	7

#define TS_PSI_GET_SECTION_LENGTH( buf )	(uint16_t)(TS_READ_16( &(buf)[TS_PSI_SECTION_LENGTH_OFFSET] ) & 0x0FFF)
#define TS_PSI_MAX_BYTE_OFFSET( buf )		(TS_PSI_GET_SECTION_LENGTH(buf) - 1)

bool TS_PAT_validate(const uint8_t *buf, unsigned bfrSize)
{
    BSTD_UNUSED(bfrSize);
    return (buf[0] == 0x00);
}

BERR_Code TS_PAT_getProgram( const uint8_t *buf, unsigned bfrSize, int programNum, int  *progNum, int *pid )
{
    int byteOffset = TS_PSI_LAST_SECTION_NUMBER_OFFSET+1;

    byteOffset += programNum*4;

    if( byteOffset >= TS_PSI_MAX_BYTE_OFFSET(buf) || byteOffset >= (int)bfrSize)
    {
        return BERR_INVALID_PARAMETER;
    }

    *progNum = TS_READ_16( &buf[byteOffset] );
    *pid = (uint16_t)(TS_READ_16( &buf[byteOffset+2] ) & 0x1FFF);

    return BERR_SUCCESS;
}

uint8_t TS_PAT_getNumPrograms( const uint8_t *buf)
{
    return (TS_PSI_MAX_BYTE_OFFSET(buf)-(TS_PSI_LAST_SECTION_NUMBER_OFFSET+1))/4;
}

// PMT -------------------------------------------------------------------------
#define PROGRAM_INFO_LENGTH_OFFSET (TS_PSI_LAST_SECTION_NUMBER_OFFSET+3)
#define PROGRAM_INFO_LENGTH(buf) (TS_READ_16(&buf[PROGRAM_INFO_LENGTH_OFFSET])&0xFFF)
#define DESCRIPTOR_BASE(buf) (&buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+5])
#define STREAM_BASE(buf) (TS_PSI_LAST_SECTION_NUMBER_OFFSET + 5 + PROGRAM_INFO_LENGTH(buf))

bool TS_PMT_validate(const uint8_t *buf, unsigned bfrSize)
{
    int sectionEnd = TS_PSI_GET_SECTION_LENGTH(buf) + TS_PSI_SECTION_LENGTH_OFFSET;
    int programInfoEnd = PROGRAM_INFO_LENGTH(buf) + PROGRAM_INFO_LENGTH_OFFSET;

    return (buf[0] == 0x2) &&
        sectionEnd < (int)bfrSize &&
        programInfoEnd < (int)bfrSize &&
        programInfoEnd < sectionEnd;
}

static int TS_PMT_P_getStreamByteOffset( const uint8_t *buf, unsigned bfrSize, int streamNum )
{
    int byteOffset;
    int i;

    /* After the last descriptor */
    byteOffset = STREAM_BASE(buf);

    for (i=0; i < streamNum; i++)
    {
        if (byteOffset >= (int)bfrSize || byteOffset >= TS_PSI_MAX_BYTE_OFFSET(buf))
            return -1;
        byteOffset += 5 + (TS_READ_16( &buf[byteOffset+3] ) & 0xFFF);
    }

    return byteOffset;
}

uint16_t TS_PMT_getPcrPid( const uint8_t *buf, unsigned bfrSize)
{
    BSTD_UNUSED(bfrSize);
    return (uint16_t)(TS_READ_16( &buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+1] ) & 0x1FFF);
}

int TS_PMT_getNumStreams( const uint8_t *buf, unsigned bfrSize)
{
    int byteOffset;
    int i = 0;

    byteOffset = STREAM_BASE(buf);

    while (byteOffset < TS_PSI_MAX_BYTE_OFFSET(buf) && byteOffset < (int)bfrSize)
    {
        byteOffset += 5 + (TS_READ_16( &buf[byteOffset+3] ) & 0xFFF);
        i++;
    }

    return i;
}

BERR_Code TS_PMT_getStream( const uint8_t *buf, unsigned bfrSize, int streamNum, int *streamType, int *elementaryPID )
{
    int byteOffset;

    byteOffset = TS_PMT_P_getStreamByteOffset( buf, bfrSize, streamNum );
    if (byteOffset == -1)
        return BERR_INVALID_PARAMETER;

    *streamType = buf[byteOffset];
    *elementaryPID = (uint16_t)(TS_READ_16( &buf[byteOffset+1] ) & 0x1FFF);

    return BERR_SUCCESS;
}
// -----------------------------------------------------------------------------

static void lock_callback(void *context, int param)
{
    BKNI_EventHandle statusEvent = (BKNI_EventHandle)context;
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)statusEvent);
}

static void message_callback(void *context, int index)
{
#if 1
    BKNI_EventHandle msgEvent = (BKNI_EventHandle)context;
    BKNI_SetEvent((BKNI_EventHandle)msgEvent);
#else
    BcmTVManager *tvm = (BcmTVManager *)context;
    NEXUS_Error rc;
    const uint8_t *buffer;
    size_t size;

    rc = NEXUS_Message_GetBuffer(tuner->hMsg[index], (const void**)&buffer, &size);
    log_msg ("%s: pidChan Index=%d size=%d rc=%d\n", __FUNCTION__, index, size, rc);
    if (rc || !size) {
        rc = BERR_TRACE(rc);
        return;
    }

    if (index == 0) {
        tvm->parsePAT(buffer, size);
    } else {
        tvm->parsePMT(buffer, size);
    }
#endif
}

NexusClient::NexusClient()
{
    mytrace();
    NEXUS_Error rc;
    rc = NxClient_Join(NULL);
    BDBG_ASSERT(!rc);
}

NexusClient::~NexusClient()
{
    mytrace();
    NxClient_Uninit();
}

BcmVideoDecoder::BcmVideoDecoder()
    : m_videoDecoder(NULL)
    , pStcChannel(NULL)
    , pParserBand(NULL)
{
    NEXUS_Error rc = 0;
    NxClient_AllocSettings allocSettings;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &m_allocResults);
    BDBG_ASSERT(!rc);
    
    m_videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(m_allocResults.simpleVideoDecoder[0].id);
    BDBG_ASSERT(m_videoDecoder);
 }

BcmVideoDecoder::~BcmVideoDecoder()
{
    if (m_videoDecoder)
        NEXUS_SimpleVideoDecoder_Release(m_videoDecoder);
    m_videoDecoder = nullptr;
    NxClient_Free(&m_allocResults);
}

unsigned BcmVideoDecoder::getId()
{
    return m_allocResults.simpleVideoDecoder[0].id;
}

int BcmVideoDecoder::start(const ES_Info &ESVideo)
{
    NEXUS_Error rc = 0;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;

    m_pidChannel = NEXUS_PidChannel_Open(*pParserBand, ESVideo.pid, NULL);
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
    if (m_pidChannel) {
        switch(ESVideo.streamType) {
            case TS_PSI_ST_11172_2_Video:
            case TS_PSI_ST_13818_2_Video:
                videoProgram.settings.codec = NEXUS_VideoCodec_eMpeg2;
                break;
            case TS_PSI_ST_14496_10_Video:
                videoProgram.settings.codec = NEXUS_VideoCodec_eH264;
                break;
            default:
                dbg_msg("%s: Unknown stream type %d pid=%d  (Using default Mpeg2)\n", ESVideo.streamType, ESVideo.pid);
                videoProgram.settings.codec = NEXUS_VideoCodec_eMpeg2;
        }
        videoProgram.settings.pidChannel = m_pidChannel;
        if (*pStcChannel)
            NEXUS_SimpleVideoDecoder_SetStcChannel(m_videoDecoder, *pStcChannel);
        rc = NEXUS_SimpleVideoDecoder_Start(m_videoDecoder, &videoProgram);
        dbg_msg ("%s: Stream type=%d pid=%d NEXUS_SimpleVideoDecoder_Start rc=%d\n", __FUNCTION__, ESVideo.streamType, ESVideo.pid, rc);
    } else {
        err_msg("%s: Unable to open pid channel\n", __FUNCTION__);
    }
    return rc;
}

int BcmVideoDecoder::stop()
{
    NEXUS_Error rc = 0;

    if (m_pidChannel)
        NEXUS_PidChannel_Close(m_pidChannel);
    if (m_videoDecoder)
        NEXUS_SimpleVideoDecoder_Stop(m_videoDecoder);

    return rc;
}

BcmAudioDecoder::BcmAudioDecoder()
    : m_audioDecoder(NULL)
    , pStcChannel(NULL)
    , pParserBand(NULL)
{
    NEXUS_Error rc = 0;
    NxClient_AllocSettings allocSettings;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &m_allocResults);
    BDBG_ASSERT(!rc);

    m_audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(m_allocResults.simpleAudioDecoder.id);
    BDBG_ASSERT(m_audioDecoder);
}

BcmAudioDecoder::~BcmAudioDecoder()
{
    if (m_audioDecoder)
        NEXUS_SimpleAudioDecoder_Release(m_audioDecoder);
    m_audioDecoder = nullptr;
    NxClient_Free(&m_allocResults);
}

unsigned BcmAudioDecoder::getId()
{
    return m_allocResults.simpleAudioDecoder.id;
}

int BcmAudioDecoder::start(const ES_Info &ESAudio)
{
    NEXUS_Error rc = 0;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    m_pidChannel = NEXUS_PidChannel_Open(*pParserBand, ESAudio.pid, NULL);
    if (m_pidChannel) {
        switch(ESAudio.streamType) {
            case TS_PSI_ST_11172_3_Audio:
            case TS_PSI_ST_13818_3_Audio:
                audioProgram.primary.codec = NEXUS_AudioCodec_eMpeg;
                break;
            case TS_PSI_ST_13818_7_AAC:
                audioProgram.primary.codec = NEXUS_AudioCodec_eAac;
                break;            
            default:
                dbg_msg("%s: Unknown stream type %d pid=%d (Using default AC3)\n", __FUNCTION__, ESAudio.streamType, ESAudio.pid);
                audioProgram.primary.codec = NEXUS_AudioCodec_eAc3;
        }
        audioProgram.primary.pidChannel = m_pidChannel; 
        if (*pStcChannel)
            NEXUS_SimpleAudioDecoder_SetStcChannel(m_audioDecoder, *pStcChannel);
        rc = NEXUS_SimpleAudioDecoder_Start(m_audioDecoder, &audioProgram);
        dbg_msg ("%s: Stream type=%d pid=%d NEXUS_SimpleAudioDecoder_Start rc=%d\n", __FUNCTION__, ESAudio.streamType, ESAudio.pid, rc);
    } else {
        err_msg("%s: Unable to open pid channel\n", __FUNCTION__);
    }

    return rc;
}

int BcmAudioDecoder::stop()
{
    NEXUS_Error rc = 0;

    if (m_pidChannel)
        NEXUS_PidChannel_Close(m_pidChannel);
    if (m_audioDecoder)
        NEXUS_SimpleAudioDecoder_Stop(m_audioDecoder);

    return rc;
}

BcmTuner::BcmTuner(bool dvb)
    : frontend(nullptr)
    , pParserBand(nullptr)
    , statusEvent(NULL)
    , bDvb(dvb)
{
    if (bDvb)
        dbg_msg("Annex-A 8 Mhz\n");
    else
        dbg_msg("Annex-B 6 Mhz\n");
    BKNI_CreateEvent(&statusEvent);
    NEXUS_FrontendAcquireSettings frontendAcquireSettings;
    NEXUS_Frontend_GetDefaultAcquireSettings(&frontendAcquireSettings);
    frontendAcquireSettings.capabilities.qam = true;
    frontend = NEXUS_Frontend_Acquire(&frontendAcquireSettings);
    if (!frontend) {
        err_msg("Unable to find QAM-capable frontend\n");
    }
}

BcmTuner::~BcmTuner()
{
    if (statusEvent) 
        BKNI_DestroyEvent(statusEvent);
    if (frontend)
        NEXUS_Frontend_Release(frontend);
}

int BcmTuner::tune(int freq)
{
    if ((!frontend) || (!pParserBand))
        return -1;

    NEXUS_Error rc;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendUserParameters userParams;
    long long tuneStart = getMS();
    
    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = freq * 1000000;
    qamSettings.mode = NEXUS_FrontendQamMode_e256;
    
    if (bDvb) {
        qamSettings.annex = NEXUS_FrontendQamAnnex_eA;
        qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e8Mhz;
        qamSettings.symbolRate = J83A_SYMBOLRATE_DEFAULT;
        qamSettings.scan.upperBaudSearch = J83A_SYMBOLRATE_UPPER;
        qamSettings.scan.lowerBaudSearch = J83A_SYMBOLRATE_LOWER;
    } else {
        qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
        qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
        qamSettings.symbolRate = J83B_SYMBOLRATE_QAM256;
        qamSettings.scan.upperBaudSearch = 5360537;
        qamSettings.scan.lowerBaudSearch = 5056941;
    }
    qamSettings.lockCallback.callback = lock_callback;
    qamSettings.lockCallback.context = statusEvent;
    qamSettings.lockCallback.param = 0;
    qamSettings.autoAcquire = true;
    qamSettings.enablePowerMeasurement = false;
    qamSettings.acquisitionMode = NEXUS_FrontendQamAcquisitionMode_eScan;
    qamSettings.spectrumMode = NEXUS_FrontendQamSpectrumMode_eAuto;
    qamSettings.enableNullPackets = false;
    qamSettings.scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e64] = true;
    qamSettings.scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e256] = true;
    qamSettings.scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e64] = true;
    qamSettings.scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e256] = true;

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    NEXUS_ParserBand_GetSettings(*pParserBand, &parserBandSettings);

    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend);
    } else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    rc = NEXUS_ParserBand_SetSettings(*pParserBand, &parserBandSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
    err_msg ("\n%s: NEXUS_Frontend_TuneQam rc=%d\n", __FUNCTION__, rc);
    BDBG_ASSERT(!rc);

    NEXUS_FrontendFastStatus status;
    status.lockStatus = NEXUS_FrontendLockStatus_eUnknown;
    do {
        rc = BKNI_WaitForEvent(statusEvent, TUNE_TIMEOUT);
        if(rc == NEXUS_TIMEOUT) {
            dbg_msg("%d Mhz NEXUS_TIMEOUT\n", freq);
            break;
        }
        NEXUS_Frontend_GetFastStatus(frontend, &status);
        log_msg("\t%d Mhz lockStatus=%d frontend=%p\n", freq, status.lockStatus, (void*)frontend);        
    } while ((status.lockStatus != NEXUS_FrontendLockStatus_eLocked) &&  (status.lockStatus != NEXUS_FrontendLockStatus_eNoSignal));

    if (status.lockStatus == NEXUS_FrontendLockStatus_eLocked) {
        NEXUS_FrontendQamScanStatus scanStatus;
        rc = NEXUS_Frontend_GetQamScanStatus(frontend, &scanStatus);
        if(rc) {
            err_msg("NEXUS_Frontend_GetQamScanStatus FAILED\n");
        } else {
            long long tuneEnd = getMS();
            dbg_msg("%d Mhz SymbolRate: %d Mode: %d Annex: %d Interleaver: %d Spectrum inverted: %s status: %d tuneTime=%d\n", 
                freq, scanStatus.symbolRate, scanStatus.mode, scanStatus.annex, scanStatus.interleaver,
                scanStatus.spectrumInverted?"True":"False", scanStatus.acquisitionStatus,
                (tuneEnd - tuneStart));
        }
    }
    
    return rc;
}

BcmTVManager::BcmTVManager(bool bDvb)
    : m_pidChanCount(0)
    , m_parserBand(0)
    , m_tuner(bDvb)
    , m_msgEvent(NULL)
{
    BKNI_CreateEvent(&m_msgEvent);
}

BcmTVManager::~BcmTVManager()
{
    if (m_msgEvent) 
        BKNI_DestroyEvent(m_msgEvent);
}

int BcmTVManager::init()
{
    NEXUS_Error rc = 0;

    m_stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    m_parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
    m_tuner.setParserBand(&m_parserBand);

    return rc;
}

int BcmTVManager::deinit()
{
    NEXUS_Error rc = 0;;
    if (m_parserBand)
        NEXUS_ParserBand_Close(m_parserBand);
    if (m_stcChannel)
        NEXUS_SimpleStcChannel_Destroy(m_stcChannel);
    return rc;
}

int BcmTVManager::tune(int freq)
{
    return m_tuner.tune(freq);
}

int BcmTVManager::tune(int freq, int progNum)
{
    NEXUS_Error rc;

    //rc = scan(freq);
    rc = tune(freq);
    if (rc == 0) {
        ES_Info ESVideo, ESAudio;
        if (getStreamInfo(freq, progNum, ESVideo, ESAudio))
            rc = decode(ESVideo, ESAudio);
    }
    return rc;
}

int BcmTVManager::decode(const ES_Info &ESVideo, const ES_Info &ESAudio)
{
    NEXUS_Error rc;
    NxClient_ConnectSettings connectSettings;
    long long start = getMS();

    NxClient_GetDefaultConnectSettings(&connectSettings);
    if (ESVideo.pid)
        connectSettings.simpleVideoDecoder[0].id = m_videoDecoder.getId();
    connectSettings.simpleAudioDecoder.id = m_audioDecoder.getId();
    
    log_msg ("%s: simpleVideoDecoder[0].id=%d simpleAudioDecoder.id=%d\n", __FUNCTION__, 
        connectSettings.simpleVideoDecoder[0].id, connectSettings.simpleAudioDecoder.id);

    int pcrPid = ESVideo.pcrPid;
    m_pcrPidChannel = NEXUS_PidChannel_Open(m_parserBand, pcrPid, NULL);

    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_SimpleStcChannel_GetSettings(m_stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_ePcr;
    stcSettings.modeSettings.pcr.pidChannel = m_pcrPidChannel;
    stcSettings.modeSettings.pcr.offsetThreshold = 0xFF;
    NEXUS_SimpleStcChannel_SetSettings(m_stcChannel, &stcSettings);

    rc = NxClient_Connect(&connectSettings, &m_connectId);
    if (rc) 
        return BERR_TRACE(rc);
    dbg_msg ("%s: NxClient_Connect SUCCESS\n", __FUNCTION__);    

    if (ESVideo.pid) {
        m_videoDecoder.setStcChannel(&m_stcChannel);
        m_videoDecoder.setParserBand(&m_parserBand);
        m_videoDecoder.start(ESVideo);
    }
    if (ESAudio.pid) {
        m_audioDecoder.setStcChannel(&m_stcChannel);
        m_audioDecoder.setParserBand(&m_parserBand);
        m_audioDecoder.start(ESAudio);
    }
    
    dbg_msg ("%s: decode Time=%d\n", __FUNCTION__, (getMS() - start));
    
    return rc;
}

int BcmTVManager::stop()
{
    NEXUS_Error rc = 0;
    m_audioDecoder.stop();
    m_videoDecoder.stop();
    if (m_pcrPidChannel)
        NEXUS_PidChannel_Close(m_pcrPidChannel);
    return rc;
}

int BcmTVManager::startFilter(int pid, const uint8_t **pBuffer, size_t *pSize)
{
    int index = m_pidChanCount++;
    NEXUS_Error rc = 0;
    NEXUS_MessageSettings openSettings;
    NEXUS_MessageStartSettings startSettings;

    log_msg ("%s: index=%d pid=%d (x%X)\n", __FUNCTION__, index, pid, pid);
    NEXUS_Message_GetDefaultSettings(&openSettings);
    openSettings.dataReady.callback = message_callback;
    //openSettings.dataReady.context = this;
    openSettings.dataReady.context = m_msgEvent;
    openSettings.dataReady.param = index; /* 0 - PAT */

    log_msg ("%s: NEXUS_Message_Open m_pidChanCount=%d\n", __FUNCTION__, m_pidChanCount);
    hMsg[index] = NEXUS_Message_Open(&openSettings);
    if (hMsg[index]) {
        hPidChannel[index] = NEXUS_PidChannel_Open(m_parserBand, pid, NULL);

        if (hPidChannel[index]) {
            NEXUS_Message_GetDefaultStartSettings(hMsg[index], &startSettings);
            startSettings.pidChannel = hPidChannel[index];
            rc = NEXUS_Message_Start(hMsg[index], &startSettings);
            if (rc)
                err_msg ("%s: NEXUS_Message_Start FAILED rc=%d\n", __FUNCTION__, rc);
            log_msg ("%s: hMsg=%u hPidChannel=%u\n", __FUNCTION__, hMsg[index], hPidChannel);
      
            do {
                rc = NEXUS_Message_GetBuffer(hMsg[index], (const void**)pBuffer, pSize);
                if (*pSize == 0) {
                    rc = BKNI_WaitForEvent(m_msgEvent, DATA_TIMEOUT);
                    if (rc) {
                        rc = BERR_TRACE(rc);
                        break;
                    }
                }
            } while (*pSize == 0);            
        } else {
            err_msg ("%s: NEXUS_PidChannel_Open FAILED\n", __FUNCTION__);
        }
    } else {
        err_msg ("%s: NEXUS_Message_Open FAILED\n", __FUNCTION__);
        rc = BERR_TRACE(NEXUS_UNKNOWN);
    }
    return rc;
}

int BcmTVManager::stopFilter()
{
    NEXUS_Error rc = 0;
    for (int i = 0; i < m_pidChanCount; ++i ) {
        if (hMsg[i]) {
            NEXUS_Message_Close(hMsg[i]);
            hMsg[i] = NULL;
        }
        if (hPidChannel[i]) {
            NEXUS_PidChannel_Close(hPidChannel[i]);
            hPidChannel[i] = NULL;
        }
    }
    m_pidChanCount = 0;
    return rc;
}

int BcmTVManager::disconnect()
{
    NEXUS_Error rc = 0;
    stop();
    NxClient_Disconnect(m_connectId);

    return rc;
}

int BcmTVManager::scan(int startFreq, int endFreq)
{
    NEXUS_Error rc = 0;
    int found = 0;
    unsigned long begin = time(NULL);

    if (endFreq == 0)
        endFreq = startFreq;

    for (int freq = startFreq; freq <= endFreq; ++freq) {
        rc = tune(freq);

        if (rc == 0) {
            ++found;
            const uint8_t *buffer;
            size_t size;
            rc = startFilter(0, &buffer, &size);
            if (rc == 0) {
                DVB_TS dvbTs;
                parsePAT(buffer, size, dvbTs.pat);                    
                dbg_msg("\t%s: Found %d PMT PIDs in PAT freq=%d\n", __FUNCTION__, dvbTs.pat.size(), freq);

                for (auto it = std::begin(dvbTs.pat); it!=std::end(dvbTs.pat); ++it) {
                    int progNum = it->first;
                    int pid = it->second;
                    startFilter(pid, &buffer, &size);
                    ElementaryStrems streams;
                    log_msg("\t%s: Prgram Number %d PMT Pid=%d\n", __FUNCTION__, progNum, pid);
                    parsePMT(buffer, size, streams);
                    dvbTs.pmt[progNum] = streams;
                }
                m_network.aTs.insert(std::pair<int,DVB_TS>(freq, dvbTs));
            }
        }
        stopFilter();
    }
    unsigned long end = time(NULL);
    log_msg("found=%d elapsed_secs=%d\n", found, (int)(end-begin));
    
    return rc;
}

int BcmTVManager::parsePAT(const uint8_t *buffer, unsigned size, DVB_PAT &pat)
{
    int rc = 0;
    int pmt_count = TS_PAT_getNumPrograms(buffer);
    log_msg ("%s: pmt_count=%d PAT valid=%d\n", __FUNCTION__, pmt_count, TS_PAT_validate(buffer, size));
    
    int progNum = 0, pid = 0;
    for(int i=0; i < pmt_count; i++ )	{
        TS_PAT_getProgram(buffer, size, i, &progNum, &pid);
        log_msg("\tprogram_number: %d, PID: 0x%04X\n", progNum, pid);
        if (progNum)    // skip progNum == 0
            pat.push_back(std::pair<int,int>(progNum, pid));
    }
    log_msg ("%s: pat.size=%d\n", __FUNCTION__, pat.size());
    return rc;
}

int BcmTVManager::parsePMT(const uint8_t *buffer, unsigned size, ElementaryStrems &streams)
{
    int rc = 0;
    bool isValid = TS_PMT_validate(buffer, size);
    int  numStreams = TS_PMT_getNumStreams(buffer, size);
    int pcrPid = TS_PMT_getPcrPid(buffer, size);
    log_msg ("%s: PMT valid=%d num streams=%d pcrPid=%d\n", __FUNCTION__, isValid, numStreams, pcrPid);
    for (int i = 0; i < numStreams; ++i) {
        int streamType;
        int elementaryPID;
        TS_PMT_getStream(buffer, size, i, &streamType, &elementaryPID);
        log_msg ("\t%s: #%d. type=%d (x%x) pid=%d (x%x)\n", __FUNCTION__, i, streamType, streamType, elementaryPID, elementaryPID);
        streams.push_back(ES_Info(streamType, elementaryPID, pcrPid));
    }
    log_msg ("%s: streams.size=%d\n", __FUNCTION__, streams.size());
    return rc;
}    

int BcmTVManager::getChannelMap(ChannelMap &chanMap)
{
    int rc = 0;
    for (auto itTS = std::begin(m_network.aTs); itTS!=std::end(m_network.aTs); ++itTS) {
        DVB_TS &ts = itTS->second;
        Channel chan;
        chan.frequency = itTS->first;
        chan.modulation = ts.modulation;
        chan.symbolRate = ts.symbolRate;
        for (auto itPAT = std::begin(ts.pat); itPAT != std::end(ts.pat); ++itPAT) {
            chan.programNumber = itPAT->first;
            ES_Info ESVideo, ESAudio;
            getStreamInfo(chan.frequency, chan.programNumber, ESVideo, ESAudio);
            if (ESVideo.pid)
                chan.type = Channel::Normal;
            else if (ESAudio.pid)
                chan.type = Channel::Radio;
            else 
                chan.type = Channel::Data;
            log_msg("%s: type=%d Video pid=%d Audio pid=%d\n", __FUNCTION__, chan.type, ESVideo.pid, ESAudio.pid);
            if (chan.type != Channel::Data)
                chanMap.push_back(chan);
        }
    }
    return rc;
}

bool BcmTVManager::getStreamInfo(int freq, int progNum, ES_Info &ESVideo, ES_Info &ESAudio)
{
    int bFound = false;
    auto it = m_network.aTs.find(freq);
    if (it != m_network.aTs.end()) {
        DVB_TS &ts = it->second;
        auto itPMT = ts.pmt.find(progNum);
        if (itPMT != ts.pmt.end()) {
            ElementaryStrems streams = itPMT->second;

            for (auto itStreams = std::begin(streams); itStreams != std::end(streams); ++itStreams) {
                int streamType = itStreams->streamType;
                // first available video & audio
                if ((ESVideo.pid == 0) && 
                    ((streamType == TS_PSI_ST_13818_2_Video) || (streamType == TS_PSI_ST_14496_10_Video))) {  // 2: Mpeg 2, 27: H.264
                    ESVideo = *itStreams;
                    bFound = true;
                }
                if ((ESAudio.pid == 0) && 
                    ((streamType == TS_PSI_ST_11172_3_Audio) || (streamType == TS_PSI_ST_13818_3_Audio) || (streamType == TS_PSI_ST_13818_7_AAC))) {
                    ESAudio = *itStreams;
                    bFound = true;
                }
            }
        } else {
            err_msg ("%s: Program Number %d not found\n", __FUNCTION__, progNum);
        }
    } else {
        err_msg ("%s: TS Not found for %d MHz\n", __FUNCTION__, freq);
    }
    return bFound;
}
