/*
 * ActionMap.h
 *
 *  Created on: 2013-7-8
 *      Author: Administrator
 */

#ifndef ACTIONMAP_H_
#define ACTIONMAP_H_

#include <common.h>
#include <Object.h>
#include <boost/signals2.hpp>

namespace zertcore{ namespace utils{

using namespace zertcore::base;

/**
 *
 */
template <typename Key1, typename Key2, typename Value>
class Key2Map
{
	typedef Key2Map<Key1, Key2, Value>	self;
public:
	typedef self&						reference;
	typedef self*						ptr;

	typedef Key1						key1_type;
	typedef const key1_type&			key1_const_reference;
	typedef Key2						key2_type;
	typedef const key2_type&			key2_const_reference;
	typedef Value						value_type;
	typedef value_type&					value_reference;
	typedef const value_type&			value_const_reference;

	typedef map<key2_type, value_type>	cell_type;
	typedef map<key1_type, cell_type>	map_type;

public:
	void set(key1_const_reference key1, key2_const_reference key2, value_const_reference value) {
		data_[key1][key2] = value;
	}
	void erase(key1_const_reference key1, key2_const_reference key2) {
		typename map_type::iterator it = data_.find(key1);
		if (it != data_.end()) {
			it->second.erase(key2);
			if (it->second.size() == 0) {
				data_.erase(key1);
			}
		}
	}

	value_reference get(key1_const_reference key1, key2_const_reference key2) {
		return data_[key1][key2];
	}
	value_const_reference get(key1_const_reference key1, key2_const_reference key2) const {
		return data_[key1][key2];
	}

private:
	map_type data_;
};

/**
 * Usage Sample:
 *   void hello(int a, int b) {printf("hello hello: %d %d\n", a, b);};
 *   void world(int a, int b) {printf("world world: %d %d\n", a, b);};
 *   ActionMap<int, void (int, int)> map;
 *   map.addHandler(0, &hello);
 *   ActionMap<int, void (int, int)>::connection conn = map.addHandler(0, &world);
 *   map(0)(1, 2); //or map[0](1, 2);
 *   conn.disconnect();
 *   map(0)(3, 4); //or map[0](1, 2);
 *
 */
template <typename Key, typename ActionHandler>
class ActionMap
{
	typedef ActionMap<Key, ActionHandler>
										self;
public:
	typedef self&						reference;
	typedef const self&					const_reference;
	typedef self*						ptr;

public:
	typedef Key							key_type;
	typedef const key_type				const_key_type;
	typedef function<ActionHandler>		action_handler_type;
	typedef signals2::signal<ActionHandler>
										slot_type;
	typedef slot_type&					slot_type_reference;
	typedef SMART_PTR(slot_type)		slot_type_ptr;
	typedef map<key_type, slot_type_ptr>
										handler_map_type;

	typedef signals2::connection		connection;
/**
public:
	typedef Key2Map<Key, void*, signals2::connection>
										value_map_type;
*/

public:
	ActionMap() {
		;
	}
	ActionMap(const_reference src) {
		copyFrom(src);
	}

	connection addHandler(const_key_type key, action_handler_type handler) {
		connection conn;

		typename handler_map_type::iterator it = handler_map_.find(key);
		if (it != handler_map_.end()) {
			conn = it->second->connect(handler);
		}
		else {
			slot_type_ptr slot_p(new slot_type);
			conn = slot_p->connect(handler);
			handler_map_.insert(pair<key_type, slot_type_ptr>(key, slot_p));
		}

		return conn;
	}
	/**
	void removeHandler(const_key_type key, action_handler_type handler) {
		//value_map_.get(key, handler.template target<void>()).disconnect();
		typename handler_map_type::iterator it = handler_map_.find(key);
		if (it != handler_map_.end()) {
			it->second->disconnect(handler);
		}
	}
	*/

	slot_type_reference at(const_key_type key) {
		typename handler_map_type::iterator it = handler_map_.find(key);
		if (it != handler_map_.end()) {
			return *(it->second);
		}

		static slot_type tmp;
		return tmp;
	}
	slot_type_reference operator() (const_key_type key) {
		return at(key);
	}
	slot_type_reference operator[] (const_key_type key) {
		return at(key);
	}

	void copyFrom(const_reference src) {
		handler_map_.insert(src.handler_map_.begin(), src.handler_map_.end());
	}
	reference operator =(const_reference src) {
		copyFrom(src);
		return *this;
	}

private:
	handler_map_type	handler_map_;
	// value_map_type		value_map_;
};

}}


#endif /* ACTIONMAP_H_ */
