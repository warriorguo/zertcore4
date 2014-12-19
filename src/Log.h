/*
 * Log.h
 *
 *  Created on: 2013-8-4
 *      Author: Administrator
 */

#ifndef LOG_H_
#define LOG_H_

#include <stdarg.h>
#include <fstream>
#include <ctime>

#include <common.h>
#include <Database.h>
#include <Buffer.h>
#include <Runtime.h>

namespace zertcore {

enum {
	NOTICE			= 0,
	NOTE			= 1,
	INFO			= 2,
	WARNING			= 3,
	EXCEPTION		= 4,
	ERROR			= 5,
	FINAL			= 10,

	DEBUG			= NOTICE,
};

}

namespace zertcore {namespace log{

using namespace zertcore::base;
using namespace zertcore::db;
using namespace zertcore::utils;

namespace plugins{
/**
 *
 * DB
 *
 */
struct DatabaseWriter
{
	void operator ()(const string& type, uuid_t key, uint level, time_type time,
			const string& file, const int line, const string& log, time_type op_time) {
		/**
		Database::clip_type clip = DB.create("log");

		clip->set("type", type);
		clip->set("time", time);
		clip->set("file", file);
		clip->set("line", line);
		clip->set("log", log);

		clip->flush();
		*/
		string db_name("log");
		if (!type.empty()) {
			db_name += "_" + type;
		}

		DB.insert(db_name, BSON(
			"key" << (long long)key <<
			"level" << level <<
			"time" << (time_type::timestamp_type)time <<
			"op_time" << (time_type::timestamp_type)op_time <<
			"file" << file <<
			"line" << line <<
			"log" << log
		));
	}
};

/**
 *
 * File
 *
 */
struct FileWriter
{
	void operator ()(const string& type, uuid_t key, uint level, time_type time,
			const string& file, const int line, const string& log, time_type op_time) {
		;
	}
};

/**
 *
 * Stdout
 *
 */
struct StdoutWriter
{
	void operator ()(const string& type, uuid_t key, uint level, time_type time,
			const string& file, const int line, const string& log, time_type op_time) {
		;
	}
};

}

/**
 *
 * LogT<Input, level>
 *
 */
template<class Input, uint Level>
class LogT
{
	typedef LogT<Input, Level>				self;
public:
	typedef self&							reference;

public:
	inline static reference Instance() {
		return instance_;
	}

public:
	struct Record
	{
		Record(const string& type, uuid_t key, uint level, const string& file,
				int line, time_type op_time, reference log)
			: type_(type), key_(key), level_(level), file_(file),
			  line_(line), op_time_(op_time), log_(log) {
		}
		~Record() {
			log_.add(type_, key_, level_, file_, line_, data_, op_time_);
		}

		template <typename T>
		Record& operator << (const T& line) {
			data_ += lexical_cast<string>(line);
			return *this;
		}
		Record& operator << (const string& line) {
			data_ += line;
			return *this;
		}
		Record& operator << (const char* line) {
			data_ += line;
			return *this;
		}
		Record& operator << (const Exception& e) {
			data_ += e.what();

			if (level_ < EXCEPTION) level_ = EXCEPTION;
			return *this;
		}
		Record& operator << (const ByteBuffer& buffer) {
//			data_ += buffer.print(false);
			return *this;
		}

	private:
		string					type_;
		uuid_t					key_;
		uint					level_;
		string					data_;
		string					file_;
		int						line_;
		time_type				op_time_;
		reference				log_;
	};

public:
	void add(const string& type, uuid_t key, uint level, const string& file, int line, const string& data,
			time_type op_time) {
		if (level < Level) {
			return ;
		}

		input_(type, key, level, time_type(), file, line, data, op_time);
	}

	/**
	Record log(uint level, const string& file, int line) {
		return Record("", 0, level, file, line, *this);
	}
	*/

	Record log(const string& type, uuid_t key, uint level, time_type op_time, const string& file, int line) {
		return Record(type, key, level, file, line, op_time, *this);
	}

private:
	Input						input_;

private:
	static self					instance_;
};

template <class Input, uint Level>
LogT<Input, Level>				LogT<Input, Level>::instance_;

}}

namespace zertcore {
using namespace std;

/**
 * Using DB as record
 */
typedef log::LogT<log::plugins::DatabaseWriter, LOG_LEVEL>
											Log;

/**
 * Log to file, intends to replace printf()
 */
inline void printf(const char* format, ...) {
	static ofstream ofs("printf.log", ofstream::out);

	char buffer[512];
	va_list args;
	va_start(args, format);
	vsprintf(buffer,format, args);
	va_end(args);

    time_t now = ::time(NULL);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);

    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "[%Y-%m-%d.%X]", &tstruct);
    ofs << buf << buffer << endl;
}

}
/**
 * LOG(type)
 */
#define LOG(level)				(::zertcore::Log::Instance().log("", 0, level, 0, __FILE__, __LINE__))
#define LOG2(level, type)		(::zertcore::Log::Instance().log(type, 0, level, 0, __FILE__, __LINE__))
#define LOG3(level, type, key)	(::zertcore::Log::Instance().log(type, key, level, 0, __FILE__, __LINE__))
#define LOG4(level, type, key, op_time)	(::zertcore::Log::Instance().log(type, key, level, op_time, __FILE__, __LINE__))


#endif /* LOG_H_ */
