/*
 * ValidityObject.h
 *
 *  Created on: 2013-7-28
 *      Author: Administrator
 */

#ifndef VALIDITYOBJECT_H_
#define VALIDITYOBJECT_H_

#include <common.h>

namespace zertcore {

class ValidityObject
{
private:
	uint validity_sentinel_;

public:
	explicit ValidityObject():validity_sentinel_(0xdeadbeaf) {}
	virtual ~ValidityObject() {validity_sentinel_ = 0;}

public:
	bool isValid() const {
		return validity_sentinel_ == 0xdeadbeaf;
	}

	void checkValid() const {
		if (!isValid()) {
			THROW_EXCEPTION(UnValidException());
		}
	}
};

}


#endif /* VALIDITYOBJECT_H_ */
