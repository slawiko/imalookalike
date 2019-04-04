#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>

#include "thread_pool.h"

void ThreadPool::init(int threadCount) {
	for (int i = 0; i < threadCount; ++i) {
		workers.push_back(std::thread([this]() {
			while (true) {
				std::function<void()> task;

				std::unique_lock<std::mutex> lock(mutex);

				while (isRunning && tasks.empty()) {
					taskCV.wait(lock);
				}

				if (!isRunning) {
					break;
				}

				task = std::move(tasks.front());
				tasks.pop();

				working++;

				lock.unlock();
				task();

				lock.lock();
				working--;
				poolCV.notify_one();
			}
		}));
	}
}

ThreadPool::ThreadPool() {
	int threadCount = std::thread::hardware_concurrency();
	init(threadCount ? threadCount : defaultThreadCount);
}

ThreadPool::~ThreadPool() {
	std::unique_lock<std::mutex> lock(mutex);

	isRunning = false;

	lock.unlock();
	taskCV.notify_all();

	for (std::thread &thread : workers) {
		thread.join();
	}
}

void ThreadPool::wait() {
	std::unique_lock<std::mutex> lock(mutex);

	while (working > 0) {
		poolCV.wait(lock);
	}
}
