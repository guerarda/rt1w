#include "event.hpp"

#include "sync.h"
#include "workq.hpp"

#include <mutex>

struct token {
    void *foo;
};

struct _lock {
    _lock(std::mutex *mutex) : m_mutex(mutex), m_next(nullptr) {}
    ~_lock() { m_mutex->unlock(); }

    std::mutex *m_mutex;
    uptr<_lock> m_next;
};

static uptr<_lock> create_lock(std::mutex *mutex)
{
    return std::make_unique<_lock>(mutex);
}

struct _notif {
    _notif(workq *q, workq_func f, const sptr<Object> &obj, const sptr<Object> &arg);
    ~_notif();

    workq *m_queue;
    workq_func m_func;
    sptr<Object> m_obj;
    sptr<Object> m_arg;
    uptr<_notif> m_next;
};

static uptr<_notif> create_notif(workq *workq,
                                 workq_func func,
                                 const sptr<Object> &obj,
                                 const sptr<Object> &arg)
{
    return std::make_unique<_notif>(workq, func, obj, arg);
}

_notif::_notif(workq *q, workq_func f, const sptr<Object> &obj, const sptr<Object> &arg)
{
    m_queue = q;
    m_func = f;
    m_obj = obj;
    m_arg = arg;
    m_next = nullptr;
}

_notif::~_notif()
{
    if (m_func) {
        if (m_queue) {
            workq_execute(m_queue, m_func, m_obj, m_arg);
        }
        else {
            m_func(m_obj, m_arg);
        }
    }
}

struct _Event : Event {
    _Event(int32_t n);
    ~_Event() override;

    int32_t notify(workq *,
                   workq_func,
                   const sptr<Object> &,
                   const sptr<Object> &) override;
    int32_t signal() override;
    bool test() const override;
    int32_t wait() override;

    uptr<_lock> m_lock;
    uptr<_notif> m_notif;
    int32_t volatile m_counter;
    void *volatile m_token;
};

_Event::_Event(int32_t n)
{
    m_counter = n;
    m_lock = nullptr;
    m_token = n > 0 ? new token : nullptr;
}

_Event::~_Event()
{
    if (m_token) {
        delete (token *)m_token;
    }
}

int32_t _Event::notify(workq *workq,
                       workq_func func,
                       const sptr<Object> &obj,
                       const sptr<Object> &arg)
{
    if (m_token) {
        void *token = sync_lock_ptr(&m_token);
        if (token) {
            uptr<_notif> notif = create_notif(workq, func, obj, arg);

            notif->m_next = std::move(m_notif);
            m_notif = std::move(notif);
            notif.reset();
            sync_unlock_ptr(&m_token, token);
        }
        else {
            m_token = nullptr;
        }
    }
    return 0;
}

int32_t _Event::signal()
{
    if (sync_add_i32(&m_counter, -1) == 0) {
        /* Delete token which marks the event as completed.
         * Release the locks which will unlock the mutexes
         * and return from the wait functions
         */
        void *tkn = sync_lock_ptr(&m_token);
        m_token = nullptr;
        delete (token *)tkn;
        m_notif.reset();
        m_lock.reset();
    }
    return 0;
}

bool _Event::test() const
{
    return m_token == nullptr;
}

int32_t _Event::wait()
{
    if (m_token) {
        void *token = sync_lock_ptr(&m_token);
        if (token) {
            uptr<_lock> lock;
            std::mutex mutex;

            mutex.lock();
            lock = create_lock(&mutex);

            /* Add the lock to the list */
            lock->m_next = std::move(m_lock);
            m_lock = std::move(lock);
            lock.reset();

            sync_unlock_ptr(&m_token, token);

            /* Exit when the mutex has been unlocked,
               i.e. lock is destroyed */
            mutex.lock();
        }
        else {
            m_token = nullptr;
        }
    }
    return 0;
}

#pragma mark - Static constructor

sptr<Event> Event::create(int32_t n)
{
    return std::make_shared<_Event>(n);
}
