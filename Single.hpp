#ifndef SINGLE_HPP
#define SINGLE_HPP

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <memory>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <thread>
#include <future>

using namespace std;

template<typename T>
class Singleton {
public:
    //
    static T &GetInstance() {
        static T instance;
        return instance;
    }
    //
    Singleton(T &&) = delete;
    //
    Singleton(const T &) = delete;
    //
    void operator= (const T &) = delete;

protected:
    //
    Singleton() = default;
    //
    virtual ~Singleton() = default;
};

//
template<typename T, bool is_thread_safe = true>
class LazySingleton {
private:
    //
    static unique_ptr<T> t_;
    //
    static mutex mtx_;

public:
    //
    static T &GetInstance() {
        if (is_thread_safe == false) {
            if (t_ == nullptr) {
                t_ = unique_ptr<T>(new T);
            }
            return *t_;
        }
        //
        if (t_ == nullptr) {
            unique_lock<mutex> unique_locker(mtx_);
            if (t_ == nullptr) {
                t_ = unique_ptr<T>(new T);
            }
            return *t_;
        }
        //
        return *t_;
    }
    //
    LazySingleton(T &&) = delete;
    //
    LazySingleton(const T &) = delete;
    //
    void operator= (const T &) = delete;

protected:
    //
    LazySingleton() = default;
    //
    virtual ~LazySingleton() = default;
};
//
template<typename T, bool is_thread_safe>
unique_ptr<T> LazySingleton<T, is_thread_safe>::t_;
//
template<typename T, bool is_thread_safe>
mutex LazySingleton<T, is_thread_safe>::mtx_;
//
//
//
template<typename T>
class EagerSingleton {
private:
    //
    static T *t_;

public:
    //
    static T &GetInstance() {
        return *t_;
    }
    //
    EagerSingleton(T &&) = delete;
    //
    EagerSingleton(const T &) = delete;
    //
    void operator= (const T &) = delete;

protected:
    //
    EagerSingleton() = default;
    //
    virtual ~EagerSingleton() = default;
};

//
template<typename T>
T *EagerSingleton<T>::t_ = new (std::nothrow) T;

#endif