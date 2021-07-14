#ifndef RING_BUFFER_H
#define RING_BUFFER_H 1

#include <condition_variable>
#include <mutex>
#include <optional>
#include <vector>

enum class PopState {
    valid,
    overflow,
    timeout
};

template <typename T>
struct PopResult {
    std::optional<T> value;
    PopState state;
};

// RingBuffer provides thread-safe buffer with bounded size
template<typename T>
class RingBuffer {
public:
    RingBuffer(std::size_t size): data(size), write_pos(0), read_pos(0) {};

    // Push value to the back of the buffer
    // Return false if there is overflow
    bool push_back(const T& value);

    PopResult<T> pop_front();

    // Pop value from the buffer with timeout
    // Returns State::valid and value if value was retrieved within given timeout.
    // If buffer has overflow then State::overflow is returned immediately.
    // If no value was present before timeout expired then State::timeout is returned.
    template<typename Rep> PopResult<T> pop_front_wait(std::chrono::duration<Rep> timeout);
private:
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<T> data;
    std::size_t write_pos, read_pos;
};

template<typename T>
bool RingBuffer<T>::push_back(const T& value) {
    {
        std::unique_lock<std::mutex> lock(mtx);

        // exit immediately if ring buffer has been overflowed
        if (write_pos - read_pos > data.size()) {
            return false;
        }

        data.at(write_pos % data.size()) = value;
        write_pos++;
    }

    cv.notify_one();

    return true;
}

template<typename T>
PopResult<T> RingBuffer<T>::pop_front() {
    std::unique_lock<std::mutex> lock(mtx);

    // exit immediately if ring buffer has been overflowed
    if (write_pos - read_pos > data.size()) {
        return {std::nullopt, PopState::overflow};
    };

    cv.wait(lock, [this]{ return read_pos != write_pos; });

    auto res = data.at(read_pos % data.size());
    read_pos++;

    return {res, PopState::valid};
}

template<typename T> 
template<typename Rep> 
PopResult<T> RingBuffer<T>::pop_front_wait(std::chrono::duration<Rep> timeout) {
    std::unique_lock<std::mutex> lock(mtx);

    // exit immediately if ring buffer has been overflowed
    if (write_pos - read_pos > data.size()) {
        return {std::nullopt, PopState::overflow};
    };

    // wait till we have data to read
    if (!cv.wait_for(lock, timeout, [this]{ return read_pos != write_pos; })) {
        return {std::nullopt, PopState::timeout};
    }

    auto res = data.at(read_pos % data.size());
    read_pos++;

    return {res, PopState::valid};
};

#endif