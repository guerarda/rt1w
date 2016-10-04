#include "wqueue.hpp"

#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "sync.h"

static void work();

struct _job {
    sptr<Object>  m_obj;
    sptr<Object>  m_arg;
    wqueue_func   m_func;
    _job         *m_next;
};

struct _wqueue {

    _wqueue(uint32_t concurrency);
    ~_wqueue();

    void  enqueue(_job *);
    _job *dequeue();

    uint32_t                 m_concurrency;
    void * volatile          m_head;
    void * volatile          m_queue;
    void * volatile          m_waiting;
    std::mutex               m_mutex;
    std::condition_variable  m_cv;
    std::vector<std::thread> m_threads;
};

_wqueue::_wqueue(uint32_t concurrency)
{
    m_concurrency = concurrency;
    m_head        = nullptr;
    m_queue       = nullptr;
    m_waiting     = nullptr;

    for (size_t i = 0; i < m_concurrency; i++) {
        m_threads.emplace_back(std::thread(work));
    }
}

void _wqueue::enqueue(_job *job)
{
    _job *q = (_job *)sync_lock_ptr(&m_queue);
    job->m_next = q;
    if (m_waiting) {
        sync_xchg_ptr(&m_waiting, this);
        m_cv.notify_all();
    }
    sync_unlock_ptr(&m_queue, job);
}

_job *_wqueue::dequeue()
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
                sync_cmpxchg_ptr(&m_waiting, this, nullptr);
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

static _wqueue *wqueue = new _wqueue(std::thread::hardware_concurrency() + 1);

static void work()
{
    while (1) {
        _job *job = wqueue->dequeue();
        if (job->m_func) {
            job->m_func(job->m_obj, job->m_arg);
        }
        delete job;
    }
}

void wqueue_execute(wqueue_func func,
                    const sptr<Object> &obj,
                    const sptr<Object> &arg)
{
    _job *job = new _job;
    job->m_arg = arg;
    job->m_obj = obj;
    job->m_func = func;
    wqueue->enqueue(job);
}
