#ifndef MOD_TREE_SCHEDULER_H
#define MOD_TREE_SCHEDULER_H
#include <boost/chrono/chrono.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <map>

class CScheduler
{
public:
	CScheduler();
	~CScheduler();
	 typedef  boost::funtion<void(void)> Function;
	 void schedule(Function f ,boost::chrono::system_lock::time_point t);

	 void scheduleFromNow(Function f ,int64_t deltaSeconds);

	 void scheduleEvery(Function f, int64_t deltaSeconds);

	 void serviceQueue();

	 void stop(bool drain = false);

	 size_t getQueueInfo(boost::chrono::system_lock::time_point& first,
	 	boost::chrono::system_clock::time_point& last) const;
private:
	std::multimap<boost::chrono::system_clock::time_point,Function> taskQueue;
	boost::condition_variable newTaskScheduled;
	mutable boost::mutex newTaskMutex;
	int nThreadServicingQueue;
	bool stopRequested;
	bool stopWhenEmpty;
	bool shouldStop()
	{
		return stopRequested || (stopWhenEmpty && taskQueue.empty());
	}
};

#endif
