/*
 * zertcore.h
 *
 *  Created on: 2013-9-11
 *      Author: Administrator
 */

#ifndef ZERTCORE_H_
#define ZERTCORE_H_

#include <common.h>
//#include <Server.h>
//#include <Map.h>
#include <Runtime.h>
//#include <FSM.h>
#include <Log.h>
#include <I18N.h>
#include <Crypto.h>
#include <PoolObject.h>
#include <Buffer.h>
#include <Random.h>
#include <DatabaseItem.h>
#include <DatabaseUtils.h>
#include <UUIDGenerator.h>
#include <VersionManager.h>
#include <ShortTermServer.h>
#include <MutableValue.h>
#include <HaffmanTree.h>
#include <ObjectTag.h>
#include <DuplicationName.h>
#include <Expired.h>

namespace zertcore{ namespace base{

using namespace zertcore::utils;

template <typename config>
void RuntimeT<config>::init() {
	ClockExpiredUnit<EXPIRED_EVERY_DAY>::update();
	ClockExpiredUnit<EXPIRED_EVERY_HOUR>::update();
	ClockExpiredUnit<EXPIRED_EVERY_MINUTE>::update();

	UUIDGenerator::Instance().init();
	IMVManager::Instance().init();
	Random::init();

	for (typename init_handler_list_type::iterator it = init_handler_list_.begin();
			it != init_handler_list_.end(); ++it) {
		(*it)();
	}
}

template <typename config>
void RuntimeT<config>::deinit() {
	IMVManager::Instance().deinit();

	for (typename deinit_handler_list_type::iterator it = deinit_handler_list_.begin();
			it != deinit_handler_list_.end(); ++it) {
		(*it)();
	}
}

}}

#endif /* ZERTCORE_H_ */
