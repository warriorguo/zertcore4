/*
 * HaffmanTree.h
 *
 *  Created on: 2013-11-20
 *      Author: Administrator
 */

#ifndef HAFFMANTREE_H_
#define HAFFMANTREE_H_

#include <common.h>
#include <PoolObject.h>

namespace zertcore{ namespace utils{

using namespace zertcore::base;

template <typename Key>
class HaffmanTree
{
public:
	typedef Key								key_type;
	typedef string							value_type;

public:
	class Node :
			public PoolObject<Node>
	{
	public:
		typedef SMART_PTR(Node)				ptr;

	public:
		Node(uint frequency = 0) : frequency_(frequency),
			right_(Null), left_(Null) {;}

		~Node() {
			right_ = Null;
			left_ = Null;
		}

	public:
		uint getFrequency() const {
			return frequency_;
		}
		void setFrequency(uint frequency) {
			frequency_ = frequency;
		}


	public:
		void addRight(ptr right) {
			right_ = right;
		}
		void addLeft(ptr left) {
			left_ = left;
		}

	public:
		value_type value() const {
			return value_;
		}

	public:
		void workOut(const value_type& value) {
			value_ = value;

			if (left_) {
				left_->workOut(value + '0');
			}
			if (right_) {
				right_->workOut(value + '1');
			}
		}

	private:
		uint					frequency_;
		value_type				value_;

	private:
		ptr						right_, left_;
	};

public:
	typedef Node							node_type;
	typedef typename Node::ptr				node_ptr;

	typedef map<key_type, node_ptr>			frequency_map_type;
	typedef multimap<uint, node_ptr>		node_list_type;

public:
	HaffmanTree() : root_(Null) {;}

	~HaffmanTree() {
		root_ = Null;

		frequency_map_.clear();
		node_list_.clear();
	}

public:
	void setFrequency(const key_type& key, uint frequency) {
		node_ptr node = load<node_type>(frequency);
		frequency_map_[key] = node;

		node_list_.insert(pair<uint, node_ptr>(frequency, node));
	}

	value_type getValue(const key_type& key) {
		typename frequency_map_type::const_iterator it =  frequency_map_.find(key);
		if (it != frequency_map_.end() && it->second)
			return it->second->value();

		return Null;
	}

public:
	bool generateOne() {
		uint index = 0;
		uint frequency = 0;
		node_ptr nodes[2];

		if (node_list_.size() < 2)
			return false;

		for (typename node_list_type::iterator it = node_list_.begin();
				it != node_list_.end(); ) {
			if (index >= 2)
				break;

			frequency += it->first;
			nodes[index] = it->second;
			node_list_.erase(it++);

			index++;
		}

		node_ptr node = load<node_type>(frequency);
		node->addLeft(nodes[0]);
		node->addRight(nodes[1]);

		node_list_.insert(pair<uint, node_ptr>(node->getFrequency(), node));
		return true;
	}

	void generate() {
		while(generateOne()) ;
		root_ = node_list_.begin()->second;
		root_->workOut("");
	}

private:
	frequency_map_type			frequency_map_;
	node_list_type				node_list_;

private:
	node_ptr					root_;
};

}}

#endif /* HAFFMANTREE_H_ */
