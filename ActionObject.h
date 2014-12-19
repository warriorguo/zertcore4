/*
 * ActionObject.h
 *
 *  Created on: 2013-9-1
 *      Author: Administrator
 */

#ifndef ACTIONOBJECT_H_
#define ACTIONOBJECT_H_

#include <common.h>
#include <PoolObject.h>
#include <Database.h>
#include <Serialize.h>
#include <InterScript.h>
#include <Map.h>

namespace zertcore {namespace object{

using namespace zertcore::base;
using namespace zertcore::utils;
using namespace zertcore::map;


class ActionObject:
		public PoolObject<WorldObject>
{
public:
/**
 * At first assume the max of action parameters amount is 3
 */
	typedef function<void ()>				action_handler_type;
	typedef function<void (any)>			action_handler_p1_type;
	typedef function<void (any, any)>		action_handler_p2_type;
	typedef function<void (any, any, any)>	action_handler_p3_type;

/**
 * At first assume the max of perform parameters amount is 5
 */
	typedef function<void ()>				perform_handler_type;
	typedef function<void (any)>			perform_handler_p1_type;
	typedef function<void (any, any)>		perform_handler_p2_type;
	typedef function<void (any, any, any)>	perform_handler_p3_type;
	typedef function<void (any, any, any, any)>
											perform_handler_p4_type;
	typedef function<void (any, any, any, any, any)>
											perform_handler_p5_type;

public:
/**
 * perform is indicating object will do a series of actions
 */
	void perform(action_index_t index) {
		script_.perform(index);
	}
	void perform(action_index_t index, any p1) {
		script_.perform(index, p1);
	}
	void perform(action_index_t index, any p1, any p2) {
		script_.perform(index, p1, p2);
	}
	void perform(action_index_t index, any p1, any p2, any p3) {
		script_.perform(index, p1, p2, p3);
	}
	void perform(action_index_t index, any p1, any p2, any p3, any p4) {
		script_.perform(index, p1, p2, p3, p4);
	}
	void perform(action_index_t index, any p1, any p2, any p3, any p4, any p5) {
		script_.perform(index, p1, p2, p3, p4, p5);
	}

	void action(action_index_t index) {
		script_.action(index);
	}
	void action(action_index_t index, any p1) {
		script_.action(index, p1);
	}
	void action(action_index_t index, any p1, any p2) {
		script_.action(index, p1, p2);
	}
	void action(action_index_t index, any p1, any p2, any p3) {
		script_.action(index, p1, p2, p3);
	}

public:
	void registerAction(action_index_t index, action_handler_type handler) {
		action_handler_.insert(pair<action_index_t, action_handler_type>(index, handler));
	}
	void registerAction(action_index_t index, action_handler_p1_type handler) {
		action_handler_p1_.insert(pair<action_index_t, action_handler_p1_type>(index, handler));
	}
	void registerAction(action_index_t index, action_handler_p2_type handler) {
		action_handler_p2_.insert(pair<action_index_t, action_handler_p2_type>(index, handler));
	}
	void registerAction(action_index_t index, action_handler_p3_type handler) {
		action_handler_p3_.insert(pair<action_index_t, action_handler_p3_type>(index, handler));
	}

	void registerPerform(action_index_t index, perform_handler_type handler) {
		perform_handler_.insert(pair<action_index_t, perform_handler_type>(index, handler));
	}
	void registerPerform(action_index_t index, perform_handler_p1_type handler) {
		perform_handler_.insert(pair<action_index_t, perform_handler_p1_type>(index, handler));
	}
	void registerPerform(action_index_t index, perform_handler_p2_type handler) {
		perform_handler_.insert(pair<action_index_t, perform_handler_p2_type>(index, handler));
	}
	void registerPerform(action_index_t index, perform_handler_p3_type handler) {
		perform_handler_.insert(pair<action_index_t, perform_handler_p3_type>(index, handler));
	}
	void registerPerform(action_index_t index, perform_handler_p4_type handler) {
		perform_handler_.insert(pair<action_index_t, perform_handler_p4_type>(index, handler));
	}
	void registerPerform(action_index_t index, perform_handler_p5_type handler) {
		perform_handler_.insert(pair<action_index_t, perform_handler_p5_type>(index, handler));
	}

protected:
	map<action_index_t, action_handler_type>	action_handler_;
	map<action_index_t, action_handler_p1_type>	action_handler_p1_;
	map<action_index_t, action_handler_p2_type>	action_handler_p2_;
	map<action_index_t, action_handler_p3_type>	action_handler_p3_;

	map<action_index_t, perform_handler_type>	perform_handler_;
	map<action_index_t, perform_handler_p1_type>	perform_handler_p1_;
	map<action_index_t, perform_handler_p2_type>	perform_handler_p2_;
	map<action_index_t, perform_handler_p3_type>	perform_handler_p3_;
	map<action_index_t, perform_handler_p4_type>	perform_handler_p4_;
	map<action_index_t, perform_handler_p5_type>	perform_handler_p5_;
};

}}


#endif /* ACTIONOBJECT_H_ */
