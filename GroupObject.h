/*
 * GroupObject.h
 *
 *  Created on: 2013-5-6
 *      Author: Administrator
 */

#ifndef GROUPOBJECT_H_
#define GROUPOBJECT_H_

#include <common.h>
#include <PoolObject.h>
#include <Condition.h>

namespace zertcore {namespace base {

using namespace utils;

template <class FC, class Container = list<FC> >
class GroupObject :
		public PoolObject<FC>
{
public:
	GroupObject();
	virtual ~GroupObject();
};

template <typename Item>
class Sort
{
public:
	typedef function<int (Item)> 			type;

public:
	Sort(type const);

	int operator ()(Item item1, Item item2);

	void exprAnd(type const, bool is_true = true);
	void exprOr(type const, bool is_true = true);
};

template <class FC, class Container = list<FC> >
class Group:
		public PoolObject<Group<FC, Container> >
{
	typedef Group<FC, Container>			self;
	typedef typename ObjectTraits<FC>::ptr	FCptr;

	typedef function<int (FCptr, FCptr)>	sort_handler_type;
public:
	typedef SMART_PTR(self)					ptr;

	typedef typename ObjectTraits<FC, Container>::iterator			iterator;
	typedef typename ObjectTraits<FC, Container>::const_iterator	const_iterator;

public:
	void filter(const Condition<FCptr>& );
	void sort(const sort_handler_type& );

	iterator begin();
	const_iterator begin() const;

	iterator end();
	const_iterator end() const;
};

/**
Group<WorldObject>::ptr group = Map.nearby(x, y);
group->filter(bind(IsType(), f));
group->sort(bind(Score()));
for (Group<WorldObject>::iterator iter = group->begin(); iter != group->end(); ++iter) {
	WorldObject::ptr object = *iter;
}
*/

}}

#endif /* GROUPOBJECT_H_ */
