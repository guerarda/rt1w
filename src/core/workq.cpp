#include "rt1w/workq.hpp"

#include "rt1w/error.h"
#include "rt1w/event.hpp"
#include "rt1w/sync.h"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

struct _job {
    sptr<Object> m_obj;
    sptr<Object> m_arg;
    workq_func m_func;
    sptr<Event> m_event;
    _job *m_next;
};

struct workq {
    workq(uint32_t concurrency) : m_concurrency(concurrency) {}

    void init();
    [[noreturn]] void work();

    void enqueue(_job *);
    _job *dequeue();

    uint32_t m_concurrency;
    void *volatile m_head = nullptr;
    void *volatile m_queue = nullptr;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::vector<std::thread> m_threads;
};

void workq::init()
{
    for (size_t i = 0; i < m_concurrency; i++) {
        m_threads.emplace_back(std::thread(&workq::work, this));
    }
}

void workq::work()
{
    while (true) {
        _job *job = dequeue();
        if (job->m_func) {
            job->m_func(job->m_obj, job->m_arg);
        }
        job->m_event->signal();
        delete job;
    }
}

void workq::enqueue(_job *job)
{
    _job *q = (_job *)sync_lock_ptr(&m_queue);
    job->m_next = q;
    m_cv.notify_one();
    sync_unlock_ptr(&m_queue, job);
}

_job *workq::dequeue()
{
    _job *job = nullptr;
    do {
        _job *head = (_job *)sync_lock_ptr(&m_head);
        if (head) {
            job = head;
            head = job->m_next;
            sync_unlock_ptr(&m_head, head);
            job->m_next = nullptr;
        }
        else {
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
            }
            else {
                sync_unlock_ptr(&m_queue, nullptr);
                sync_unlock_ptr(&m_head, nullptr);
                std::unique_lock<std::mutex> lk(m_mutex);
                m_cv.wait(lk);
            }
        }
    } while (!job);
    return job;
}

#pragma mark - Static

static workq *global_workq = []() -> workq * {
    auto *queue = new workq(std::thread::hardware_concurrency());
    queue->init();

    return queue;
}();

workq *workq_get_queue()
{
    return global_workq;
}

sptr<Event> workq_execute(workq *workq,
                          workq_func func,
                          const sptr<Object> &obj,
                          const sptr<Object> &arg)
{
    sptr<Event> event;
    if (workq) {
        _job *job = new _job;
        event = Event::create(1);
        job->m_arg = arg;
        job->m_obj = obj;
        job->m_func = func;
        job->m_event = event;
        workq->enqueue(job);
    }
    else if (func) {
        func(obj, arg);
        event = Event::create(0);
    }
    return event;
}

void workq_execute(workq *workq,
                   const sptr<Event> &event,
                   workq_func func,
                   const sptr<Object> &obj,
                   const sptr<Object> &arg)
{
    if (workq) {
        _job *job = new _job;
        job->m_arg = arg;
        job->m_obj = obj;
        job->m_func = func;
        job->m_event = event;
        workq->enqueue(job);
    }
    else if (func) {
        func(obj, arg);
        event->signal();
    }
}
