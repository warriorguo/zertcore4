/*
 * ObjectTag.h
 *
 *  Created on: 2013-11-28
 *      Author: Administrator
 */

#ifndef OBJECTTAG_H_
#define OBJECTTAG_H_

#include <common.h>
#include <Database.h>
#include <DatabaseUtils.h>

namespace zertcore{ namespace utils{

using namespace zertcore::base;
using namespace zertcore::db;

typedef string								tag_key_type;

class ObjectTag
{
public:
	typedef tag_key_type					tag_key;
	typedef map<tag_key, uint>				map_type;

public:
	ObjectTag() : uid_(0) {}

	void init(const uuid_t& id) {
		uid_ = id;

		BSONObj rs = DB.get("object_tag", BSON(
			"_id" << (long long)uid_
		));
		if (rs.isEmpty())
			return ;

		data(rs["data"].Obj());
	}

public:
	bool has(const tag_key& tag) const {
		return tag_map_.find(tag) != tag_map_.end();
	}
	void add(const tag_key& tag) {
		tag_map_[tag]++;
		save();
	}
	void set(const tag_key& tag) {
		if (!has(tag)) add(tag);
	}
	uint weight(const tag_key& tag) const {
		map_type::const_iterator it = tag_map_.find(tag);
		if (it != tag_map_.end())
			return it->second;

		return 0;
	}

public:
	void save() {
		if (!uid_)
			return ;

		DB.set("object_tag", BSON(
			"_id" << (long long)uid_ <<
			"data" << data()
		));
	}

	BSONObj data() const {
		BSONObjBuilder b;
		for (map_type::const_iterator it = tag_map_.begin();
				it != tag_map_.end(); ++it) {
			b.append(cast<string>(it->first), it->second);
		}
		return b.obj();
	}
	void data(BSONObj data) {
		typedef ::std::set<string>			keys_type;

		keys_type names;
		data.getFieldNames(names);

		for (keys_type::iterator it = names.begin();
				it != names.end(); ++it) {
			uint amount = 0;
			getValue(amount, data[*it]);

			if (amount) {
				tag_map_.insert(pair<tag_key, uint>(cast<tag_key>(*it), amount));
			}
		}
	}

private:
	uuid_t						uid_;
	map_type					tag_map_;
};

}}

#endif /* OBJECTTAG_H_ */
