#ifndef SERVER_BUFFER_H
#define SERVER_BUFFER_H 1

#include <condition_variable>
#include <deque>
#include <mutex>

// Buffer provides thread-safe buffer with unbounded size
template <typename T>
class Buffer {
public:
    Buffer() {};

    void emplace_back(T&& value);
    T pop_front();

    template <typename ValueT, typename Compare>
    void discard_until(const ValueT& value, Compare comp);
private:
    std::mutex mtx;
    std::condition_variable cv;
    std::deque<T> data;
};

template <typename T>
void Buffer<T>::emplace_back(T&& value) {
    {
        std::unique_lock lock(mtx);
        data.emplace_back(std::move(value));
    }

    cv.notify_one();
};

template <typename T>
T Buffer<T>::pop_front() {
    std::unique_lock lock(mtx);

    cv.wait(lock, [this]{ return !data.empty(); });

    auto res = data.front();
    data.pop_front();

    return res;
};

template <typename T>
template <typename ValueT, typename Compare>
void Buffer<T>::discard_until(const ValueT& value, Compare comp) {
    std::unique_lock lock(mtx);

    auto it = std::upper_bound(data.begin(), data.end(), value, comp);
    data.erase(data.begin(), it);
}

#endif