/***************************************************************************
 *   Copyright (C) 2005 by xihe   *
 *   xihels@163.com                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
// 代码转换操作类
#ifndef CODE_CONVERTER_H
#define CODE_CONVERTER_H
#include <stdexcept>
#include <string>
#include <iconv.h>

using std::runtime_error;
using std::string;

class CodeConverter {
    private:
	iconv_t cd;
    public:
	// 构造
	CodeConverter(const char *from_charset,const char *to_charset) {
	    cd = iconv_open(to_charset,from_charset);
	}

	// 析构
	~CodeConverter() {
	    iconv_close(cd);
	}

	// 转换输出
	string convert(const string& inbuf) {
	    return convert(inbuf.c_str(), inbuf.size());
	}

	string convert(const char* inbuf, int inlen) {
	    int outlen = inlen * 3;
	    char *pin = (char*)inbuf;
	    char outbuf[outlen];

	    char *pout = &outbuf[0];
	    //bzero(pout, outbuf);
	    int oldoutlen = outlen;

	    if(-1 == iconv(cd,&pin,(size_t *)&inlen
			, &pout,(size_t *)&outlen) ) {
		return string();
	    } else {
		outlen = oldoutlen - outlen;
		outbuf[outlen] = '\0';
		return string(outbuf, outbuf + outlen);
	    }
	}

};

#endif //CODE_CONVERTER_H
