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
#include <vector>
#include <string>


/**
 * @brief 处理棋盘数据结构
 * 暂时来说是保存FEN串为历史局面，把当前局面保存到棋盘数组并输出
 *
 */
class Engine {
	public:
		Engine();
		~Engine();

		/**
		 * @brief 添加棋局快照，即是保存当前的局面
		 * @param fen FEN串
		 */
		void add_snapshot(const char* fen);
		/**
		 * @brief 得到某步时的快照
		 * @param num 某一步时
		 */
		void get_snapshot(int num);
		
		/**
		 * @brief 棋盘数组生成Fen串 
		 * @param szFen 生成的Fen串
		 */
		void to_fen(char* szFen);
		/**
		 * @brief 添加棋子信息
		 * @param sq 棋盘数组的序号
		 * @param pc 棋子的类型
		 */
		void add_piece(int sq, int pc);
		/**
		 * @brief FEN串转成棋子标识数字，
		 * @param nArg FEN串字母
		 * @return 返回棋子标识
		 */
		int fen_to_piece(int nArg);
		/** 由x，y位置获得棋盘数组的位置*/
		int COORD_XY(int x,int y){ return x+(y<<4);};
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
		/** 棋子代号转成FEN串字符 */
		inline char piece_to_fen(int pt) {
			  return cszPieceBytes[pt];
		}
		/** 返回棋子的类型 */
		inline int PIECE_TYPE(int pc) {
			  return PieceTypes[pc];
		}
		
		/** 交换走棋方*/
		void change_side(){ now_player = ~now_player;};
		/** 引擎重置 */
		void reset();
		/** @brief 返回x，y位置上的棋子 为真实棋盘的坐标，
		 *  @param rx x 坐标
		 *  @param ry y 坐标
		 *  @return 返回棋子代号，如果0表示没有棋子
		 */
		int get_piece(int rx,int ry);
		
		//着法中的位置坐标全是在棋盘数组里的位置，0x33-0xcb以内
		/**
		 * @brief 由起点和终点生成着法
		 * @param p_src 棋子的起点
		 * @param p_dst 棋子的终点
		 * @return 返回着法，着法表示：高位是终点，低位是起点
		 */
		int move_from(int p_src,int p_dst){ return p_src + (p_dst<<8);}
		/** 得到着法的起点 */
		int get_move_src(int mv){ return mv & 255 ;}
		/** 得到着法的终点 */
		int get_move_dst(int mv){ return mv >>8 ; }
		/** 执行着法 */
		int do_move(int mv);

	private:
		/**
		 * @brief 从Fen串生成棋盘数组
		 * @param szFen Fen串
		 */
		void from_fen(const char *szFen);

	private:
		/** 当前局面的棋盘数组*/
		int chessboard[256];
		/** 当前局面的棋子数组，相应棋子的值为棋盘上的坐标,0表示被吃了
		 * 16-31表示红方棋子  32-47 表示黑方棋子
		 **/
		int chessmans[48];
		/** 用于保存所有历史局面的FEN串数组*/
		std::vector<std::string> fen_snapshots;
		/** 
		 * @brief 谁走子的信息
		 * 0 是红方先走，1是黑方先走
		 */
		bool now_player;
		/** 走棋计数*/
		int count;




};


#endif   /* ----- #ifndef ENGINE_FILE_HEADER_INC  ----- */

