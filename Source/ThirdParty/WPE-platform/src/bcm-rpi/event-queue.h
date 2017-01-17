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

#ifndef bcm_rpi_event_queue_h
#define bcm_rpi_event_queue_h

#include <mutex>
#include <condition_variable>
#include <deque>

template <typename T>
class EventQueue {
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_condition;
    std::deque<T> m_queue;

public:
    EventQueue()
        : m_condition(PTHREAD_COND_INITIALIZER)
        , m_mutex(PTHREAD_MUTEX_INITIALIZER)
    {
    }

    void pushEvent(T const& value)
    {
        pthread_mutex_lock(&m_mutex);
        m_queue.push_back(value);
        pthread_cond_signal(&m_condition);
        pthread_mutex_unlock(&m_mutex);
    }

    void pushPriorityEvent(T const& value)
    {
        pthread_mutex_lock(&m_mutex);
        m_queue.push_front(value);
        pthread_cond_signal(&m_condition);
        pthread_mutex_unlock(&m_mutex);
    }

    T getEvents()
    {
        pthread_mutex_lock(&m_mutex);
        while (m_queue.empty()) {
            pthread_cond_wait(&m_condition, &m_mutex);
        }
        T ret = m_queue.front();
        m_queue.pop_front();
        pthread_mutex_unlock(&m_mutex);
        return ret;
    }
};

#endif // bcm_rpi_event_queue_h