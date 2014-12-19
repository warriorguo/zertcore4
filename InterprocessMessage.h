/*
 * InterprocessMessage.h
 *
 *  Created on: 2013-5-25
 *      Author: Administrator
 */

#ifndef INTERPROCESSMESSAGE_H_
#define INTERPROCESSMESSAGE_H_

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/errors.hpp>
#include <iterator>

#include <common.h>

namespace zertcore{ namespace utils {
namespace ip = interprocess;

namespace message {

enum {
	MODE_NONE		= 0,
	MODE_READ		= 1 << 0,
	MODE_WRITE		= 1 << 1,
	MODE_CREATE		= 1 << 2,
	MODE_ALL		= MODE_READ | MODE_WRITE | MODE_CREATE,
};

}

// those two maroc should be define in user-defined config.h file
//
#ifndef INTERPROCESS_MESSAGE_VOLUME
#  define INTERPROCESS_MESSAGE_VOLUME 32
#endif

#ifndef INTERPROCESS_MESSAGE_SIZE
#  define INTERPROCESS_MESSAGE_SIZE 10240
#endif

#ifndef INTERPROCESS_MAX_TARGET_SIZE
#  error INTERPROCESS_MAX_TARGET_SIZE is not defined
#endif

///
namespace message {

typedef uuid_t	target_type;

struct package
{
	size_t	size;
	uint	command;
	uint	priority;
	uint32	index_size;
	target_type	target_indexes[INTERPROCESS_MAX_TARGET_SIZE];
	byte	data[INTERPROCESS_MESSAGE_SIZE];
};

}

/**
 * used for InterprocessMessage MODE_WRITE only mode
 */
struct WriterOnlyStreamList
{
	void dispatch(const uint command, const byte* data, size_t size,
					const uuid_t* target_indexes, size_t index_size) {;}
};
static WriterOnlyStreamList WriteronlyStreamlist;

template <class StreamList = WriterOnlyStreamList>
class InterprocessMessage
{
	typedef ip::message_queue			MessageQueue;
	typedef SMART_PTR(MessageQueue)		MessageQueuePtr;

public:
	typedef vector<message::package>	buffer_type;

public:
	explicit InterprocessMessage(StreamList& streamlist, const string& pipe_name,
			uint mode = message::MODE_READ):
			stream_list_(streamlist) {
		if ((mode & message::MODE_READ) && is_base_of<WriterOnlyStreamList, StreamList>::value) {
			THROW_EXCEPTION(TypeErrorException());
		}
		this->open(pipe_name, mode);
	}
	explicit InterprocessMessage(const string& pipe_name, uint mode = message::MODE_WRITE):
				stream_list_(WriteronlyStreamlist) {
		if (mode & message::MODE_READ) {
			THROW_EXCEPTION(TypeErrorException());
		}
		this->open(pipe_name, mode);
	}

	virtual ~InterprocessMessage() {
		if (mode_ & message::MODE_CREATE) {
			ip::shared_memory_object::remove(pipe_name_.c_str());
		}
	}

public:
	void open(const string& pipe_name, uint mode = message::MODE_READ) {
		pipe_name_ = pipe_name;
		mode_ = mode;

		if (mode & message::MODE_CREATE) {
			MessageQueue::remove(pipe_name_.c_str());

			message_list_ = MessageQueuePtr(new MessageQueue(ip::create_only, pipe_name.c_str(),
					INTERPROCESS_MESSAGE_VOLUME, sizeof(message::package)));
		}
		else {
			message_list_ = MessageQueuePtr(new MessageQueue(ip::open_only, pipe_name.c_str()));
		}
	}

public:
	template <class Stream>
	bool send(const Stream& stream, const uint32 *target_indexes, uint index_size, uint priority = 0) {
		return send(stream.data(), stream.size(), target_indexes, index_size, priority);
	}

	bool send(const byte* data, size_t size, const uuid_t *target_indexes, uint index_size, uint priority = 0) {
		if ((mode_ & message::MODE_WRITE) == 0) {
			SET_ERROR("Its not write mode");
			return false;
		}

		if (size >= INTERPROCESS_MESSAGE_SIZE) {
			SET_ERROR("message size is too large");
			return false;
		}

		if (index_size >= INTERPROCESS_MAX_TARGET_SIZE) {
			SET_ERROR("target index size is too large");
			return false;
		}

		message::package package;

		package.size = size;
		package.priority = priority;

		for (uint i = 0; i < index_size; ++i) {
			package.target_indexes[i] = target_indexes[i];
		}
		package.index_size = index_size;
		memcpy(package.data, data, size);

		if (add2List(package)) {
			refresh();
		}

		return true;
	}

	bool receive() {
		if ((mode_ & message::MODE_READ) == 0) {
			SET_ERROR("Its not read mode");
			return false;
		}

		MessageQueue::size_type size = message_list_->get_num_msg();
		for (MessageQueue::size_type i = 0; i < size; ++i) {
			message::package package;
			uint priority;
			MessageQueue::size_type recvd_size;

			if (!message_list_->try_receive(&package, sizeof(package), recvd_size, priority)) {
				break;
			}

			stream_list_.dispatch(package.command, package.data, package.size, package.target_indexes, package.index_size);
		}

		return true;
	}

	void refresh() {
		if (buffer_.size() == 0) return;

		for (buffer_type::iterator it = buffer_.begin(); buffer_.size(); ++it) {
			if (!add2List(*it, false))
				break;

			buffer_.erase(it);
			printf("[2]buffer size=%u\n", buffer_.size());
		}
	}

private:
	// return false if the list is full
	bool add2List(message::package& package, bool failed_add_buffer = true) {
		/*try {*/
		if (!message_list_->try_send(&package, sizeof(message::package), package.priority)) {
			if (failed_add_buffer) {
				printf("[1]buffer size=%u\n", buffer_.size());
				buffer_.push_back(package);
			}

			return false;
		}
		/*
		}catch(interprocess_error& error) {
			return false;
		}
		*/

		return true;
	}

private:
	uint mode_;
	string pipe_name_;

	buffer_type buffer_;
	StreamList& stream_list_;
	MessageQueuePtr message_list_;
};

}}


#endif /* INTERPROCESSMESSAGE_H_ */
