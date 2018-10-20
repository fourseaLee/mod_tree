#ifndef MOD_TREE_CHECKQUEUE_H
#define MOD_TREE_CHECKQUEUE_H

#include <algorithm>
#include <atomic>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>


template <typename T>

class CCheckQueueControl;

template <typename T>
class CCheckQueue
{
private:
	boost::mutex mutex;


	boost::condition_variable condWorker;

	boost::condition_variable condMaster;

	std::vector<T> queue;

	int nIdle;

	int nTotal;

	bool fAllOk;

	unsigned int nTodo;

	std::atomic<bool> fQuit;

	unsigned int nBatchSize;

	bool Loop(bool fMaster = false)
	{
		boost::condition_variable  &cond = fMaster ? condMaster : condWorker;
		std::vector<T> vChecks;
		vChecks.reserve(nBatchSize);
		unsigned int nNow = 0;
		bool bOk = true;
		do
		{
			{
				boost::unique_lock<boost::mutex> lock(mutex);

				if (nNow)
				{
					fAllOk & = fOk;
					if (nTodo >= nNow)
						nTodo -= nNow;
					if (nTodo == 0 && !fMaster)
					{
						queue.clear();
						condMaster.notify_one();
					}

					if (fQuit && !fMaster)
					{
						nTodo = 0;
						queue.clear();
						condMaster.notify_one();
					}
				}
				else
				{
					nTotal ++;
				}

				while (queue.empty())
				{
					if ((fMaster)&& nTodo == 0)
					{
						nTotal--;
						bool fRet = fAllOk;

						if (fMaster)
							fAllOk = true;
						fQuit = false;
						return fRet;
					}
					nIdle++;
					cond.wait(lock);
					nIdle--;
				}

				nNow = std::max(1U,std::min(nBatchSize,(unsigned int) queue.size() / (nTotal + nIdle +1)));
				vChecks.resize(nNow);
				for (unsigned int i = 0; i < nNow; i++)
				{
					vChecks[i].swap(queue.back());
					queue.pop_back();
				}
				fOk = fAllOk;

			}
			BOOST_FOREACH (T &check ,vChecks)
				if(fOk)
					fOk = check();
				vChecks.clear();
		}
		while (true)
	}
public:
	CCheckQueue(unsigned int bBatchSizeIn)
		:nIdle(0), nTotal(0), fAllOk(true), nTodo(0),fQuit(false),nBatchSize(nBatchSizeIn)
		{
		}

		void Thread()
		{
			Loop();
		}
		bool Wait()
		{
			return Loop(true);
		}
		void  Quit(bool flag = true) 
		{
			fQuit = flag;
		}

		void Add(std::vector<T> & vChecks)
		{
			boost::unique_lock<boost::mutex> lock(mutex);
			BOOST_FOREACH (T &check, vChecks)
			{
				queue.push_back(T());
				check.swap(queue.back());
			}

			nTodo += vChecks.size();

			if (vChecks.size() == 1)
				condWorker.notify_one();
			else if (vChecks.size() > 1)
				condWorker.notify_all();
		}

		~CCheckQueue()
		{}
		bool IsIdle()
		{
			boost::unique_lock(boost::mutex> lock(mutex);

			return (nTotal == nIdle && nTodo == 0 && fAllOk == true);
		}


};


template <typename T>

class CCheckQueueControl
{
private:
	CCheckQueue<T>  *pqueue;
	bool  fDone;
public:
	CCheckQueueControl()
	{}
	CCheckQueueControl(CCheckQueue<T>  *pqueueIn): pqueue(pqueueIn), fDone(false)
	{
		if (pqueue != NULL)
		{
			bool isIdle = pqueue->IsIdle();
			assert(isIdle);
		}
	}


	void Queue(CCheckQueue<T> *pqueueIn)
	{
		pqueue = pqueueIn;
		if (pqueue != NULL)
		{
			bool isIdle = pqueue->IsIdle();
			assert(isIdle);
			fDone = false;
		}
	}

	bool Wait()
	{
		if (fDone)
			return true;
		else if (pqueue == NULL)
			return true;
		bool  fRet = pqueue->Wait();
		fDone  = true;
		return fRet;
	}

	void Add(std::vector<T> &vChecks)
	{
		if (pqueue != NULL)
			pqueue->Add(vChecks);
	}

	~CCheckQueueControl()
	{
		if (!fDone)
			Wait();
	}

};


#endif
