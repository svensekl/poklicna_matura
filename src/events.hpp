#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include "storage/storage.hpp"


class EventQueue {
public:
	EventQueue();
	~EventQueue();

	event &peek();
	void pop();

	void push(const event &item);
	void push(event &&item);

	bool empty();

private:
	std::queue<event> queue_;
	std::mutex mutex_;
	std::condition_variable cond_;
};

#endif
