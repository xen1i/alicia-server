#include "server/Scheduler.hpp"

#include <condition_variable>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <thread>

#include <spdlog/spdlog.h>

namespace server
{

namespace
{

// holds the task_executor parameters
struct task_executor_param
{
  std::function<void()> task;
  std::condition_variable* cv;
};

// pthread start_routine function
void* task_executor(void* param)
{
  // make the thread cancellable
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
  // make the thread stop execution anywhere
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, nullptr);

  // cast parameters to the correct struct
  const auto p = static_cast<struct task_executor_param*>(param);

  p->task();
  p->cv->notify_one();

  return nullptr;
}

} // namespace

void TaskLoop::Begin()
{
  _shouldRun = true;

  while (true)
  {
    while (!_queue.empty())
    {
      // Poll the task from the queue.
      std::function<void()> task;
      task = _queue.front();
      _queue.pop();

      // Process the task.
      task();
    }

    if (_queue.empty())
    {
      if (!_shouldRun)
      {
        // Break the task loop if the task queue is empty
        // and the task loop should not run.
        break;
      }
    }

    // Wait for queue notification.
    std::unique_lock queueLock(_queueMutex);
    _queueNotification.wait(queueLock);
  }
}

void TaskLoop::End()
{
  _shouldRun = false;
  _queueNotification.notify_all();
}

void TaskLoop::Queue(const Task& task)
{
  {
    std::scoped_lock queueLock(_queueMutex);
    _queue.emplace(task);
  }

  _queueNotification.notify_all();
}

void Executor::Submit(const Task& task)
{
  _taskLoop.Queue(task);
}

void SingleThreadedExecutor::Begin()
{
  _thread = std::thread([this]()
  {
    _taskLoop.Begin();
  });
}

void SingleThreadedExecutor::End()
{
  _taskLoop.End();
}

void MultiThreadedExecutor::Begin()
{
  // todo
  // - spawn worker if no free worker available (unless max worker thread count reached)
  // - workers idle and poll the queue, and they despawn when they idle more than they work

  // std::condition_variable condition;
  //
  // task_executor_param param = {
  //   .task = task,
  //   .cv = &condition,
  // };
  //
  // pthread_t taskThread;
  // if (pthread_create(&taskThread, nullptr, &task_executor, &param) != 0)
  // {
  //   spdlog::error("pthread_create failed: {}", strerror(errno));
  //   return;
  // }
  //
  // if (timeout == 0)
  //   return;
  //
  // std::mutex mutex;
  // std::unique_lock lock(mutex);
  //
  // // wait for timeout milliseconds before terminating the execution thread
  // if (condition.wait_for(lock, std::chrono::milliseconds(timeout)) == std::cv_status::timeout)
  // {
  //   if (pthread_cancel(taskThread) != 0)
  //   {
  //     spdlog::error("pthread_cancel failed: {}", strerror(errno));
  //     exit(0XDEAD);
  //   }
  //
  //   void* ret;
  //
  //   // the only way to know if the thread was cancelled it to join with it, and compare the
  //   return
  //   // value to PTHREAD_CANCELED
  //   if (pthread_join(taskThread, &ret) != 0)
  //   {
  //     spdlog::error("pthread_join failed: {}", strerror(errno));
  //     exit(0XDEAD);
  //   }
  //
  //   if (ret == PTHREAD_CANCELED)
  //   {
  //     spdlog::error("task was cancelled, timeout reached");
  //     // free(ret);
  //     //  i'm pretty sure free needs to be called, but this throws sigsegv
  //     exit(0xDEAD);
  //   }
  //
  //   // free(&ret);
  // }
}

void MultiThreadedExecutor::End() {}

Scheduler::Scheduler()
{
  _mainThreadExecutor.Begin();
}

Scheduler::~Scheduler()
{
  _mainThreadExecutor.End();
}

void Scheduler::RunOnMainThread(const Task& task)
{
  _mainThreadExecutor.Submit(task);
}

void Scheduler::RunOnWorkerThread(const Task& task)
{
  // todo
  throw std::runtime_error("not implemented");
}

} // namespace server
