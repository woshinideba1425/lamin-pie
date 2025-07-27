#ifndef __ASYNC_DEQUE_HPP__
#define __ASYNC_DEQUE_HPP__

#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>
#include <future>
#include <type_traits>
#include <memory>
#include <iostream>

namespace Assist {
    namespace AsyncDeque {
        template<typename T, typename R = bool>
        struct AsyncWorkerDequeItem {
            T item;
            std::function<R(T&)> process;
            std::function<void(const T&, const R&)> callback;
            std::function<void(const T&)> error;
            std::condition_variable cv;
            std::mutex resultx;
            bool completed = false;
            R result;
            
            // 默认构造函数，不初始化 item
            AsyncWorkerDequeItem() = default;
            
            // 使用完美转发构造函数，直接初始化 item
            template<typename U, 
                    typename = std::enable_if_t<!std::is_same_v<std::decay_t<U>, AsyncWorkerDequeItem>>>
            explicit AsyncWorkerDequeItem(U&& i) : item(std::forward<U>(i)) {}
        };

        template<typename T, typename R = bool>
        class AsyncWorkerDeque {
        private:
            std::deque<std::shared_ptr<AsyncWorkerDequeItem<T, R>>> m_deque;
            std::mutex m_mutex;
            std::condition_variable m_condition;
            std::atomic<bool> m_running;
            std::thread m_thread;
            std::atomic<bool> m_stop;

        public:
            AsyncWorkerDeque() : m_running(true), m_stop(false) {
                m_thread = std::thread([this]() {
                    while (m_running) {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        m_condition.wait(lock, [this]() { return !m_deque.empty() || m_stop; });
                        if (m_stop) {
                            break;
                        }
                        auto workItem = m_deque.front();
                        m_deque.pop_front();
                        lock.unlock();
                        
                        R result{};
                        bool success = false;
                        
                        if (workItem->process) {
                            try {
                                result = workItem->process(workItem->item);
                                success = true;
                            } catch (...) {
                                success = false;
                                if (workItem->error) {
                                    workItem->error(workItem->item);
                                }
                            }
                            
                            if (success && workItem->callback) {
                                workItem->callback(workItem->item, result);
                            } else if (!success && workItem->error) {
                                workItem->error(workItem->item);
                            }
                        }
                        
                        {
                            std::unique_lock<std::mutex> resultLock(workItem->resultx);
                            workItem->completed = true;
                            workItem->result = result;
                        }
                        workItem->cv.notify_one();
                    }
                });
            }

            ~AsyncWorkerDeque() {
                m_stop = true;
                m_condition.notify_all();
                if(m_thread.joinable()) {
                    m_thread.join();
                }
            }

            void push(std::shared_ptr<AsyncWorkerDequeItem<T, R>> item) {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_deque.push_back(item);
                m_condition.notify_one();
            }

            template<typename U>
            std::pair<bool, std::pair<T, R>> submitAndWait(
                                        U&& item, 
                                        std::function<R(T&)> process,
                                        std::function<void(const T&, const R&)> callback = nullptr,
                                        std::function<void(const T&)> error = nullptr) {
                auto workItem = createWorkItem(std::forward<U>(item), process, callback, error);
                
                push(workItem);
                
                {
                    std::unique_lock<std::mutex> lock(workItem->resultx);
                    workItem->cv.wait(lock, [&]() { return workItem->completed; });
                }
                
                return std::make_pair(true, std::make_pair(std::move(workItem->item), std::move(workItem->result)));
            }

            template<typename U>
            std::shared_ptr<AsyncWorkerDequeItem<T, R>> createWorkItem(
                                U&& item,
                                std::function<R(T&)> process,
                                std::function<void(const T&, const R&)> callback = nullptr,
                                std::function<void(const T&)> error = nullptr) {
                // 直接在构造时初始化 item
                auto workItem = std::make_shared<AsyncWorkerDequeItem<T, R>>(std::forward<U>(item));
                workItem->process = process;
                workItem->callback = callback;
                workItem->error = error;
                return workItem;
            }

            void stop() {
                m_stop = true;
                m_condition.notify_all();
                if(m_thread.joinable()) {
                    m_thread.join();
                }
            }
        };
    }
}

#endif