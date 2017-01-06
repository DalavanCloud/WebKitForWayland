#ifndef bcm_rpi_event_queue_h
#define bcm_rpi_event_queue_h

#include <mutex>
#include <condition_variable>
#include <deque>

template <typename T>
class EventQueue
{
private:
    pthread_mutex_t         m_mutex;
    pthread_cond_t          m_condition;
    std::deque<T>           m_queue;
public:
    EventQueue(): m_condition(PTHREAD_COND_INITIALIZER), 
                  m_mutex(PTHREAD_MUTEX_INITIALIZER) {}

    void pushEvent(T const& value) {
        pthread_mutex_lock(&m_mutex);
        m_queue.push_back(value);
        pthread_cond_signal(&m_condition);
        pthread_mutex_unlock(&m_mutex);
    }
    
    void pushPriorityEvent(T const& value) {
        pthread_mutex_lock(&m_mutex);
        m_queue.push_front(value);
        pthread_cond_signal(&m_condition);
        pthread_mutex_unlock(&m_mutex);
    }
    
    T getEvents() {
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
