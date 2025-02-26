/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#ifndef SERVER_SCHEDULER_HPP
#define SERVER_SCHEDULER_HPP

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <queue>
#include <thread>

namespace server
{

using Task = std::function<void()>;

//!
class TaskLoop final
{
public:
  //! Begin the task loop.
  void Begin();
  //! End the task loop.
  void End();
  //! Queue a task.
  void Queue(const Task& task);

protected:
  //! Task queue mutex.
  std::mutex _queueMutex;
  //! Task queue notification.
  std::condition_variable _queueNotification;
  //! Task queue.
  std::queue<Task> _queue;
  //! A flag indicating whether the task loop should run.
  std::atomic<bool> _shouldRun;
};

//!
class Executor
{
public:
  virtual ~Executor()
  {}

  virtual void Begin()
  {}
  virtual void End()
  {}
  virtual void Synchronize()
  {}

  void Submit(const Task& task);

protected:
  TaskLoop _taskLoop;
};

//! Single-threaded task executor.
class SingleThreadedExecutor final
  : public Executor
{
public:
  ~SingleThreadedExecutor() override = default;

  void Begin() override;
  void End() override;
  void Synchronize() override;

private:
  std::thread _thread;
};

//! Pooled multi-threaded task executor.
class MultiThreadedExecutor final
  : public Executor
{
public:
  ~MultiThreadedExecutor() override = default;

  void Begin() override;
  void End() override;

private:
  std::vector<std::thread> _threadPool;
};

//! Task scheduler.
class Scheduler final
{
public:
  Scheduler();
  ~Scheduler();
  //! Runs a task on the main thread.
  //! @param task Task to run on a main thread.
  void RunOnMainThread(const Task& task);
  //! Runs a task on a available worker thread.
  //! @param task Task to run on a worker thread.
  void RunOnWorkerThread(const Task& task);

  //! @return The single threaded executor.
  SingleThreadedExecutor& GetMainThreadExecutor();

private:
  SingleThreadedExecutor _mainThreadExecutor;
  MultiThreadedExecutor _workerThreadExecutor;
};

} // namespace server

#endif // SERVER_SCHEDULER_HPP
