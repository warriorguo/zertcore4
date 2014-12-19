/*
 * common.h
 *
 *  Created on: 2013-5-5
 *      Author: Administrator
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <functional>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <list>
#include <vector>
#include <map>
#include <set>
#include <deque>

#include <boost/array.hpp>
#include <boost/any.hpp>
#include <boost/call_traits.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/throw_exception.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/type_traits.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/regex.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/locale.hpp>

#include <exception.h>
#include <precompute.h>
#include <time_type.h>
#include <bit_set.h>
#include <utils.h>

#define THROW_EXCEPTION(x)					throw(x)
#define SET_ERROR(error_message)			printf(error_message);

#ifdef __GNUC__
#define VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#define VARIABLE_IS_NOT_USED
#endif

#define DECLARE_STATIC_VAR(var)				static var;
#define SMART_PTR(x)						::boost::shared_ptr<x >
#define WEAK_PTR(x)							::boost::weak_ptr<x >

namespace zertcore {

using namespace std;
using namespace boost;

using namespace boost::locale;

#define STRUCT_OFFSET(Type, Item)	\
	((size_t)(((Type*)0)->Item))

typedef uint8_t								uint8;
typedef uint16_t							uint16;
typedef uint32_t							uint32;
typedef uint64_t							uint64;

typedef int8_t								int8;
typedef int16_t								int16;
typedef int32_t								int32;
typedef int64_t								int64;

typedef unsigned char						byte;

/**
 * Utils
 */
template <typename T>
void del(T* it) {
	delete it;
}
template <typename T>
void del(SMART_PTR(T) it) {
	; // DO NOTHING for smart point
}

/**
 * for client config
 */
const uint LOG_LEVEL						= 0;

typedef uint64_t							uuid_t;
typedef uint32_t							type_t;

typedef uint64_t							action_index_t;

typedef uint8								eof_key_type;

/**
 * time::TimeType
 */
typedef time::TimeType						time_type;

enum {
	TYPE_NONE								= 0,
	TYPE_CONFIG								= 1,
	TYPE_LOG								= 2,
	TYPE_SYSTEM								= 3,
	TYPE_SERVER								= 4,

	TYPE_IMV								= 5,

	TYPE_BASE								= 10,
};

template <typename T, typename U>
T cast(const U& p) {
	T ret;
	try {
		ret = lexical_cast<T>(p);
	}
	catch(bad_lexical_cast&) {
		;
	}

	return ret;
}

template <typename T>
struct BufferPointer
{
	typedef T								value_type;
	typedef value_type*						data_type;
	typedef const value_type*				const_data_type;
	typedef size_t							size_type;

	const_data_type				data;
	size_type					size;
/**
	data_type	io_data;
	size_type	io_size;
*/
	explicit BufferPointer() : data(NULL), size(0){
		;
	}
	/**
	 *
	 * NOTICE: size s is the AMOUNT OF elements in data
	 * NOT in bytes size of data.
	 *
	 */
	explicit BufferPointer(const T*	d, const size_t& s) :
		data(d), size(s) {;}

	/**
	 * input to the data
	 */
	data_type input() {return const_cast<data_type>(data);}

	value_type& operator [](size_type index) {
		return at(index);
	}
	value_type& at(size_type index) {
		if (index >= size) {
			THROW_EXCEPTION(OutOfRangeException());
		}

		data_type d = input();
		return d[index];
	}
};

typedef BufferPointer<byte>					BytePointer;

/**
 *
 * Set the global mutex type
 *
 */
typedef ::boost::mutex						mutex_type;

}
/**
 * enable what_type<>
 */

BUILD_WHAT_TYPE_TRAP(bufferpointer, typename T, BufferPointer<T>);

#include <geometry.h>

namespace zertcore {

typedef size_t								position_t;
typedef uint								coord_t;

typedef Point2d<coord_t>					location_t;
typedef list<location_t>					location_list_type;
typedef vector<location_t>					location_vector_type;
typedef set<location_t, Point2dLess<location_t::value_type> >
											location_set_type;

}

#include <nullval.h>

namespace zertcore {

enum {
	non_action		= 0,
};

enum {
	SERVER_IO_PACKAGE_SIZE					= 1024 * 1024,
	SERVER_IO_EOF							= 0xff,
};

enum {
	AND, OR
};

enum {
	MAX, MIN
};

enum {
	NEGATIVE 								= -1,
	POSITIVE								= 1,
};

enum {
	MAX_UINT								= (uint)-1,
};

#define COMMON_SAVE_INTERVAL			5.0

}

#endif /* COMMON_H_ */
