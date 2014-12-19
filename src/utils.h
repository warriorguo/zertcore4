/*
 * utils.h
 *
 *  Created on: 2013-10-17
 *      Author: Administrator
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <cstdio>
#include <string>
#include <fstream>
#include <libgen.h>

#include <curl/curl.h>

#include <mongo/client/dbclient.h>
#include <mongo/util/net/httpclient.h>
#include <boost/regex.hpp>
#include <boost/crc.hpp>

namespace zertcore {namespace utils{

using namespace std;
using namespace boost;
using namespace mongo;

/**
inline static size_t __curl_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	string& content = *(string *)userdata;
	content.append(ptr, size * nmemb);
	return size * nmemb;
}
*/

inline static bool postHttpContent(const string& url, const string& post, string& content) {
	/**
	CURL *curl = curl_easy_init();
	if (!curl)
		return false;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post.size());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, __curl_write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return res == CURLE_OK;
	*/
	HttpClient client;
	HttpClient::Result result;

	int status = client.post(url, post, &result);
	if (200 != status) {
		return false;
	}

	content = result.getBody();
	return true;
}

inline static bool getHttpContent(const string& url, string& content) {
	/**
	CURL *curl = curl_easy_init();
	if (!curl)
		return false;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, __curl_write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return res == CURLE_OK;
	*/
	HttpClient client;
	HttpClient::Result result;

	int status = client.get(url, &result);
	if (200 != status) {
		return false;
	}

	content = result.getBody();
	return true;
}

inline static bool getHttpJsonContent(const string& url, BSONObj& data) {
	string content;
	if (getHttpContent(url, content)) {
		try {
			data = fromjson(content);
		}
		catch(MsgAssertionException&) {
			::printf("from json failed:%s\n", content.c_str());
			return false;
		}
		return true;
	}

	return false;
}

inline static bool getFileContent(const string& filename, string& content) {
	vector<char> path;
	path.assign(filename.begin(), filename.end());

	dirname(&path[0]);
	mkdir(&path[0], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	ifstream file(filename.c_str(), ifstream::binary | ifstream::in);


	if (!file.is_open()) {
		printf("Open %s failed\n", filename.c_str());
		return false;
	}

	file.seekg(0, file.end);
	int length = file.tellg();
	file.seekg(0, file.beg);

	char* buffer = new char[length];
	file.read(buffer, length);
	content.append(buffer, length);

	delete[] buffer;

	file.close();
	return true;
}

inline static bool saveFileContent(const string& filename, const string& content) {
	ofstream file(filename.c_str(), ofstream::binary | ofstream::out);

	if (!file.is_open()) {
		printf("Open %s failed\n", filename.c_str());
		return false;
	}

	file.write(content.c_str(), content.size());

	file.close();
	return true;
}

inline static bool saveHttpFile(const string& url, const string& filename) {
	string content;
	if (!getHttpContent(url, content))
		return false;

	return saveFileContent(filename, content);
}

inline static bool getJsonFileContent(const string& filename, BSONObj& data) {
	string content;
	if (!getFileContent(filename, content)) {
		return false;
	}

	try {
		data = fromjson(content);
	}
	catch(MsgAssertionException&) {
		return false;
	}

	return true;
}

class Email
{
public:
	Email(const string& email) : verify_(false), email_(email) {;}

public:
	bool verify() {
		regex valid_expr = regex("\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*");
		return verify_ = regex_match(email_, valid_expr);
	}

	string getName() {
		if (verify_) {
			size_t pos = email_.find("@");
			if (pos == string::npos) {
				return "";
			}

			return email_.substr(0, pos);
		}

		return "";
	}

private:
	bool						verify_;
	string						email_;
};

inline static bool isEmail(const string& str) {
	Email email(str);
	return email.verify();
}

inline static uint crc32(const void* buffer, size_t size) {
	crc_32_type result;
	result.process_bytes(buffer, size);

	uint ret = result.checksum();
	return ret;
}

inline static uint crc32(const string& s) {
	return crc32(s.c_str(), s.size());
}

}}


#endif /* UTILS_H_ */
