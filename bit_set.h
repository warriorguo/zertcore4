/*
 * Bitset.h
 *
 *  Created on: 2013-11-19
 *      Author: Administrator
 */

#ifndef BITSET_H_
#define BITSET_H_

#include <cstdio>
#include <string>

namespace zertcore{ namespace utils{

template <typename T = unsigned long long>
class Bitset
{
public:
	typedef T								container_type;
	typedef size_t							size_type;
	enum {
		BIT_SIZE							= (sizeof(container_type) * 8),
	};

public:
	Bitset() : index_(0), size_(0), container_size_(0), container_(NULL) {
		clear();
	}
	/**
	 * size count by byte
	 */
	Bitset(const void* data, const size_type& size) : index_(0), size_(0),
			container_size_(0), container_(NULL) {
		clear();

		size_type container_size = size / sizeof(container_type) + 1;

		init(container_size * BIT_SIZE);

		const container_type* container = (const container_type *)data;
		for (size_type i = 0; i < container_size; ++i) {
			container_[i] = container[i];
		}

		index_ = size * 8;
	}

	virtual ~Bitset() {
		deinit();
	}

public:
	bool init(size_type size = 0) {
		if (size) {
			container_size_ = size / BIT_SIZE;
			if (size % BIT_SIZE)
				container_size_++;
		}

		if (container_)
			delete[] container_;

		if (container_size_)
			container_ = new container_type[container_size_];

		size_ = container_size_ * BIT_SIZE;
		clear();

		return true;
	}

public:
	bool append(container_type p, size_type size) {
		for (size_type i = 0; i < size; ++i) {
			if (!append(p & (1 << i)))
				return false;
		}
		return true;
	}
	bool append(const string& v) {
		for (size_t i = 0; i < v.size(); ++i) {
			if (!append(v[i] == '1'))
				return false;
		}
		return true;
	}
	bool append(int flag) {
		return set(index_, (bool)flag);
	}

	bool set(size_type index, bool flag) {
		if (index >= size_) {
			return false;
		}

		if (index >= index_)
			index_ = index + 1;

		return bitSetc(container_[index / BIT_SIZE],
				index % BIT_SIZE, flag);
	}

	bool get(const size_type& index) const {
		if (index >= size_)
			return false;

		return bitGetc(container_[index / BIT_SIZE],
				index % BIT_SIZE);
	}
	bool get(container_type& p, const size_type& index, const size_type& size) const {
		p = 0;

		for (size_type i = 0; i < size; ++i) {
			if (get(i + index)) {
				p |= 1L << i;
			}
		}

		return true;
	}
	void clear() {
		index_ = 0;
		for (size_type i = 0; i < container_size_; ++i) {
			container_[i] = 0;
		}
	}
	void print(const size_type& size) {
		printf("[");
		for (size_type i = 0; i < size; ++i) {
			if (get(i)) printf("1");
			else printf("0");
		}
		printf("]\n");
	}

public:
	size_type size() const {
		size_type ext = index_ % 8? 1: 0;
		return index_ / 8 + ext;
	}
	size_type bitSize() const {
		return index_;
	}
	size_type capacity() const {
		return sizeof(container_type) * container_size_;
	}
	const char* data() const {
		return (const char *)container_;
	}
	bool data(char* data, size_type& size /* IN out OUT*/) {
		size_type container_size = size / sizeof(container_type);

		container_type* container = (container_type *)data;

		size_type i = 0;
		for (; i < container_size && i < container_size_; ++i) {
			container_[i] = container[i];
		}
		size = i * sizeof(container_type);

		return i == container_size_;
	}
private:
	void deinit() {
		if (container_)
			delete[] container_;
		container_ = NULL;
	}

	bool bitGetc(const container_type& c, const size_type& index) const {
		if (index >= BIT_SIZE)
			return false;

		return c & (1L << index);
	}

	bool bitSetc(container_type& c, const size_type& index, bool flag) {
		if (index >= BIT_SIZE)
			return false;

		if (flag) {
			c |= 1L << index;
		}
		else {
			container_type tc = ~c;
			tc |= 1L << index;
			c = ~tc;
		}

		return true;
	}

private:
	size_type					index_;
	size_type					size_;
	size_type					container_size_;
	container_type*				container_;
};

}}

#endif /* BITSET_H_ */
