//
// Created by carlo on 2024-11-24.
//

#ifndef OBSERVER_SYSTEM_HPP
#define OBSERVER_SYSTEM_HPP
namespace systems
{

class watcher
{
  public:
	virtual ~watcher()           = default;
	virtual void update_watcher() = 0;
};

class subject
{
  public:
	virtual ~subject()                    = default;
	virtual void attach(watcher *watcher) = 0;
	virtual void notify()                 = 0;
	virtual void detach(watcher *watcher) = 0;
};

}        // namespace systems
#endif        // OBSERVER_SYSTEM_HPP
