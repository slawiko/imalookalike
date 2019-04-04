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

	ThreadPool(ThreadPool &&other) {
		init(other.workers.size());
	}

	ThreadPool& operator=(const ThreadPool &other) {
		init(other.workers.size());
	}

	template<class Function, class ...Args>
	void enqueu(Function &&task, Args &&...args) {
		std::unique_lock<std::mutex> lock(mutex);

		tasks.push(std::bind(std::forward<Function>(task), std::forward<Args>(args)...));

		lock.unlock();
		taskCV.notify_one();
	}

	void wait();
};

#endif
