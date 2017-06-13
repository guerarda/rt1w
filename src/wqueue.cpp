#include "wqueue.hpp"
#include "event.hpp"
#include "sync.h"

#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

static void work();

struct _job {
    sptr<Object>  m_obj;
    sptr<Object>  m_arg;
    wqueue_func   m_func;
    sptr<Event>   m_event;
    _job         *m_next;
};

struct wqueue {

    wqueue(uint32_t concurrency);
    ~wqueue();

    void  enqueue(_job *);
    _job *dequeue();

    uint32_t                 m_concurrency;
    void * volatile          m_head;
    void * volatile          m_queue;
    std::mutex               m_mutex;
    std::condition_variable  m_cv;
    std::vector<std::thread> m_threads;
};

wqueue::wqueue(uint32_t concurrency)
{
    m_concurrency = concurrency;
    m_head        = nullptr;
    m_queue       = nullptr;

    for (size_t i = 0; i < m_concurrency; i++) {
        m_threads.emplace_back(std::thread(work));
    }
}

void wqueue::enqueue(_job *job)
{
    _job *q = (_job *)sync_lock_ptr(&m_queue);
    job->m_next = q;
    m_cv.notify_one();
    sync_unlock_ptr(&m_queue, job);
}

_job *wqueue::dequeue()
{
    _job *job = nullptr;
    do {
        _job *head = (_job *)sync_lock_ptr(&m_head);
        if (head) {
            job = head;
            head = job->m_next;
            sync_unlock_ptr(&m_head, head);
            job->m_next = nullptr;
        } else {
            job = (_job *)sync_lock_ptr(&m_queue);
            if (job) {
                sync_unlock_ptr(&m_queue, nullptr);
                while (job->m_next) {
                    _job *next = job->m_next;
                    job->m_next = head;
                    head = job;
                    job = next;
                }
                sync_unlock_ptr(&m_head, head);
            } else {
                sync_unlock_ptr(&m_queue, nullptr);
                sync_unlock_ptr(&m_head, nullptr);
                std::unique_lock<std::mutex> lk(m_mutex);
                m_cv.wait(lk);
            }
        }
    } while(!job);
    return job;
}

#pragma mark - Static

static wqueue *global_wqueue = new wqueue(std::thread::hardware_concurrency() + 1);

__attribute__((noreturn)) static void work()
{
    while (1) {
        _job *job = global_wqueue->dequeue();
        if (job->m_func) {
            job->m_func(job->m_obj, job->m_arg);
        }
        job->m_event->signal();
        delete job;
    }
}

wqueue *wqueue_get_queue()
{
    return global_wqueue;
}

sptr<Event> wqueue_execute(wqueue *wqueue,
                    wqueue_func func,
                    const sptr<Object> &obj,
                    const sptr<Object> &arg)
{
    sptr<Event> event;
    if (wqueue) {
        _job *job = new _job;
        event = Event::create(1);
        job->m_arg = arg;
        job->m_obj = obj;
        job->m_func = func;
        job->m_event = event;
        wqueue->enqueue(job);
    } else if (func) {
        func(obj, arg);
        event = Event::create(0);
    }
    return event;
}
