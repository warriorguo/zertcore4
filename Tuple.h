/*
 * Tuple.h
 *
 *  Created on: 2013-8-1
 *      Author: Administrator
 */

#ifndef TUPLE_H_
#define TUPLE_H_

namespace zertcore {namespace utils{

template<class U> class ListTpl;

class Tuple
{
public:

private:
};

/**
 *
 * Usage:
 *   Tuple pkg("show me the money", 11, string("operation cwal"), 1.2);
 *   const char* s1; int v1; string s2; float v2;
 *   list(pkg, s1, v1, s2, v2);
 *
 *
 * TupleTpl<Stream>
 *
 */
template <class Stream>
class TupleTpl
{
	template<class U> friend class ListTpl;

	typedef TupleTpl<Stream>		self;

public:
	typedef Stream					stream_type;

public:
	template <typename T1>
	explicit TupleTpl(T1& t1) {
		stream_ << &t1;
	}

	template <typename T1, typename T2>
	explicit TupleTpl(T1& t1, T2& t2) {
		stream_ << &t1 << &t2;
	}

private:
	mutable stream_type	stream_;
};


template <class Stream, typename T1>
void list(const TupleTpl<Stream>& tuple, T1& t1) {
	tuple.stream_ >> t1;
}

template <class Stream, typename T1, typename T2>
void list(const TupleTpl<Stream>& tuple, T1& t1, T2& t2) {
	tuple.stream_ >> t1 >> t2;
}

/**
 *
 * ListTpl<Stream>
 *
 */
template <class Stream>
class ListTpl
{
	typedef ListTpl<Stream>			self;
public:
	typedef Stream					stream_type;
	typedef TupleTpl<Stream>		tuple_type;

public:
	template <typename T1>
	explicit ListTpl(const tuple_type& tuple, T1& t1) {
		tuple.stream_ >> t1;
	}

	template <typename T1, typename T2>
	explicit ListTpl(const tuple_type& tuple, T1& t1, T2& t2) {
		tuple.stream_ >> t1 >> t2;
	}
};

}}


#endif /* TUPLE_H_ */
