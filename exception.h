/*
 * exception.h
 *
 *  Created on: 2013-5-21
 *      Author: Administrator
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <cstdio>
#include <boost/exception/all.hpp>

namespace zertcore {

struct Exception :
		public std::exception
{
};

struct IOException :
		public Exception
{
};

struct UnValidException:
		public Exception
{
};

struct NullException:
		public Exception
{
};

struct UnSupportException:
		public Exception
{
};

struct TypeErrorException :
		public Exception
{
};

struct OutOfRangeException :
		public Exception
{
};

struct DataVerifyException :
		public Exception
{
};

struct UnExpectedException :
		public Exception
{
};

struct AssertionException :
		public Exception
{
};

struct ZeroDivideException :
		public Exception
{
};

}
#endif /* EXCEPTION_H_ */
