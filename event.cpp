#include "event.hpp"
#include "sync.h"
#include <mutex>

struct token {
    void *foo;
};

struct _lock {

    _lock(std::mutex *mutex) : m_mutex(mutex), m_next(nullptr) { }
    ~_lock() { m_mutex->unlock(); }

    std::mutex *m_mutex;
    sptr<_lock> m_next;
};

static sptr<_lock> create_lock(std::mutex *mutex)
{
    return std::make_shared<_lock>(mutex);
}

struct _event : event {

    _event(uint32_t n);
    ~_event();

    int32_t signal();
    bool test() const;
    int32_t wait();

    sptr<_lock>      m_lock;
    int32_t volatile m_counter;
    void * volatile  m_token;
};

_event::_event(uint32_t n)
{
    m_counter = n;
    m_token = new token;
    m_lock = nullptr;
}

_event::~_event()
{
    if (m_token) {
        delete (token *)m_token;
    }
    m_lock = nullptr;
}

int32_t _event::signal()
{
    if (sync_add_i32(&m_counter, -1) == 0) {
        /* Delete token which marks the event as completed.
         * Release the locks which will unlock the mutexes
         * and return from the wait functions
         */
        void *tkn = sync_lock_ptr(&m_token);
        m_token = nullptr;
        delete (token *)tkn;
        m_lock.reset();
    }
    return 0;
}

bool _event::test() const
{
    return m_token == nullptr;
}

int32_t _event::wait()
{
    if (m_token) {
        void *token = sync_lock_ptr(&m_token);
        if (token) {
            sptr<_lock> lock;
            std::mutex mutex;

            mutex.lock();
            lock = create_lock(&mutex);

            /* Add the lock to the list */
            lock->m_next = m_lock;
            m_lock = lock;
            lock.reset();

            sync_unlock_ptr(&m_token, token);

            /* Exit when the mutex has been unlocked,
               i.e. lock is destroyed */
            mutex.lock();
        } else {
            m_token = nullptr;
        }
    }
    return 0;
}

#pragma mark - Static constructor

sptr<event> event::create(uint32_t n)
{
    return std::make_shared<_event>(n);
}
