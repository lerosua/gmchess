/*
 * =====================================================================================
 *
 *       Filename:  ec_throw.h
 *
 *    Description:  检查一个系统调用，如果有错误，抛出一个异常。
 *
 *        Version:  1.0
 *        Created:  2007年08月12日 12时53分55秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  wind (xihe), xihels@gmail.com
 *        Company:  cyclone
 *
 * =====================================================================================
 */
#ifndef _EC_THROW_H_
#define _EC_THROW_H_

#include <cerrno>
#include <stdexcept>
#include <sstream>
#include <string.h>

std::runtime_error throw_except(const char* func, const char* file, long line, const char* var);

#define EC_THROW(VAR) if ((VAR))  throw throw_except(__func__, __FILE__, __LINE__, #VAR)

#endif // _EC_THROW_H_

//void test()
//{
//	int f;
//	EC_THROW( -1 == (f = open("noexist", O_RDONLY)));
//}

