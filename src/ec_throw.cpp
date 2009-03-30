/*
 * =====================================================================================
 *
 *       Filename:  ec_throw.cpp
 *
 *    Description:  检查一个系统调用，如果有错误，抛出一个异常。
 *
 *        Version:  1.0
 *        Created:  2007年11月28日 20时30分02秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  wind (xihe), xihels@gmail.com
 *        Company:  cyclone
 *
 * =====================================================================================
 */

#include "ec_throw.h"

std::runtime_error throw_except(const char* func, const char* file, long line, const char* var)
{
	std::stringstream stream;
	stream << "File: " 	<< file << std::endl;
	stream << "Fucntion: " 	<< func << std::endl;
	stream << "Expression:"	<< std::endl << line << ":\t" << var << std::endl; 
	stream	<< "Errno: "	<< errno << std::endl;
	stream << strerror(errno) << std::endl; 
	return std::runtime_error(stream.str());
}


