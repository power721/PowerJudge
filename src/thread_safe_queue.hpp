//
// Created by w703710691d on 18-8-27.
//

#ifndef POWERJUDGE_THREAD_SAFE_QUEUE_HPP
#define POWERJUDGE_THREAD_SAFE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

template<class T>
class ThreadSafeQueue {
private:
    std::queue<T> q;
    std::mutex mut;
    std::condition_variable data_cond;
public:
    T GetFrontAndPop() {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !this->q.empty(); });
        T ret = q.front();
        q.pop();
        return ret;
    }

    void push(T x) {
        std::unique_lock<std::mutex> lk(mut);
        q.push(x);
        data_cond.notify_one();
    }
};

#endif //POWERJUDGE_THREAD_SAFE_QUEUE_HPP
