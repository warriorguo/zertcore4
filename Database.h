/*
 * Database.h
 *
 *  Created on: 2013-5-5
 *      Author: Administrator
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include <common.h>
#include <PoolObject.h>

#include <mongo/client/dbclient.h>

namespace zertcore {namespace db {

using namespace mongo;
using namespace zertcore::base;


template <typename T>
inline bool getValue(T& value, const BSONObj& data, const string& item,
		const T& default_value = Null);

template <typename T>
inline bool getValue(T& value, const BSONElement& elemt,
		const T& default_value = Null);

template <typename T>
inline void setValue(BSONObj& data, const string& item, const T& value) {
	data = BSONObjBuilder().append(item, value).appendElementsUnique(data).obj();
}
/**
 *
 * DatabaseClip
 *
 */
class DatabaseClip:
		public PoolObject<DatabaseClip>
{
	template <typename _> friend class DatabaseT;

	template <typename T>
	friend bool getValue(T& value, const BSONObj& data, const string& item,
		const T& default_value);
public:
	typedef SMART_PTR(DatabaseClip)			ptr;

public:
	explicit DatabaseClip(const string& ns, const BSONObj& object, const uuid_t& uid) :
		object_(object), ns_(ns), uid_(uid), cache_flag_(true), dirty_flag_(false) {
		;
	}

public:
	template <typename T>
	T get(const string& key) {
		T val;
		getValue(val, object_, key);

		return val;
	}
	BSONObj get() {
		return object_;
	}
	template <typename T>
	void set(const string& key, const T& value) {
		object_ = BSONObjBuilder().append(key, value).appendElementsUnique(object_).obj();
		setDirty();
	}
	void set(const string& key, const uint64& value) {
		object_ = BSONObjBuilder().append(key, (long long)value).appendElementsUnique(object_).obj();
		setDirty();
	}
	void set(const BSONObj& data) {
		object_ = BSONObjBuilder().appendElements(data).appendElementsUnique(object_).obj();
		setDirty();
	}
	bool has(const string& key) const {
		return object_.hasField(key);
	}
	void remove(const string& key = Null);

public:
	/**
	 * if disable cache, the flush will always return true
	 */
	bool flush() {
		if (!isCached()) {
			return true;
		}

		if (!isDirty())
			return false;

		_flush();
		return true;
	}

	bool isDirty() const {
		return dirty_flag_;
	}
	void setDirty() {
		if (!isCached()) {
			_flush();
			return ;
		}

		dirty_flag_ = true;
	}

	bool isCached() {
		return cache_flag_;
	}
	void enableCache(bool fl) {
		cache_flag_ = fl;

		/**
		 * if cancel the cache option, flush it immediately
		 */
		if (!cache_flag_) {
			_flush();
		}
	}

protected:
	void _flush();

private:
	BSONObj object_;
	string	ns_;
	const uuid_t uid_;

	bool	cache_flag_;
	bool	dirty_flag_;
};

template <typename T>
inline bool getValue(T& value, DatabaseClip::ptr clip, const string& item,
		const T& default_value = Null) {
	return getValue(value, clip->object_, item, default_value);
}

/**
template <>
BSONElement DatabaseClip::get(const string& key) {
	return object_.getField(key);
}
template <>
BSONObj DatabaseClip::get(const string& key) {
	return object_.getField(key).Obj();
}
template <>
int DatabaseClip::get(const string& key) {
	return object_.getField(key).Int();
}

template <>
double DatabaseClip::get(const string& key) {
	return object_.getField(key).Double();
}

template <>
int64 DatabaseClip::get(const string& key) {
	return object_.getField(key).Long();
}
template <>
time_type DatabaseClip::get(const string& key) {
	return object_.getField(key).Double();
}

template <>
uint64 DatabaseClip::get(const string& key) {
	return (uint64)object_.getField(key).Long();
}

template <>
bool DatabaseClip::get(const string& key) {
	return object_.getField(key).Bool();
}

template <>
string DatabaseClip::get(const string& key) {
	return object_.getField(key).String();
}
*/

template <>
inline void DatabaseClip::set(const string& key, const time_type& value) {
	object_ = BSONObjBuilder().append(key, (double)value)
			.appendElementsUnique(object_).obj();
	setDirty();
}

/**
 *
 * Utils
 *
 */

typedef auto_ptr<DBClientCursor>			db_cursor;

/**
 *
 * DatabaseT<_>
 *
 */
template <typename _>
class DatabaseT
{
	friend class DatabaseClip;

	typedef DatabaseT<_>					self;
public:
	typedef self&							reference;

public:
	typedef DBClientConnection				db_type;
public:
	typedef typename DatabaseClip::ptr		clip_type;

public:
	inline static reference Instance() {
		return instance_;
	}

public:
	explicit DatabaseT() : connected_(false) {
		;
	}
	virtual ~DatabaseT() {
		;
	}

public:
	auto_ptr<DBClientCursor> query(const string& type_name, Query query = Query(),
			int limit_amount = 0, int skip_amount = 0) {
		string ns = db_ + '.' + type_name;
		return conn_.query(ns, query, limit_amount, skip_amount);
	}

	uint64 count(const string& type_name) {
		string ns = db_ + '.' + type_name;
		return conn_.count(ns);
	}

public:
	bool connect(string host, int port = 27017, const string& db = Null) {
		host = host + ":" + lexical_cast<string>(port);
		string errmsg;

		if (!conn_.connect(host, errmsg)) {
			printf("Failed to connect to database: %s \n", errmsg.c_str());
			return false;
		}

		selectDB(db);
		connected_ = true;

		return true;
	}
	void selectDB(const string& db) {
		db_ = db;
	}

	DatabaseClip::ptr get(const string& type_name, uuid_t uid,
			const BSONObj& query = Null) {
		if (!connected_)
			return Null;

		string ns = db_ + '.' + type_name;

		BSONObj res;

		time_type time_f1;
		time_f1.fresh();

		if (query.isEmpty()) {
			res = conn_.findOne(ns, BSON("_id" << (long long)uid));
		}
		else {
			res = conn_.findOne(ns, query);
		}

		time_type time_f2;
		time_f2.fresh();

		time_type offset = time_f2 - time_f1;
		if (offset.timestamp > 0.01) {
			::printf("Warning : DB.get %s TOO SLOW TAKE %f, UID:%ld QUERY:%s\n", type_name.c_str(), offset.timestamp, uid, query.toString().c_str());
		}

		if (res.isEmpty())
			return Null;

		if (!uid) {
			uid = (uuid_t)res["_id"].Long();
		}

		return load<DatabaseClip>(ns, res, uid);
	}
	BSONObj get(const string& key, const BSONObj& query = Null) {
		if (!connected_)
			return Null;

		string ns(db_ + '.' + key);

		time_type time_f1;
		time_f1.fresh();

		BSONObj rs = conn_.findOne(ns, query);

		time_type time_f2;
		time_f2.fresh();

		time_type offset = time_f2 - time_f1;
		if (offset.timestamp > 0.01) {
			::printf("Warning : DB.get %s TOO SLOW TAKE %f, QUERY:%s\n", key.c_str(), offset.timestamp, query.toString().c_str());
		}

		if (rs.isEmpty())
			return Null;

		return rs.getOwned();
	}
	bool exist(const string& key, const BSONObj& query = Null) {
		if (!connected_)
			return false;

		string ns(db_ + '.' + key);
		return !conn_.findOne(ns, query).isEmpty();
	}
	DatabaseClip::ptr create(const string& type_name, uuid_t uid = Null) {
		if (!connected_)
			return Null;

		string ns(db_ + '.' + type_name);

		BSONObj res;
		if (!uid) {
			res = BSON(GENOID);
		}
		else {
			res = BSON("_id" << (long long)uid);
		}

		conn_.insert(ns, res);
		return load<DatabaseClip>(ns, res, uid);
	}
	void insert(const string& key, const BSONObj& res) {
		if (!connected_)
			return ;

		string ns(db_ + '.' + key);
		conn_.insert(ns, res);
	}
	void set(const string& key, const BSONObj& query, const BSONObj& res,
			bool upsert = true) {
		if (!connected_)
			return ;

		string ns(db_ + '.' + key);
		conn_.update(ns, query, res, upsert, false);
	}
	void set(const string& key, const BSONObj& res, bool upsert = true) {
		if (!connected_)
			return ;

		string ns(db_ + '.' + key);
		conn_.update(ns, Null, res, upsert);
	}
	void set(const string& key, const BSONObj& res, const string& uid, bool upsert = true) {
		if (!connected_)
			return ;

		string ns(db_ + '.' + key);
		conn_.update(ns, BSON("_id" << uid), res, upsert);
	}
	void set(const string& key, const BSONObj& res, uuid_t uid, bool upsert = true) {
		if (!connected_)
			return ;

		string ns(db_ + '.' + key);
		conn_.update(ns, BSON("_id" << (long long)uid), res, upsert);
	}
	void remove(const string& key, uuid_t uid) {
		if (!connected_)
			return ;

		string ns(db_ + '.' + key);
		conn_.remove(ns, BSON("_id" << (long long)uid));
	}
	void remove(const string& key, const BSONObj& query) {
		if (!connected_)
			return ;

		string ns(db_ + '.' + key);
		conn_.remove(ns, query);
	}

public:
	bool updateClip(DatabaseClip::ptr clip) {
		conn_.update(clip->ns_, BSON(
				"_id" << (long long)clip->uid_
			  ), clip->object_);
		return true;
	}
	bool removeClip(DatabaseClip::ptr clip) {
		conn_.remove(clip->ns_, BSON(
				"_id" << (long long)clip->uid_
				), /*just one*/true);
		return true;
	}

private:
	static self	instance_;

private:
	bool		connected_;

	db_type		conn_;
	string		db_;
};

template <typename _>
DatabaseT<_>	DatabaseT<_>::instance_;

typedef DatabaseT<void>	Database;

#define DB						(::zertcore::db::Database::Instance())

inline void DatabaseClip::_flush() {
	Database::Instance().updateClip(thisPtr());
}
inline void DatabaseClip::remove(const string& key) {
	if (key.empty()) {
		Database::Instance().removeClip(thisPtr());
	}
	else {
		object_ = object_.removeField(key);
	}
}
/**
uuid_t uuid = 1;
DatabaseClip::ptr daff_db_clip = construct<DatabaseClip>();
Daff::ptr daff = construct<daff>(uuid);
daff_db_clip->bindSQL("SELECT type,username FROM `daff` WHERE uuid='$1'" % uuid);
daff_db_clip->bindVariable(0, &daff->type);
daff_db_clip->bindVariable(1, &daff->username);
daff_db_clip->execute();
*/

}}

#endif /* DATABASE_H_ */
