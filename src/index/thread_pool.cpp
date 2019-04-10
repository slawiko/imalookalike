#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>

#include "thread_pool.h"

void ThreadPool::DoubleLock::lock() {
	if (!isLocked) {
		std::lock(mutex1, mutex2);

		isLocked = true;
	}
}

void ThreadPool::DoubleLock::unlock() {
	if (isLocked) {
		mutex1.unlock();
		mutex2.unlock();

		isLocked = false;
	}
}

void ThreadPool::init(int threadCount) {
	for (int i = 0; i < threadCount; ++i) {
		workers.push_back(std::thread([this]() {
			while (true) {
				std::function<void()> task;

				DoubleLock taskLock(runningMutex, tasksMutex);

				while (isRunning && tasks.empty()) {
					taskCV.wait(taskLock);
				}

				if (!isRunning) {
					break;
				}

				task = std::move(tasks.front());
				tasks.pop();

				taskLock.unlock();

				std::unique_lock<std::mutex> workingLock(workingMutex);
				working++;
				workingLock.unlock();

				task();

				workingLock.lock();
				working--;
				workingLock.unlock();

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
	wait();

	std::unique_lock<std::mutex> lock(runningMutex);
	isRunning = false;
	lock.unlock();

	taskCV.notify_all();

	for (std::thread &thread : workers) {
		thread.join();
	}
}

void ThreadPool::enqueu(std::function<void()> task) {
	std::unique_lock<std::mutex> lock(tasksMutex);
	tasks.push(task);
	lock.unlock();

	taskCV.notify_one();
}

void ThreadPool::wait() {
	DoubleLock lock(tasksMutex, workingMutex);

	while (tasks.size() > 0 || working > 0) {
		poolCV.wait(lock);
	}
}
