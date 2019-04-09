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
	int defaultThreadCount = 4;

	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	std::mutex mutex;
	std::condition_variable taskCV;
	std::condition_variable poolCV;

	bool isRunning = true;
	int working = 0;

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

	ThreadPool(const ThreadPool &&other) {
		init(other.workers.size());
	}

	ThreadPool& operator=(const ThreadPool &other) {
		init(other.workers.size());
	}

	ThreadPool& operator=(const ThreadPool &&other) {
		init(other.workers.size());
	}

	void enqueu(std::function<void()> task);

	void wait();
};

#endif
