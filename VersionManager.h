/*
 * VersionManager.h
 *
 *  Created on: 2013-10-23
 *      Author: Administrator
 */

#ifndef VERSIONMANAGER_H_
#define VERSIONMANAGER_H_

#include <common.h>
#include <Database.h>
#include <DatabaseUtils.h>

namespace zertcore{ namespace utils{
using namespace zertcore::db;

typedef uint								version_type;
const static version_type MINI_VERSION		= 10000;

struct FileInfo
{
	uint						type;
	uint						size;
	string						name;
	uint						crc32;

	void data(BSONObj data) {
		BSONObjWrapper bw(data);

		type = bw.get<uint>("type");
		size = bw.get<uint>("size");
		name = bw.get<string>("name");
		crc32 = bw.get<uint>("crc32");
	}

	BSONObj data() const {
		return BSON(
			"type" << type <<
			"size" << size <<
			"name" << name <<
			"crc32" << crc32
		);
	}

	bool calculate(const string& work_path) {
		if (name.empty())
			return false;

		string content;
		if(!getFileContent(work_path + name, content)) {
			return false;
		}

		size = content.size();
		crc32 = ::zertcore::utils::crc32(content.c_str(), content.size());

		return true;
	}
};

struct FileInfoLess :
			binary_function <FileInfo, FileInfo, bool>
{
	bool operator() (const FileInfo& x, const FileInfo& y) const {
		less<string> string_less;
		return string_less(x.name, y.name);
	}
};

typedef set<FileInfo, FileInfoLess>			file_list_type;
typedef map<version_type, file_list_type>	file_data_map_type;

class FileVersionManager
{
public:
	typedef file_list_type					list_type;
	typedef file_data_map_type				data_map_type;

public:
	bool loadVersionFile(const string& filename, const string& work_path) {
		BSONObj data;
		if (!getJsonFileContent(filename, data)) {
			return false;
		}

		return setup(data, work_path);
	}
	bool expired(const version_type& version) {
		return version < last_version_;
	}
	void getLog(list_type& list, const version_type& version = Null) {
		if (version) {
			data_map_type::iterator it = data_map_.find(version);
			if (it != data_map_.end()) {
				for (; it != data_map_.end(); ++it) {
					list_type& log = it->second;
					list.insert(log.begin(), log.end());
				}

				return ;
			}
		}

		list = list_all_;
	}

	bool setup(BSONObj data, const string& work_path) {
		BSONObjWrapper bw(data);

		last_version_ = bw.get<version_type>("__version__");
		if (last_version_ < MINI_VERSION) {
			return false;
		}
		BSONObj log = bw.get<BSONObj>("log");
		if (log.isEmpty())
			return true;

		set<string> keys;
		log.getFieldNames(keys);

		for (set<string>::iterator key_it = keys.begin();
				key_it != keys.end(); ++key_it) {
			const string& key = *key_it;
			version_type version = cast<version_type>(key);
			if (version < MINI_VERSION)
				continue;

			list_type list;

			if (log[key].type() == 4 || log[key].type() == 3) {
				BSONObj log_data = log[key].Obj();
				for (BSONObjIterator it = log_data.begin(); it.more();
						++it) {
					FileInfo file_info;

					file_info.name = (*it).String();
					if (file_info.calculate(work_path)) {
						list.insert(file_info);
						list_all_.insert(file_info);
					}
					else {
						LOG(WARNING) << "VersionManager: calculate file [" <<
								work_path + file_info.name << "failed\n";
					}
				}
			}

			data_map_.insert(pair<version_type, list_type>(version, list));
		}

		return true;
	}

private:
	version_type				last_version_;

private:
	data_map_type				data_map_;
	list_type					list_all_;
};

}}

#endif /* VERSIONMANAGER_H_ */
