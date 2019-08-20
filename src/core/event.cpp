#include "rt1w/event.hpp"

#include "rt1w/error.h"
#include "rt1w/sync.h"
#include "rt1w/workq.hpp"

#include <memory>
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
    _notif(workq *q,
           const sptr<Event> &e,
           workq_func f,
           const sptr<Object> &obj,
           const sptr<Object> &arg) :
        m_queue(q),
        m_event(e),
        m_func(f),
        m_obj(obj),
        m_arg(arg)
    {}
    ~_notif() { workq_execute(m_queue, m_event, m_func, m_obj, m_arg); }

    workq *m_queue;
    sptr<Event> m_event;
    workq_func m_func;
    sptr<Object> m_obj;
    sptr<Object> m_arg;
    uptr<_notif> m_next = nullptr;
};

struct _Event : Event, std::enable_shared_from_this<Event> {
    _Event(int32_t n) : m_counter(n), m_token(n > 0 ? new token : nullptr) {}

    ~_Event() override
    {
        if (m_token) {
            delete (token *)m_token;
        }
    }
    sptr<Event> notify(workq *,
                       workq_func,
                       const sptr<Object> &,
                       const sptr<Object> &) override;
    int32_t signal() override;
    bool test() const override;
    int32_t wait() override;

    uptr<_lock> m_lock = nullptr;
    uptr<_notif> m_notif;
    int32_t volatile m_counter;
    void *volatile m_token;
};

sptr<Event> _Event::notify(workq *workq,
                           workq_func func,
                           const sptr<Object> &obj,
                           const sptr<Object> &arg)
{
    if (m_token) {
        void *token = sync_lock_ptr(&m_token);
        if (token) {
            auto event = Event::create(1);
            auto notif = std::make_unique<_notif>(workq, event, func, obj, arg);
            notif->m_next = std::move(m_notif);
            m_notif = std::move(notif);
            notif.reset();
            sync_unlock_ptr(&m_token, token);
            return event;
        }
        m_token = nullptr;
    }
    return workq_execute(workq, func, obj, arg);
}

int32_t _Event::signal()
{
    ASSERT(m_counter > 0);
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

static void signal_event(const sptr<Object> &obj, const sptr<Object> &)
{
    if (sptr<Event> e = std::dynamic_pointer_cast<Event>(obj)) {
        e->signal();
    }
}

#pragma mark - Static constructor

sptr<Event> Event::create(int32_t n)
{
    return std::make_shared<_Event>(n);
}

sptr<Event> Event::create(const std::vector<sptr<Event>> &events)
{
    auto re = Event::create((int32_t)events.size());
    for (auto &e : events) {
        e->notify(nullptr, signal_event, re, {});
    }
    return re;
}
