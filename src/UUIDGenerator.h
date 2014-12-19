/*
 * UUIDGenerator.h
 *
 *  Created on: 2013-8-5
 *      Author: Administrator
 */

#ifndef UUIDGENERATOR_H_
#define UUIDGENERATOR_H_

#include <common.h>
// #include <Runtime.h>
#include <Database.h>
#include <DatabaseUtils.h>

namespace zertcore{ namespace base{

using namespace zertcore::db;

/**
 *
 * UUIDGeneratorT<_>
 *
 */

template <typename _>
class UUIDGeneratorT
{
	typedef UUIDGeneratorT<_>				self;
public:
	typedef map<type_t, uuid_t>				counter_map_type;

public:
	explicit UUIDGeneratorT() {
		// init();
	}

public:
	inline static self& Instance() {
		return instance_;
	}

	uuid_t inc(const type_t& type) {
		uuid_t uid;
		do {
			mutex_type::scoped_lock lock(mutex_);
			uid = ++counter_map_[type];
		}while(0);

		update();

		return uid;
	}

public:
	bool update() {
		BSONObjBuilder bob;

		bob.append("_id", (long long)0);
		for (typename counter_map_type::iterator it = counter_map_.begin();
				it != counter_map_.end(); ++it) {
			bob.append(lexical_cast<string>(it->first), (long long)it->second);
		}

		DB.set("uuid", BSON(
			"_id" << 0
		), bob.obj());
		return true;
	}

public:
	/**
	 * read the the uuid from database
	 */
	void init() {
		BSONObj data = DB.get("uuid");

		set<string> keys;
		data.getFieldNames(keys);

		for (set<string>::iterator it = keys.begin();
				it != keys.end(); ++it) {
			string key = *it;

			try {
				counter_map_[lexical_cast<type_t>(key)] = data[key].Long();
			}
			catch(bad_lexical_cast &) {
				continue;
			}
		}
	}

private:
	counter_map_type			counter_map_;
	mutex_type					mutex_;

private:
	static self		instance_;
};

template <typename _>
UUIDGeneratorT<_>	UUIDGeneratorT<_>::instance_;

typedef UUIDGeneratorT<void>				UUIDGenerator;


/**
 *
 * uuid_t UUIDGenerate(const type_t& type_id);
 *
 */
inline uuid_t UUIDGenerate(const type_t& type_id) {
	return UUIDGenerator::Instance().inc(type_id);
}

}}


#endif /* UUIDGENERATOR_H_ */
