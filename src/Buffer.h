/*
 * Buffer.h
 *
 *  Created on: 2013-6-7
 *      Author: Administrator
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <common.h>
#include <ValidityObject.h>

#ifndef BUFFER_INITED_SIZE
#  define BUFFER_INITED_SIZE	1024
#endif

#ifndef BUFFER_PAGE_SIZE
#  define BUFFER_PAGE_SIZE		4096
#endif

namespace zertcore{ namespace utils{

/**
 *
 * Buffer<_>
 *
 */
template <typename _ = void>
class Buffer
		: public ValidityObject
{
	typedef Buffer<_>		self;
public:
	typedef self*			ptr;
	typedef const self*		const_ptr;

	typedef self&			reference;
	typedef const self&		const_reference;

public:
	typedef size_t			size_type;
	typedef byte			value_type;
	typedef value_type*		value_type_ptr;
	typedef const value_type*	value_type_const_ptr;
	typedef value_type&		value_type_ref;
	typedef const value_type&	value_type_const_ref;

public:
	explicit Buffer() : ValidityObject(), data_(NULL), data_dump_(NULL) {
		_init();
	}
	virtual ~Buffer() {
		_deinit();
	}

public:
	value_type_ptr writeBuffer(size_type size) {
		checkValid();

		if (data_capacity_ - data_index_ < size) {
			_realloc(size + data_index_);
		}

		data_dump_ = data_;
		return data_ + data_index_;
	}
	void writeSize(size_type size) {
		checkValid();

		if (data_dump_ != data_) {
			THROW_EXCEPTION(UnValidException());
		}
		data_index_ += size;
		if (data_capacity_ < data_index_) {
			THROW_EXCEPTION(OutOfRangeException());
		}
	}

	const value_type_ptr readBuffer(size_type offset = 0) const {
		checkValid();

		if (offset >= data_capacity_) return NULL;
		return data_ + offset;
	}

	void clear() {
		data_index_ = 0;
	}
	bool empty() const {
		return data_index_ == 0;
	}
	size_type size() const {
		return data_index_;
	}
	size_type capacity() const {
		return data_capacity_;
	}
	bool erase(size_type begin, size_type end) {
		checkValid();

		if (end <= begin) return false;
		if (end > data_index_) return false;

		_copy(data_ + begin, data_ + end, data_index_ - end);
		data_index_ -= end - begin;

		return true;
	}
	void fillWith(const value_type& v) {
		checkValid();

		_fill(0, data_index_, v);
	}
	void fillWithRest(const value_type& v) {
		checkValid();

		_fill(data_index_, data_capacity_, v);
	}
/**
	value_type& operator[] (size_type index) throw (OutOfRangeException) {
		if (index > data_capacity_)
			throw OutOfRangeException();
		return data_[index];
	}
	const value_type& operator[] (size_type index) const throw (OutOfRangeException) {
		if (index > data_capacity_)
			throw OutOfRangeException();
		return data_[index];
	}
*/
	template <typename T>
	void copy(const T& d) {
		checkValid();

		if (data_capacity_ < sizeof(T)) {
			_realloc(sizeof(T));
		}

		_copy(data_, reinterpret_cast<value_type_const_ptr>(&d), sizeof(T));
		data_index_ = sizeof(T);
	}
	template <typename T>
	void copy(const BufferPointer<T>& bp) {
		copy(bp.data, bp.size);
	}

	void copy(const_reference buffer) {
		checkValid();

		if (data_capacity_ < buffer.size()) {
			_realloc(buffer.size());
		}

		_copy(data_, buffer, buffer.size());
	}

	template <typename T>
	void copy(const T* d, size_type size) {
		checkValid();

		if (!d) return ;

		if (data_capacity_ < size * sizeof(T)) {
			_realloc(size * sizeof(T));
		}

		_copy(data_, reinterpret_cast<value_type_const_ptr>(d), size * sizeof(T));
		data_index_ = size;
	}

	reference append(const_reference buffer) {
		checkValid();

		if (data_capacity_ - data_index_ < buffer.size()) {
			_realloc(buffer.size() + data_index_);
		}

		_copy(data_ + data_index_, buffer, buffer.size());

		return *this;
	}

	template <typename T>
	reference append(const BufferPointer<T>& bp) {
		append(bp.data, bp.size);
		return *this;
	}

	template <typename T>
	reference append(const T& d) {
		checkValid();

		if (data_capacity_ - data_index_ < sizeof(T)) {
			_realloc(sizeof(T) + data_index_);
		}

		_copy(data_ + data_index_, reinterpret_cast<value_type_const_ptr>(&d), sizeof(T));
		data_index_ += sizeof(T);

		return *this;
	}

	reference append(const string& data) {
		return append(data.c_str(), data.size());
	}

	template <typename T>
	reference append(const T* d, size_type size) {
		checkValid();

		if (!d) {
			THROW_EXCEPTION(UnValidException());
		}

		if (data_capacity_ - data_index_ < size * sizeof(T)) {
			_realloc(size * sizeof(T) + data_index_);
		}

		_copy(data_ + data_index_, reinterpret_cast<value_type_const_ptr>(d), size * sizeof(T));
		data_index_ += size;

		return *this;
	}

	template <typename T>
	const T& offset(size_type index) const {
		checkValid();
		if (int(size() - index) < (int)sizeof(T)) {
			printf("size:%ld index:%ld\n", size(), index);
			THROW_EXCEPTION(OutOfRangeException());
		}

		return *reinterpret_cast<T *>(data_ + index);
	}

	operator value_type_ptr() {
		checkValid();

		return data_;
	}
	operator value_type_const_ptr() const {
		return data_;
	}
	const void* data() const {
		return reinterpret_cast<const void*>(data_);
	}
	string print(bool show = true, const char* format = "%3x ") {
		checkValid();
		char buf[128];

		string p("\n========BEGIN(BUFFER)========");
		for (size_type i = 0; i < size(); ++i) {
			if (i % 16 == 0) p += "\n";
			vsprintf(buf, format, data_[i]);
			p += buf;
		}

		vsprintf(buf, "\n=========END(SIZE:%d)=========\n", size());
		p += buf;

		if (show) {
			printf("%s", p.c_str());
		}

		return p;
	}

	template <typename T>
	reference operator >> (T& value) {
		checkValid();

		if (size() < sizeof(T)) {
			THROW_EXCEPTION(OutOfRangeException());
		}

		_copy((value_type_ptr)&value, data_, sizeof(T));
		erase(0, sizeof(T));

		return *this;
	}

	template <typename T>
	reference operator >> (BufferPointer<T>& bp) {
		checkValid();
		size_t byte_size = bp.size * sizeof(T);
		size_type suit_size = byte_size > size()? size(): byte_size;

		bp.size = suit_size;
		_copy(bp.input(), data_, suit_size);

		erase(0, suit_size);
		return *this;
	}

	template <typename T>
	reference operator << (const T& value) {
		checkValid();
		append(value);

		return *this;
	}

	template <typename T>
	reference operator << (const BufferPointer<T>& bp) {
		checkValid();
		append(bp);

		return *this;
	}

protected:
	void _init() {
		_deinit();

		size_t adjust_size = adjustPageSize(BUFFER_INITED_SIZE);
		data_dump_ = data_ = (value_type_ptr)malloc(adjust_size);
		data_capacity_ = adjust_size;
	}

	void _deinit() {
		if (data_) {
			free(data_);
		}

		data_ = NULL;
		data_index_ = 0;
		data_capacity_ = 0;
	}

	void _realloc(size_type size) {
		if (size <= data_capacity_) return ;
		size = adjustPageSize(size);

		data_ = (value_type_ptr)realloc(data_, size);

		data_dump_ = data_;
		data_capacity_ = size;
	}

	void _copy(value_type_ptr dst, value_type_const_ptr src, size_type size) {
		memcpy(dst, src, size);
		/**
		for (size_type i = 0; i < size; ++i) {
			dst[i] = src[i];
		}
		*/
	}

	void _fill(size_type from, size_type to, const value_type& v) {
		for (size_type i = from; i < to; ++i) {
			data_[i] = v;
		}
	}

	size_t adjustPageSize(size_t size) const {
		if (size < BUFFER_PAGE_SIZE)
			return BUFFER_PAGE_SIZE;

		if (size % BUFFER_PAGE_SIZE) {
			size = (size / BUFFER_PAGE_SIZE + 1) * BUFFER_PAGE_SIZE;
		}
		else {
			size = (size / BUFFER_PAGE_SIZE) * BUFFER_PAGE_SIZE;
		}

		return size;
	}

private:
	value_type_ptr	data_, data_dump_;
	size_type		data_capacity_;
	size_type		data_index_;
};

/**
template <typename T, class Allocator = allocator<T> >
void Buffer<T, Allocator>::copy(const Buffer<T, Allocator>& buffer) {
	if (data_capacity_ < buffer.size()) {
		_realloc(buffer.size());
	}

	_copy(data_, buffer, buffer.size());
}


template <typename T>
void Buffer<T>::copy(const Buffer<T>* pbuffer) {
	copy(*pbuffer);
}

template <typename T>
void Buffer<T>::append(const Buffer<T>& buffer) {
	if (data_capacity_ - data_index_ < buffer.size()) {
		_realloc(buffer.size() + data_index_);
	}

	_copy(data_ + data_index_, buffer, buffer.size());
}

template <typename T>
void Buffer<T>::append(const Buffer<T>* pbuffer) {
	append(*pbuffer);
}
*/

/**
 * for general using.
 */
typedef Buffer<void>						ByteBuffer;

}}


#endif /* BUFFER_H_ */
