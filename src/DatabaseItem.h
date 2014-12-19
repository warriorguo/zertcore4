/*
 * DatabaseItem.h
 *
 *  Created on: 2013-9-11
 *      Author: Administrator
 */

#ifndef DATABASEITEM_H_
#define DATABASEITEM_H_

#include <common.h>
#include <Runtime.h>
#include <Database.h>
#include <DatabaseUtils.h>

namespace zertcore{ namespace db {

/**
 *
 * Generate the BSON object, and build the class from the BSON
 *
 */
class DatabaseItem
{
public:
	DatabaseItem() : is_inited_(false)		{;}
	virtual ~DatabaseItem()					{;}
public:
	void save() {
		printf("DatabaseItem: data=%s\n", data_.jsonString().c_str());
		db_session_->set(key(), data_);
	}
	void reload(const BSONObj& data) {
		onLoaded(data);
	}

public:
	virtual const string& key() const		= 0;
protected:
	virtual void onLoaded(const BSONObj& data)= 0;
	virtual	void onSaved()		{;}
public:
	/**
	 * set the host DatabaseClip
	 */
	void setHost(DatabaseClip::ptr db_session) {
		db_session_ = db_session;
	}
public:
	inline BSONObj& data() {
		return data_;
	}
	template <typename T>
	T get(const string& key) {
		T val;
		getValue(val, data_, key);

		return val;
	}

	BSONObj get() const {
		return data_;
	}

	template <typename T>
	void set(const string& key, const T& value) {
		data_ = BSONObjBuilder().append(key, value).appendElementsUnique(data_).obj();
		save();
	}
	void set(const string& key, const uint64& value) {
		data_ = BSONObjBuilder().append(key, (long long)value).appendElementsUnique(data_).obj();
		save();
	}
	void set(const BSONObj& obj) {
		data_ = obj;
		save();
	}
	void erase(const string& key = Null) {
		data_ = data_.removeField(key);
		save();
	}

private:
	DatabaseClip::ptr			db_session_;

private:
	bool						is_inited_;
	BSONObj						data_;
	string						key_;
};

/**
template <>
BSONElement DatabaseItem::get(const string& key) {
	return data_.getField(key);
}
template <>
BSONObj DatabaseItem::get(const string& key) {
	return data_.getField(key).Obj();
}
template <>
int DatabaseItem::get(const string& key) {
	return data_.getField(key).Int();
}

template <>
double DatabaseItem::get(const string& key) {
	return data_.getField(key).Double();
}

template <>
int64 DatabaseItem::get(const string& key) {
	return data_.getField(key).Long();
}
template <>
time_type DatabaseItem::get(const string& key) {
	return data_.getField(key).Double();
}

template <>
uint64 DatabaseItem::get(const string& key) {
	return (uint64)data_.getField(key).Long();
}

template <>
bool DatabaseItem::get(const string& key) {
	return data_.getField(key).Bool();
}

template <>
string DatabaseItem::get(const string& key) {
	return data_.getField(key).String();
}
*/

template <>
inline void DatabaseItem::set(const string& key, const time_type& value) {
	data_ = BSONObjBuilder().append(key, (double)value).appendElementsUnique(data_).obj();
	save();
}

}}


#endif /* DATABASEITEM_H_ */
