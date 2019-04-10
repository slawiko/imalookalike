#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <functional>
#include <utility>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool {
	class DoubleLock {
		std::mutex &mutex1;
		std::mutex &mutex2;

		bool isLocked = false;

	public:

		DoubleLock(std::mutex &mutex1, std::mutex &mutex2) : mutex1(mutex1), mutex2(mutex2) {
			lock();
		};

		~DoubleLock() {
			unlock();
		}

		DoubleLock(const DoubleLock&) = delete;
		DoubleLock& operator=(const DoubleLock&) = delete;

		void lock();
		void unlock();
	};

	int defaultThreadCount = 4;

	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	bool isRunning = true;
	int working = 0;

	std::mutex tasksMutex;
	std::mutex runningMutex;
	std::mutex workingMutex;
	std::condition_variable_any taskCV;
	std::condition_variable_any poolCV;

	void init(int threadCount);

public:
	ThreadPool();
	~ThreadPool();

	ThreadPool(int threadCount) {
		init(threadCount);
	}

	ThreadPool(const ThreadPool &other) {
		init(other.workers.size());
	}

	ThreadPool(ThreadPool &&other) {
		init(other.workers.size());
	}

	ThreadPool& operator=(const ThreadPool &other) {
		init(other.workers.size());
	}

	void enqueu(std::function<void()> task);

	void wait();
};

#endif
