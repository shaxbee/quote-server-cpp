#ifndef DISPATCHER_H
#define DISPATCHER_H 1

#include <condition_variable>
#include <mutex>
#include <optional>
#include <set>
#include <tuple>

#include "ring_buffer.h"

template<typename T>
class Subscriber;

template<typename T>
class Dispatcher {
public:
    Dispatcher(std::size_t size): size(size) { };

    // Subscribe creates subscriber that dispatcher will forward values to.
    // When subscriber is destroyed then associated buffer is removed.
    std::shared_ptr<Subscriber<T>> subscribe();

    // Dispatch forwards values to all subscribers.
    // If subscriber was destroyed or has overflowed then associated buffer will be removed.
    void dispatch(const T& value);

private:
    std::mutex mtx;
    std::size_t size;
    // NOTE: std::set is used instead of std::unordered_set due to the fact that weak_ptr cannot be hashed
    std::set<std::weak_ptr<Subscriber<T>>, std::owner_less<std::weak_ptr<Subscriber<T>>>> subscribers;
};


template<typename T>
class Subscriber {
public:
    bool push(const T& value);
    template<typename Rep> PopResult<T> pop(std::chrono::duration<Rep> timeout);

private:
    friend class Dispatcher<T>;

    Subscriber(Dispatcher<T>& dispatcher, std::size_t size) noexcept: dispatcher(dispatcher), buffer(size) {};

    Dispatcher<T>& dispatcher;
    RingBuffer<T> buffer;
};

template<typename T>
std::shared_ptr<Subscriber<T>> Dispatcher<T>::subscribe() {
    std::unique_lock<std::mutex> lock(mtx);

    auto subscriber = std::shared_ptr<Subscriber<T>>(new Subscriber<T>{*this, size});
    subscribers.insert(subscriber);

    return subscriber;
}

template<typename T> 
void Dispatcher<T>::dispatch(const T& value) {
    std::unique_lock<std::mutex> lock(mtx);
   
    // push message to subscribers, removing those that are expired or overflowed
    for (auto it = subscribers.cbegin(); it != subscribers.cend();) {
        auto subscriber = it->lock();
        if (!subscriber || !subscriber->push(value)) {
            it = subscribers.erase(it);
        } else {
            it++;
        }
    }
};

template<typename T>
bool Subscriber<T>::push(const T& value) {
    return buffer.push(value);
}

template<typename T> 
template<typename Rep> 
PopResult<T> Subscriber<T>::pop(std::chrono::duration<Rep> timeout) {
    return buffer.pop_wait(timeout);
};

#endif