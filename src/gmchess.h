/*
 * =====================================================================================
 * 
 *       Filename:  gmchess.h
 * 
 *    Description:  定义一些常量
 * 
 *        Version:  1.0
 *        Created:  2009年02月14日 18时28分19秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 *        Copyright (C) lerosua 2009 <lerosua@gmail.com>
 * 
 * =====================================================================================
 */

#ifndef  GMCHESS_FILE_HEADER_INC
#define  GMCHESS_FILE_HEADER_INC

#include <stdio.h>
#include <stdint.h>
#include "define.h"
#include <map>
#include <string>

typedef std::map<std::string,std::string> TGMConf;
extern TGMConf GMConf;
//保存的最大历史局面数
//save the max number of history chess station
const int MAX_COUNT=512;
const int GMPORT=20125;

// 每种子力的开始序号和结束序号
 const int KING_FROM = 0;
 const int ADVISOR_FROM = 1;
 const int ADVISOR_TO = 2;
 const int BISHOP_FROM = 3;
 const int BISHOP_TO = 4;
 const int KNIGHT_FROM = 5;
 const int KNIGHT_TO = 6;
 const int ROOK_FROM = 7;
 const int ROOK_TO = 8;
 const int CANNON_FROM = 9;
 const int CANNON_TO = 10;
 const int PAWN_FROM = 11;
 const int PAWN_TO = 15;

const int RANK_TOP = 3;
const int RANK_BOTTOM = 12;
const int FILE_LEFT = 3;
const int FILE_CENTER = 7;
const int FILE_RIGHT = 11;


//初始化的FEN串
const char *const start_fen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w";

/** 棋子类型对应的棋子符号*/
const char *const chessman_bytes = "KABNRCPkabnrcp";

const int PieceTypes[48] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 6,
  7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 13, 13, 13
};

/**
 * 棋子的类型, type of chess
 *0-6 是红方棋子，分别是 帅仕相马车炮兵
 *7-13是黑方棋子，分别是 将士象马车炮卒
 *		英文字符是K A B N R C P
 */
enum {  
	RED_KING=0,RED_ADVISOR,RED_BISHOP,RED_KNIGHT,RED_ROOK,RED_CANNON,RED_PAWN,
	BLACK_KING,BLACK_ADVISOR,BLACK_BISHOP,BLACK_KNIGHT,BLACK_ROOK,BLACK_CANNON,BLACK_PAWN,
	RED_KING_DIE,BLACK_KING_DIE,
	PROPMT,SELECTED_CHESSMAN,
};

/** 
 * 程序棋局状态，分别为读谱，电脑对战，网络对战,自由摆棋
 * board station mode
 */
enum BOARD_STATUS{ READ_STATUS , FIGHT_STATUS, NETWORK_STATUS,FREE_STATUS,NULL_STATUS };

/** 棋子数组示例*/
const int PieceExample[48] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0xc7, 0xc6, 0xc8, 0xc5, 0xc9, 0xc4, 0xca, 0xc3, 0xcb, 0xa4,0xaa,0x93,0x95,0x97,0x99,0x9b,
  0x37,  0x36,  0x38,  0x35,  0x39, 0x34,  0x3a,  0x33,  0x3b,  0x54,  0x5a,  0x63,  0x65,  0x67,  0x69,0x6b
};

/** 棋盘区域表 */
/** the area of chess in board */
const bool chessInBoard[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/** 九宫格表*/
/** Fort array */
const bool chessInFort[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
union Hanzi{
		char word[4];
		uint32_t digit;
	};

/** 用于生成一些着法的数组
 * 这四个数组用来判断棋子的走子方向，以马为例就是：dst = src + KnightMoveTab[i]
 */
static const int KingMoveTab[4]    = {-0x10, -0x01, +0x01, +0x10};
static const int AdvisorMoveTab[4] = {-0x11, -0x0f, +0x0f, +0x11};
static const int BishopMoveTab[4]  = {-0x22, -0x1e, +0x1e, +0x22};
static const int KnightMoveTab[8]  = {-0x21, -0x1f, -0x12, -0x0e, +0x0e, +0x12, +0x1f, +0x21};

enum BOARDSIZE {BIG_BOARD=0, SMALL_BOARD};
enum OVERSTATUS {ROBOT_WIN=0,ROBOT_LOSE,ROBOT_DRAW, ROBOT_OVER_TIME,HUMAN_OVER_TIME};
#endif   /* ----- #ifndef GMCHESS_FILE_HEADER_INC  ----- */

