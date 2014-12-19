/*
 * Crypto.h
 *
 *  Created on: 2013-10-17
 *      Author: Administrator
 */

#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <common.h>
#include <time.h>
#include <sstream>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK		1

#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <cryptopp/md5.h>
#include <cryptopp/base64.h>

namespace zertcore{ namespace utils{

class Crypto
{
public:
	static const uint STREAM_KEY_LENGTH		= 16;

public:
	inline static string sha256(const string& str) {
		string output;
		CryptoPP::SHA256 hash;
		byte digest[CryptoPP::SHA256::DIGESTSIZE];

		hash.CalculateDigest(digest,(const byte *)str.c_str(),str.size());

		CryptoPP::HexEncoder encoder;

		encoder.Attach(new CryptoPP::StringSink(output));
		encoder.Put(digest, sizeof(digest));
		encoder.MessageEnd();

		return output;
	}

	inline static string md5(const string& str) {
		string output;
		CryptoPP::Weak::MD5 hash;
		byte digest[CryptoPP::Weak::MD5::DIGESTSIZE];

		hash.CalculateDigest(digest,(const byte *)str.c_str(),str.size());

		CryptoPP::HexEncoder encoder;

		encoder.Attach(new CryptoPP::StringSink(output));
		encoder.Put(digest, sizeof(digest));
		encoder.MessageEnd();

		return output;
	}

public:
	inline static string streamEncrypt(const string& source, const string& secret) {
		return streamAuthcode(source, secret, true);
	}

	inline static string streamDecrypt(const string& source, const string& secret) {
		try {
			return streamAuthcode(source, secret, false);
		}
		catch(::std::exception&) {
			;
		}
		return Null;
	}

private:
	inline static string streamAuthcode(string source, const string& secret, bool encrypt) {
		const static byte codes_replace[] = {13, 17, 111, 54, 249, 1, 157, 194, 228, 233, 7, 153, 179, 195, 184, 78, 170, 250, 43, 174, 143, 2, 69, 182, 150, 79, 58, 29, 247, 216, 230, 94, 97, 70, 213, 50, 62, 10, 75, 37, 55, 67, 107, 81, 218, 82, 95, 212, 18, 110, 210, 160, 38, 175, 25, 169, 181, 11, 183, 48, 101, 35, 152, 237, 136, 137, 149, 211, 187, 166, 165, 229, 155, 12, 5, 77, 156, 253, 23, 173, 86, 159, 14, 236, 140, 168, 214, 30, 66, 109, 93, 163, 49, 100, 242, 206, 251, 89, 209, 9, 248, 27, 90, 135, 244, 125, 40, 20, 197, 44, 205, 103, 133, 105, 15, 131, 59, 139, 36, 130, 116, 121, 32, 234, 215, 42, 127, 202, 232, 31, 28, 128, 4, 63, 141, 221, 76, 235, 85, 129, 119, 91, 193, 83, 60, 126, 98, 199, 171, 46, 147, 186, 185, 208, 132, 180, 71, 178, 57, 145, 33, 223, 47, 162, 231, 189, 255, 106, 21, 207, 22, 45, 39, 240, 192, 26, 198, 64, 52, 138, 123, 8, 124, 154, 87, 96, 80, 102, 24, 190, 117, 239, 243, 113, 108, 56, 114, 92, 99, 172, 222, 41, 73, 16, 74, 112, 217, 227, 142, 51, 241, 226, 122, 34, 88, 219, 84, 220, 203, 200, 176, 252, 245, 120, 0, 19, 177, 164, 238, 68, 115, 151, 196, 254, 53, 134, 204, 144, 167, 201, 161, 225, 104, 158, 188, 148, 246, 118, 3, 224, 61, 65, 6, 146, 191, 72};

		string key = md5(secret);
		string keya = key.substr(0, 24);
		string keyb = key.substr(24, 32);

		string keyc;

		if (encrypt) {
			stringstream ss;

			ss << "zertcore_auth" << ::time(NULL);
			keyc = md5(ss.str()).substr(0, (size_t)STREAM_KEY_LENGTH);

			source = keyb + source;
		}
		else {
			if (source.size() <= STREAM_KEY_LENGTH + 8)
				return Null;

			keyc = source.substr(0, (size_t)STREAM_KEY_LENGTH);

			source = source.substr(STREAM_KEY_LENGTH);
		}

		string crypto_key = keya + md5(keya + keyc);
		vector<char> rndkey;
		rndkey.resize(256);

		for (uint i = 0; i < 256; ++i) {
			rndkey[i] = crypto_key[codes_replace[i] % crypto_key.size()];
		}

		string result;
		for (uint i = 0, j = 0, a = 0; i < source.size(); ++i) {
			a = (a + 1) % 256;
			j = (rndkey[a] + j) % 256;

			char tmp = rndkey[a];
			rndkey[a] = rndkey[j];
			rndkey[j] = tmp;

			result += source[i] ^ rndkey[(rndkey[a] + rndkey[j]) % 256];
		}

		if (encrypt) {
			return keyc + result;
		}

		if (result.substr(0, 8) == keyb) {
			return result.substr(8);
		}

		return Null;
	}
};

}}

#endif /* CRYPTO_H_ */
