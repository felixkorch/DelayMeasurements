//===- DispatchSchedule -----------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Dispatcher that triggers scheduled tasks.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_DISPATCHSCHEDULE_H
#define REQSERVER_DISPATCHSCHEDULE_H

#include "reqserver/Time.h"
#include <algorithm>
#include <forward_list>
#include <memory>

namespace reqserver
{

///===--------------------------------------------------------------------===///
/// class: BaseTask
///
/// Base of objects that are to be stored in a 'DispatchSchedule'.
class BaseTask
{
  const Clock::time_point exc_time_point;

public:
  BaseTask() {}
  BaseTask(Clock::time_point t) : exc_time_point(t) {}

  Clock::time_point time() const
  {
    return exc_time_point;
  }

  virtual void trigger() const {}
};

///===--------------------------------------------------------------------===///
/// class: Task<Lambda> : public BaseTask
///
/// Subclass of 'BaseTask' that overrides the virtual function 'trigger' with
/// a stored clousure object. This way it can be used in a 'DispatchSchedule'.
template<typename Lambda>
class Task : public BaseTask
{
  Lambda f;

public:
  Task(Clock::time_point t, const Lambda& f) : BaseTask(t), f(f) {}

  void trigger() const
  {
    f();
  }
};

///===--------------------------------------------------------------------===///
/// func: auto unique_task<typename Lambda>
///   @t: Point in time the task should be triggered.
///   @f: Lambda to be triggered.
///
/// Helper function creating unique pointers to 'Task' objects.
template<typename Lambda>
auto unique_task(const Clock::time_point& t, const Lambda& f)
{
  return std::make_unique<Task<Lambda>>(t, f);
}

///===--------------------------------------------------------------------===///
/// class: DispatchSchedule
///
class DispatchSchedule
{
  bool ready;
  int proc_count;
  std::forward_list<std::unique_ptr<const BaseTask>> list;

public:
  DispatchSchedule() : ready(true), proc_count(0) {}

  auto schedule(std::unique_ptr<const BaseTask>&& element)
  {
    if (list.empty()) {
      list.push_front(std::move(element));
      return list.cbegin();
    }
    auto it = list.cbefore_begin();
    for (auto end = list.cend(); std::next(it) != end; ++it) {
      if (element->time() < (*std::next(it))->time())
        break;
    }
    list.insert_after(it, std::move(element));
    return std::next(it);
  }

  template<typename Lambda>
  auto schedule(Clock::time_point time, const Lambda& f)
  {
    return schedule(unique_task(time, f));
  }

  auto trigger_until(const Clock::time_point& time)
  {
    ready = false;

    decltype(list) execute_list;
    execute_list.splice_after(
        execute_list.cbefore_begin(), list, list.cbefore_begin(),
        std::find_if(list.cbegin(), list.cend(),
                     [time](auto& x) { return x->time() > time; }));
    ready = true;
    ++proc_count;
    std::for_each(execute_list.cbegin(), execute_list.cend(),
                  [](auto& x) { x->trigger(); });
    --proc_count;
    return std::distance(execute_list.cbegin(), execute_list.cend());
  }

  auto is_empty() const
  {
    return list.empty();
  }

  auto is_ready() const
  {
    return ready;
  }

  auto is_running() const
  {
    return proc_count > 0;
  }
};

} // namespace reqserver

#endif // REQSERVER_DISPATCHSCHEDULE_H
