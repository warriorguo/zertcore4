/*
 * Session.h
 *
 *  Created on: 2013-5-7
 *      Author: Administrator
 */

#ifndef SESSION_H_
#define SESSION_H_

#include <common.h>
#include <Log.h>
#include <Serialize.h>
#include <GroupObject.h>
#include <ActionMap.h>
#include <Runtime.h>

namespace zertcore{ namespace net{

using namespace zertcore::base;

class SessionStream :
		public BasicStream<SessionStream, BytePointer>
{
public:
	explicit SessionStream(BytePointer& bp) :
		BasicStream<SessionStream, BytePointer>(bp) {
		;
	}
};

template <class Proxy>
class Session :
	public PoolObject<Session<Proxy> >,
	public Updater<Session<Proxy> >
{
	typedef Session<Proxy>					self;
public:
	struct SessionAction
	{
		//bool			is_encrypted; /* encryption should be operated on the whole package */
		action_index_t	index;

		template <typename IO>
		void serializer(IO& io, uint type, action_index_t action) {
			io & index & uid;
		}
	};

public:
	typedef self*							ptr;
	typedef self&							reference;

	typedef typename Proxy::ptr				proxy_ptr;
	typedef ActionMap<action_index_t, void (proxy_ptr, Unserialize<SessionStream>&) >
											action_map_type;
	typedef typename action_map_type::reference
											action_map_reference;

public:
	explicit Session(action_map_reference action_map, uuid_t uid, proxy_ptr proxy):
			 action_map_(action_map), uid_(uid), proxy_(proxy)
											{;}
	virtual ~Session() {
		;
	}

public:
	uuid_t getUID() const {
		return uid_;
	}

	void stream(SessionStream::reference stream) {
		Unserialize<SessionStream> unserializar(stream);
		SessionAction action;

		unserializar & action;
		action_map_[action.index](object, unserializar);
	}
/**
	bool update(const time_type& interval) {
		if (!pstream_) {
			return false;
		}

		Unserialize<SessionStream> unserializar(*pstream_);

		SessionAction action;
		unserializar & action;

		action_map_[action.index](object, unserializar);
		return true;
	}
*/
	// this function should be in ObjectManager
	//proxy_ptr getProxy(const uuid_t& uid);

private:
	action_map_reference		action_map_;
	uuid_t						uid_;
	proxy_ptr					proxy_;
};


template <class Proxy>
class SessionManager
{
	typedef SessionManager<Proxy>			self;
public:
	typedef self&							reference;

public:
	typedef Session<Proxy>					session_type;
	typedef typename session_type::proxy_ptr
											proxy_ptr;
	typedef typename session_type::ptr		session_ptr;

	typedef typename session_type::action_map_type
											action_map_type;
public:
	typedef map<uuid_t, session_ptr>		session_map_type;

public:
	inline static reference Instance() {
		return instance_;
	}

public:
	/**
	void dispatch(const uint command, const BytePointer& bp,
				const BufferPointer<uuid_t>& targets) {
		;
	}
	*/
	void dispatch(const BytePointer& bp, const BufferPointer<uuid_t>& targets) {
		SessionStream stream(const_cast<BytePointer &>(bp));

		for (size_t i = 0; i < targets.size(); ++i) {
			uuid_t uid = targets.data[i];

			session_ptr session = get(uid);
			if (!session) {
				printf("Cant find session by uid=%d\n", uid);
				continue ;
			}

			try {
				session->stream(stream);
			}
			catch(Exception& e) {
				LOG(WARNING) << uid << e;
			}
			stream.reset();
		}
	}

	session_ptr create(uuid_t uid, proxy_ptr proxy) {
		session_ptr session = load<session_type, action_map_type, uuid_t, proxy_ptr>
								(action_map_, uuid, proxy);

		session_map_.insert(pair<uuid_t, session_ptr>(uid, session));
		return session;
	}
	void destroy(session_ptr session) {
		session_map_.erase(session->getUID());
		delete session;
	}
	/**
	 * get session ptr
	 */
	session_ptr get(uuid_t uid) {
		session_map_type::iterator it = session_map_.find(uid);
		if (it != session_map_.end()) {
			return Null;
		}

		return it->second;
	}

private:
	action_map_type				action_map_;
	session_map_type			session_map_;

private:
	static self	instance_;
};

template <class Proxy>
SessionManager<Proxy> SessionManager<Proxy>::instance_;


/**
//in update
//Session<Player>::ptr session
session->update(getThis());
*/

/**
template <class Proxy>
struct WalkAction
{
	point2d from, to;

	void operator()(action_index_t index, typename Proxy::ptr proxy, Unserialize<SessionStream>::ptr us) {
		us & from & to;
		proxy->perform(ACTION_MOVE, from, to);
	}
};
ActionMap action_map.registerAction(ACTION_MOVE, new WalkAction());
*/

}}
#endif /* SESSION_H_ */
