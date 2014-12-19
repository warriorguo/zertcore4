/*
 * HaffmanTreeHelper.h
 *
 *  Created on: 2013-12-9
 *      Author: Administrator
 */

#ifndef HAFFMANTREESTREAM_H_
#define HAFFMANTREESTREAM_H_

#include <common.h>
#include <Buffer.h>
#include <HaffmanTree.h>

namespace zertcore{ namespace utils{

template <typename T>
class HaffmanTreeStream
{
public:
	typedef T								value_type;
	typedef HaffmanTree<value_type>			tree_type;

	typedef ByteBuffer						buffer_type;

public:
	typedef map<value_type, uint>			type_amount_map;
	typedef map<value_type, string>			type_bits_map;
	typedef list<value_type>				value_list_type;

public:
	void input(const value_type& value) {
		type_amount_[value]++;
		value_list_.push_back(value);
	}

public:
	void generate() {
		calculate();

		bit_set_.init(value_list_.size() * sizeof(T) * 8);
		for (typename value_list_type::iterator it = value_list_.begin();
				it != value_list_.end(); ++it) {
			bit_set_.append(type_bits_[*it]);
		}
	}

public:
	bool output(buffer_type& output) {
		stringstream ss;

		if (type_bits_.empty())
			return false;

		for (typename type_bits_map::iterator it = type_bits_.begin();
				it != type_bits_.end(); ++it) {
			ss << it->second << ":" << it->first << ";";
		}

		output.append((uint)ss.str().size());
		output.append(ss.str());

		output[output.size() - 1] = 0;
		output.append((uint)value_list_.size());
		output.append(bit_set_.data(), bit_set_.size());

		return true;
	}

	template <typename U>
	inline bool translate(const char* buffer, const uint size, vector<U>& output) {
		if (!buffer)
			return false;

		uint head_size = *((uint *)buffer);
		if (head_size + sizeof(uint) >= size) {
			printf("1\n");
			return false;
		}

		string source(buffer + sizeof(uint));

		map<string, U> tr_map;
		uint last_pos = 0; string key;

		uint max_key_size = 0;
		for (uint i = 0; i < head_size; ++i) {
			if (source[i] == ':') {
				key = source.substr(last_pos, i - last_pos);
				last_pos = i + 1;
			}
			else if (source[i] == ';' || i == head_size - 1) {
				string value = source.substr(last_pos, i - last_pos);
				tr_map[key] = lexical_cast<U>(value);
				last_pos = i + 1;

				if (max_key_size < key.size())
					max_key_size = key.size();
			}
		}

		uint count = *((const uint *)(buffer + head_size + sizeof(uint)));
		Bitset<uint> bitset(buffer + head_size + sizeof(uint) * 2, size - head_size - sizeof(uint) * 2);
		printf("COUNT=%u\n", count);

		do{
			uint index = 0;
			key.clear();

			for (; index < bitset.bitSize(); ++index) {
				key += bitset.get(index)? '1': '0';

				if (key.size() > max_key_size) {
					printf("2\n");
					return false;
				}

				typename map<string, U>::iterator it = tr_map.find(key);
				if (it == tr_map.end()) continue;

				output.push_back(it->second);
				if (output.size() >= count) return true;
				key.clear();
			}

			printf("INDEX:%u\n", index);

		}while(0);

		return false;
	}

private:
	void calculate() {
		for (typename type_amount_map::iterator it = type_amount_.begin();
				it != type_amount_.end(); ++it) {
			ht_.setFrequency(it->first, it->second);
		}
		ht_.generate();

		for (typename type_amount_map::iterator it = type_amount_.begin();
				it != type_amount_.end(); ++it) {
			type_bits_[it->first] = ht_.getValue(it->first);
		}
	}

private:
	tree_type					ht_;

private:
	type_amount_map				type_amount_;
	type_bits_map				type_bits_;
	value_list_type				value_list_;
	Bitset<>					bit_set_;
};



}}


#endif /* HAFFMANTREEHELPER_H_ */
