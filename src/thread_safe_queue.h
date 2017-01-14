/*		
 * Copyright 2016 w703710691d <w703710691d#163.com>
 * PowerOJ GPLv2		
 */

#include<bits/stdc++.h>
#include<mutex>

using namespace std;

template<class T>
class ThreadSafeQueue {
private:
    queue <T> q;
    mutex mut;
    condition_variable data_cond;
public:
    T GetFrontAndPop() {
      unique_lock <mutex> lk(mut);
      data_cond.wait(lk, [this] { return !this->q.empty(); });
      T ret = q.front();
      q.pop();
      return ret;
    }

    void push(T x) {
      unique_lock <mutex> lk(mut);
      q.push(x);
      data_cond.notify_one();
    }
};
