/*
 * precompute.h
 *
 *  Created on: 2013-5-21
 *      Author: Administrator
 */

#ifndef PRECOMPUTE_H_
#define PRECOMPUTE_H_

#include <list>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <string>

#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

namespace zertcore {
using namespace std;
using namespace boost;

struct type_unknown				{};
struct type_string				{};
struct type_stl_container		{};
struct type_stl_map				{};
struct type_smart_ptr			{};
// struct type_class			{};
struct type_arithmetic			{};
struct type_ptr					{};

template <typename T, typename _ = void>
struct is_stl_container: public false_type
{
	enum {
		value = false,
	};
};

template <typename T>
struct is_stl_container<vector<T> >: public true_type
{
	enum {
		value = true,
	};
};

template <typename T>
struct is_stl_container<set<T> >: public true_type
{
	enum {
		value = true,
	};
};

template <typename T>
struct is_stl_container<deque<T> >: public true_type
{
	enum {
		value = true,
	};
};

template <typename T>
struct is_stl_container<list<T> >: public true_type
{
	enum {
		value = true,
	};
};

template <typename T, typename _ = void>
struct is_stl_map: public false_type
{
	enum {
		value = false,
	};
};

template <typename T, typename U>
struct is_stl_map<map<T, U> >: public true_type
{
	enum {
		value = true,
	};
};

template <typename T>
struct is_string: public false_type
{
	enum {
		value = false,
	};
};

template <>
struct is_string<string>: public true_type
{
	enum {
		value = true,
	};
};

template <typename T>
struct is_smart_ptr: public false_type
{
	enum {
		value = false,
	};
};

template <typename T>
struct is_smart_ptr<shared_ptr<T> >: public true_type
{
	enum {
		value = true,
	};
};

template <typename T>
struct is_string_ptr: public false_type
{
	enum {
		value = false,
	};
};

template <>
struct is_string_ptr<char *>: public true_type
{
	enum {
		value = true,
	};
};

template <>
struct is_string_ptr<const char *>: public true_type
{
	enum {
		value = true,
	};
};

template <typename T, typename _ = void>
struct what_type {
	typedef type_unknown type;
};

template <typename T>
struct what_type<T, typename enable_if<is_string<T> >::type > {
	typedef type_string type;
};

template <typename T>
struct what_type<T, typename enable_if<
							is_stl_container<T> >::type > {
	typedef type_stl_container type;
};

template <typename T>
struct what_type<T, typename enable_if<
							is_stl_map<T> >::type> {
	typedef type_stl_map type;
};

template <typename T>
struct what_type<T, typename enable_if<
							is_smart_ptr<T> >::type>  {
	typedef type_smart_ptr type;
};

template <typename T>
struct what_type<T, typename enable_if<
							is_arithmetic<T> >::type>  {
	typedef type_arithmetic type;
};

template <typename T>
struct what_type<T, typename enable_if<
							is_pointer<T> >::type>  {
	typedef type_ptr type;
};

#define BUILD_WHAT_TYPE_TRAP(type, tpl, s) namespace zertcore {struct type_##type{};\
template <typename T>	\
struct is_##type: public false_type \
{\
	enum {\
		value = false,\
	};\
};\
template <tpl > struct is_##type<s >: public true_type\
{\
	enum {\
		value = true,\
	};\
};\
template <typename T>\
struct what_type<T, typename enable_if<\
					is_##type<T> >::type>  {\
	typedef type_##type type;\
};}

/**
 // would be ambiguous
template <typename T >
struct what_type<T, typename enable_if<
							is_class<T> >::type> {
	typedef type_class type;
};
*/

//helper functions
template <typename T>
inline bool check_stl_container(T& c) {
	return is_stl_container<T>::value;
}

template <typename T>
inline bool check_stl_map(T& c) {
	return is_stl_map<T>::value;
}

template <typename T>
inline bool check_string(T& c) {
	return is_string<T>::value;
}

template <typename T>
inline bool check_smart_ptr(T& c) {
	return is_smart_ptr<T>::value;
}

template <typename T>
inline bool check_string_ptr(T& c) {
	return is_string_ptr<T>::value;
}

template <typename T>
inline bool check_ptr(T& c) {
	return is_pointer<T>::value;
}

template <typename T>
inline bool check_class(T& c) {
	return is_class<T>::value;
}

#ifdef PRECOMPUTE_TESTING
//testing
void tellme(type_unknown v) {
	printf("type_unknown\n");
}

void tellme(type_string v) {
	printf("type_string\n");
}

void tellme(type_stl_container v) {
	printf("type_stl_container\n");
}

void tellme(type_stl_map v) {
	printf("type_stl_map\n");
}

void tellme(type_smart_ptr v) {
	printf("type_smart_ptr\n");
}

void tellme(type_class v) {
	printf("type_class\n");
}

template<typename T>
void test_type(const T& v) {
	typedef typename what_type<T>::type type;
	tellme(type());
}
#endif

class TypeNull;

template<typename Head, typename Tail>
struct TypeList
{
    typedef Head							head;
    typedef Tail							tail;
};


/**
 *
 * TypeListAt
 *
 */
template <class TypeListT, unsigned int index>
struct TypeListAt;

template <typename Head, typename Tail>
struct TypeListAt<TypeList<Head, Tail>, 0>
{
	typedef Head							type;
};

template<typename Head, typename Tail, unsigned int index>
struct TypeListAt<TypeList<Head, Tail>, index>
{
	typedef typename TypeListAt<Tail, index-1>::type
											type;
};


/**
 *
 * TypeListIndexOf
 *
 */
template <class TypeListT, class T>
struct TypeListIndexOf;

template <class T>
struct TypeListIndexOf<TypeNull, T>
{
	enum {
		value								= -1,
	};
};

template <class T, class Tail>
struct TypeListIndexOf<TypeList<T, Tail>, T >
{
	enum {
		value								= 0,
	};
};

template <class Head, class Tail, class T>
struct TypeListIndexOf<TypeList<Head, Tail>, T >
{
private:
	enum {
		temp								= TypeListIndexOf<Tail, T>::value,
	};
public:
	enum {
		value								= temp == -1? -1: temp + 1,
	};
};


// enough for now.. can be expand at any point in time as needed
#define TYPELIST_1(T1)                  	TypeList<T1, TypeNull>
#define TYPELIST_2(T1, T2)              	TypeList<T1, TYPELIST_1(T2) >
#define TYPELIST_3(T1, T2, T3)          	TypeList<T1, TYPELIST_2(T2, T3) >
#define TYPELIST_4(T1, T2, T3, T4)      	TypeList<T1, TYPELIST_3(T2, T3, T4) >
#define TYPELIST_5(T1, T2, T3, T4, T5)  	TypeList<T1, TYPELIST_4(T2, T3, T4, T5) >
#define TYPELIST_6(T1, T2, T3, T4, T5, T6)  TypeList<T1, TYPELIST_5(T2, T3, T4, T5, T6) >
#define TYPELIST_7(T1, T2, T3, T4, T5, T6, T7)	TypeList<T1, TYPELIST_6(T2, T3, T4, T5, T6, T7) >

}


#endif /* PRECOMPUTE_H_ */
