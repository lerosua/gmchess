/*
CCM->PGN Converter - a Chinese Chess Score Conversion Program
Designed by Morning Yellow, Version: 3.14, Last Modified: Jun. 2008
Copyright (C) 2004-2007 www.elephantbase.net

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else				/*  */
#include <dlfcn.h>
#endif				/*  */
#include <fstream>
#include <iostream>
#include "base2.h"
#include "parse.h"
#include "position.h"
#include "cchess.h"
#include "ecco.h"
#include "pgnfile.h"
#include "xqffile.h"
#include "CodeConverter.h"
static const int _ERROR_OPEN = -2;
static const int _ERROR_CREATE = -1;
static const int _OK = 0;
static const int _ERROR_FORMAT = -4;
static const int XQF2PGN_ERROR_OPEN = -3;
static const int XQF2PGN_ERROR_FORMAT = -2;

static const int CBR_HEADER_SIZE = 2214;
static const int CBL_HEADER_SIZE = 576;
static const int CBL_RECORD_SIZE = 4096;

/** 中游象棋格式*/
int Ccm2Pgn(const char *szCcmFile, const char *szPgnFile,
	     const EccoApiStruct & EccoApi)
{
	int mv, nStatus;
	bool bRead, bFlip;
	PgnFileStruct pgn;
	PositionStruct pos;
	char cCcm[8];
	FILE * fp;
	uint32_t dwEccoIndex, dwFileMove[20];
	fp = fopen(szCcmFile, "rb");
	if (fp == NULL) {
		return _ERROR_OPEN;
	}
	pgn.posStart.FromFen(cszStartFen);
	pos = pgn.posStart;
	bRead = bFlip = false;
	while (fread(&cCcm, 7, 1, fp) > 0) {
		if (!bRead) {
			bRead = true;
			if (cCcm[5] < 5) {
				bFlip = true;
			}
		}
		if (bFlip) {
			cCcm[3] = 8 - cCcm[3];
			cCcm[5] = 9 - cCcm[5];
			cCcm[4] = 8 - cCcm[4];
			cCcm[6] = 9 - cCcm[6];
		}
		mv =
		    MOVE(COORD_XY(cCcm[3] + FILE_LEFT, cCcm[5] + RANK_TOP),
			 COORD_XY(cCcm[4] + FILE_LEFT,
				   cCcm[6] + RANK_TOP));
		mv &= 0xffff;	// 防止TryMove时数组越界
		pgn.nMaxMove++;
		if (pgn.nMaxMove <= 20) {
			dwFileMove[pgn.nMaxMove - 1] = Move2File(mv, pos);
		}
		// 中游可能允许把将吃掉，但ElephantEye不允许，所以跳过非法着法
		if (TryMove(pos, nStatus, mv)) {
			pgn.wmvMoveTable[pgn.nMaxMove] = mv;
		} else {
			pgn.nMaxMove--;
		}
		if (pos.nMoveNum == MAX_MOVE_NUM) {
			pos.SetIrrev();
		}
	}
	if (pgn.nMaxMove < 20) {
		dwFileMove[pgn.nMaxMove] = 0;
	}
	if (EccoApi.Available()) {
		dwEccoIndex =
		    EccoApi.EccoIndex((const char *) dwFileMove);
		strcpy(pgn.szEcco, (const char *) &dwEccoIndex);
		strcpy(pgn.szOpen, EccoApi.EccoOpening(dwEccoIndex));
		strcpy(pgn.szVar, EccoApi.EccoVariation(dwEccoIndex));
	}
	fclose(fp);
	return (pgn.Write(szPgnFile) ? _OK : _ERROR_CREATE);
}


/** QQ象棋格式*/
static int ReadInt(FILE * fp)
{
	int nResult, n;
	nResult = 0;
	n = fgetc(fp) - '0';
	while (n >= 0 && n <= 9) {
		nResult *= 10;
		nResult += n;
		n = fgetc(fp) - '0';
	}
	return nResult;
}
int Che2Pgn(const char *szCheFile, const char *szPgnFile,
	       const EccoApiStruct & EccoApi)
{
	int i, nMoveNum, mv, nStatus;
	int xSrc, ySrc, xDst, yDst;
	PgnFileStruct pgn;
	PositionStruct pos;
	FILE * fp;
	uint32_t dwEccoIndex, dwFileMove[20];
	fp = fopen(szCheFile, "rb");
	if (fp == NULL) {
		return _ERROR_OPEN;
	}
	pgn.posStart.FromFen(cszStartFen);
	pos = pgn.posStart;
	ReadInt(fp);
	nMoveNum = ReadInt(fp);
	for (i = 0; i < nMoveNum; i++) {
		ReadInt(fp);
		ReadInt(fp);
		ReadInt(fp);
		ySrc = 12 - ReadInt(fp);
		xSrc = ReadInt(fp) + 3;
		yDst = 12 - ReadInt(fp);
		xDst = ReadInt(fp) + 3;
		ReadInt(fp);
		ReadInt(fp);
		ReadInt(fp);
		mv = MOVE(COORD_XY(xSrc, ySrc), COORD_XY(xDst, yDst));
		mv &= 0xffff;	// 防止TryMove时数组越界
		pgn.nMaxMove++;
		if (pgn.nMaxMove <= 20) {
			dwFileMove[pgn.nMaxMove - 1] = Move2File(mv, pos);
		}
		// QQ象棋允许把将吃掉，但ElephantEye不允许，所以跳过非法着法
		if (TryMove(pos, nStatus, mv)) {
			pgn.wmvMoveTable[pgn.nMaxMove] = mv;
		} else {
			pgn.nMaxMove--;
		}
		if (pos.nMoveNum == MAX_MOVE_NUM) {
			pos.SetIrrev();
		}
	}
	if (pgn.nMaxMove < 20) {
		dwFileMove[pgn.nMaxMove] = 0;
	}
	if (EccoApi.Available()) {
		dwEccoIndex =
		    EccoApi.EccoIndex((const char *) dwFileMove);
		strcpy(pgn.szEcco, (const char *) &dwEccoIndex);
		strcpy(pgn.szOpen, EccoApi.EccoOpening(dwEccoIndex));
		strcpy(pgn.szVar, EccoApi.EccoVariation(dwEccoIndex));
	}
	fclose(fp);
	return (pgn.Write(szPgnFile) ? _OK : _ERROR_CREATE);
}


/** 联众象棋格式*/
    struct ChnRecord {
	uint16_t wReserved1[2];
	uint16_t wxSrc, wySrc, wxDst, wyDst;
	uint16_t wReserved2[10];
};
int Chn2Pgn(const char *szChnFile, const char *szPgnFile,
	      const EccoApiStruct & EccoApi)
{
	int mv, nStatus;
	bool bRead, bFlip;
	PgnFileStruct pgn;
	PositionStruct pos;
	ChnRecord Chn;
	FILE * fp;
	uint32_t dwEccoIndex, dwFileMove[20];
	fp = fopen(szChnFile, "rb");
	if (fp == NULL) {
		return _ERROR_OPEN;
	}
	pgn.posStart.FromFen(cszStartFen);
	pos = pgn.posStart;
	bRead = bFlip = false;
	fseek(fp, 188, SEEK_SET);
	while (fread(&Chn, sizeof(ChnRecord), 1, fp) > 0) {
		if (!bRead) {
			bRead = true;
			if (Chn.wySrc < 5) {
				bFlip = true;
			}
		}
		if (bFlip) {
			Chn.wxSrc = 8 - Chn.wxSrc;
			Chn.wySrc = 9 - Chn.wySrc;
			Chn.wxDst = 8 - Chn.wxDst;
			Chn.wyDst = 9 - Chn.wyDst;
		}
		mv =
		    MOVE(COORD_XY
			 (Chn.wxSrc + FILE_LEFT, Chn.wySrc + RANK_TOP),
			 COORD_XY(Chn.wxDst + FILE_LEFT,
				   Chn.wyDst + RANK_TOP));
		mv &= 0xffff;	// 防止TryMove时数组越界
		pgn.nMaxMove++;
		if (pgn.nMaxMove <= 20) {
			dwFileMove[pgn.nMaxMove - 1] = Move2File(mv, pos);
		}
		// 联众可能允许把将吃掉，但ElephantEye不允许，所以跳过非法着法
		if (TryMove(pos, nStatus, mv)) {
			pgn.wmvMoveTable[pgn.nMaxMove] = mv;
		} else {
			pgn.nMaxMove--;
		}
		if (pos.nMoveNum == MAX_MOVE_NUM) {
			pos.SetIrrev();
		}
	}
	if (pgn.nMaxMove < 20) {
		dwFileMove[pgn.nMaxMove] = 0;
	}
	if (EccoApi.Available()) {
		dwEccoIndex =
		    EccoApi.EccoIndex((const char *) dwFileMove);
		strcpy(pgn.szEcco, (const char *) &dwEccoIndex);
		strcpy(pgn.szOpen, EccoApi.EccoOpening(dwEccoIndex));
		strcpy(pgn.szVar, EccoApi.EccoVariation(dwEccoIndex));
	}
	fclose(fp);
	return (pgn.Write(szPgnFile) ? _OK : _ERROR_CREATE);
}
inline void ReadRecord(FILE * fp, char *sz)
{
	uint8_t ucLen;
	fread(&ucLen, 1, 1, fp);
	fread(sz, 1, ucLen, fp);
	sz[ucLen] = '\0';
} 

/** 弈天象棋软件格式*/
int Mxq2Pgn(const char *szMxqFile, const char *szPgnFile,
	     const EccoApiStruct & EccoApi)
{
	int i, mv, nStatus;
	char *lpEvent;
	char szRecord[256], szComment[256];
	PgnFileStruct pgn;
	PositionStruct pos;
	FILE * fp;
	uint32_t dwEccoIndex, dwFileMove[20];
	fp = fopen(szMxqFile, "rb");
	if (fp == NULL) {
		return _ERROR_OPEN;
	}
	ReadRecord(fp, pgn.szSite);
	ReadRecord(fp, pgn.szDate);
	ReadRecord(fp, pgn.szEvent);
	lpEvent = pgn.szEvent;
	if (false) {
	} else if (StrScanSkip(lpEvent, "-胜-")) {
		pgn.nResult = 1;
	} else if (StrScanSkip(lpEvent, "-衊-")) {
		pgn.nResult = 1;
	} else if (StrScanSkip(lpEvent, "-和-")) {
		pgn.nResult = 2;
	} else if (StrScanSkip(lpEvent, "-㎝-")) {
		pgn.nResult = 2;
	} else if (StrScanSkip(lpEvent, "-负-")) {
		pgn.nResult = 3;
	} else if (StrScanSkip(lpEvent, "-璽-")) {
		pgn.nResult = 3;
	} else if (StrScanSkip(lpEvent, "-負-")) {
		pgn.nResult = 3;
	} else {
		pgn.nResult = 0;
	}
	if (pgn.nResult != 0) {
		strcpy(pgn.szRed, pgn.szEvent);
		*(pgn.szRed + (lpEvent - pgn.szEvent - 4)) = '\0';
		strcpy(pgn.szBlack, lpEvent);
	}
	ReadRecord(fp, pgn.szRedElo);
	ReadRecord(fp, pgn.szBlackElo);
	for (i = 0; i < 5; i++) {
		ReadRecord(fp, szRecord);
	}
	ReadRecord(fp, szComment);
	ReadRecord(fp, szRecord);
	pgn.posStart.FromFen(cszStartFen);
	pos = pgn.posStart;
	ReadRecord(fp, szRecord);
	while (!StrEqv(szRecord, "Ends")
		&& pgn.nMaxMove < MAX_MOVE_LEN - 1) {
		mv =
		    MOVE(COORD_XY
			 (szRecord[0] - '0' + 3, 'J' - szRecord[1] + 3),
			 COORD_XY(szRecord[3] - '0' + 3,
				  'J' - szRecord[4] + 3));
		mv &= 0xffff;	// 防止TryMove时数组越界
		pgn.nMaxMove++;
		if (pgn.nMaxMove <= 20) {
			dwFileMove[pgn.nMaxMove - 1] = Move2File(mv, pos);
		}
		// 弈天可能允许把将吃掉，但ElephantEye不允许，所以跳过非法着法
		if (TryMove(pos, nStatus, mv)) {
			pgn.wmvMoveTable[pgn.nMaxMove] = mv;
		} else {
			pgn.nMaxMove--;
		}
		if (pos.nMoveNum == MAX_MOVE_NUM) {
			pos.SetIrrev();
		}
		ReadRecord(fp, szRecord);
	}
	pgn.szCommentTable[pgn.nMaxMove] = new char[256];
	strcpy(pgn.szCommentTable[pgn.nMaxMove], szComment);
	if (pgn.nMaxMove < 20) {
		dwFileMove[pgn.nMaxMove] = 0;
	}
	if (EccoApi.Available()) {
		dwEccoIndex =
		    EccoApi.EccoIndex((const char *) dwFileMove);
		strcpy(pgn.szEcco, (const char *) &dwEccoIndex);
		strcpy(pgn.szOpen, EccoApi.EccoOpening(dwEccoIndex));
		strcpy(pgn.szVar, EccoApi.EccoVariation(dwEccoIndex));
	}
	fclose(fp);
	return (pgn.Write(szPgnFile) ? _OK : _ERROR_CREATE);
}


/** 象棋演播室软件格式*/
static const int cnResultTrans[4] = { 0, 1, 3, 2
};
static const unsigned char cucsqXqf2Square[96] =
    { 0xc3, 0xb3, 0xa3, 0x93, 0x83, 0x73, 0x63, 0x53, 0x43, 0x33, 0xc4,
	0xb4, 0xa4, 0x94, 0x84, 0x74, 0x64, 0x54, 0x44, 0x34, 0xc5, 0xb5,
	0xa5, 0x95, 0x85, 0x75, 0x65, 0x55, 0x45,
	0x35, 0xc6, 0xb6, 0xa6, 0x96, 0x86, 0x76, 0x66, 0x56, 0x46, 0x36,
	0xc7, 0xb7, 0xa7, 0x97, 0x87, 0x77, 0x67,
	0x57, 0x47, 0x37, 0xc8, 0xb8, 0xa8, 0x98, 0x88, 0x78, 0x68, 0x58,
	0x48, 0x38, 0xc9, 0xb9, 0xa9, 0x99, 0x89,
	0x79, 0x69, 0x59, 0x49, 0x39, 0xca, 0xba, 0xaa, 0x9a, 0x8a, 0x7a,
	0x6a, 0x5a, 0x4a, 0x3a, 0xcb, 0xbb, 0xab,
	0x9b, 0x8b, 0x7b, 0x6b, 0x5b, 0x4b, 0x3b, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};
static const int cpcXqf2Piece[32] =
    { 23, 21, 19, 17, 16, 18, 20, 22, 24, 25, 26, 27, 28, 29, 30, 31, 39,
	37, 35, 33, 32, 34, 36, 38, 40, 41, 42, 43, 44, 45, 46, 47
};


// 密钥流掩码
static const char *const cszEncStreamMask =
    "[(C) Copyright Mr. Dong Shiwei.]";
inline int Square54Plus221(int x)
{
	return x * x * 54 + 221;
}
inline void ReadAndDecrypt(FILE * fp, void *lp, int nLen,
			     const int *nEncStream, int &nEncIndex)
{
	int i;
	fread(lp, nLen, 1, fp);
	for (i = 0; i < nLen; i++) {
		((uint8_t *) lp)[i] -= nEncStream[nEncIndex];
		nEncIndex = (nEncIndex + 1) % 32;
	}
}
inline void GetXqfString(char *szPgn, const char *szXqf)
{
	strncpy(szPgn, szXqf + 1, szXqf[0]);
	szPgn[szXqf[0]] = '\0';
} 

int Xqf2Pgn(const char *szXqfFile, const char *szPgnFile,
		    const EccoApiStruct & EccoApi)
{
	int i, nArg0, nArgs[4];
	int nCommentLen, mv, nStatus;
	bool bHasNext;
	PgnFileStruct pgn;
	PositionStruct pos;
	FILE * fp;
	XqfHeaderStruct xqfhd;
	XqfMoveStruct xqfmv;

	// 版本号和加密偏移值
	int nXqfVer, nPieceOff, nSrcOff, nDstOff, nCommentOff;

	// 密钥流
	int nEncStream[32];

	// 密钥流索引号
	int nEncIndex;

	// 局面初始位置
	int nPiecePos[32];
	uint32_t dwEccoIndex, dwFileMove[20];
	fp = fopen(szXqfFile, "rb");
	if (fp == NULL) {
		return XQF2PGN_ERROR_OPEN;
	}
	fread(&xqfhd, sizeof(xqfhd), 1, fp);
	fseek(fp, sizeof(xqfhd), SEEK_CUR);
	if (xqfhd.szTag[0] == 'X' && xqfhd.szTag[1] == 'Q') {

		// PGN文件可以打开，现在正式解析XQF文件
		nXqfVer = xqfhd.szTag[2];
		if (nXqfVer < 11) {
			nPieceOff = nSrcOff = nDstOff = nCommentOff = 0;
			for (i = 0; i < 32; i++) {
				nEncStream[i] = 0;
			}
		} else {

			// 局面初始位置的加密偏移值
			nPieceOff =
			    (uint8_t) (Square54Plus221
				       ((uint8_t) xqfhd.szTag[13]) *
				       (uint8_t) xqfhd.szTag[13]);

			// 着法起点的加密偏移值
			nSrcOff =
			    (uint8_t) (Square54Plus221
				       ((uint8_t) xqfhd.szTag[14]) *
				       nPieceOff);

			// 着法终点的加密偏移值
			nDstOff =
			    (uint8_t) (Square54Plus221
				       ((uint8_t) xqfhd.szTag[15]) *
				       nSrcOff);

			// 注释的加密偏移值
			nCommentOff =
			    ((uint8_t) xqfhd.szTag[12] * 256 +
			     (uint8_t) xqfhd.szTag[13]) % 32000 + 767;

			// 基本掩码
			nArg0 = xqfhd.szTag[3];

			// 密钥 = 前段密钥 | (后段密钥 & 基本掩码)
			for (i = 0; i < 4; i++) {
				nArgs[i] =
				    xqfhd.szTag[8 +
						i] | (xqfhd.szTag[12 +
								  i] &
						      nArg0);
			}

			// 密钥流 = 密钥 & 密钥流掩码
			for (i = 0; i < 32; i++) {
				nEncStream[i] =
				    (uint8_t) (nArgs[i % 4] &
					       cszEncStreamMask[i]);
			}
		}
		nEncIndex = 0;
		
		    // 记录棋谱信息
		    if (xqfhd.szEvent[0] == 0) {
			GetXqfString(pgn.szEvent, xqfhd.szTitle);
		} else {
			GetXqfString(pgn.szEvent, xqfhd.szEvent);
		}
		GetXqfString(pgn.szDate, xqfhd.szDate);
		GetXqfString(pgn.szSite, xqfhd.szSite);
		GetXqfString(pgn.szRed, xqfhd.szRed);
		GetXqfString(pgn.szBlack, xqfhd.szBlack);
		pgn.nResult = cnResultTrans[(int) xqfhd.szResult[3]];
		if (xqfhd.szSetUp[0] < 2) {

			// 如果是开局或者全局，那么直接设置起始局面
			pgn.posStart.FromFen(cszStartFen);
		} else {

			// 如果是中局或者排局，那么根据"xqfhd.szPiecePos[32]"的内容摆放局面
			// 当版本号达到12时，还要进一步解密局面初始位置
			if (nXqfVer < 12) {
				for (i = 0; i < 32; i++) {
					nPiecePos[i] =
					    (uint8_t) (xqfhd.
						       szPiecePos[i] -
						       nPieceOff);
				}
			} else {
				for (i = 0; i < 32; i++) {
					nPiecePos[(nPieceOff + 1 +
						    i) % 32] =
					    (uint8_t) (xqfhd.
						       szPiecePos[i] -
						       nPieceOff);
				}
			}

			// 把"nPiecePos[32]"的数据放到"PositionStruct"中
			pgn.posStart.ClearBoard();
			for (i = 0; i < 32; i++) {
				if (nPiecePos[i] < 90) {
					pgn.posStart.
					    AddPiece(cucsqXqf2Square
						     [nPiecePos[i]],
						     cpcXqf2Piece[i]);
				}
			}
			pgn.posStart.SetIrrev();
		}
		pos = pgn.posStart;
		bHasNext = true;
		while (bHasNext && pgn.nMaxMove < MAX_MOVE_LEN) {

			// 读取着法记录
			if (nXqfVer < 11) {
				fread(&xqfmv, sizeof(xqfmv), 1, fp);
				fread(&nCommentLen, sizeof(int), 1, fp);
				if ((xqfmv.ucTag & 0xf0) == 0) {
					bHasNext = false;
				}
			} else {
				ReadAndDecrypt(fp, &xqfmv, sizeof(xqfmv),
						nEncStream, nEncIndex);
				if ((xqfmv.ucTag & 0x20) != 0) {
					ReadAndDecrypt(fp, &nCommentLen,
							sizeof(int),
							nEncStream,
							nEncIndex);
					nCommentLen -= nCommentOff;
				} else {
					nCommentLen = 0;
				}
				if ((xqfmv.ucTag & 0x80) == 0) {
					bHasNext = false;
				}
			}
			if (pgn.nMaxMove > 0) {

				// 记录着法
				mv = MOVE(cucsqXqf2Square[(uint8_t)
							  (xqfmv.ucSrc -
							   24 - nSrcOff)],
					  cucsqXqf2Square[(uint8_t)
							  (xqfmv.ucDst -
							   32 - nDstOff)]);
				if (pgn.nMaxMove == 1) {
					if ((pgn.posStart.
					      ucpcSquares[SRC(mv)] & 32) !=
					     0) {
						pgn.posStart.ChangeSide();
						pos.ChangeSide();
					}
				}
				if (xqfhd.szSetUp[0] < 2
				     && pgn.nMaxMove <= 20) {
					dwFileMove[pgn.nMaxMove - 1] =
					    Move2File(mv, pos);
				}
				TryMove(pos, nStatus, mv);
				pgn.wmvMoveTable[pgn.nMaxMove] = mv;
				if (pos.nMoveNum == MAX_MOVE_NUM) {
					pos.SetIrrev();
				}
			}
			if (nCommentLen > 0) {
				pgn.szCommentTable[pgn.nMaxMove] =
				    new char[nCommentLen + 1];
				ReadAndDecrypt(fp,
						pgn.szCommentTable[pgn.
								   nMaxMove],
						nCommentLen, nEncStream,
						nEncIndex);
				pgn.szCommentTable[pgn.
						    nMaxMove][nCommentLen]
				    = '\0';
			}
			pgn.nMaxMove++;
		} pgn.nMaxMove--;
		
		    // 解析ECCO
		    if (xqfhd.szSetUp[0] < 2) {
			if (pgn.nMaxMove < 20) {
				dwFileMove[pgn.nMaxMove] = 0;
			}
			if (EccoApi.Available()) {
				dwEccoIndex =
				    EccoApi.
				    EccoIndex((const char *) dwFileMove);
				strcpy(pgn.szEcco,
					(const char *) &dwEccoIndex);
				strcpy(pgn.szOpen,
					EccoApi.EccoOpening(dwEccoIndex));
				strcpy(pgn.szVar,
					EccoApi.
					EccoVariation(dwEccoIndex));
			}
		}
		fclose(fp);
		return (pgn.Write(szPgnFile) ? _OK : _ERROR_CREATE);
	} else {
		fclose(fp);
		return XQF2PGN_ERROR_FORMAT;
	}
}

/** 只是将大写的PGN后缀文件copy一下 */
int Pgn2Pgn(const char *szFile, const char *szPgnFile,
	     const EccoApiStruct & EccoApi)
{
	FILE * infp;
	FILE * outfp;
	char c;
	infp = fopen(szFile, "rb");
	if (infp == NULL) {
		return _ERROR_OPEN;
	}
	if((outfp=fopen(szPgnFile,"wb"))==NULL) {
		fclose(infp);
		return _ERROR_OPEN;
	}
	while(!feof(infp)){
		c=fgetc(infp);
		fputc(c,outfp);
	}
	fclose(infp);
	fclose(outfp);
	return 0;

}

static uint32_t ReadLe32(const uint8_t *data)
{
	return (uint32_t) data[0] | ((uint32_t) data[1] << 8) |
	    ((uint32_t) data[2] << 16) | ((uint32_t) data[3] << 24);
}

static uint16_t ReadLe16(const uint8_t *data)
{
	return (uint16_t) data[0] | ((uint16_t) data[1] << 8);
}

static bool ReadFileBytes(const char *filename, std::vector<uint8_t> &bytes)
{
	FILE *fp;
	long size;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		return false;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	if (size < 0) {
		fclose(fp);
		return false;
	}
	fseek(fp, 0, SEEK_SET);
	bytes.resize((size_t) size);
	if (size > 0
	    && fread(&bytes[0], 1, (size_t) size, fp) != (size_t) size) {
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}

static std::string Utf16LeToGbk(const uint8_t *data, size_t len)
{
	size_t text_len = len;
	CodeConverter converter("utf-16le", "gbk");

	for (size_t i = 0; i + 1 < len; i += 2) {
		if (data[i] == 0 && data[i + 1] == 0) {
			text_len = i;
			break;
		}
	}

	if (text_len == 0) {
		return std::string();
	}
	return converter.convert((const char *) data, text_len);
}

static void CopyPgnText(char *dest, const std::string &src)
{
	strncpy(dest, src.c_str(), MAX_STR_LEN - 1);
	dest[MAX_STR_LEN - 1] = '\0';
}

static char *NewPgnComment(const std::string &comment)
{
	char *result;

	if (comment.empty()) {
		return NULL;
	}

	result = new char[comment.size() + 1];
	strcpy(result, comment.c_str());
	return result;
}

static int CbrPieceType(uint8_t piece)
{
	switch (piece & 0x0f) {
	case 1:
		return ROOK_FROM;
	case 2:
		return KNIGHT_FROM;
	case 3:
		return BISHOP_FROM;
	case 4:
		return ADVISOR_FROM;
	case 5:
		return KING_FROM;
	case 6:
		return CANNON_FROM;
	case 7:
		return PAWN_FROM;
	default:
		return -1;
	}
}

static bool CbrBoardToPosition(const uint8_t *boards, uint16_t move_side,
			       PositionStruct &pos)
{
	int next_piece[2][7] = {
		{ ROOK_FROM, KNIGHT_FROM, BISHOP_FROM, ADVISOR_FROM,
		  KING_FROM, CANNON_FROM, PAWN_FROM },
		{ ROOK_FROM, KNIGHT_FROM, BISHOP_FROM, ADVISOR_FROM,
		  KING_FROM, CANNON_FROM, PAWN_FROM },
	};
	int last_piece[7] = {
		ROOK_TO, KNIGHT_TO, BISHOP_TO, ADVISOR_TO,
		KING_FROM, CANNON_TO, PAWN_TO
	};

	pos.ClearBoard();

	for (int y = 0; y < 10; y++) {
		for (int x = 0; x < 9; x++) {
			uint8_t piece = boards[y * 9 + x];
			int side;
			int type;
			int piece_index;

			if ((piece & 0xf0) == 0x10) {
				side = 0;
			} else if ((piece & 0xf0) == 0x20) {
				side = 1;
			} else {
				continue;
			}

			type = CbrPieceType(piece);
			if (type < 0) {
				continue;
			}

			int type_index = (piece & 0x0f) - 1;
			piece_index = next_piece[side][type_index];
			if (piece_index > last_piece[type_index]) {
				return false;
			}

			pos.AddPiece(COORD_XY(x + FILE_LEFT, y + RANK_TOP),
				     SIDE_TAG(side) + piece_index);
			next_piece[side][type_index]++;
		}
	}

	if (move_side != 1) {
		pos.ChangeSide();
	}
	pos.SetIrrev();
	return true;
}

static int CbrSquare(uint8_t pos)
{
	int x = pos % 9;
	int y = pos / 9;

	if (pos >= 90) {
		return 0;
	}
	return COORD_XY(x + FILE_LEFT, y + RANK_TOP);
}

static bool CbrReadStep(const uint8_t *data, size_t len, size_t &offset,
			PgnFileStruct &pgn, PositionStruct &pos,
			bool keep_mainline)
{
	uint8_t step_mark;
	uint8_t step_from;
	uint8_t step_to;
	uint32_t comment_len = 0;
	std::string comment;
	PositionStruct before_move;
	int mv, nStatus;

	if (offset + 4 > len) {
		return true;
	}

	step_mark = data[offset];
	step_from = data[offset + 2];
	step_to = data[offset + 3];
	offset += 4;

	if (step_mark == 0 && data[offset - 3] == 0 && step_from == 0
	    && step_to == 0) {
		return true;
	}

	if ((step_mark & 0x04) != 0) {
		if (offset + 4 > len) {
			return false;
		}
		comment_len = ReadLe32(data + offset);
		offset += 4;
		if (offset + comment_len > len) {
			return false;
		}
		comment = Utf16LeToGbk(data + offset, comment_len);
		offset += comment_len;
	}

	before_move = pos;
	mv = MOVE(CbrSquare(step_from), CbrSquare(step_to));
	if (mv == 0) {
		return false;
	}

	if (pos.ucpcSquares[SRC(mv)] >= SIDE_TAG(1)) {
		if (pos.sdPlayer == 0) {
			pos.ChangeSide();
		}
	} else if (pos.ucpcSquares[SRC(mv)] >= SIDE_TAG(0)) {
		if (pos.sdPlayer == 1) {
			pos.ChangeSide();
		}
	} else {
		return false;
	}

	if (!TryMove(pos, nStatus, mv)) {
		return false;
	}
	if (pos.nMoveNum == MAX_MOVE_NUM) {
		pos.SetIrrev();
	}

	if (keep_mainline && pgn.nMaxMove < MAX_MOVE_LEN - 1) {
		pgn.nMaxMove++;
		pgn.wmvMoveTable[pgn.nMaxMove] = mv;
		pgn.szCommentTable[pgn.nMaxMove] = NewPgnComment(comment);
	}

	if ((step_mark & 0x01) == 0) {
		if (!CbrReadStep(data, len, offset, pgn, pos, keep_mainline)) {
			return false;
		}
	}

	if ((step_mark & 0x02) != 0) {
		if (!CbrReadStep(data, len, offset, pgn, before_move, false)) {
			return false;
		}
	}

	return true;
}

static bool ParseCbrBuffer(const uint8_t *data, size_t len, PgnFileStruct &pgn)
{
	PositionStruct pos;
	size_t offset = CBR_HEADER_SIZE;

	if (len < CBR_HEADER_SIZE
	    || memcmp(data, "CCBridge Record", 15) != 0) {
		return false;
	}

	pgn.Reset();
	CopyPgnText(pgn.szEvent, Utf16LeToGbk(data + 180, 128));
	if (pgn.szEvent[0] == '\0') {
		CopyPgnText(pgn.szEvent, Utf16LeToGbk(data + 692, 64));
	}
	CopyPgnText(pgn.szRound, Utf16LeToGbk(data + 180, 128));
	CopyPgnText(pgn.szRed, Utf16LeToGbk(data + 1076, 64));
	CopyPgnText(pgn.szBlack, Utf16LeToGbk(data + 1300, 64));
	pgn.nResult = data[2076] <= 3 ? data[2076] : 0;

	if (!CbrBoardToPosition(data + 2120, ReadLe16(data + 2116), pos)) {
		return false;
	}
	pgn.posStart = pos;

	if (offset + 4 <= len) {
		uint32_t initial_comment_flag = ReadLe32(data + offset);
		offset += 4;
		if (initial_comment_flag != 0) {
			uint32_t comment_len;
			if (offset + 4 > len) {
				return false;
			}
			comment_len = ReadLe32(data + offset);
			offset += 4;
			if (offset + comment_len > len) {
				return false;
			}
			pgn.szCommentTable[0] =
			    NewPgnComment(Utf16LeToGbk(data + offset,
						       comment_len));
			offset += comment_len;
		}
	}

	if (offset < len) {
		return CbrReadStep(data, len, offset, pgn, pos, true);
	}
	return true;
}

int Cbr2Pgn(const char *szCbrFile, const char *szPgnFile,
	     const EccoApiStruct & EccoApi)
{
	std::vector<uint8_t> bytes;
	PgnFileStruct pgn;

	if (!ReadFileBytes(szCbrFile, bytes)) {
		return _ERROR_OPEN;
	}
	if (bytes.empty() || !ParseCbrBuffer(&bytes[0], bytes.size(), pgn)) {
		return _ERROR_FORMAT;
	}
	return (pgn.Write(szPgnFile) ? _OK : _ERROR_CREATE);
}

static int CblDataOffset(int book_count)
{
	if (book_count <= 128) {
		return 101952;
	} else if (book_count <= 256) {
		return 137280;
	} else if (book_count <= 384) {
		return 151080;
	} else if (book_count <= 512) {
		return 207936;
	}
	return 349248;
}

static bool AppendFile(FILE *outfp, const char *filename)
{
	FILE *infp = fopen(filename, "rb");
	char buffer[4096];
	size_t read_size;

	if (infp == NULL) {
		return false;
	}
	while ((read_size = fread(buffer, 1, sizeof(buffer), infp)) > 0) {
		fwrite(buffer, 1, read_size, outfp);
	}
	fclose(infp);
	return true;
}

static size_t FindBytes(const std::vector<uint8_t> &bytes, const char *needle,
			size_t start)
{
	size_t needle_len = strlen(needle);

	if (needle_len == 0 || start >= bytes.size()
	    || bytes.size() - start < needle_len) {
		return bytes.size();
	}

	for (size_t i = start; i <= bytes.size() - needle_len; i++) {
		if (memcmp(&bytes[i], needle, needle_len) == 0) {
			return i;
		}
	}
	return bytes.size();
}

int Cbl2Pgn(const char *szCblFile, const char *szPgnFile,
	     const EccoApiStruct & EccoApi)
{
	std::vector<uint8_t> bytes;
	const uint8_t *records;
	size_t record_offset;
	int book_count;
	int converted = 0;
	FILE *outfp;
	const char *tmp_pgn = "/tmp/gmchess-cbr-record.pgn";

	if (!ReadFileBytes(szCblFile, bytes)) {
		return _ERROR_OPEN;
	}

	if (bytes.size() >= 8 && memcmp(&bytes[0], "CCBridge", 8) == 0
	    && (bytes.size() < 16
		|| memcmp(&bytes[0], "CCBridgeLibrary", 15) != 0)) {
		return _ERROR_FORMAT;
	}

	if (bytes.size() < CBL_HEADER_SIZE
	    || memcmp(&bytes[0], "CCBridgeLibrary", 15) != 0) {
		return _ERROR_FORMAT;
	}

	book_count = (int) ReadLe32(&bytes[60]);
	record_offset = (size_t) CblDataOffset(book_count);
	if (record_offset >= bytes.size()) {
		return _ERROR_FORMAT;
	}
	record_offset = FindBytes(bytes, "CCBridge Record", record_offset);
	if (record_offset >= bytes.size()) {
		return _ERROR_FORMAT;
	}

	records = &bytes[record_offset];
	outfp = fopen(szPgnFile, "wb");
	if (outfp == NULL) {
		return _ERROR_CREATE;
	}

	for (size_t pos = 0; record_offset + pos + CBR_HEADER_SIZE <= bytes.size();
	     pos += CBL_RECORD_SIZE) {
		PgnFileStruct pgn;
		const uint8_t *record = records + pos;

		if (memcmp(record, "CCBridge Record", 15) != 0) {
			continue;
		}
		if (!ParseCbrBuffer(record,
				    MIN((size_t) CBL_RECORD_SIZE,
					bytes.size() - record_offset - pos),
				    pgn)) {
			continue;
		}
		if (!pgn.Write(tmp_pgn) || !AppendFile(outfp, tmp_pgn)) {
			fclose(outfp);
			remove(tmp_pgn);
			return _ERROR_CREATE;
		}
		fprintf(outfp, "\r\n\r\n");
		converted++;
		if (converted >= book_count) {
			break;
		}
	}

	fclose(outfp);
	remove(tmp_pgn);
	return converted > 0 ? _OK : _ERROR_FORMAT;
}

/** 
 *  CCM=中国游戏中心象棋
 *  CHE=QQ象棋
 *  CHN=联众象棋
 *  MXQ=弈天象棋
 *  XQF=象棋演播室
 */
enum { PGN,CCM, CHE, CHN, MXQ, XQF, CBR, CBL, ERR };
static bool has_extension(const char *filename, const char *extension)
{
	const char *file_extension;

	if (filename == NULL || extension == NULL) {
		return false;
	}

	file_extension = strrchr(filename, '.');
	if (file_extension == NULL) {
		return false;
	}

	while (*file_extension != '\0' && *extension != '\0') {
		if (tolower((unsigned char) *file_extension) !=
		    tolower((unsigned char) *extension)) {
			return false;
		}
		file_extension++;
		extension++;
	}

	return *file_extension == '\0' && *extension == '\0';
}

int format(char *filename)
{
	if (has_extension(filename, ".pgn")) {
		return PGN;
	} else if (has_extension(filename, ".ccm")) {
		return CCM;
	} else if (has_extension(filename, ".che")) {
		return CHE;
	} else if (has_extension(filename, ".chn")) {
		return CHN;
	} else if (has_extension(filename, ".mxq")) {
		return MXQ;
	} else if (has_extension(filename, ".xqf")) {
		return XQF;
	} else if (has_extension(filename, ".cbr")) {
		return CBR;
	} else if (has_extension(filename, ".cbl")) {
		return CBL;
	} else
		return ERR;
}


int file_gbk_to_utf8(const std::string& f_infile,const std::string& f_outfile)
{
    CodeConverter g2u("gbk","utf-8");
    std::fstream infile;
    infile.open(f_infile.c_str(),std::ios_base::in);
	if(!infile){
		printf("open %s file error\n",f_infile.c_str());
		return -1;
	}
	std::fstream outfile;
	outfile.open(f_outfile.c_str(),std::ios_base::out);
	if(!outfile){
		printf("open write file error\n");
		return -1;
	}
	std::string line;
	while(std::getline(infile,line)){
		std::string uline= g2u.convert(line);
		outfile<<uline<<std::endl;;
	}
	infile.close();
	outfile.close();
	return 0;
}


int main(int argc, char **argv)
{
	EccoApiStruct EccoApi;
	char szLibEccoPath[1024];
	int (*fun) (const char *szFile, const char *szPgnFile,
		     const EccoApiStruct & EccoApi);
	if (argc < 2) {
		printf("=== PGN Converter ===\n");
		printf("Usage: pgnconvert File [PGN-File]\n");
		printf("It support [.ccm|.che|.chn|.mxq|.xqf|.cbr|.cbl] \n");
		return 0;
	}
	switch (format(argv[1])) {
	case PGN:
		fun = Pgn2Pgn;
		break;
	case CCM:
		fun = Ccm2Pgn;
		break;
	case CHE:
		fun = Che2Pgn;
		break;
	case CHN:
		fun = Chn2Pgn;
		break;
	case MXQ:
		fun = Mxq2Pgn;
		break;
	case XQF:
		fun = Xqf2Pgn;
		break;
	case CBR:
		fun = Cbr2Pgn;
		break;
	case CBL:
		fun = Cbl2Pgn;
		break;
	case ERR:
		printf("file not support\n");
		return -1;
		break;
	}
	PreGenInit();
	ChineseInit();
	LocatePath(szLibEccoPath, cszLibEccoFile);
	EccoApi.Startup(szLibEccoPath);
	switch (fun
		  (argv[1],  "/tmp/gb.pgn",
		   EccoApi)) {
	case _ERROR_OPEN:
		printf("%s: File Opening Error!\n", argv[1]);
		break;
	case _ERROR_CREATE:
		printf("File Creation Error!\n");
		break;
	case _ERROR_FORMAT:
		printf("%s: File Format Error!\n", argv[1]);
		return -1;
		break;
	case _OK:
		printf("File convert finish\n");

		int out= file_gbk_to_utf8("/tmp/gb.pgn",argc==2?"/tmp/gmchess.pgn":argv[2]);
		if(out<0){
			printf("file code change error\n");
			return -1;
		}
		remove("/tmp/gb.pgn");

		break;
	}
	EccoApi.Shutdown();
	
	return 0;
}
