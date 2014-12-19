/*
 * I18N.h
 *
 *  Created on: 2013-10-14
 *      Author: Administrator
 */

#ifndef I18N_H_
#define I18N_H_

#include <common.h>
#include <Database.h>

namespace zertcore{ namespace utils{

using namespace zertcore::db;

/**
 * MapBuilder
 */
template <typename T, typename U>
class MapBuilder
{
	typedef MapBuilder<T, U>				self;
public:
	typedef self&							reference;

public:
	typedef T								key_type;
	typedef U								value_type;
	typedef map<key_type, value_type>		type;

public:
	MapBuilder() : count_(0) {;}

public:
	reference append(const key_type& key, value_type& value) {
		map_.insert(pair<key_type, value_type>(key, value));
		return *this;
	}

	operator map<key_type, value_type>() {
		return map_;
	}

	type& get() {
		return map_;
	}

	template <typename Type>
	reference operator << (const Type& value) {
		if ((count_++) % 2) {
			last_value_ = lexical_cast<value_type>(value);
			append(last_key_, last_value_);
		}
		else {
			last_key_ = lexical_cast<key_type>(value);
		}

		return *this;
	}

	reference operator << (const char* value) {
		return this->template operator << (string(value));
	}
private:
	key_type					last_key_;
	value_type					last_value_;

private:
	uint						count_;
	type						map_;
};

/**
 * MAP("map" << 3 & "")
 *
 *
 */

typedef MapBuilder<string, string>			i18n_params;
#define I18N_P(x)			((::zertcore::utils::i18n_params() << x).get())
/**
 * I18N
 */
class I18N
{
	typedef I18N							self;
public:
	typedef self&							reference;

public:
	typedef string							key_type;
	typedef map<key_type, string>			package_type;
	typedef map<string, package_type>		package_map_type;

public:
	inline static reference Instance() {
		static self instance_;
		return instance_;
	}

public:
	void setupPackage(const string& lang, const package_type& data) {
		package_map_[lang] = data;
	}

	string perform(const string& lang, const key_type& key) {
		package_map_type::iterator it = package_map_.find(lang);
		if (it == package_map_.end()) {
			printf("Unknown Lang:%s\n", lang.c_str());
			return key;
		}
		const package_type& package = it->second;

		package_type::const_iterator pit = package.find(key);
		if (pit == package.end()) {
			printf("Unknown Key:%s\n", key.c_str());
			return key;
		}

		return pit->second;
	}

	string perform(const string& lang, const key_type& key, const i18n_params::type& p) {
		return replaceKey(perform(lang, key), p);
	}

	void langs(vector<string>& vals) {
		for (package_map_type::iterator it = package_map_.begin();
				it != package_map_.end(); ++it) {
			vals.push_back(it->first);
		}
	}

	bool support(const string& lang) const {
		package_map_type::const_iterator it = package_map_.find(lang);
		if (it == package_map_.end())
			return false;

		return true;
	}

	uint version(const string& lang) {
		return cast<uint>(perform(lang, "__version__"));
	}

public:
	string replaceKey(const string& source,
			const i18n_params::type& p) {
		string ret = source;
		for (i18n_params::type::const_iterator it = p.begin();
				it != p.end(); ++it) {
			string key = "{" + it->first + "}";

			replace_all(ret, key, it->second);
		}

		return ret;
	}

private:
	package_map_type			package_map_;
};

/**
 * Interface helper function
 */
inline static uint VersionTr(const string& lang) {
	return I18N::Instance().version(lang);
}

inline static bool SupportTr(const string& lang) {
	return I18N::Instance().support(lang);
}

inline static void SetupTr(const string& lang, const I18N::package_type& package) {
	I18N::Instance().setupPackage(lang, package);
}

inline static void SetupTr(const string& lang, BSONObj data) {
	I18N::package_type package;

	set<string> keys;
	data.getFieldNames(keys);

	for (set<string>::iterator it = keys.begin(); it != keys.end(); ++it) {
		const string& key = *it;
		package.insert(pair<string, string>(key, data[key].String()));
	}

	SetupTr(lang, package);
}

inline static string Tr(const string& lang, const I18N::key_type& key,
		const i18n_params::type& p) {
	return I18N::Instance().perform(lang, key, p);
}

inline static string Tr(const string& lang, const I18N::key_type& key) {
	return I18N::Instance().perform(lang, key);
}

}}


#endif /* I18N_H_ */
