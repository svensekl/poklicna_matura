#include "events.hpp"

EventQueue::EventQueue() {}

EventQueue::~EventQueue() {}

event &EventQueue::peek() {
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty()) {
		cond_.wait(mlock);
	}
	return queue_.front();
}

void EventQueue::pop() {
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty()) {
		cond_.wait(mlock);
	}
	queue_.pop();
}

void EventQueue::push(const event &item) {
	std::unique_lock<std::mutex> mlock(mutex_);
	queue_.push(item);
	mlock.unlock();     // unlock before notificiation to minimize mutex con
	cond_.notify_one(); // notify one waiting thread
}

void EventQueue::push(event &&item) {
	std::unique_lock<std::mutex> mlock(mutex_);
	queue_.push(std::move(item));
	mlock.unlock();     // unlock before notificiation to minimize mutex con
	cond_.notify_one(); // notify one waiting thread
}

bool EventQueue::empty() {
	std::unique_lock<std::mutex> mlock(mutex_);
	bool empty = queue_.empty();
	mlock.unlock();
	return empty;
}
