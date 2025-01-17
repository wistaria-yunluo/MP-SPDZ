/*
 * ThreadQueues.cpp
 *
 */

#include "ThreadQueues.h"

#include <assert.h>

int ThreadQueues::distribute(ThreadJob job, int n_items, int base,
        int granularity)
{
    find_available();
    return distribute_no_setup(job, n_items, base, granularity);
}

int ThreadQueues::find_available()
{
    available.clear();
    for (size_t i = 1; i < size(); i++)
        if (at(i)->available())
            available.push_back(i);
    return available.size();
}

int ThreadQueues::get_n_per_thread(int n_items, int granularity)
{
    int n_per_thread = n_items / (available.size() + 1) / granularity
            * granularity;
    return n_per_thread;
}

int ThreadQueues::distribute_no_setup(ThreadJob job, int n_items, int base,
        int granularity, const vector<void*>* supplies)
{
    int n_per_thread = get_n_per_thread(n_items, granularity);
    for (size_t i = 0; i < available.size(); i++)
    {
        if (supplies)
            job.supply = supplies->at(i);
        job.begin = base + i * n_per_thread;
        job.end = base + (i + 1) * n_per_thread;
        at(available[i])->schedule(job);
    }
    return base + available.size() * n_per_thread;
}

void ThreadQueues::wrap_up(ThreadJob job)
{
    for (int i : available)
        assert(at(i)->result().output == job.output);
}
