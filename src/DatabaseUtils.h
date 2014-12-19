/*
 * DatabaseUtils.h
 *
 *  Created on: 2013-9-19
 *      Author: Administrator
 */

#ifndef DATABASEUTILS_H_
#define DATABASEUTILS_H_

#include <common.h>
#include <Database.h>

namespace zertcore{ namespace db{

#define BSONOBJ_SET(bson_builder, _stl_map) \
	do{for (BOOST_TYPEOF(_stl_map.begin()) it = _stl_map.begin(); it != _stl_map.end(); ++it) {\
		(bson_builder).append(cast<string>(it->first), it->second);\
	}}while(0);

#define BSONOBJ_SET_T(bson_builder, _stl_map, type) \
	do{for (BOOST_TYPEOF(_stl_map.begin()) it = _stl_map.begin(); it != _stl_map.end(); ++it) {\
		(bson_builder).append(cast<string>(it->first), (type)it->second);\
	}}while(0);

#define BSONARRAY_SET(bson_array, _stl_set) \
	do{for (BOOST_TYPEOF(_stl_set.begin()) it = _stl_set.begin(); it != _stl_set.end(); ++it) {\
		(bson_array).append(*it);}}while(0);

#define BSONARRAY_SET_T(bson_array, _stl_set, type) \
	do{for (BOOST_TYPEOF(_stl_set.begin()) it = _stl_set.begin(); it != _stl_set.end(); ++it) {\
		(bson_array).append((type)*it);}}while(0);

#define BSONARRAY_SETC(bson_array, array, length) \
	for (int i = 0; i < length; ++i) {\
		(bson_array).append((array)[i]); \
	}

class BSONObjBuilderLite
{
public:
	BSONObjBuilderLite() : is_key_(true), builder_(64) {}

public:
	template <typename T>
	bool isNull(const T& v) {
		return !v;
	}

	bool isNull(const BSONObj& obj) {
		return obj.isEmpty();
	}

	bool isNull(const string& obj) {
		return obj.empty();
	}

	bool isNull(const char* v) {
		return strlen(v) == 0;
	}

	template <typename T>
	BSONObjBuilderLite& operator << (const T& v) {
		if (is_key_) {
			;
		}
		else {
			if (!isNull(v)) {
				builder_ << last_key_ << v;
			}
		}

		is_key_ = !is_key_;
		return *this;
	}

	BSONObjBuilderLite& operator << (const char* v) {
		if (is_key_) {
			last_key_ = v;
		}
		else {
			if (!isNull(v)) {
				builder_ << last_key_ << v;
			}
		}

		is_key_ = !is_key_;
		return *this;
	}

	BSONObjBuilderLite& operator << (const string& v) {
		if (is_key_) {
			last_key_ = v;
		}
		else {
			if (!isNull(v)) {
				builder_ << last_key_ << v;
			}
		}

		is_key_ = !is_key_;
		return *this;
	}

	BSONObj obj() {return builder_.obj();}

private:
	string						last_key_;
	bool						is_key_;
	BSONObjBuilder				builder_;
};

#define BSONLite(x)							(::zertcore::db::BSONObjBuilderLite() << x).obj()

/**
 * RecordingReader
 */
class RecordingReader
{
public:
	RecordingReader(const string& document, const string& key = Null) : read_flag_(false) {
		data_ = DB.get(document);
		if (!key.empty()) {
			if (data_.hasField(key)) {
				const BSONElement& element = data_.getField(key);
				if (element.type() == 3) {
					data_ = element.Obj();
					read_flag_ = true;
				}
			}
		}
		else {
			read_flag_ = true;
		}
	}
	RecordingReader(DatabaseClip::ptr clip, const string& key = Null) : read_flag_(true) {
		if (!key.empty()) {
			data_ = clip->get<BSONObj>(key);
		}
		else {
			data_ = clip->get();
		}
	}
	RecordingReader(const BSONObj& data) : read_flag_(true), data_(data) {
		;
	}

	template <typename T>
	bool read(T& val, const string& item, const T& default_val = Null) {
		return getValue(val, data_, item, default_val);
	}

	operator bool() const {
		return read_flag_;
	}

private:
	bool						read_flag_;
	BSONObj						data_;
};

/**
 * BSONObjWrapper
 */
class BSONObjWrapper
{
	typedef BSONObjWrapper					self;
public:
	typedef self&							reference;

public:
	explicit BSONObjWrapper(BSONObj& data) : data_(data)	{;}

public:
	template <typename T>
	T get(const string& item) {
		T val = Null;
		getValue(val, data_, item);

		return val;
	}

	template <typename T>
	void set(const string& item, const T& value) {
		setValue(data_, item, value);
	}

	bool has(const string& key) {
		return data_.hasField(key);
	}

	void remove(const string& item) {
		data_ = data_.removeField(item);
	}

	string toString() const {
		return data_.jsonString();
	}

	BSONType getType(const string& item) const {
		return data_[item].type();
	}

private:
	BSONObj&					data_;
};

template <typename T>
inline bool getValue(T& value, const BSONObj& data, const string& item,
		const T& default_value) {
	try {
		if (!data.hasElement(item))
			return false;
	}
	catch(MsgAssertionException&) {
		return false;
	}

	return getValue(value, data[item], default_value);
}
/**
template <>
bool getValue(vector<BSONElement>& value, const BSONObj& data,
		const string& item, const vector<BSONElement>& default_value) {
	BSONElement elemt = data[item];
	if (!elemt.ok() || elemt.type() != Array) {
		value = default_value;
		return false;
	}
	value = elemt.Array();
	return true;
}
*/

template <>
inline bool getValue(vector<BSONElement>& value, const BSONElement& elemt,
		const vector<BSONElement>& default_value) {
	if (!elemt.ok() || elemt.type() != Array) {
		value = default_value;
		return false;
	}
	value = elemt.Array();
	return true;
}

/**
template <>
bool getValue(BSONObj& value, const BSONObj& data, const string& item,
		const BSONObj& default_value) {
	BSONElement elemt = data[item];
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != 3) {
		value = default_value;
		return false;
	}
	value = elemt.Obj();
	return true;
}
*/

template <>
inline bool getValue(BSONObj& value, const BSONElement& elemt,
		const BSONObj& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != 3 && elemt.type() != Array) {
		value = default_value;
		return false;
	}
	value = elemt.Obj().getOwned();
	return true;
}

template <>
inline bool getValue(BSONElement& value, const BSONObj& data, const string& item,
		const BSONElement& default_value) {
	value = data[item];
	if (value.ok())
		return true;

	value = default_value;
	return true;
}
/**
template <>
bool getValue(int& value, const BSONObj& data, const string& item,
		const int& default_value) {
	BSONElement elemt = data[item];
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberInt) {
		try {
			switch (elemt.type()) {
			case NumberDouble:
				value = (int)elemt.Double();
				return true;
			case String:
				value = lexical_cast<int>(elemt.String());
				return true;
			case NumberLong:
				value = int(elemt.Long());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		return false;
	}
	value = elemt.Int();
	return true;
}
*/

template <>
inline bool getValue(int& value, const BSONElement& elemt,
		const int& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberInt) {
		try {
			switch (elemt.type()) {
			case NumberDouble:
				value = (int)elemt.Double();
				return true;
			case String:
				value = lexical_cast<int>(elemt.String());
				return true;
			case NumberLong:
				value = int(elemt.Long());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		return false;
	}
	value = elemt.Int();
	return true;
}
/**
template <>
bool getValue(uint& value, const BSONObj& data, const string& item,
		const uint& default_value) {
	BSONElement elemt = data[item];
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberInt) {
		try {
			switch (elemt.type()) {
			case NumberDouble:
				value = (uint)elemt.Double();
				return true;
			case String:
				value = lexical_cast<uint>(elemt.String());
				return true;
			case NumberLong:
				value = uint(elemt.Long());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		return false;
	}
	value = (uint)elemt.Int();
	return true;
}
*/
template <>
inline bool getValue(uint& value, const BSONElement& elemt,
		const uint& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberInt) {
		try {
			switch (elemt.type()) {
			case NumberDouble:
				value = (uint)elemt.Double();
				return true;
			case String:
				value = lexical_cast<uint>(elemt.String());
				return true;
			case NumberLong:
				value = uint(elemt.Long());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		return false;
	}
	value = (uint)elemt.Int();
	return true;
}
/**
template <>
bool getValue(double& value, const BSONObj& data, const string& item,
		const double& default_value) {
	BSONElement elemt = data[item];
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberDouble) {
		try {
			switch (elemt.type()) {
			case NumberInt:
				value = (double)elemt.Int();
				return true;
			case String:
				value = lexical_cast<double>(elemt.String());
				return true;
			case NumberLong:
				value = double(elemt.Long());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		value = default_value;
		return false;
	}
	value = elemt.Double();
	return true;
}
*/
template <>
inline bool getValue(double& value, const BSONElement& elemt,
		const double& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberDouble) {
		try {
			switch (elemt.type()) {
			case NumberInt:
				value = (double)elemt.Int();
				return true;
			case String:
				value = lexical_cast<double>(elemt.String());
				return true;
			case NumberLong:
				value = double(elemt.Long());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		value = default_value;
		return false;
	}
	value = elemt.Double();
	return true;
}
/**
template <>
bool getValue(float& value, const BSONObj& data, const string& item,
		const float& default_value) {
	BSONElement elemt = data[item];
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberDouble) {
		try {
			switch (elemt.type()) {
			case NumberInt:
				value = (float)elemt.Int();
				return true;
			case String:
				value = lexical_cast<float>(elemt.String());
				return true;
			case NumberLong:
				value = float(elemt.Long());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		value = default_value;
		return false;
	}

	value = float(elemt.Double());
	return true;
}
*/
template <>
inline bool getValue(float& value, const BSONElement& elemt,
		const float& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberDouble) {
		try {
			switch (elemt.type()) {
			case NumberInt:
				value = (float)elemt.Int();
				return true;
			case String:
				value = lexical_cast<float>(elemt.String());
				return true;
			case NumberLong:
				value = float(elemt.Long());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		value = default_value;
		return false;
	}

	value = float(elemt.Double());
	return true;
}
/**
template <>
bool getValue(string& value, const BSONObj& data, const string& item,
		const string& default_value) {
	BSONElement elemt = data[item];
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != String) {
		try {
			switch (elemt.type()) {
			case NumberDouble:
				value = lexical_cast<string>(elemt.Double());
				return true;
			case NumberInt:
				value = lexical_cast<string>(elemt.Int());
				return true;
			case NumberLong:
				value = lexical_cast<string>(elemt.Long());
				return true;
			case jstOID:
				value = elemt.__oid ().str();
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}

		value = default_value;
		return false;
	}

	value = elemt.String();
	return true;
}
*/
template <>
inline bool getValue(string& value, const BSONElement& elemt,
		const string& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != String) {
		try {
			switch (elemt.type()) {
			case NumberDouble:
				value = lexical_cast<string>(elemt.Double());
				return true;
			case NumberInt:
				value = lexical_cast<string>(elemt.Int());
				return true;
			case NumberLong:
				value = lexical_cast<string>(elemt.Long());
				return true;
			case jstOID:
				value = elemt.__oid ().str();
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}

		value = default_value;
		return false;
	}

	value = elemt.String();
	return true;
}
/**
template <>
bool getValue(time_type& value, const BSONObj& data, const string& item,
		const time_type& default_value) {
	BSONElement elemt = data[item];
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberDouble) {
		try {
			switch (elemt.type()) {
			case NumberInt:
				value = (double)elemt.Int();
				return true;
			case NumberLong:
				value = (double)elemt.Long();
				return true;
			case String:
				value = lexical_cast<double>(elemt.String());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		value = default_value;
		return false;
	}
	value = elemt.Double();
	return true;
}
*/
template <>
inline bool getValue(time_type& value, const BSONElement& elemt,
		const time_type& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberDouble) {
		try {
			switch (elemt.type()) {
			case NumberInt:
				value = (time_type::timestamp_type)elemt.Int();
				return true;
			case NumberLong:
				value = (time_type::timestamp_type)elemt.Long();
				return true;
			case String:
				value = lexical_cast<time_type::timestamp_type>(elemt.String());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		value = default_value;
		return false;
	}
	value = elemt.Double();
	return true;
}
/**
template <>
bool getValue(int64& value, const BSONObj& data, const string& item,
		const int64& default_value) {
	BSONElement elemt = data[item];
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberLong) {
		try {
			switch (elemt.type()) {
			case NumberInt:
				value = (int64)elemt.Int();
				return true;
			case NumberDouble:
				value = (int64)elemt.Double();
				return true;
			case String:
				value = lexical_cast<int64>(elemt.String());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		value = default_value;
		return false;
	}
	value = elemt.Long();
	return true;
}
*/
template <>
inline bool getValue(int64& value, const BSONElement& elemt,
		const int64& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberLong) {
		try {
			switch (elemt.type()) {
			case NumberInt:
				value = (int64)elemt.Int();
				return true;
			case NumberDouble:
				value = (int64)elemt.Double();
				return true;
			case String:
				value = lexical_cast<int64>(elemt.String());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		value = default_value;
		return false;
	}
	value = elemt.Long();
	return true;
}

template <>
inline bool getValue(long long& value, const BSONElement& elemt,
		const long long& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberLong) {
		try {
			switch (elemt.type()) {
			case NumberInt:
				value = (long long)elemt.Int();
				return true;
			case NumberDouble:
				value = (long long)elemt.Double();
				return true;
			case String:
				value = lexical_cast<long long>(elemt.String());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		value = default_value;
		return false;
	}
	value = elemt.Long();
	return true;
}
/**
template <>
bool getValue(uint64& value, const BSONObj& data, const string& item,
		const uint64& default_value) {
	BSONElement elemt = data[item];
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}
	if (elemt.type() != NumberLong) {
		try {
			switch (elemt.type()) {
			case NumberInt:
				value = (uint64)elemt.Int();
				return true;
			case NumberDouble:
				value = (uint64)elemt.Double();
				return true;
			case String:
				value = lexical_cast<uint64>(elemt.String());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		value = default_value;
		return false;
	}
	value = (uint64)elemt.Long();
	return true;
}
*/
template <>
inline bool getValue(uint64& value, const BSONElement& elemt,
		const uint64& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}

	if (elemt.type() != NumberLong) {
		try {
			switch (elemt.type()) {
			case NumberInt:
				value = (uint64)elemt.Int();
				return true;
			case NumberDouble:
				value = (uint64)elemt.Double();
				return true;
			case String:
				value = lexical_cast<uint64>(elemt.String());
				return true;
			default:
				break;
			}
		}
		catch(bad_lexical_cast&) {
			value = default_value;
			return false;;
		}
		value = default_value;
		return false;
	}
	value = (uint64)elemt.Long();
	return true;
}

template <typename T>
inline bool getValue(list<T>& value, const BSONElement& elemt,
		const list<T>& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}

	if (elemt.type() != Array) {
		value = default_value;
		return false;
	}

	vector<BSONElement> v = elemt.Array();
	for (vector<BSONElement>::iterator it = v.begin(); it != v.end();
			++it) {
		T e;
		getValue<T>(e, *it);

		value.push_back(value);
	}

	return true;
}

template <typename T>
inline bool getValue(circular_buffer<T>& value, const BSONElement& elemt,
		const circular_buffer<T>& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}

	if (elemt.type() != Array) {
		value = default_value;
		return false;
	}

	vector<BSONElement> v = elemt.Array();
	for (vector<BSONElement>::iterator it = v.begin(); it != v.end();
			++it) {
		T e;
		getValue<T>(e, *it);

		value.push_back(value);
	}

	return true;
}

template <typename T>
inline bool getValue(set<T>& value, const BSONElement& elemt,
		const set<T>& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}

	if (elemt.type() != Array) {
		value = default_value;
		return false;
	}

	vector<BSONElement> v = elemt.Array();
	for (vector<BSONElement>::iterator it = v.begin(); it != v.end();
			++it) {
		T e;
		getValue<T>(e, *it);

		value.insert(value);
	}

	return true;
}

template <typename T>
inline bool getValue(vector<T>& value, const BSONElement& elemt,
		const vector<T>& default_value) {
	if (!elemt.ok()) {
		value = default_value;
		return false;
	}

	if (elemt.type() != Array) {
		value = default_value;
		return false;
	}

	vector<BSONElement> v = elemt.Array();

	value.clear();
	for (vector<BSONElement>::iterator it = v.begin(); it != v.end();
			++it) {
		T e;
		getValue<T>(e, *it);

		value.push_back(value);
	}

	return true;
}

template <typename T>
inline bool getValue(list<T>& value, const BSONObj& data) {
	value.clear();
	for (BSONObjIterator it = data.begin(); it.more();) {
		T v;
		getValue(v, it.next());

		value.push_back(v);
	}

	return true;
}

template <typename T>
inline bool getValue(circular_buffer<T>& value, const BSONObj& data) {
	value.clear();
	for (BSONObjIterator it = data.begin(); it.more();) {
		T v;
		getValue(v, it.next());

		value.push_back(v);
	}

	return true;
}

template <typename T>
inline bool getValue(vector<T>& value, const BSONObj& data) {
	value.clear();
	for (BSONObjIterator it = data.begin(); it.more();) {
		T v;
		getValue(v, it.next());

		value.push_back(v);
	}

	return true;
}

template <typename T>
inline bool getValue(set<T>& value, const BSONObj& data) {
	value.clear();
	for (BSONObjIterator it = data.begin(); it.more();) {
		T v;
		getValue(v, it.next());

		value.insert(v);
	}

	return true;
}

template <typename T, typename U>
inline bool getValue(map<T, U>& value, const BSONObj& data) {
	set<string> keys;
	data.getFieldNames(keys);

	value.clear();
	for (set<string>::iterator it = keys.begin(); it != keys.end(); ++it) {
		U v = Null;
		getValue(v, data[*it]);
		value[cast<T>(*it)] = v;
	}

	return !value.empty();
}

template <>
inline void setValue(BSONObj& data, const string& item, const time_type& value) {
	data = BSONObjBuilder().append(item, (time_type::timestamp_type)value).appendElementsUnique(data).obj();
}
template <typename T>
inline void setValue(BSONObj& data, const string& item, const list<T>& value) {
	data = BSONObjBuilder().append(item, BSONArrayBuilder().append(value).obj()).appendElementsUnique(data).obj();
}
template <typename T>
inline void setValue(BSONObj& data, const string& item, const set<T>& value) {
	data = BSONObjBuilder().append(item, BSONArrayBuilder().append(value).obj()).appendElementsUnique(data).obj();
}
template <typename T>
inline void setValue(BSONObj& data, const string& item, const vector<T>& value) {
	BSONArrayBuilder b;
	for (typename vector<T>::iterator it = value.begin(); it != value.end(); ++it) {
		b.append(*it);
	}

	data = BSONObjBuilder().append(item, b.obj()).appendElementsUnique(data).obj();
}

/**
template <>
bool RecordingReader::read(int& value, const string& item,
		const int& default_value) {
	BSONElement elemt = data_[item];
	if (elemt.ok()) {
		value = elemt.Int();
		return true;
	}

	value = default_value;
	return false;
}

template <>
bool RecordingReader::read(uint& value, const string& item,
		const uint& default_value) {
	BSONElement elemt = data_[item];
	if (elemt.ok()) {
		value = (uint)elemt.Int();
		return true;
	}

	value = default_value;
	return false;
}

template <>
bool RecordingReader::read(double& value, const string& item,
		const double& default_value) {
	BSONElement elemt = data_[item];
	if (elemt.ok()) {
		value = elemt.Double();
		return true;
	}
	return false;
}

template <>
bool RecordingReader::read(float& value, const string& item,
		const float& default_value) {
	BSONElement elemt = data_[item];
	if (elemt.ok()) {
		value = (float)elemt.Double();
		return true;
	}

	value = default_value;
	return false;
}

template <>
bool RecordingReader::read(string& value, const string& item,
		const string& default_value) {
	BSONElement elemt = data_[item];
	if (elemt.ok()) {
		value = elemt.String();
		return true;
	}

	value = default_value;
	return false;
}

template <>
bool RecordingReader::read(time_type& value, const string& item,
		const time_type& default_value) {
	BSONElement elemt = data_[item];
	if (elemt.ok()) {
		value = (double)elemt.Double();
		return true;
	}

	value = default_value;
	return false;
}
*/

template <typename T>
inline BSONObj convertBSONObj(const T& data, const false_type& type) {
	return data.data();
}

template <typename T>
inline BSONObj convertBSONObj(const T& data, const true_type& type) {
	return BSON(
		"_v" << data
	);
}

template <>
inline BSONObj convertBSONObj(const string& data, const false_type& type) {
	return BSON(
		"_v" << data
	);
}

template <typename T>
inline BSONObj convertBSONObj(const T& data) {
	return convertBSONObj<T>(data, typename is_arithmetic<T>::type());
}

template <typename T>
inline T BSONObjConvert(const BSONObj& data, const true_type& type) {
	BSONObj d(data);
	BSONObjWrapper bw(d);
	return bw.get<T>("_v");
}

template <typename T>
inline T BSONObjConvert(const BSONObj& data, const false_type& type) {
	BSONObj d(data);
	T ts; ts.template data(d);
	return ts;
}

template <>
inline string BSONObjConvert(const BSONObj& data, const false_type& type) {
	BSONObj d(data);
	BSONObjWrapper bw(d);
	return bw.get<string>("_v");
}

template <typename T>
inline T BSONObjConvert(const BSONObj& data) {
	return BSONObjConvert<T>(data, typename is_arithmetic<T>::type());
}

}}

#endif /* DATABASEUTILS_H_ */
