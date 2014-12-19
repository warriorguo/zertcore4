/*
 * TimeType.h
 *
 *  Created on: 2013-8-6
 *      Author: Administrator
 */

#ifndef TIMETYPE_H_
#define TIMETYPE_H_

#include <ctime>
#include <time.h>
#include <sys/time.h>

namespace zertcore{ namespace time{

struct TimeType{
	typedef TimeType						self;
	typedef self&							reference;
	typedef const self&						const_reference;

	typedef double							timestamp_type;

	/**
	 *
	 */
	mutable timestamp_type timestamp;

	TimeType() : timestamp(0) {
		fresh();
	}
	TimeType(timestamp_type t) {
		timestamp = t;
	}
	TimeType(const_reference tt) {
		timestamp = tt.timestamp;
	}

public:
	operator bool() const {
		return timestamp != 0;
	}

public:
	reference operator =(const_reference tt) {
		timestamp = tt.timestamp;
		return *this;
	}
	reference operator =(timestamp_type t) {
		timestamp = t;
		return *this;
	}
	reference operator +=(const_reference tt) {
		timestamp += tt.timestamp;
		return *this;
	}
	reference operator +=(timestamp_type t) {
		timestamp += t;
		return *this;
	}
	reference operator -=(const_reference tt) {
		timestamp -= tt.timestamp;
		return *this;
	}
	reference operator -=(timestamp_type t) {
		timestamp -= t;
		return *this;
	}

	template <typename T>
	reference operator *=(const T& v) {
		timestamp *= v;
		return *this;
	}
	template <typename T>
	reference operator /=(const T& v) {
		timestamp /= v;
		return *this;
	}

public:
	bool operator ==(const_reference tt) const {
		return timestamp == tt.timestamp;
	}
	bool operator !=(const_reference tt) const {
		return timestamp != tt.timestamp;
	}
	bool operator >(const_reference tt) const {
		return timestamp > tt.timestamp;
	}
	bool operator >=(const_reference tt) const {
		return timestamp >= tt.timestamp;
	}
	bool operator <(const_reference tt) const {
		return timestamp < tt.timestamp;
	}
	bool operator <=(const_reference tt) const {
		return timestamp <= tt.timestamp;
	}

	operator timestamp_type() const {
		return timestamp;
	}

public:
	TimeType fresh() const {
		struct timeval tv;
		gettimeofday(&tv, NULL);

		timestamp = (timestamp_type)tv.tv_sec + (timestamp_type)tv.tv_usec / 1000000.0;
		return *this;
	}
	string toString(const char* format = "%Y-%m-%d %H:%m:%s") {
		char tm[32];

		time_t rawtime = (time_t)timestamp;
		struct tm *dt = localtime(&rawtime);
		strftime(tm, sizeof(tm), format, dt);

		return tm;
	}

};


inline TimeType operator + (TimeType::const_reference t1, TimeType::const_reference t2) {
	TimeType r(t1); r += t2;
	return r;
}
inline TimeType operator - (TimeType::const_reference t1, TimeType::const_reference t2) {
	TimeType r(t1); r -= t2;
	return r;
}

template <typename T>
inline TimeType operator * (TimeType::const_reference t1, const T& v) {
	TimeType r(t1); r *= v;
	return r;
}

template <typename T>
inline TimeType operator / (TimeType::const_reference t1, const T& v) {
	TimeType r(t1); r /= v;
	return r;
}

inline double operator / (TimeType::const_reference t1, const TimeType::const_reference t2) {
	return t1.timestamp / t2.timestamp;
}

}}

namespace zertcore{ namespace utils{

enum {
	EXPIRED_EVERY_MINUTE					= 1,
	EXPIRED_EVERY_HOUR						= 2,
	EXPIRED_EVERY_DAY						= 3,
};

template <uint type>
class ClockExpiredUnit
{
public:
	typedef string							update_key_type;
public:
	bool expired() const {
		return update_key_ != new_key_;
	}

	inline static update_key_type newKey() {
		time_t rawtime;
		struct tm* timeinfo;
		::time(&rawtime);
		timeinfo = ::localtime(&rawtime);

		update_key_type::value_type buf[32] = {0};

		switch(type) {
		case EXPIRED_EVERY_DAY:
			sprintf(buf, "%d%d", timeinfo->tm_year, timeinfo->tm_yday);
			break;
		case EXPIRED_EVERY_HOUR:
			sprintf(buf, "%d%d%d", timeinfo->tm_year, timeinfo->tm_yday,
					timeinfo->tm_hour);
			break;
		case EXPIRED_EVERY_MINUTE:
			sprintf(buf, "%d%d%d%d", timeinfo->tm_year, timeinfo->tm_yday,
					timeinfo->tm_hour, timeinfo->tm_min);
			break;
		}

		return buf;
	}

	inline static void update() {
		new_key_ = newKey();
	}

	update_key_type key() const {
		return update_key_;
	}

	void key(const update_key_type& update_key) {
		update_key_ = update_key;
	}

	void reset() {
		update_key_ =  newKey();
	}

private:
	update_key_type				update_key_;
private:
	static update_key_type		new_key_;
};

template <uint type>
typename ClockExpiredUnit<type>::update_key_type ClockExpiredUnit<type>::new_key_;

}}

#endif /* TIMETYPE_H_ */
