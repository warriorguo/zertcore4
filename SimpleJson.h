/*
 * SimpleJson.h
 *
 *  Created on: 2014Äê7ÔÂ18ÈÕ
 *      Author: Administrator
 */

#ifndef SIMPLEJSON_H_
#define SIMPLEJSON_H_

#include <string>
#include <map>
#include <vector>

namespace zertcore{ namespace json{
using namespace std;

typedef char								char_type;
typedef string								key_type;

class Value
{
	typedef Value							self;

public:
	struct ParseFailed : Exception {};
	struct InvildToken : Exception {};
	struct InvildNumber : Exception {};
	struct InvildValue : Exception {};
	struct InvildAction : Exception {};

public:
	typedef Value*							ptr;

public:
	enum {
		TYPE_NONE							= 0,
		TYPE_LONG							= 1,
		TYPE_DOUBLE							= 2,

		TYPE_NUMBER							= 2,
		TYPE_STRING							= 3,

		TYPE_OBJECT							= 11,
		TYPE_ARRAY							= 12,
	};

public:
	typedef map<key_type, Value::ptr>		value_map_type;
	typedef vector<Value::ptr>				value_list_type;

private:
	typedef struct val_type_
	{
		int						type;

		double					fval;
		string					sval;

		int						decimal;

		val_type_() : type(TYPE_NONE), fval(0), decimal(0) {}

		void append(char_type ch) {
			switch (type) {
			case TYPE_NUMBER:
				if (ch == '.') {
					if (decimal > 0) {
						throw InvildNumber();
					}
					decimal = 1;
					break;
				}

				ch -= '0';
				if (ch >= 10) {
					throw ParseFailed();
				}

				if (decimal > 0) {
					fval += ch / decimal;
					decimal++;
				}
				else {
					fval = fval * 10 + ch;
				}
				break;
			}
			case TYPE_STRING:
				sval += ch;
				break;

			default:
				throw InvildAction();
				break;
		}
	}										val_type;

public:
	Value() : index_(0), stream_(NULL), last_open_tag_(0) {}
	virtual ~Value() {
		for (value_map_type::iterator it = children_.begin(); it != children_.end(); ++it) {
			delete it->second;
		}

		for (value_list_type::iterator it = array_.begin(); it != array_.end(); ++it) {
			delete *it;
		}
	}

public:
	uint parse(const char_type* stream) {
		if (!stream) return false;
		stream_ = stream;

		return index_;
	}

	void abort() {
		skipSpace();
	}

	void match(const char_type& token, bool escape = false) {
		while (true) {
			char_type ch = 0;
			while (isSpace()) {
				ch = moveNext();
			}

			if (escape && ch == '\\') {
				moveNext(2);
				continue;
			}

			if (token == ch) {
				moveNext();
				return ;
			}
		}
	}

	void doKey() {
		match('"');
		int key_begin = index_;
		match('"', true);

		match(':');
		doValue();
	}

	void doValue() {
		switch(char_type ch = nextToken()) {
		case '"':
			valueString();
			break;

		case 't':
			if (moveNext() != 'r' || moveNext() != 'u' || moveNext() != 'e') {
				throw InvildValue();
			}
			break;

		case 'f':
			if (moveNext() != 'a' || moveNext() != 'l' || moveNext() != 's' || moveNext() != 'e') {
				throw InvildValue();
			}
			break;

		case 'n':
			if (moveNext() != 'u' || moveNext() != 'l' || moveNext() != 'l') {
				throw InvildValue();
			}
			break;

		case '{':
			last_open_tag_ = ch;
			valueNewObject();
			break;

		case '[':
			last_open_tag_ = ch;
			valueNewArray();
			break;

		default:
			if (ch >= '0' && ch <= '9') {
				valueNumber();
			}
			else {
				throw InvildValue();
			}
			break;
		}

		switch(char_type ch = nextToken()) {
		case ',':

			break;

		case '}':
			if (last_open_tag_ != '{') {
				throw InvildToken();
			}
			break;

		case ']':
			if (last_open_tag_ != '[') {
				throw InvildToken();
			}
			break;
		}
	}

	void valueNewObject() {
		index_++;
		Value::ptr value = Value::ptr(new self);
		value->parse(&stream_[index_]);

		children_[value->getKey()] = value;
	}

	void valueNewArray() {
		;
	}

	void valueString() {
		;
	}

	void valueNumber() {
		;
	}

private:
	char_type moveNext(int offset = 1) {
		while(offset--) {
			index_++;
			if (!stream_[index_]) abort();
		}

		return stream_[index_];
	}

	char_type nextToken() {
		skipSpace();
		return stream_[index_];
	}

	bool isSpace() {
		return (stream_[index_] == ' ' || stream_[index_] == '	' ||
				stream_[index_] == '\r' || stream_[index_] == '\n');
	}

	void skipSpace() {
		while (!isSpace()) {
			moveNext();
		}
	}

public:
	const key_type& getKey() const;

private:
	val_type					val_;
	int							index_;
	const char_type*			stream_;

private:
	char_type					last_open_tag_;

private:
	value_map_type				children_;
	value_list_type				array_;
};

}}



#endif /* SIMPLEJSON_H_ */
