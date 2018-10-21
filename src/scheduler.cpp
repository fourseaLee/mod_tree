#include "scheduler.h"

#include <assert.h>
#include <boost/bind.hpp>
#include <utility>


CScheduler::CScheduler() :nThreadsServicingQueue(0),stopRequested(false),stopEmpty(false)
{}
CScheduler::~CScheduler()
{
	assert(nThreadsServicingQueue == 0);
}

void CScheduler::serviceQueue()
{
	boost::unique_lock<boost::mutex> lock(newTaskMutex);

	++nThreadsServicingQueue;
	while(!shouldStop())
	{
		try
		{
			while (!shouldStop() && taskQueue.empty())
			{
				newTaskScheduled.wait(lock);
			}


			if (shouldStop() || taskQueue.empty())
				continue;

			Function f = taskQueue.begin()->second;
			taskQueue.erase(taskQueue.begin())
			{
				reverse_lock<lock::unique_lock<boost::mutex> >rlock(lock);
				f();
			}
		}
		catch (...)
		{
			--nThreadServicingQueue;
			throw;
		}
	}

	--nThreadsServicingQueue;
	newTaskScheduled.notify_one();
}


void CScheduler::stop(bool drain)
{
	{
		boost::unique_lock<boost::mutex> lock(newTaskMutex);
		if (drain)
			stopWhenEmpty = true;
		else 
			stopRequested = true;
	}
	newTaskScheduled.notify_all();
}


void CScheduler::schedule(CScheduler::Function f,boost::chrono::system_clock::time_point t)
{
	{
		boost::unique_lock<boost::mutex> lock(newTaskMutex);
		taskQueue.insert(std::make_pair(t,f));
	}
	newTaskScheduled.notify_one();
}

void CScheduler::scheduleFromNow(CScheduler::Funtion f,int64_t deltaSeconds)
{
	f();
	s->scheduleFromNow(boost::bind(&Repeat,s,f,deltaSeconds),deltaSeconds);
}

void CScheduler::scheduleEvery(CScheduler::Function f,int64_t deltaSeconds)
{
	scheduleFromNow(boost::bind(&Repeat,this,f,deltaSeconds),deltaSeconds);
}

size_t CScheduler::getQueueInfo(boost::chrono::system_clock::time_point *first,
	boost::chrono::system_clock::time_point &last) const

{
	boost::unique_lock<boost::mutex> lock(newTaskMutex);

	size_t result  = taskQueue.size();
	if(!taskQueue.empty())
	{
		first = taskQueue.begin()->first;
		last = taskQueue.rbegin()->first;
	}
	return result;
}
