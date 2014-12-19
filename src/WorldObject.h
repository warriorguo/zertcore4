/*
 * WorldObject.h
 *
 *  Created on: 2013-3-22
 *      Author: Administrator
 */

#ifndef WORLDOBJECT_H_
#define WORLDOBJECT_H_

#include <common.h>
#include <PoolObject.h>
#include <Database.h>
#include <Serialize.h>
#include <InterScript.h>
#include <Map.h>

namespace zertcore {namespace object{

using namespace zertcore::base;
using namespace zertcore::utils;
using namespace zertcore::map;

class WorldObject:
		public PoolObject<WorldObject>
{
public:


public:
	virtual ~WorldObject();
/**
	virtual void fire(WorldObject::ptr target, Weapon::ptr weapon, DamageData::ptr data) {

		// handle the action

	}
*/

	template <typename IO>
	void serializer(IO& io, uint type, action_index_t action) {
		// & uuid, name, type, location, etc
	}

	location_t& getLocation() {
		return location_;
	}

	const location_t& getLocation() const {
		return location_;
	}

protected:
	location_t				location_;
	Map<WorldObject>::ptr	map_;

protected:
	InterScript					script_;
	FSM							fsm_;
};


/**
 * using WorldObjectPtr globally in zertcore
 */
typedef WorldObject::ptr					WorldObjectPtr;

namespace utils {

/**
 * mainly for WorldObject Manager, for generaly, using WorldObjectManager::create(uuid) instead.
 */
template <class FC>
typename FC::ptr WorldObjectConstruct(const uuid_t& uid) {
	typename FC::ptr object = construct<FC>(uid);
	return object;
}

}

}}

namespace zertcore {

/**
 * Enable the what_type<WorldObject>
 */

/**
struct type_worldobject					{};

template <typename T>
struct is_worldobject: public false_type
{
	enum {
		value = false,
	};
};

template <> struct is_worldobject<object::WorldObject>: public true_type
{
	enum {
		value = true,
	};
};

template <typename T>
struct what_type<T, typename enable_if<
								is_worldobject<T> >::type>  {
	typedef type_worldobject type;
};
*/

}
BUILD_WHAT_TYPE_TRAP(worldobject, ,object::WorldObject);

#endif /* WORLDOBJECT_H_ */
