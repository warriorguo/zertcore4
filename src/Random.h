/*
 * Random.h
 *
 *  Created on: 2013-9-29
 *      Author: Administrator
 */

#ifndef RANDOM_H_
#define RANDOM_H_

#include <common.h>

namespace zertcore{ namespace utils{

/**
 * Random
 */
class Random
{
public:
	typedef double							value_type;

public:
	inline static void init() {
		srand(::time(Null));
	}
	inline static bool happen(const double& r) {
		value_type random = value();
		return random <= r;
	}
	inline static value_type value() {
		return (rand() + 0.0) / RAND_MAX;
	}

	inline static int range(const int& from, const int& to) {
		if (to <= from)
			return 0;

		return rand() % (to - from) + from;
	}

	template <typename T, typename R>
	inline static int range(const T& from, const R& to) {
		if (to <= from)
			return 0;

		return (T)rand() % (to - from) + from;
	}
};


template <class T>
struct GetRandomValue
{
	typedef typename T::iterator			Iterator;
	typedef typename T::const_iterator		ConstIterator;

	Iterator operator() (T& list) {
		int r = Random::range((size_t)0, list.size());

		Iterator it = list.begin();
		for (; it != list.end() && r; --r, ++it);

		return it;
	}
	ConstIterator operator() (const T& list) {
		int r = Random::range(0, list.size());

		ConstIterator it = list.begin();
		for (; it != list.end() && r; --r, ++it);

		return it;
	}
};

/**
template <class T, class Iterator = typename T::const_iterator >
Iterator getRandomValue(const T& list) {
	int r = Random::range(0, list.size());

	Iterator it = list.begin();
	for (; it != list.end() && r; --r, ++it);

	return it;
}
*/


template <class T>
class RateMap :
		public multimap<Random::value_type, T>
{
public:
	const T& dice() const {
		Random::value_type r = Random::value();

		Random::value_type acc(0);
		for (typename multimap<Random::value_type, T>::const_iterator it =
				this->template begin(); it != this->template end(); ++it) {
			acc += it->first;
			if (acc >= r) {
				return it->second;
			}
		}

		static T tmp;
		return tmp;
	}
};

}}


#endif /* RANDOM_H_ */
