/*
 * Copyright (C) 2017 TATA ELXSI
 * Copyright (C) 2017 Metrological
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CHANNEL_BACKEND_H_
#define CHANNEL_BACKEND_H_

#include <string>
#include <wpe/tvcontrol-backend.h>

using namespace std;

namespace BCMRPi {

class ChannelBackend {

public:
    ChannelBackend() {}
    virtual ~ChannelBackend() {}

    void setNetworkId(uint64_t networkId) { m_networkId = networkId; }
    void setTransportStreamId(uint64_t transportStreamId) { m_transportStreamId = transportStreamId; }
    void setServiceId(uint64_t serviceId) { m_serviceId = serviceId; }
    void setName(string name) { m_name = name; }
    void setNumber(uint64_t number) { m_number = number; }
    void setProgramNumber(int programNumber) { m_programNumber = programNumber; }
    void setFrequency(int frequency) { m_frequency = frequency; }

    uint64_t getLCN() const { return m_number; }
    int getFrequency() const { return m_frequency; }
    int getProgramNumber() const { return m_programNumber; }
    string getName() const { return m_name; }
    uint64_t getServiceId() const { return m_serviceId; }
    uint64_t getTransportStreamId() const { return m_transportStreamId; }
    uint64_t getNetworkId() const { return m_networkId; }

private:
    string m_name;
    uint64_t m_networkId;
    uint64_t m_serviceId;
    uint64_t m_transportStreamId;
    uint64_t m_number;
    int m_programNumber;
    int m_frequency;

    bool m_isEmergency;
    bool m_isFree;
    bool m_isParentalLocked;
};

} // namespace BCMRPi

#endif // CHANNEL_BACKEND_H_