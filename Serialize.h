/*
 * Serialize.h
 *
 *  Created on: 2013-5-5
 *      Author: Administrator
 */

#ifndef SERIALIZE_H_
#define SERIALIZE_H_

#include <common.h>

namespace zertcore {namespace utils {

namespace serialize{
	enum {
		sentinel = 0xdeadbeef,
	};

	enum {
		serialize		= 0,
		unserialize		= 1,
	};
}

template<typename Stream>
class Serialize
{
public:
	typedef Serialize<Stream>				self;
	typedef self&							reference;

	typedef SMART_PTR(self)					ptr;

public:
	explicit Serialize(Stream& stream, action_index_t action = non_action): stream_(stream), action_(action)
											{;}

public:
	template <typename T, typename U>
	void handleMap(const map<T, U>& map_data) {
		typedef typename map<T, U>::iterator it_type;
		stream_ << map_data.size();

		map<T, U>& stlmap = const_cast<map<T, U>&>(map_data);
		for (it_type it = stlmap.begin(); it != stlmap.end(); ++it) {
			(*this) & (it->first);
			(*this) & (it->second);
		}
	}

	template <typename T>
	void handleSTLContainer(const T& stl_data) {
		typedef typename T::iterator it_type;
		stream_ << stl_data.size();

		T& stl = const_cast<T&>(stl_data);
		for (it_type it = stl.begin(); it != stl.end(); ++it) {
			(*this) & *it;
		}
	}

	void handleString(const string& str) {
		size_t length = str.size();
		stream_ << length;
		const char* b = str.c_str();

		for (size_t i = 0; i < length; ++i) {
			stream_ << b[i];
		}
	}

	template <typename T>
	void handleSmartPtr(const T& ptr) {
		const_cast<T&>(ptr)->serializer(*this);
	}

	template <typename T>
	void handleClass(const T& cls) {
		const_cast<T&>(cls).serializer(*this, serialize::serialize, action_);
	}

	template <typename T>
	void handleBufferPointer(const BufferPointer<T>& bp) {
		stream_ << bp.size;
		for (size_t i = 0; i < bp.size; ++i) {
			(*this) & bp.data[i];
		}
	}

	void handleStringPtr(const char *& str) {
		int length = strlen(str);
		stream_ << length;
		for (--length; length >= 0; --length) {
			stream_ << str[length];
		}
	}

protected:
	template <typename T>
	void handle(const T& data, type_unknown v) {
		handleClass(data);
		makeSentinel();
	}

	template <typename T>
	void handle(const T& data, type_string v) {
		handleString(data);
		makeSentinel();
	}

	template <typename T>
	void handle(const T& data, type_stl_container v) {
		handleSTLContainer(data);
		makeSentinel();
	}

	template <typename T>
	void handle(const T& data, type_stl_map v) {
		handleMap(data);
		makeSentinel();
	}

	template <typename T>
	void handle(const T& data, type_smart_ptr v) {
		handleSmartPtr(data);
		makeSentinel();
	}

	template <typename T>
	void handle(const T& data, type_arithmetic v) {
		stream_ << data;
	}

	template <typename T>
	void handle(const BufferPointer<T>& bp, type_bufferpointer v) {
		handleBufferPointer(bp);
		makeSentinel();
	}

	template <typename T>
	void handle(const T& ptr, type_ptr v) {
		handleClass(*ptr);
		makeSentinel();
	}

public:
	template <typename T>
	reference operator &(const T& data)// throw (IOException, UnExpectedException)
	{
		typedef typename what_type<T>::type type;
		handle(data, type());
		return *this;
	}
	// for class string, vector, set, deque, map, list

private:
	// for each class, insert a serialize::sentinel in
	void makeSentinel() {
		uint32 s = serialize::sentinel;
		stream_ << s;
	}

private:
	Stream& stream_;
	action_index_t	action_;
};

template<typename Stream>
class Unserialize
{
public:
	typedef Unserialize<Stream>				self;
	typedef self&							reference;

	typedef SMART_PTR(self)					ptr;

public:
	explicit Unserialize(Stream& stream, action_index_t action = Null):
		stream_(stream), action_(action)	{;}

public:
	template <typename T, typename U>
	void handleMap(map<T, U>& map_data) {
		size_t size; stream_ >> size;

		map_data.reserve(size);
		for (size_t i = 0; i < size; ++i) {
			T k; U v;
			*this & k & v;
			map_data.insert(pair<T, U>(k, v));
		}
	}

	template <typename T>
	void handleSTLContainer(T& stl_data) {
		typedef typename T::value_type value_type;
		size_t size; stream_ >> size;

		stl_data.reserve(size);
		for (size_t i = 0; i < size; ++i) {
			value_type v;
			*this & v;

			stl_data.insert(stl_data.end(), v);
		}
	}

	void handleString(string& str) {
		size_t length;
		stream_ >> length;

		str.reserve(length);
		for (size_t i = 0; i < length; ++i) {
			string::value_type ch;
			stream_ >> ch;
			str += ch;
		}
	}

	template <typename T>
	void handleBufferPointer(BufferPointer<T>& bp) {
		size_t length;
		stream_ >> length;

		T* data = bp.input();
		for (size_t i = 0; i < length; ++i) {
			if (i < bp.size) {
				(*this) & data[i];
			}
		}
	}

	template <typename T>
	void handleSmartPtr(T& ptr) {
		ptr->serializer(*this);
	}


protected:
	template <typename T>
	void handleClass(T& cls) {
		cls.serializer(*this, serialize::unserialize, action_);
	}

	template <typename T>
	void handle(T& data, type_unknown v) {
		handleClass(data);
		checkSentinel();
	}

	template <typename T>
	void handle(T& data, type_string v) {
		handleString(data);
		checkSentinel();
	}

	template <typename T>
	void handle(T& data, type_stl_container v) {
		handleSTLContainer(data);
		checkSentinel();
	}

	template <typename T>
	void handle(T& data, type_stl_map v) {
		handleMap(data);
		checkSentinel();
	}

	template <typename T>
	void handle(T& data, type_smart_ptr v) {
		handleSmartPtr(data);
		checkSentinel();
	}

	template <typename T>
	void handle(T& data, type_arithmetic v) {
		stream_ >> data;
	}

	template <typename T>
	void handle(T& ptr, type_ptr v) {
		handleClass(*ptr);
		checkSentinel();
	}

	template <typename T>
	void handle(BufferPointer<T>& bp, type_bufferpointer v) {
		handleBufferPointer(bp);
		checkSentinel();
	}

public:
	template <typename T>
	reference operator &(T& data) // never know what would be throw out,
								  // throw (IOException, OutOfRangeException, DataVerifyException)
	{
		typedef typename what_type<T>::type type;
		handle(data, type());

		return *this;
	}
		// for class string, vector, set, deque, map, list

private:
	// for each class, insert a serialize::sentinel in
	void checkSentinel() {
		uint32 sec;
		stream_ >> sec;
		if (sec != serialize::sentinel)
			THROW_EXCEPTION(DataVerifyException());
	}

private:
	Stream& stream_;
	action_index_t	action_;
};

/**
 * this is useless, just disable it
 */
template <class IO, class FC>
void serializer(IO& io, FC& fc);


template <class FC, class Container>
class BasicStream
{
public:
	typedef FC&							reference;
	typedef const FC&					const_reference;

	typedef FC*							ptr;

public:
	explicit BasicStream(Container& container) :
		index_(0), container_(container) {
		;
	}

public:
	template <typename T>
	reference operator >> (T& data) {
		byte* p = (byte *)&data;
		if (index_ + sizeof(T) > size())
			THROW_EXCEPTION(OutOfRangeException());

		for (size_t i = 0; i < sizeof(T); ++i)
			p[i] = container_[i + index_];

		index_ += sizeof(T);
		return *this;
	}

	template <typename T>
	reference operator << (const T& data) {
		byte* p = (byte *)&data;
		for (size_t i = 0; i < sizeof(T); ++i)
			container_[i + index_] = p[i];

		index_ += sizeof(T);
		return *this;
	}

public:
	const byte* data() const {
		return &container_[0];
	}

	size_t size() const {
		return container_.size();
	}

	void reset() {
		index_ = 0;
	}

private:
	size_t		index_;
	Container&	container_;
};


class SimpleStream :
		public BasicStream<SimpleStream, vector<byte> >
{
public:
	explicit SimpleStream() :
		BasicStream<SimpleStream, vector<byte> >(data_) {
		;
	}

private:
	vector<byte> data_;
};

struct WriterOnlyStreamList
{
	void dispatch(const uint command, const BytePointer& bp,
			const BufferPointer<uuid_t>& targets) {;}
};

DECLARE_STATIC_VAR(WriterOnlyStreamList WriteronlyStreamlist);



/**
 * for sample and unit testing
 */

class ExampleStream:
		public SimpleStream
{

public:
	void setData(const void* data, size_t size) {
		;
	}
};

/**
 *
 */
template <class Stream>
class StreamList
{
public:
	typedef Stream			value_type;

public:
	void dispatch(const uint command, const BytePointer& bp,
			const BufferPointer<uuid_t>& targets) {
		const char* d = (const char *)bp.data;
		for (size_t i = 0; i < bp.size; ++i) {
			printf("%c", d[i]);
		}

		printf("\n");
	}
};

/**
SessionStream stream;
Serialize<SessionStream> serialize(stream);

SessionAction action;
serializer(serialize, action);
action.perform();
*/

}}
#endif /* SERIALIZE_H_ */
