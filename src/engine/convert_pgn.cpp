/*
CCM->PGN Convertor - a Chinese Chess Score Convertion Program
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
static const int XQF2PGN_ERROR_OPEN = -3;
static const int XQF2PGN_ERROR_FORMAT = -2;

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

/** 
 *  CCM=中国游戏中心象棋
 *  CHE=QQ象棋
 *  CHN=联众象棋
 *  MXQ=弈天象棋
 *  XQF=象棋演播室
 */
enum { PGN,CCM, CHE, CHN, MXQ, XQF, ERR };
int format(char *filename)
{
	if ((strstr(filename, ".pgn") != NULL)
	     || strstr(filename, ".PGN") != NULL) {
		return PGN;
	}else if ((strstr(filename, ".ccm") != NULL)
	     || strstr(filename, ".CCM") != NULL) {
		return CCM;
	} else if ((strstr(filename, ".che") != NULL)
		    || strstr(filename, ".CHE") != NULL) {
		return CHE;
	} else if ((strstr(filename, ".chn") != NULL)
		    || strstr(filename, ".CHN")) {
		return CHN;
	} else if ((strstr(filename, ".xqf") != NULL)
		    || strstr(filename, ".XQF")) {
		return XQF;
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
		printf("=== PGN Convertor ===\n");
		printf("Usage: pgnconvert File [PGN-File]\n");
		printf("It support [.ccm|.che|.chn|.mxq|.xqf] \n");
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


