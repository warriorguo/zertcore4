/*
 * Expired.h
 *
 *  Created on: 2014-3-24
 *      Author: Administrator
 */

#ifndef EXPIRED_H_
#define EXPIRED_H_

#include <common.h>
#include <Runtime.h>
#include <PoolObject.h>

namespace zertcore{ namespace utils{
using namespace zertcore::base;

enum {
	EXPIRED_USER							= 0x10000,
};

/**
 * ExpiredBase
 */
struct ExpiredBase
{
	typedef SMART_PTR(ExpiredBase)			ptr;

	struct ExpiredManagerBase
	{
		virtual ~ExpiredManagerBase() {}
		virtual void remove(const uuid_t& key)
											= 0;
	};

	ExpiredBase(ExpiredManagerBase& manager) : manager_(manager), key_(0) {}
	virtual ~ExpiredBase() {
	}

	void remove() {
		manager_.remove(key_);
	}

	uuid_t getKey() {
		return key_;
	}
	void setKey(const uuid_t& key) {
		key_ = key;
	}

private:
	ExpiredManagerBase&			manager_;

private:
	uuid_t						key_;
};

/**
 * Expired<T>
 */
template <typename T>
class Expired :
		public ExpiredBase,
		public PoolObject<Expired<T> >,
		public Updater<Expired<T> >
{
public:
	typedef SMART_PTR(Expired<T>)			ptr;

public:
	typedef Expired<T>						self;
	typedef self&							reference;
	typedef const Expired<T>&				const_reference;

public:
	typedef T								value_type;
	typedef T&								value_ref_type;
	typedef const T&						const_value_ref_type;

public:
	explicit Expired(ExpiredBase::ExpiredManagerBase& manager, const_value_ref_type value) :
		ExpiredBase(manager), value_(value) {}
	explicit Expired(ExpiredBase::ExpiredManagerBase& manager, const_reference ref) :
		ExpiredBase(manager) {
		value_ = ref.value_;
	}

public:
	value_ref_type value() {
		return value_;
	}
	const_value_ref_type value() const {
		return value_;
	}

	reference operator =(const_value_ref_type value) {
		value_ = value;
		return *this;
	}
	reference operator =(const_reference ref) {
		value_ = ref.value_;
		return *this;
	}

public:
	void init(time_type expired) {
		this->template enableOnce(expired);
	}
	bool once(const time_type& interval) {
		remove();
		return false;
	}

private:
	value_type					value_;
};

/**
 * ExpiredManager
 */
template <typename _>
class ExpiredManagerT :
		public ExpiredBase::ExpiredManagerBase
{
public:
	typedef map<uuid_t, ExpiredBase::ptr>	contain_map_type;

public:
	static ExpiredManagerT& Instance() {
		return instance_;
	}

public:
	virtual ~ExpiredManagerT() {}

public:
	template <typename T>
	bool generate(const uuid_t& key, const T& value, time_type expired_time) {
		typename Expired<T>::ptr object = load<Expired<T> >(*this, value);
		object->init(expired_time);
		object->setKey(key);

		pair<contain_map_type::iterator, bool> ret =
				contain_map_.insert(contain_map_type::value_type(key, object));
		return ret.second;
	}
	template <typename T>
	typename Expired<T>::ptr get(const uuid_t& key) {
		contain_map_type::iterator it = contain_map_.find(key);
		if (it == contain_map_.end())
			return Null;

		return dynamic_pointer_cast<Expired<T> >(it->second);
	}

	bool exists(const uuid_t& key) {
		return contain_map_.find(key) != contain_map_.end();
	}

public:
	virtual void remove(const uuid_t& key) {
		contain_map_.erase(key);
	}

	void clear() {
		contain_map_.clear();
	}

private:
	contain_map_type			contain_map_;

private:
	static ExpiredManagerT<_>	instance_;
};
template <typename _>
ExpiredManagerT<_>				ExpiredManagerT<_>::instance_;

typedef ExpiredManagerT<void>				ExpiredManager;

}}


#endif /* EXPIRED_H_ */
