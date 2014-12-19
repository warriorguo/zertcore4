/*
 * FSM.h
 *
 *  Created on: 2013-5-14
 *      Author: Administrator
 */

#ifndef FSM_H_
#define FSM_H_

#include <common.h>
#include <PoolObject.h>

namespace zertcore{ namespace utils{

using namespace zertcore::base;

class FSM;

class FSMNode :
		public PoolObject<FSMNode>
{
	friend class FSM;
public:
	typedef function<void ()>				stat_type;
	typedef function<bool (const time_type& interval)>
											trans_type;

public:
	explicit FSMNode()					{;}

	explicit FSMNode(stat_type stat) {
		stat_ = stat;
	}

protected:
	struct FSMNodePack
	{
		trans_type	tran;
		ptr			target;
	};
	typedef list<FSMNodePack>	link_container;

protected:
	void addLink(ptr target, trans_type tran) {
		FSMNodePack pack;
		pack.tran = tran; pack.target = target;
		links_.push_back(pack);
	}

	ptr update(const time_type& interval) {
		stat_();

		for (link_container::iterator it = links_.begin(); it != links_.end(); ++it) {
			if ((*it).tran(interval)) {
				return (*it).target;
			}
		}

		return thisPtr();
	}

protected:
	link_container				links_;
	stat_type					stat_;
};

class FSM
{
public:
	typedef FSMNode::stat_type				stat_type;
	typedef FSMNode::trans_type				trans_type;

	typedef FSMNode::ptr					stat_index;

protected:
	typedef list<stat_index>				stat_container;

public:
	FSM()						{;}

	void clear() {
		stat_list_.clear();
		current_stat_ = Null;
	}

	stat_index stat(stat_type type) {
		stat_index stat = load<FSMNode>(type);
		stat_list_.push_back(stat);

		return stat;
	}

	void start(stat_index s) {
		current_stat_ = s;
	}

	void link(stat_index s1, stat_index s2, trans_type trans) {
		s1->addLink(s2, trans);
	}

	bool update(const time_type& interval) {
		if (current_stat_) {
			current_stat_ = current_stat_->update(interval);
			return true;
		}

		return false;
	}

private:
	stat_container				stat_list_;
	stat_index					current_stat_;
};


}}

#endif /* FSM_H_ */
