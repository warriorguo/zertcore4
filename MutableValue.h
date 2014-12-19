/*
 * MutableValue.h
 *
 *  Created on: 2013-11-11
 *      Author: Administrator
 */

#ifndef MUTABLEVALUE_H_
#define MUTABLEVALUE_H_

#include <common.h>
#include <Runtime.h>
#include <UUIDGenerator.h>
#include <Database.h>
#include <DatabaseUtils.h>

namespace zertcore{ namespace utils{

using namespace zertcore::base;
using namespace zertcore::db;


/**
 * IMVBase
 */
class IMVBase
{
public:
	typedef IMVBase*						ptr;

public:
	virtual ~IMVBase() {;}

public:
	virtual bool set(BSONObj data)			= 0;
public:
	virtual string getKey() const			= 0;
	virtual time_type getActionTime() const	= 0;
	virtual bool action(const time_type& now)
											= 0;
	virtual void remove()					= 0;
};


/**
 * IMVManager
 */
class IMVManager :
		public Updater<IMVManager>
{
public:
	typedef IMVManager						self;
	typedef IMVManager&						reference;

public:
	typedef multimap<time_type::timestamp_type, IMVBase::ptr>
											imv_map_type;
	typedef map<string, IMVBase::ptr>		imv_key_map_type;
	typedef imv_map_type::iterator			connection_type;

public:
	inline static reference Instance() {
		static self					instance_;
		return instance_;
	}

	void init() {
		enableUpdate(1);
	}

	void deinit() {
		disableUpdate();
		imv_map_.clear();
	}

public:
	bool update(const time_type& interval) {
		time_type now = RT.now();

		for (imv_map_type::iterator it = imv_map_.begin();
				it != imv_map_.end(); ++it) {
			if (now.timestamp < it->first) {
				break;
			}

			it->second->action(now);
		}

		return false;
	}

	bool set(const string& key, const BSONObj& data) {
		imv_key_map_type::iterator it = imv_key_map_.find(key);
		if (it != imv_key_map_.end()) {
			it->second->set(data);
			return true;
		}

		return false;
	}

public:
	bool add(IMVBase::ptr imv, connection_type& connection) {
		if (!imv) {
			return false;
		}

		imv_key_map_.insert(pair<string, IMVBase::ptr>(
			imv->getKey(), imv
		));

		if (imv->getActionTime().timestamp <= 0.1)
			return false;

		connection = imv_map_.insert(pair<time_type::timestamp_type, IMVBase::ptr>(
			imv->getActionTime().timestamp, imv
		));

		return true;
	}
	void remove(connection_type connection) {
		imv_map_.erase(connection);
	}


private:
	imv_map_type				imv_map_;
	imv_key_map_type			imv_key_map_;
};


/**
 * IMV<T>
 */
template <typename T>
class IMV :
		public IMVBase
{
public:
	typedef T								value_type;

public:
	IMV() : added_(false) {;}
	IMV(const string& key) : added_(false) {
		init(key);
	}
	virtual ~IMV() {;}

	void init(const string& key) {
		key_ = key;

		begin_time_ = 0;
		end_time_ = 0;

		BSONObj d = DB.get("imv", BSON(
			"_id" << key_
		));

		if (!d.isEmpty())
			data(d);

		action(RT.now());
		if (IMVManager::Instance().add(this, connection_)) {
			added_ = true;
		}
	}

public:
	const value_type& get() const {
		return value_;
	}
	value_type& get() {
		return value_;
	}
	void set(const value_type& value) {
		old_value_ = value;
		action(RT.now());

		if (!added_) {
			value_ = value;
		}
	}

	IMV& operator =(const value_type& value) {
		set(value);
		return *this;
	}

public:
	virtual string getKey() const {
		return key_;
	}
	virtual time_type getActionTime() const {
		return action_time_;
	}
	virtual bool action(const time_type& now) {
		if (now >= begin_time_ && now < end_time_) {
			value_ = new_value_;
			action_time_ = end_time_;

			return true;
		}
		else if (now >= end_time_) {
			value_ = old_value_;
			action_time_ = 0;

			return false;
		}
		else if (now < begin_time_) {
			action_time_ = begin_time_;

			return true;
		}

		action_time_ = 0;
		return false;
	}
	virtual void remove() {
		if (added_) {
			IMVManager::Instance().remove(connection_);
		}
	}

public:
	virtual bool set(BSONObj data) {
		BSONObjWrapper bw(data);

		return set(bw.get<time_type::timestamp_type>("begin"),
				bw.get<time_type::timestamp_type>("end"),
				BSONObjConvert<value_type>(bw.get<BSONObj>("value")));
	}

	bool set(const time_type& begin, const time_type& end, const value_type& value) {
		new_value_ = value;
		// old_value_ = value_;

		begin_time_ = begin;
		end_time_ = end;

		printf("begin:%f end:%f\n", begin.timestamp, end.timestamp);

		save();

		remove();
		action(RT.now());
		if (IMVManager::Instance().add(this, connection_)) {
			added_ = true;
		}

		return true;
	}

public:
	void save() {
		DB.set("imv", data(), key_);
	}

	void data(BSONObj data) {
		BSONObjWrapper bw(data);

		value_ = BSONObjConvert<value_type>(bw.get<BSONObj>("value"));
		new_value_ = BSONObjConvert<value_type>(bw.get<BSONObj>("new_value"));
		old_value_ = BSONObjConvert<value_type>(bw.get<BSONObj>("old_value"));

		begin_time_ = bw.get<time_type::timestamp_type>("begin");
		end_time_ = bw.get<time_type::timestamp_type>("end");
	}
	BSONObj data() const {
		return BSON(
			"_id" << key_ <<
			"value" << convertBSONObj(value_) <<
			"new_value" << convertBSONObj(new_value_) <<
			"old_value" << convertBSONObj(old_value_) <<
			"begin" << begin_time_.timestamp <<
			"end" << end_time_.timestamp
		);
	}

private:
	string						key_;
	bool						added_;
	IMVManager::connection_type	connection_;

private:
	value_type					value_;
	value_type					new_value_;
	value_type					old_value_;

private:
	time_type					begin_time_;
	time_type					end_time_;
	time_type					action_time_;
};


}}

#endif /* MUTABLEVALUE_H_ */
