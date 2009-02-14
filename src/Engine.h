/*
 * =====================================================================================
 * 
 *       Filename:  Engine.h
 * 
 *    Description:  处理棋盘信息--伪引擎
 * 
 *        Version:  1.0
 *        Created:  2009年02月14日 16时43分59秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 * 
 * =====================================================================================
 */

#ifndef  ENGINE_FILE_HEADER_INC
#define  ENGINE_FILE_HEADER_INC

#include <stdio.h>
#include "gmchess.h"

#if 0
//保存的最大历史局面数
const int MAX_COUNT=512;

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
const char *const cszStartFen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w";

// 棋子类型对应的棋子符号
const char *const cszPieceBytes = "KABNRCP";

const int PieceTypes[48] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 6,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 6
};

#endif


/**
 * @brief 处理棋盘数据结构
 * 暂时来说是保存FEN串为历史局面，把当前局面保存到棋盘数组并输出
 *
 */
class Engine
{
	public:
		Engine();
		~Engine();

		/**
		 * @brief 从Fen串生成棋盘数组
		 * @param szFen Fen串
		 */
		void FromFen(const char *szFen);
		/**
		 * @brief 棋盘数组生成Fen串
		 * @param szFen 生成的Fen串
		 */
		void ToFen(char* szFen);
		/**
		 * @brief 添加棋子信息
		 * @param sq 棋盘数组的序号
		 * @param pc 棋子的类型
		 */
		void AddPiece(int sq, int pc);
		/**
		 * @brief FEN串转成棋子标识，此函数只识别大写字母
		 * @param nArg FEN串字母
		 * @return 返回棋子标识
		 */
		int FenPiece(int nArg);
		/** 由x，y位置获得棋盘数组的位置*/
		inline int COORD_XY(int x,int y){ return x+(y<<4);};
		/** 获取y坐标*/
		inline int RANK_Y(int sq) {	  return sq >> 4;};

		/**获取x坐标*/
		inline int RANK_X(int sq) {
			  return sq & 15;
		}
		
		inline int SIDE_TAG(int sd) {
			  int pc = 16 + (sd << 4);
			    return pc;
		}
		inline char PIECE_BYTE(int pt) {
			  return cszPieceBytes[pt];
		}
		/** 返回棋子的类型 */
		inline int PIECE_TYPE(int pc) {
			  return PieceTypes[pc];
		}
		
		/** 交换走棋方*/
		void ChangeSide(){ sdPlayer = ~sdPlayer;};
		/** 引擎重置 */
		void reset();
		/** @brief 返回x，y位置上的棋子
		 *  @param x x 坐标
		 *  @param y y 坐标
		 *  @return 返回棋子类型，如果-1表示没有棋子
		 */
		int getPieces(int x,int y);


	private:
		/** 当前局面的棋盘数组*/
		int Square[256];
		/** 当前局面的棋子数组，相应棋子的值为棋盘上的坐标,0表示被吃了*/
		int Pieces[48];
		/** 用于保存所有历史局面的FEN串数组*/
		char* FenLogList[MAX_COUNT];
		/** 
		 * @brief 谁走子的信息
		 * 0 是红方先走，1是黑方先走
		 */
		bool sdPlayer;
		/** 走棋计数*/
		int count;




};


#endif   /* ----- #ifndef ENGINE_FILE_HEADER_INC  ----- */

