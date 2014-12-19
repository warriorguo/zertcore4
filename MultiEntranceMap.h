/*
 * MultiEntranceMap.h
 *
 *  Created on: 2013-7-3
 *      Author: Administrator
 */

#ifndef MULTIENTRANCEMAP_H_
#define MULTIENTRANCEMAP_H_

#include <common.h>
#include <PoolObject.h>

namespace zertcore{ namespace utils{

using namespace zertcore::base;

template <typename Key, typename Value>
class MultiEntranceMapNode;

template <typename Key, typename Value>
class MultiEntranceMap
{
	template <typename T, typename R> friend class MultiEntranceMapNode;

	typedef MultiEntranceMap<Key, Value>	self;
public:
	typedef	self&	reference;
	typedef self*	ptr;

	typedef Key		key_type;
	typedef Value	value_type;
	typedef const value_type	const_value_type;

public:
	typedef reference	map_ref;


public:
	typedef MultiEntranceMapNode<key_type, value_type>
										node;
	typedef typename node::ptr			node_ptr;

	typedef map<key_type, node_ptr>		map_type;
	typedef set<node_ptr>				list_type;

public:
	node_ptr find(const key_type& key) {
		typename map_type::iterator it = key_map_.find(key);
		if (it != key_map_.end()) {
			return it->second;
		}

		return Null;
	}

	node_ptr insert(const_value_type& value);
	node_ptr insert(const key_type& key, const_value_type& value);
	void dispose(node_ptr nd);

private:
	void addKey(const key_type& key, node_ptr nd) {
		key_map_.insert(pair<key_type, node_ptr>(key, nd));
	}
	void removeKey(const key_type& key) {
		key_map_.erase(key);
	}
	void addList(node_ptr nd) {
		node_list_.insert(nd);
	}
	void removeList(node_ptr nd) {
		node_list_.erase(nd);
	}

private:
	map_type	key_map_;
	list_type	node_list_;
};

/**
 *
 */
template <typename Key, typename Value>
class MultiEntranceMapNode :
		public PoolObject<MultiEntranceMapNode<Key, Value> >
{
	typedef MultiEntranceMapNode<Key, Value>	self;
public:
	typedef self*	ptr;
	typedef self&	reference;

	typedef Key		key_type;
	typedef Value	value_type;
	typedef const value_type	const_value_type;

	typedef MultiEntranceMap<Key, Value>	map_type;
	typedef map_type&		map_reference;
	typedef set<key_type>	key_list_type;

public:
	MultiEntranceMapNode(map_reference map, const_value_type& value)
		: value_(value), map_(map) {
		map.addList(this);
	}

public:
	void addKey(const key_type& key) {
		key_list_.insert(key);
		map_.addKey(key, this);
	}
	void removeKey(const key_type& key) {
		key_list_.erase(key);
		map_.removeKey(key);
	}
	void dispose() {
		for (typename key_list_type::iterator it = key_list_.begin();
				it != key_list_.end(); ++it) {
			map_.removeKey(*it);
		}
		map_.removeList(this);
		delete this;
	}
	value_type& data() {
		return value_;
	}
	const value_type& data() const {
		return value_;
	}

	value_type& operator*() {
		return value_;
	}
	const value_type& operator*() const {
		return value_;
	}
private:
	key_list_type	key_list_;
	value_type		value_;
	map_reference	map_;
};

template <typename Key, typename Value>
void MultiEntranceMap<Key, Value>::dispose(node_ptr nd) {
	if (nd) nd->dispose();
}

template <typename Key, typename Value>
typename MultiEntranceMap<Key, Value>::node_ptr
MultiEntranceMap<Key, Value>::insert(const_value_type& value) {
	node_ptr nd = load<MultiEntranceMapNode<Key, Value>, self, const_value_type>
								(*this, value);
	return nd;
}

template <typename Key, typename Value>
typename MultiEntranceMap<Key, Value>::node_ptr
MultiEntranceMap<Key, Value>::insert(const key_type& key, const_value_type& value) {
	node_ptr nd = load<MultiEntranceMapNode<Key, Value>, self, const_value_type>
								(*this, value);
	nd->addKey(key);
	return nd;
}

}}

#endif /* MULTIENTRANCEMAP_H_ */
