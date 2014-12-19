/*
 * Runtime.h
 *
 *  Created on: 2013-7-17
 *      Author: Administrator
 */

#ifndef RUNTIME_H_
#define RUNTIME_H_

#include <common.h>
#include <Database.h>
#include <UUIDGenerator.h>

namespace zertcore{ namespace base{

using namespace utils;

template <typename config>
class RuntimeT;
/**
 *
 * ExpiredTimer
 *
 */
template <class TimeHost = RuntimeT<void> >
class ExpiredTimerT
{
public:
	typedef TimeHost						time_host;
public:
	ExpiredTimerT() : interval_(0) {
		;
	}
	ExpiredTimerT(time_type interval) : interval_(interval) {
		last_update_ = now();
	}

public:
	operator bool() const {
		return interval_;
	}

public:
	bool expired(bool reset_flag = true) const {
		if (interval_ && now() - last_update_ >= interval_) {
			last_update_ = now();

			if (reset_flag) {
				reset();
			}
			else {
				interval_ = 0;
			}
			return true;
		}

		return false;
	}

	void set(time_type interval) const {
		reset(interval);
	}

	void reset(time_type interval = 0) const {
		if (interval)
			interval_ = interval;
		last_update_ = now();
	}

	time_type getInterval() const {
		return interval_;
	}

	time_type lastUpdate() const {
		return last_update_;
	}

private:
	time_type now() const {
		return time_host::Instance().now();
	}

private:
	mutable time_type			interval_;
	mutable time_type			last_update_;
};

typedef ExpiredTimerT<RuntimeT<void> >		ExpiredTimer;

/**
 * load and save config
 * load and save runtime contents
 */
template <typename config>
class RuntimeT
{
	typedef RuntimeT						self;

public:
	typedef self*							ptr;
	typedef self&							reference;

public:
	typedef string							config_key_type;

public:
	typedef function<bool (const time_type& interval)>
											updater_handler_type;
public:
	class UpdaterBase
	{
	public:
		UpdaterBase() : client_(Null), updater_added_(false), updater_once_flag_(false) {;}
		virtual ~UpdaterBase()				{;}
	public:
		virtual void enableUpdate(time_type interval = 0)	= 0;
		virtual void disableUpdate()						= 0;
		virtual void disableOnce()							= 0;

	protected:
		void setEnabledFlag(bool flag) {
			updater_added_ = flag;
		}
		void setOnceEnabledFlag(bool flag) {
			updater_once_flag_ = flag;

			/**
			if (flag)
				updater_once_amount_++;
			else if (updater_once_amount_ > 0) {
				updater_once_amount_--;
			}
			*/
		}

	public:
		bool isEnabled() const {
			return updater_added_;
		}
		bool isOnceEnabled() const {
			return updater_once_flag_;
		}
	private:
		UpdaterBase*			client_;
		bool					updater_added_;
		bool					updater_once_flag_;
	};

	typedef struct UpdaterType {
		updater_handler_type	handler;
		time_type				deadtime;
		time_type				interval;
		bool					once;
		UpdaterBase*			client;

		bool operator < (const UpdaterType& rhs) const {
			/**
			if (deadtime == rhs.deadtime) {
				return (int64)client < (int64)rhs.client;
			}
			*/
			return deadtime < rhs.deadtime;
		}

		bool operator == (const UpdaterType& rhs) const {
			return client == rhs.client;
		}
	}										updater_type;

	struct UpdaterHandlerLess :
			binary_function <updater_type, updater_type, bool>
	{
		bool operator() (updater_type const& x, updater_type const& y) const {
			return x.deadtime < y.deadtime;
		}
	};

public:
	typedef list<updater_type/*,UpdaterHandlerLess*/>
											updater_list_type;
	typedef typename updater_list_type::iterator
											updater_connection_type;
	typedef function<void ()>				init_handler_type;
	typedef function<void ()>				deinit_handler_type;

	typedef list<init_handler_type>			init_handler_list_type;
	typedef list<deinit_handler_type>		deinit_handler_list_type;

public:
	inline static reference Instance() {
		return instance_;
	}

public:
	RuntimeT() : now_(), last_update_(now_),
		expiredclock_day_key_timer(60),
		expiredclock_hour_key_timer(60),
		expiredclock_min_key_timer(10) {
		;
	}

public:
	void init();

	void deinit();

public:
	void registerInitHandler(init_handler_type handler) {
		init_handler_list_.push_back(handler);
	}
	void registerDeinitHandler(deinit_handler_type handler) {
		deinit_handler_list_.push_back(handler);
	}

public:
	void loadConfig();
	void saveConfig();

	template <typename T>
	const T& setConfig(const config_key_type& key, const T& value);
	template <typename T>
	T& getConfig(const config_key_type& key);

public:
	time_type now() const {
		return now_;
	}

public:
	time_type updateInterval() const {
		return interval_;
	}

	size_t updaterCount() const {
		return updater_list_.size();
	}
	size_t update() {
		size_t invoked_amount = 0;

		if (expiredclock_day_key_timer.expired()) {
			ClockExpiredUnit<EXPIRED_EVERY_DAY>::update();
		}
		if (expiredclock_hour_key_timer.expired()) {
			ClockExpiredUnit<EXPIRED_EVERY_HOUR>::update();
		}
		if (expiredclock_min_key_timer.expired()) {
			ClockExpiredUnit<EXPIRED_EVERY_MINUTE>::update();
		}

		now_.fresh();
		interval_ = now_ - last_update_;

		uint max_count = updater_list_.size();
		for (typename updater_list_type::iterator it = updater_list_.begin();
				it != updater_list_.end() && max_count > 0; --max_count) {

			typename updater_list_type::iterator hit = it; ++it;

			/**
			 * Run the update
			 */
			updater_type& updater = *hit;

			if (updater.deadtime <= now_) {
				invoked_amount++;

				updater_handler_type handler = updater.handler;
				resetUpdater(updater);

				handler(interval_);
			}
			/**
			else {
				break ;
			}
			*/

		}
/**
		if (invoked_amount > 0)
			printf("%u %u\n", invoked_amount, updater_list_.size());
*/
		last_update_ = now_;
		return invoked_amount;
	}

	void resetUpdater(updater_type& updater) {
		if (updater.once) {
			updater.client->disableOnce();
		}
		else if (updater.client->isEnabled()) {
			updater.deadtime = now_ + updater.interval;
		}
	}
	updater_connection_type addUpdater(UpdaterBase* client, updater_handler_type handler,
			time_type interval = 0) {
		updater_type updater;

		updater.client = client;
		updater.handler = handler;
		updater.deadtime = now() + interval;
		updater.interval = interval;
		updater.once = false;

		return updater_list_.insert(updater_list_.end(), updater);
	}
	updater_connection_type addUpdaterOnce(UpdaterBase* client, updater_handler_type handler,
			time_type interval = 0) {
		updater_type updater;

		updater.client = client;
		updater.handler = handler;
		updater.deadtime = now() + interval;
		updater.interval = interval;
		updater.once = true;

		return updater_list_.insert(updater_list_.end(), updater);
	}
	void removeUpdater(updater_connection_type& conn) {
		// printf("begin removeUpdater:%ud\n", updater_list_.size());
		updater_list_.erase(conn);
		// printf("end removeUpdater:%ud\n", updater_list_.size());
	}

private:
	static self					instance_;

private:
	updater_list_type			updater_list_;
	time_type					now_,
								last_update_,
								interval_;

private:
	init_handler_list_type		init_handler_list_;
	deinit_handler_list_type	deinit_handler_list_;

private:
	ExpiredTimer				expiredclock_day_key_timer,
								expiredclock_hour_key_timer,
								expiredclock_min_key_timer;
};

template <typename config>
RuntimeT<config>				RuntimeT<config>::instance_;

typedef RuntimeT<void>						Runtime;
#define RT						(::zertcore::base::Runtime::Instance())

/**
 *
 * Updater<FC>
 *
 */
template <class FC>
class Updater :
		public Runtime::UpdaterBase
{
public:
	explicit Updater() : is_static_(false) {
		;
	}
	virtual ~Updater() {
		if (!is_static_) {
			disableUpdate();

			if (isOnceEnabled()) {
				RT.removeUpdater(updater_once_conn_);
			}
		}
	}

	bool update(const time_type& interval) {
		return false;
	}

	bool once(const time_type& interval) {
		return false;
	}

public:
	void setStatic(bool flag = true) {
		is_static_ = flag;
	}

public:
	void enableOnce(time_type interval = 0) {
		/*if (!isOnceEnabled())*/
		disableOnce();

		do{
			updater_once_conn_ = RT.addUpdaterOnce(this, bind(&FC::once,
					dynamic_cast<FC*>(this), _1), interval);
			setOnceEnabledFlag(true);
		}
		while(false);
	}
	virtual void disableOnce() {
		if (isOnceEnabled()) {
			setOnceEnabledFlag(false);
			RT.removeUpdater(updater_once_conn_);
		}
	}

	virtual void enableUpdate(time_type interval = 0) {
		if (!isEnabled()) {
			updater_conn_ = RT.addUpdater(this, bind(&FC::update,
					dynamic_cast<FC*>(this), _1), interval);

			setEnabledFlag(true);
		}
	}
	virtual void disableUpdate() {
		if (isEnabled()) {
			RT.removeUpdater(updater_conn_);
			setEnabledFlag(false);
		}
	}

private:
	bool						is_static_;
	Runtime::updater_connection_type
								updater_conn_,
								updater_once_conn_;
};

}}

#endif /* RUNTIME_H_ */
