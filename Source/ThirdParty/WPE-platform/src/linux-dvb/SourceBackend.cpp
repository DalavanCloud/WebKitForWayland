#include "SourceBackend.h"
#define TV_DEBUG 1

namespace BCMRPi {

#define TVControlPushEvent(eventId, tunerId, evtState /*optional*/, channelInfo /*optional*/, tvControlBackend) \
{                                                                 \
    struct wpe_tvcontrol_event* event = reinterpret_cast<struct wpe_tvcontrol_event*>(malloc(sizeof(struct wpe_tvcontrol_event))); \
    event->eventID = eventId;                                     \
    event->tuner_id.data = strndup(tunerId, TUNER_ID_LEN);        \
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);         \
    printf("Tuner id platform  =  %s \n", event->tuner_id.data);  \
    event->tuner_id.length = strlen(tunerId);                     \
    event->state = evtState;                                      \
    event->channel_info = channelInfo;                            \
    m_eventQueue->pushEvent(event);                \
}

SourceBackend::SourceBackend(EventQueue<wpe_tvcontrol_event*>* eventQueue, SourceType type, TunerData* tunerData)
    : m_sType(type)
    , m_tunerData(tunerData)
    , m_isScanStopped(false)
    , m_eventQueue(eventQueue)
    , m_scanIndex(0) {
    m_adapter = stoi(m_tunerData->tunerId.substr(0, m_tunerData->tunerId.find(":")));
    m_demux = stoi(m_tunerData->tunerId.substr(m_tunerData->tunerId.find(":")+1));
}

void SourceBackend::clearChannelList() {
    while (!m_channelList.empty()) {
        delete(m_channelList.back());
        m_channelList.pop_back();
    }
}

tvcontrol_return SourceBackend::startScanning(bool isRescanned) {
    tvcontrol_return ret = TVControlFailed;
    uint64_t modulation = m_tunerData->modulation;
    int length =  (m_tunerData->frequency).size();
    struct dvbfe_handle* feHandle = openFE(m_tunerData->tunerId);
    if (isRescanned) {
        clearChannelList();
        TVControlPushEvent(ScanningChanged, m_tunerData->tunerId.c_str(), Cleared, nullptr, m_tvControlBackend);
        m_scanIndex = 0;
    }
    if (feHandle) {
        int i;
        for (i = m_scanIndex; i < length; i++) {
            int frequency = m_tunerData->frequency[i];
            if (tuneToFrequency(frequency, modulation, feHandle)) {
                switch(m_sType) {
                case Atsc:
                case AtscMH:
                    ret = atscScan(frequency, modulation);
                    break;
                case DvbT:
                case DvbT2:
                case DvbC:
                case DvbC2:
                case DvbS:
                case DvbS2:
                case DvbH:
                case DvbSh:
                    ret = dvbScan();
                    break;
                default:
                    printf("Type Not supported!!");
                    break;
                }
            }
            if (m_isScanStopped) {
                TVControlPushEvent(ScanningChanged, m_tunerData->tunerId.c_str(), Stopped, nullptr, m_tvControlBackend);
                m_isScanStopped = false;
                m_scanIndex = i + 1;
                break;
            }
        }
        if (i == length) {
            TVControlPushEvent(ScanningChanged, m_tunerData->tunerId.c_str(), Completed, nullptr, m_tvControlBackend);
            m_scanIndex = 0;
        }
        dvbfe_close(feHandle);
    }
    return ret;
}

void SourceBackend::processPMT(int pmtFd, std::map<int, int>& streamInfo)
{
    int size;
    uint8_t siBuf[4096];

    // read the section
    if ((size = read(pmtFd, siBuf, sizeof(siBuf))) < 0) {
        return;
    }

    // parse section
    struct section *section = section_codec(siBuf, size);
    if (section == NULL) {
        return;
    }

    // parse section_ext
    struct section_ext *sectionExt = section_ext_decode(section, 0);
    if (sectionExt == NULL) {
        return;
    }

    // parse PMT
    struct mpeg_pmt_section *pmt = mpeg_pmt_section_codec(sectionExt);
    if (pmt == NULL) {
        return;
    }

    struct mpeg_pmt_stream *curStream;
    mpeg_pmt_section_streams_for_each(pmt, curStream) {
        printf("stream_type : %x pid : %x \n", curStream->stream_type, curStream->pid);
        streamInfo[curStream->pid] = curStream->stream_type;
    }
}

tvcontrol_return SourceBackend::stopScanning() {
    m_isScanStopped = true;
    return TVControlSuccess;
}

ChannelBackend* SourceBackend::getChannelByLCN(uint64_t channelNo) {
    for (auto& channel : m_channelList) {
         if (channelNo == channel->getLCN()){
            return channel;
        }
    }
    return NULL;
}

static void  parse(char *line, char **argv)
{
     while (*line != '\0') {       /* if not the end of line ....... */
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          while (*line != '\0' && *line != ' ' &&
                 *line != '\t' && *line != '\n')
               line++;             /* skip the argument until ...    */
     }
     *argv = '\0';                 /* mark the end of argument list  */
}

void SourceBackend::execute(char **argv)
{
    pid_t  pid;
    int    status;

    if ((pid = fork()) < 0) {     /* fork a child process           */
         printf("*** ERROR: forking child process failed\n");
         exit(1);
    }
    else if (pid == 0) {          /* for the child process:         */
         if (execvp(*argv, argv) < 0) {     /* execute the command  */
              printf("*** ERROR: exec failed\n");
              exit(1);
         }
    }
    else {
        m_pid = pid;
    }
}

void SourceBackend::startPlayBack(int frequency, uint64_t modulation, int pmtPid, int videoPid, int audioPid) {
    char  command[1024];
    char  *argv[64];
    snprintf ( command, 1024, "gst-launch-1.0 dvbsrc frequency=%d delsys=\"atsc\" modulation=\"8vsb\" pids=%d:%d:%d ! decodebin name=dec dec. ! videoconvert ! autovideosink dec. ! audioconvert ! autoaudiosink", frequency, pmtPid, videoPid, audioPid);
    printf("Command : %s\n", command);
    parse(command, argv);
    execute(argv);
}

tvcontrol_return SourceBackend::setCurrentChannel(uint64_t channelNo) {
    tvcontrol_return ret = TVControlFailed;
    printf("\nTune to Channel %" PRIu64 "\n",channelNo);
    ChannelBackend* channel = getChannelByLCN(channelNo);
    if (channel) {
        kill(m_pid, SIGTERM);
        int freq = channel->getFrequency();
        int programNumber = channel->getProgramNumber();
        unsigned modulation = m_tunerData->modulation;
        struct dvbfe_handle* feHandle = openFE(m_tunerData->tunerId);
        if (feHandle) {
            if (tuneToFrequency(freq, modulation, feHandle)) {
                std::map<int, int> streamInfo;
                mpegScan(programNumber, streamInfo);
                dvbfe_close(feHandle);
                if (!streamInfo.empty()) {
                    ret = TVControlSuccess;
                    int pmtPid = streamInfo[0];
                    int videoPid = 0;
                    int audioPid = 0;
                    for (std::map<int,int>::iterator it = streamInfo.begin(); it != streamInfo.end(); ++it){
                        printf("Stream Type: %d Pid : %d \n" , it->second,it->first);
                         if (videoPid == 0 && it->second == 0x2){
                            videoPid = it->first;
                        }
                         if (audioPid == 0 && it->second == 0x81){
                            audioPid = it->first;
                        }
                    }
                    startPlayBack(freq, modulation, pmtPid, videoPid, audioPid);
                }
            }
            else {
                dvbfe_close(feHandle);
                printf("Cannot tune to channel \n");
            }
        }
    }
    return ret;
}

tvcontrol_return SourceBackend::getChannels(wpe_tvcontrol_channel_vector* channelVector) {
    /*Populate channel list */
    tvcontrol_return ret = TVControlFailed;
    printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
    if (!m_channelList.empty()) {
        printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
        channelVector->length = m_channelList.size();
        channelVector->channels = new wpe_tvcontrol_channel[m_channelList.size()];
        std::vector<ChannelBackend*>::iterator it;
        int i = 0;
        for (auto& channel : m_channelList) {
            if (!(channel->getNetworkId()).empty())
                channelVector->channels[i].networkId = strdup((channel->getNetworkId()).c_str());
            if (!(channel->getName()).empty())
                channelVector->channels[i].name = strdup((channel->getName()).c_str());
            if (!(channel->getServiceId()).empty())
                channelVector->channels[i].serviceId = strdup((channel->getServiceId()).c_str());
            if (!(channel->getTransportStreamId()).empty())
                channelVector->channels[i].transportSId = strdup((channel->getTransportStreamId()).c_str());
            channelVector->channels[i].number = channel->getLCN();
            i++; 
        }
        ret = TVControlSuccess;
    }
    else {
        channelVector->channels = NULL;
        printf("Channel list is empty .Scanning is incomplete \n");
    }
    return ret;
}

tvcontrol_return SourceBackend::dvbScan(){
    /* */
    return TVControlNotImplemented;
}

bool SourceBackend::tuneToFrequency(int frequency, uint64_t modulation, struct dvbfe_handle* feHandle){
    int timeout = 3;

    struct dvbfe_info feInfo;
    dvbfe_get_info(feHandle, DVBFE_INFO_FEPARAMS, &feInfo, DVBFE_INFO_QUERYTYPE_IMMEDIATE, 0);

    switch (m_sType) {
    case Atsc:
    case AtscMH:
        if (DVBFE_TYPE_ATSC != feInfo.type) {
            fprintf(stderr, "%s(): only ATSC frontend supported currently\n", __FUNCTION__);
            return false;
        }
        switch (modulation) {
        case DVBFE_ATSC_MOD_VSB_8:
                feInfo.feparams.frequency = frequency;
                feInfo.feparams.inversion = DVBFE_INVERSION_AUTO;
                feInfo.feparams.u.atsc.modulation = DVBFE_ATSC_MOD_VSB_8;
                break;
            default:
                printf("Modulation not supported!!!\n");
                return false;
        }
        break;
    default:
        printf("Only Atsc supported!!\n");
        return false;
    }
    fprintf(stdout, "tuning to %d Hz, please wait...\n", frequency);
    if (dvbfe_set(feHandle, &feInfo.feparams, timeout * 1000)) {
        fprintf(stderr, "%s(): cannot lock to %d Hz in %d seconds\n",
            __FUNCTION__, frequency, timeout);
        return false;
    }
    fprintf(stdout, "tuner locked.\n");
    return true;
}

void SourceBackend::mpegScan(int programNumber, std::map<int, int>& streamInfo){
    struct pollfd pollFd[2];
    int pmtFd;
    int patFd = createSectionFilter(PID_PAT, TABLE_PAT);
    pollFd[0].fd = patFd;                           // PAT
    pollFd[0].events = POLLIN|POLLPRI|POLLERR;
    pollFd[1].fd = 0;                               // PMT
    pollFd[1].events = 0;

    bool flag = true;;
    while (flag){
        int count = poll(pollFd, 2, 100);
                if (count < 0) {
            fprintf(stderr, "Poll error\n");
            break;
        }
        if (count == 0) {
            continue;
        }
        if (pollFd[0].revents & (POLLIN|POLLPRI)) {
            if (processPAT(pollFd[0].fd, programNumber, &pollFd[1], streamInfo)) {
                dvbdemux_stop(pollFd[0].fd);
            }
            else {
                flag = false;
            }
        }
        if (pollFd[1].revents & (POLLIN|POLLPRI)) {
            processPMT(pollFd[1].fd, streamInfo);
            dvbdemux_stop(pollFd[1].fd);
            flag = false;
        }
    }
}

tvcontrol_return SourceBackend::atscScan(int frequency, uint64_t modulation) {
    int vctFd;
    struct pollfd pollFd;
    tvcontrol_return ret = TVControlFailed;
    switch (modulation) {
    case DVBFE_ATSC_MOD_VSB_8:
        vctFd = createSectionFilter(PID_VCT, TABLE_VCT_TERR);
        break;
    default:
        printf("Modulation not supported!!\n");
        return ret;
    }
    pollFd.fd = vctFd;
    pollFd.events = POLLIN|POLLPRI|POLLERR;
    bool flag = true;;
    while (flag) {
        int count = poll(&pollFd, 1, 100);
                if (count < 0) {
            fprintf(stderr, "Poll error\n");
            break;
        }
        if (count == 0) {
            continue;
        }
        if (pollFd.revents & (POLLIN|POLLPRI)) {
            switch(modulation){
            case DVBFE_ATSC_MOD_VSB_8:
                if (processTVCT(pollFd.fd, frequency) ) {
                    flag = false;
                    dvbdemux_stop(pollFd.fd);
                    ret = TVControlSuccess;
                }
                else {
                    fprintf(stderr, "%s(): error calling parse_stt()\n", __FUNCTION__);
                }
                break;
            default:
                break;
            }
        }
    }
    return ret;
}

int SourceBackend::createSectionFilter(uint16_t pid, uint8_t tableId) {
    int demuxFd = -1;
    uint8_t filter[18];
    uint8_t mask[18];

    // open the demuxer
    if ((demuxFd = dvbdemux_open_demux(m_adapter, m_demux, 0)) < 0) {
        return -1;
    }

    // create a section filter
    memset(filter, 0, sizeof(filter));
    memset(mask, 0, sizeof(mask));
    filter[0] = tableId;
    mask[0] = 0xFF;
    if (dvbdemux_set_section_filter(demuxFd, pid, filter, mask, 1, 1)) {
        close(demuxFd);
        return -1;
    }

    // done
    return demuxFd;
}

bool SourceBackend::processPAT(int patFd, int programNumber, struct pollfd *pollfd, std::map<int, int>& streamInfo) {
    int size;
    uint8_t siBuf[4096];
    // read the section
    if ((size = read(patFd, siBuf, sizeof(siBuf))) < 0) {
        return false;
    }

    // parse section
    struct section *section = section_codec(siBuf, size);
    if (section == NULL) {
        return false;
    }
    // parse sectionExt
    struct section_ext *sectionExt = section_ext_decode(section, 0);
    if (sectionExt == NULL) {
        return false;
    }
    /*if (pat_version == sectionExt->version_number) { //TODO: recheck again
        return false;
    }*/

    // parse PAT
    struct mpeg_pat_section *pat = mpeg_pat_section_codec(sectionExt);
    if (pat == NULL) {
        return false;
    }

    // try and find the requested program
    struct mpeg_pat_program *cur_program;
    mpeg_pat_section_programs_for_each(pat, cur_program) {
        printf("Program Number:- %d PMT Pid:- %x\n", cur_program->program_number, cur_program->pid);
        if (cur_program->program_number == programNumber) {
            if (pollfd->fd != -1)
                    close(pollfd->fd);
            if ((pollfd->fd = createSectionFilter(cur_program->pid, TABLE_PMT)) < 0) {
                    return false;
            }
            pollfd->events = POLLIN|POLLPRI|POLLERR;
            streamInfo[0] = cur_program->pid;
            break;
        }
    }

    // remember the PAT version
    //pat_version = sectionExt->version_number;
    return true;
}

bool SourceBackend::processTVCT(int dmxfd, int frequency) {
    int numSections;
    uint32_t sectionPattern;
    struct atsc_tvct_section *tvct;
    struct atsc_tvct_channel *ch;
    int i, k, ret;
    struct section *section;
    struct section_ext *sectionExt;
    struct atsc_section_psip *psip;
    int size;
    unsigned char siBuf[4096];
    sectionPattern = 0;
    numSections = -1;
    do {
         /* read it */
        if ((size = read(dmxfd, siBuf, sizeof(siBuf))) < 0) {
            fprintf(stderr, "%s(): error calling read()\n", __FUNCTION__);
            return false;
        }

        section = section_codec(siBuf, size);
        if (NULL == section) {
            fprintf(stderr, "%s(): error calling section_codec()\n",
                __FUNCTION__);
            return false;
        }
        sectionExt = section_ext_decode(section, 0);
        if (NULL == sectionExt) {
            fprintf(stderr, "%s(): error calling section_ext_decode()\n",
                __FUNCTION__);
            return false;
        }

        psip = atsc_section_psip_decode(sectionExt);
        if (NULL == psip) {
            fprintf(stderr,
                "%s(): error calling atsc_section_psip_decode()\n",
                __FUNCTION__);
            return false;
        }

        tvct = atsc_tvct_section_codec(psip);
        if (NULL == tvct) {
            fprintf(stderr, "%s(): error decode table section\n",
                __FUNCTION__);
            return false;
        }

        if (-1 == numSections) {
            numSections = 1 + tvct->head.ext_head.last_section_number;
             if (32 < numSections) {
                fprintf(stderr, "%s(): no support yet for "
                    "tables having more than 32 sections\n",
                    __FUNCTION__);
                return false;
            }
        } else {
            if (numSections !=
                1 + tvct->head.ext_head.last_section_number) {
                fprintf(stderr,
                    "%s(): last section number does not match\n",
                    __FUNCTION__);
                return false;
            }
        }
        if (sectionPattern & (1 << tvct->head.ext_head.section_number)) {
            continue;
        }
        sectionPattern |= 1 << tvct->head.ext_head.section_number;

        atsc_tvct_section_channels_for_each (tvct, ch, i) {
            /* initialize the currInfo structure */
            /* each EIT covers 3 hours */
            ChannelBackend *currInfo = new ChannelBackend();
            char *serviceName = reinterpret_cast<char*>(malloc(sizeof(char)*8));
            for (k = 0; k < 7; k++) {
                serviceName[k] =
                    getBits((const uint8_t *)ch->short_name,
                    k * 16, 16);
            }
            serviceName[7] = '\0';

            int majorNum = ch->major_channel_number;
            int minorNum = ch->minor_channel_number;
            int logicalChannelNumber = majorNum << 10 | minorNum;

            printf("TSID : %d\n", ch->channel_TSID);
            printf("ProgramNumber : %d\n", ch->program_number);
            printf("LCN : %d\n",logicalChannelNumber);
            printf("ServiceID : %d\n",ch->source_id);

            currInfo->setNumber(logicalChannelNumber);
            currInfo->setTransportStreamId(to_string(ch->channel_TSID));
            currInfo->setServiceId(to_string(ch->source_id));
            currInfo->setProgramNumber(ch->program_number);
            currInfo->setFrequency(frequency);
            printf("Short Name : %s\n", serviceName);
            struct descriptor *desc;
            int i, j;
            atsc_tvct_channel_descriptors_for_each(ch, desc) {
                printf("Descriptor : %x\n",desc->tag);
                switch(desc->tag){
                case dtag_atsc_extended_channel_name:
                    parseAtscExtendedChannelNameDescriptor(&serviceName, const_cast<const unsigned char*>(reinterpret_cast<unsigned char*>(desc)));
                    break;
                default:
                    break;
                }
            }
            string name(serviceName);
            printf("Extended name : %s\n",name.c_str());
            currInfo->setName(name);
            m_channelList.push_back(currInfo);

            printf("Sending channel Info\n"); fflush(stdout);
            wpe_tvcontrol_channel* channelInfo = new wpe_tvcontrol_channel;
            channelInfo->networkId  = "";
            channelInfo->transportSId  = strdup(to_string(ch->channel_TSID).c_str());
            channelInfo->serviceId  = strdup(to_string(ch->source_id).c_str());
            channelInfo->name  = strdup(name.c_str());
            channelInfo->number  = logicalChannelNumber;
            TVControlPushEvent(ScanningChanged, m_tunerData->tunerId.c_str(), Scanned, channelInfo, m_tvControlBackend);
        }
    } while(sectionPattern != (uint32_t)((1 << numSections) - 1));

    return true;
}

void SourceBackend::parseAtscExtendedChannelNameDescriptor(char **serviceName, const unsigned char *buf) {
    unsigned char *b = (unsigned char *) buf+2;
    int i,j;
    int num_str = b[0];
    char *name = *serviceName;
    #define uncompressed_string 0x00

    b++;
    for(i = 0; i < num_str; i++) {
        int num_seg = b[3];
        b += 4; /* skip lang code */
        for(j = 0; j < num_seg; j++) {
           int compression_type = b[0],/* mode = b[1],*/ num_bytes = b[2];

            switch (compression_type) {
            case uncompressed_string:
                if (name)
                    free(name);
                name = reinterpret_cast<char*>(malloc(num_bytes * sizeof(char) + 1));
                memcpy(name,&b[3],num_bytes);
                name[num_bytes] = '\0';
                *serviceName = name;
                break;
            default:
                break;
            }
            b += 3 + num_bytes;
        }
    }
}


uint32_t SourceBackend::getBits(const uint8_t *buf, int startbit, int bitlen)
{
    const uint8_t *b;
    uint32_t mask,tmp_long;
    int bitHigh,i;

    b = &buf[startbit / 8];
    startbit %= 8;

    bitHigh = 8;
    tmp_long = b[0];
    for (i = 0; i < ((bitlen-1) >> 3); i++) {
        tmp_long <<= 8;
        tmp_long  |= b[i+1];
        bitHigh   += 8;
    }

    startbit = bitHigh - startbit - bitlen;
    tmp_long = tmp_long >> startbit;
    mask     = (1ULL << bitlen) - 1;
    return tmp_long & mask;
}

} // namespace BCMRPi
