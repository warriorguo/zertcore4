/*
 * Condition.h
 *
 *  Created on: 2013-5-17
 *      Author: Administrator
 */

#ifndef CONDITION_H_
#define CONDITION_H_

#include <common.h>

namespace zertcore { namespace utils{

template <typename Item>
class Condition
{
public:
	typedef function<bool (Item)>	function_t;

public:
	struct Pack_ {
		function_t handler;
		uint type;
		uint relation;
	};
	typedef list<Pack_>				list_type;

public:
	Condition()							{;}
	Condition(const function_t handler, uint type = POSITIVE) {
		addPack(handler, type, AND);
	}

	bool operator () (Item item) {
		for (typename list_type::iterator it = conditions_.begin();
				it != conditions_.end(); ++it) {
			if (judge(*it, item)) {
				if ((*it).relation == OR)
					return true;
			}
		}

		return true;
	}

	void addAnd(const function_t handler, uint type = POSITIVE) {
		addPack(handler, type, AND);
	}

	void addOr(const function_t handler, uint type = POSITIVE) {
		addPack(handler, type, OR);
	}

protected:
	void addPack(const function_t handler, uint type, uint relation) {
		Pack_ pack; pack.handler = handler; pack.type = type; pack.relation = relation;
		if (relation == AND) {
			conditions_.push_back(pack);
		}
		else {
			conditions_.push_front(pack);
		}
	}

	bool judge(Pack_& pack, Item item) {
		if (pack.handler(item)) {
			if (pack.type == POSITIVE) {
				return true;
			}
			return false;
		}

		if (pack.type == NEGATIVE) {
			return true;
		}
		return false;
	}

private:
	list_type conditions_;
};

template <>
class Condition<void>
{
public:
	typedef function<bool (void)> function_t;

public:
	struct Pack_ {
		function_t handler;
		uint type;
		uint relation;
	};
	typedef list<Pack_>				list_type;

public:
	Condition()							{;}
	Condition(const function_t handler, uint type = POSITIVE) {
		addPack(handler, type, AND);
	}

	bool operator () (void) {
		for (list_type::iterator it = conditions_.begin();
				it != conditions_.end(); ++it) {
			if (judge(*it)) {
				if ((*it).relation == OR)
					return true;
			}
		}

		return true;
	}

	void addAnd(const function_t handler, uint type = POSITIVE) {
		addPack(handler, type, AND);
	}

	void addOr(const function_t handler, uint type = POSITIVE) {
		addPack(handler, type, OR);
	}

protected:
	void addPack(const function_t handler, uint type, uint relation) {
		Pack_ pack; pack.handler = handler; pack.type = type; pack.relation = relation;
		if (relation == AND) {
			conditions_.push_back(pack);
		}
		else {
			conditions_.push_front(pack);
		}
	}

	bool judge(Pack_& pack) {
		if (pack.handler()) {
			if (pack.type == POSITIVE) {
				return true;
			}
			return false;
		}

		if (pack.type == (uint)NEGATIVE) {
			return true;
		}
		return false;
	}

private:
	list_type	 conditions_;
};

}}


#endif /* CONDITION_H_ */
