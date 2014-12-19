/*
 * DuplicationName.h
 *
 *  Created on: 2014-3-8
 *      Author: Administrator
 */

#ifndef DUPLICATIONNAME_H_
#define DUPLICATIONNAME_H_

#include <common.h>
#include <Database.h>
#include <DatabaseUtils.h>

namespace zertcore{ namespace utils{

class DuplicationName
{
public:
	string						TABLE_NAME;

public:
	explicit DuplicationName(const string& table_name) : TABLE_NAME(table_name) {}

public:
	bool exists(string name) {
		to_lower(name);
		return DB.exist(TABLE_NAME, BSON(
			"_id" << name
		));
	}
	bool exists(string name, uuid_t& uid) {
		to_lower(name);
		BSONObj rs = DB.get(TABLE_NAME, BSON(
			"_id" << name
		));

		if (rs.isEmpty())
			return false;

		db::BSONObjWrapper bw(rs);
		uid = bw.get<uuid_t>("uid");

		return true;
	}
	bool add(string name, const uuid_t& uid) {
		to_lower(name);
		DB.insert(TABLE_NAME, BSON(
			"_id" << name <<
			"uid" << (long long)uid
		));
		return true;
	}
	bool remove(string name) {
		to_lower(name);
		DB.remove(TABLE_NAME, BSON(
			"_id" << name
		));
		return true;
	}

};

}}


#endif /* DUPLICATIONNAME_H_ */
