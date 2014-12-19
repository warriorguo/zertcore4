/*
 * PoolObject.h
 *
 *  Created on: 2013-5-5
 *      Author: Administrator
 */

#ifndef POOLOBJECT_H_
#define POOLOBJECT_H_

#include <Object.h>

namespace zertcore {namespace base {

/**
 * every pool object
 */
#define POOLOBJECT_STATIC(FC)	template<> object_pool<FC> PoolObject<FC>::pobject_pool_ = object_pool<FC>();

template <class FC>
typename FC::ptr load();
template <class F, class _Traits>
typename _Traits::ptr load();

template <class FC, class T1>
typename FC::ptr load(T1& t1);
template <class F, typename T1, class _Traits>
typename _Traits::ptr load(T1& t1);

template <class FC, class T1, class T2>
typename FC::ptr load(T1& t1, T2& t2);
template <class F, typename T1, typename T2, class _Traits>
typename _Traits::ptr load(T1& t1, T2& t2);

template <class FC, class T1, class T2, class T3>
typename FC::ptr load(T1& t1, T2& t2, T3& t3);
template <class F, typename T1, typename T2, typename T3, class _Traits>
typename _Traits::ptr load(T1& t1, T2& t2, T3& t3);
/**
template <class FC, class T1, class T2, class T3, class T4>
typename FC::ptr load(T1& t1, T2& t2, T3& t3, T4& t4);
template <class F, typename T1, typename T2, typename T3, class T4, class _Traits>
typename _Traits::ptr load(T1& t1, T2& t2, T3& t3, T4& t4);

template <class FC, class T1, class T2, class T3, class T4, class T5>
typename FC::ptr load(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5);
template <class F, typename T1, typename T2, typename T3, class T4, class T5, class _Traits>
typename _Traits::ptr load(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5);
*/

/**
 * Usage:
 *   MUST be using in the final class!
 */
template <class FC>
class PoolObject:
		public Object<FC>
{
	template <class F> friend typename F::ptr load();
	template <class F, class _Traits> friend typename _Traits::ptr load();

	template <class F, typename T1> friend typename F::ptr load(T1& t1);
	template <class F, typename T1, class _Traits> friend typename _Traits::ptr load(T1& t1);

	template <class F, typename T1, typename T2> friend typename F::ptr load(T1& t1, T2& t2);
	template <class F, typename T1, typename T2, class _Traits> friend typename _Traits::ptr load(T1& t1, T2& t2);

	template <class F, typename T1, typename T2, typename T3>
	friend typename F::ptr load(T1& t1, T2& t2, T3& t3);
	template <class F, typename T1, typename T2, typename T3, class _Traits>
	friend typename _Traits::ptr load(T1& t1, T2& t2, T3& t3);
/**
	template <class F, typename T1, typename T2, typename T3, typename T4>
	friend typename F::ptr load(T1& t1, T2& t2, T3& t3, T4& t4);
	template <class F, typename T1, typename T2, typename T3, typename T4, class _Traits>
	friend typename _Traits::ptr load(T1& t1, T2& t2, T3& t3, T4& t4);

	template <class F, typename T1, typename T2, typename T3, typename T4, typename T5>
	friend typename F::ptr load(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5);
	template <class F, typename T1, typename T2, typename T3, typename T4, typename T5, class _Traits>
	friend typename _Traits::ptr load(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5);
*/
public:
	PoolObject() {;}

public:
	void operator delete(void *ptr) {
		if (pobject_pool_.is_from((typename FC::original_ptr)ptr))
			pobject_pool_.destroy((typename FC::original_ptr)ptr);
		else
			::operator delete(ptr);
	}

public:
	/**
	 * new cant call directly since it return the original pointer,
	 * using load<FC>(..);
	 */
/**
	void* operator new(size_t size) {
		return pobject_pool_.construct();
	}
	template <class T1>
	void* operator new(size_t size, T1& t1) {
		return pobject_pool_.construct(t1);
	}
	template <class T1, class T2>
	void* operator new(size_t size, T1& t1, T2& t2) {
		return pobject_pool_.construct(t1, t2);
	}
	template <class T1, class T2, class T3>
	void* operator new(size_t size, T1& t1, T2& t2, T3& t3) {
		return pobject_pool_.construct(t1, t2, t3);
	}
*/

private:
	static object_pool<FC>	pobject_pool_;
};

template<class FC>
object_pool<FC> PoolObject<FC>::pobject_pool_;

template <class FC, class _Traits>
typename _Traits::ptr load() {
	typename _Traits::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct());
	return ptr;
}

template <class FC>
typename FC::ptr load() {
	typename FC::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct());
	return ptr;
}

template <class FC, typename T1, class _Traits>
typename _Traits::ptr load(T1& t1) {
	typename _Traits::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct(t1));
	return ptr;
}

template <class FC, class T1>
typename FC::ptr load(T1& t1) {
	typename FC::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct(t1));
	return ptr;
}

template <class FC, typename T1, typename T2, class _Traits>
typename _Traits::ptr load(T1& t1, T2& t2) {
	typename _Traits::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct(t1, t2));
	return ptr;
}

template <class FC, typename T1, typename T2>
typename FC::ptr load(T1& t1, T2& t2) {
	typename FC::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct(t1, t2));
	return ptr;
}

template <class FC, typename T1, typename T2, typename T3, class _Traits >
typename _Traits::ptr load(T1& t1, T2& t2, T3& t3) {
	typename _Traits::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct(t1, t2, t3));
	return ptr;
}

template <class FC, typename T1, typename T2, typename T3>
typename FC::ptr load(T1& t1, T2& t2, T3& t3) {
	typename FC::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct(t1, t2, t3));
	return ptr;
}

/**
template <class FC, typename T1, typename T2, typename T3, typename T4, class _Traits >
typename _Traits::ptr load(T1& t1, T2& t2, T3& t3, T4& t4) {
	typename _Traits::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct(t1, t2, t3, t4));
	return ptr;
}

template <class FC, typename T1, typename T2, typename T3, typename T4>
typename FC::ptr load(T1& t1, T2& t2, T3& t3, T4& t4) {
	typename FC::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct(t1, t2, t3, t4));
	return ptr;
}

template <class FC, typename T1, typename T2, typename T3, typename T4, typename T5, class _Traits >
typename _Traits::ptr load(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5) {
	typename _Traits::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct(t1, t2, t3, t4, t5));
	return ptr;
}

template <class FC, typename T1, typename T2, typename T3, typename T4, typename T5>
typename FC::ptr load(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5) {
	typename FC::ptr ptr((FC *)PoolObject<FC>::pobject_pool_.construct(t1, t2, t3, t4, t5));
	return ptr;
}
*/
}}

#endif /* POOLOBJECT_H_ */
