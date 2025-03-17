#include "server/Scheduler.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>

#include <spdlog/spdlog.h>

namespace server
{

void TaskLoop::Begin()
{
  _shouldRun = true;

  while (true)
  {
    std::unique_lock queueLock(_queueMutex);

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

} // namespace server
