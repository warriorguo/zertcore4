/*
 * Object.h
 *
 *  Created on: 2013-5-5
 *      Author: Administrator
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include <common.h>
#include <ValidityObject.h>

#include <boost/enable_shared_from_this.hpp>

namespace zertcore {

/**
 *
 * SmartTraits<FC>
 *
 */
template <class FC>
struct SmartTraits {
	typedef SMART_PTR(FC)			ptr;
	typedef const SMART_PTR(FC)		const_ptr;
	typedef FC*						original_ptr;
	typedef const FC*				original_const_ptr;
	typedef FC&						reference;
	typedef const FC&				const_reference;
};

/**
 *
 * TypeTraits<FC>
 *
 */
template <class FC>
struct TypeTraits {
	typedef FC*						ptr;
	typedef const FC*				const_ptr;
	typedef FC*						original_ptr;
	typedef const FC*				original_const_ptr;
	typedef FC&						reference;
	typedef const FC&				const_reference;
};

}

namespace zertcore {namespace base{

/**
 * ID generator, should be based on database.
 */
uuid_t UUIDGenerate(const type_t& type_id);

/**
 *
 * use Null instead
static struct {
	template <typename FC> operator SMART_PTR(FC)() {
		return SMART_PTR(FC)();
	}
	template <typename FC> operator FC* const () {
		return NULL;
	}
} nullPtr;
 */

/**
 *
 * ObjectTypeManagerT
 *
 */
template <typename _>
class ObjectTypeManagerT
{
	typedef ObjectTypeManagerT<_>			self;
public:
	typedef self&							reference;

public:
	typedef map<type_t, string>				map_type;

public:
	inline static reference Instance() {
		return instance_;
	}

public:
	void registerType(const type_t& type, const string& short_name) {
		map_.insert(pair<type_t, string>(type, short_name));
	}
	const string& getShortName(const type_t& type) {
		return map_[type];
	}

private:
	map_type					map_;

private:
	static self					instance_;
};

template <typename _>
ObjectTypeManagerT<_>	ObjectTypeManagerT<_>::instance_;

typedef ObjectTypeManagerT<void>			ObjectTypeManager;


/**
 *
 * Object<FC>
 *
 */
template <class FC>
class Object :
	public ValidityObject,
	public enable_shared_from_this<FC>
{
public:
	typedef SMART_PTR(FC)					ptr;
	typedef SMART_PTR(FC const)				ptr_const;
	typedef SMART_PTR(const FC)				const_ptr;
	typedef FC*								original_ptr;
	typedef const FC*						original_const_ptr;
	typedef FC&								reference;
	typedef const FC&						const_reference;

public:
	uuid_t uuid;
	type_t type;

public:
	Object() : ValidityObject(),uuid(Null), type(Null) {;}
	virtual ~Object()						{;}

	ptr thisPtr() {
		return this->shared_from_this();
	}
};

}}

#endif /* OBJECT_H_ */
