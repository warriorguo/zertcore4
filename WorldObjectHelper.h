/*
 * WorldObjectHelper.h
 *
 *  Created on: 2013-6-2
 *      Author: Administrator
 */

#ifndef WORLDOBJECTHELPER_H_
#define WORLDOBJECTHELPER_H_

#include <common.h>
#include <WorldObject.h>

namespace zertcore { namespace object{ namespace helper{

template<typename T>
struct InDistance
{
	explicit InDistance(WorldObjectPtr obj1, WorldObjectPtr obj2, const T& distance = (T)0):
			object1(obj1), object2(obj2), distance_(distance)
	{;}

	bool operator()(const T& distance) {
		return object1->getLocation().distance(object2->getLocation()) <= distance;
	}

	bool operator()() {
		return object1->getLocation().distance(object2->getLocation()) <= distance_;
	}

	WorldObjectPtr	object1;
	WorldObjectPtr	object2;
	T				distance_;
};

struct IsDead
{
	explicit IsDead(WorldObjectPtr obj);
};

struct IsDying;

}}}



#endif /* WORLDOBJECTHELPER_H_ */
