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
#else
  #include <dlfcn.h>
#endif
#include "base2.h"
#include "position.h"
#include "cchess.h"
#include "ecco.h"
#include "pgnfile.h"

static const int _ERROR_OPEN = -2;
static const int _ERROR_CREATE = -1;
static const int _OK = 0;

int Ccm2Pgn(const char *szCcmFile, const char *szPgnFile, const EccoApiStruct &EccoApi) {
  int mv, nStatus;
  bool bRead, bFlip;
  PgnFileStruct pgn;
  PositionStruct pos;
  char cCcm[8];
  FILE *fp;
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
    mv = MOVE(COORD_XY(cCcm[3] + FILE_LEFT, cCcm[5] + RANK_TOP),
        COORD_XY(cCcm[4] + FILE_LEFT, cCcm[6] + RANK_TOP));
    mv &= 0xffff; // 防止TryMove时数组越界
    pgn.nMaxMove ++;
    if (pgn.nMaxMove <= 20) {
      dwFileMove[pgn.nMaxMove - 1] = Move2File(mv, pos);
    }
    // 中游可能允许把将吃掉，但ElephantEye不允许，所以跳过非法着法
    if (TryMove(pos, nStatus, mv)) {
      pgn.wmvMoveTable[pgn.nMaxMove] = mv;
    } else {
      pgn.nMaxMove --;
    }
    if (pos.nMoveNum == MAX_MOVE_NUM) {
      pos.SetIrrev();
    }
  }

  if (pgn.nMaxMove < 20) {
    dwFileMove[pgn.nMaxMove] = 0;
  }
  if (EccoApi.Available()) {
    dwEccoIndex = EccoApi.EccoIndex((const char *) dwFileMove);
    strcpy(pgn.szEcco, (const char *) &dwEccoIndex);
    strcpy(pgn.szOpen, EccoApi.EccoOpening(dwEccoIndex));
    strcpy(pgn.szVar, EccoApi.EccoVariation(dwEccoIndex));
  }

  fclose(fp);
  return (pgn.Write(szPgnFile) ? _OK : _ERROR_CREATE);
}


static int ReadInt(FILE *fp) {
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


int Che2Pgn(const char *szCheFile, const char *szPgnFile, const EccoApiStruct &EccoApi) {
  int i, nMoveNum, mv, nStatus;
  int xSrc, ySrc, xDst, yDst;
  PgnFileStruct pgn;
  PositionStruct pos;
  FILE *fp;
  uint32_t dwEccoIndex, dwFileMove[20];

  fp = fopen(szCheFile, "rb");
  if (fp == NULL) {
    return _ERROR_OPEN;
  }

  pgn.posStart.FromFen(cszStartFen);
  pos = pgn.posStart;
  ReadInt(fp);
  nMoveNum = ReadInt(fp);

  for (i = 0; i < nMoveNum; i ++) {
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
    mv &= 0xffff; // 防止TryMove时数组越界
    pgn.nMaxMove ++;
    if (pgn.nMaxMove <= 20) {
      dwFileMove[pgn.nMaxMove - 1] = Move2File(mv, pos);
    }
    // QQ象棋允许把将吃掉，但ElephantEye不允许，所以跳过非法着法
    if (TryMove(pos, nStatus, mv)) {
      pgn.wmvMoveTable[pgn.nMaxMove] = mv;
    } else {
      pgn.nMaxMove --;
    }
    if (pos.nMoveNum == MAX_MOVE_NUM) {
      pos.SetIrrev();
    }
  }

  if (pgn.nMaxMove < 20) {
    dwFileMove[pgn.nMaxMove] = 0;
  }
  if (EccoApi.Available()) {
    dwEccoIndex = EccoApi.EccoIndex((const char *) dwFileMove);
    strcpy(pgn.szEcco, (const char *) &dwEccoIndex);
    strcpy(pgn.szOpen, EccoApi.EccoOpening(dwEccoIndex));
    strcpy(pgn.szVar, EccoApi.EccoVariation(dwEccoIndex));
  }

  fclose(fp);
  return (pgn.Write(szPgnFile) ? _OK : _ERROR_CREATE);
}


struct ChnRecord {
  uint16_t wReserved1[2];
  uint16_t wxSrc, wySrc, wxDst, wyDst;
  uint16_t wReserved2[10];
};

int Chn2Pgn(const char *szChnFile, const char *szPgnFile, const EccoApiStruct &EccoApi) {
  int mv, nStatus;
  bool bRead, bFlip;
  PgnFileStruct pgn;
  PositionStruct pos;
  ChnRecord Chn;
  FILE *fp;
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
    mv = MOVE(COORD_XY(Chn.wxSrc + FILE_LEFT, Chn.wySrc + RANK_TOP),
        COORD_XY(Chn.wxDst + FILE_LEFT, Chn.wyDst + RANK_TOP));
    mv &= 0xffff; // 防止TryMove时数组越界
    pgn.nMaxMove ++;
    if (pgn.nMaxMove <= 20) {
      dwFileMove[pgn.nMaxMove - 1] = Move2File(mv, pos);
    }
    // 联众可能允许把将吃掉，但ElephantEye不允许，所以跳过非法着法
    if (TryMove(pos, nStatus, mv)) {
      pgn.wmvMoveTable[pgn.nMaxMove] = mv;
    } else {
      pgn.nMaxMove --;
    }
    if (pos.nMoveNum == MAX_MOVE_NUM) {
      pos.SetIrrev();
    }
  }

  if (pgn.nMaxMove < 20) {
    dwFileMove[pgn.nMaxMove] = 0;
  }
  if (EccoApi.Available()) {
    dwEccoIndex = EccoApi.EccoIndex((const char *) dwFileMove);
    strcpy(pgn.szEcco, (const char *) &dwEccoIndex);
    strcpy(pgn.szOpen, EccoApi.EccoOpening(dwEccoIndex));
    strcpy(pgn.szVar, EccoApi.EccoVariation(dwEccoIndex));
  }

  fclose(fp);
  return (pgn.Write(szPgnFile) ? _OK : _ERROR_CREATE);
}




int main(int argc, char **argv) {
  EccoApiStruct EccoApi;
  char szLibEccoPath[1024];

  if (argc < 2) {
    printf("=== CCM->PGN Convertor ===\n");
    printf("Usage: CCM2PGN CCM-File [PGN-File]\n");
    return 0;
  }

  PreGenInit();
  ChineseInit();
  LocatePath(szLibEccoPath, cszLibEccoFile);
  EccoApi.Startup(szLibEccoPath);

  switch (Ccm2Pgn(argv[1], argc == 2 ? "/tmp/tmp.pgn" : argv[2], EccoApi)) {
  case _ERROR_OPEN:
    printf("%s: File Opening Error!\n", argv[1]);
    break;
  case _ERROR_CREATE:
    printf("File Creation Error!\n");
    break;
  case _OK:
#ifdef _WIN32
    if (argc == 2) {
      ShellExecute(NULL, NULL, "CCM2PGN.PGN", NULL, NULL, SW_SHOW);
    }
#endif
    break;
  }
  EccoApi.Shutdown();
  return 0;
}

