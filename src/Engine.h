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
#include <map>
#include <glibmm.h>
#include <gtkmm.h>


/**
 * @brief 处理棋盘数据结构
 * 暂时来说是保存FEN串为历史局面，把当前局面保存到棋盘数组并输出
 * 此处的x,y坐标相对于棋盘数组chessboard[256]来说，而这个数组中有效位置见
 * 棋盘区域表chessInBoard[256].
 * 获取的x，y坐标是数组序号的两个十六进制位，比如0xa3,x坐标是3,y坐标是a
 * 而它的9x10棋盘坐标就是0,7
 * 
 * save the FEN array for the history station.
 * this way x,y crood is in chessboard[256],and the valid bit you can see
 * chessInBoardp256].
 */
class Engine {
	public:
		Engine();
		~Engine();

		/**
		 * @brief 初始化棋局快照，设置开始的局面
		 * initial the first board station
		 * @param fen 局面FEN串
		 */
		void init_snapshot(const char* fen);
		/**
		 * @brief 得到某步时的快照
		 *	  get the snapshot of some moves.
		 * @param num 某一步时
		 */
		void get_snapshot(int num);
		std::string get_current_snapshot(){ return current_fen_snapshots; }
		/** 添加中文表达着法 */
		/** add the chinese moves */
		void add_move_chinese(Glib::ustring f_line){ move_chinese.push_back(f_line); };
		/** 返回中文着法表达示快照集*/
		/** return the snapshot of chinese moves*/
		const std::vector<Glib::ustring>& get_move_chinese_snapshot() {return move_chinese;};
		const Glib::ustring& get_chinese_last_move(){ return move_chinese.back(); }
		
		/**
		 * @brief 棋盘数组生成Fen串 
		 *	  the board array convert to fen array
		 * @param szFen 生成的Fen串
		 */
		void to_fens(char* szFen);
		/**
		 * @brief 添加棋子信息
		 *	   add a chess
		 * @param sq 棋盘数组的序号
		 * @param pc 棋子的类型
		 */
		void add_piece(int sq, int pc);
		/**
		 * @brief FEN串转成棋子标识数字，
		 *	  FEN array convert to chess number
		 * @param nArg FEN串字母
		 * @return 返回棋子类型标识
		 */
		int fen_to_piece(int nArg);
		/** 由棋盘数组的值获取iccs坐标，纵线(x轴)为a-i,横线(y轴)为9-0*/
		/** get iccs crood from board array*/
		char get_iccs_x(int nArg);
		char get_iccs_y(int nArg);
		/** 数字转换成iccs坐标的横坐标 */
		/** digit convert to x of iccs */
		char digit_to_alpha(int nArg);
		char alpha_to_digit(int nArg);
		/** 由x，y位置获得棋盘数组中的序号（它所在的位置）*/
		int get_coord(int x,int y){ return x+(y<<4);};
		/** 由棋子代号获取9x10棋盘坐标的x，y, rev 标识是否反转棋盘*/
		void get_xy_from_chess(int f_chess,int& x,int& y,bool rev=false);
		/** 获取y坐标位*/
		inline int RANK_Y(int sq) {	  return sq >> 4;};

		/**获取x坐标位*/
		inline int RANK_X(int sq) {
			  return sq & 15;
		}
		
		inline int side_tag(int sd) {
			  int pc = 16 + (sd << 4);
			    return pc;
		}
		/** 棋子类型转成FEN串字符 */
		/** chess type convert to FEN character */
		inline char piece_to_fen(int pt) {
			  return chessman_bytes[pt];
		}
		/** 由棋子代号转成棋子的类型 */
		int get_chessman_type(int pc) {
			if(pc<16||pc>47)
				return -1;
			else
			  return PieceTypes[pc];
		}
		/** 测试位置sq是否在棋盘内 */
		/** check sq is in board */
		bool in_board(int sq) {return  chessInBoard[sq] ; }
		/** 测试位置sq是否在九宫格内 */
		/**  check sq is in fort */
		bool in_fort(int sq) { return chessInFort[sq]; }
		/** 交换走棋方*/
		/** change player go*/
		inline void change_side(){black_player = 1-black_player;}
		/** 是否红方走棋 */
		/** check  is red player going*/
		inline bool red_player() { return 1-black_player ; }
		/** 引擎重置 reset the engine */
		void reset();
		/** @brief 返回x，y位置上的棋子 为9x10棋盘的坐标，
		 *		return the chess of 9x10 by (x,y)
		 *  @param rx x 坐标
		 *  @param ry y 坐标
		 *  @param rev 标识是否反转棋盘,默认是false,true表示返回反转棋盘中的棋子
		 *  @return 返回棋子代号，如果0表示没有棋子
		 */
		int get_piece(int rx,int ry,bool rev=false);
		/** @brief 返回反转棋盘的棋子
		 * return the reverse board chess
		 * other like get_piece
		 */
		int get_rev_piece(int rx,int ry);
		/**
		 * @brief 由棋盘数组里的位置返回棋子代号
		 */
		int get_piece(int site){ return chessboard[site];}
		
		/**着法中的位置坐标全是在棋盘数组里的位置，0x33-0xcb以内*/
		/**
		 * @brief 由起点和终点生成着法
		 * @param p_src 棋子的起点
		 * @param p_dst 棋子的终点
		 * @return 返回着法，着法表示：高位是终点，低位是起点
		 */
		int get_move(int p_src,int p_dst){  return  p_src + (p_dst<<8)+ (chessboard[p_dst] <<16);}
		/** 从棋盘9x10坐标(rx,ry)返回棋盘数组里的序号,通常用于获取着法的落点 rev 表示当前反转棋盘*/
		/** get the location of chess by 9x10(rx,ry) */
		int get_dst_xy(int rx, int ry,bool rev=false);
		/** 返回棋子所在棋盘数组里的坐标(序号) */
		inline int get_chessman_xy(int f_chess){return chessmans[f_chess] ; }

		/** 得到着法的起点 */
		/** get the moves start postion*/
		int get_move_src(int mv){ return (int)mv & 255 ;}
		/** 得到着法的终点 */
		/** get the moves end postion */
		int get_move_dst(int mv){ return (int) (mv >>8)&255 ; }
		/** 把着法转成ICCS坐标格式，比如 h2e2（炮二平五)*/
		/**  convert moves to iccs format h2e2（炮二平五)*/
		uint32_t move_to_iccs(int mv);
		std::string move_to_iccs_str(int mv);
		int iccs_str_to_move(const std::string& iccs_str);
		/** 把ICCS坐标格式转成着法*/
		/** convert iccs format to moves */
		int iccs_to_move(uint32_t iccs);

		/** ICCS坐标格式转成中文纵线表达方式 */
		/** convert iccs format to hanzi format */
		uint32_t iccs_to_hanzi(uint32_t iccs);
		uint32_t hanzi_to_iccs(uint32_t hanzi);

		Glib::ustring hanzi_to_move_chinese(uint32_t hanzi);
		Glib::ustring digit_to_word(char digit);
		Glib::ustring action_to_word(char action);
		Glib::ustring code_to_word(char code);

		/** 
		 * @brief 高级着法执行,被外部调用，本身调用do_move及logic_move
		 *	  the high level do move, it call do_move and logic_move 
		 * @param mv 着法
		 * @return 真即执行着法成功，false 执行失败
		 *	   retrun true means sucess. false means fail.
		 */
		bool make_move(int mv);
		/** 无检测执行着法 */
		/** just do moves */
		int do_move(int mv);
		/** 检测着法是否合逻辑(合法）--基本着法检测*/
		/** check the moves is valid */
		bool logic_move(int mv);
		/** 将帅碰面检测*/
		/** the king meet */
		bool king_meet();
		/** 
		 * @brief 将军检测 is checked
		 * @param player true表示检测红方是否被将，false表示检测黑方是否被将
		 * @return 返回0即没被将，非0则将军，不用bool是为了以后检测多重将军作扩展*/
		int checked_by(bool player);
		/**
		 * @brief 同将军检测
		 * 默认根据black_player的值来检测
		 */
		int checked_by(void);
		/** 
		 * @brief 检测是否被将死 check is mate.
		 */
		bool mate();
		/** 
		 * @brief 撤消此着法  undo the moves.
		 * @param mv 着法,已包含了被吃子信息
		 * */
		void undo_move(int mv);
		/**
		 * @brief 获取被吃的棋子
		 *	  get the eated chessman.
		 * @param mv 着法
		 * @return 棋子代号
		 */
		int get_move_eat(int mv){ return (int)(mv >>16)&255;  }
		/** 给着法加入被吃子信息*/
		/** add the eated chessman to moves */
		int set_move_eat(int mv,int eated) { return mv + (eated <<16) ;}
		/** 由目标及起点获得绊相角的位置坐标 */
		/** get the bishop leg from target and start postion */
		int get_bishop_leg(int f_src,int f_dst)	{ return f_src + (f_dst-f_src)/2;}
		/** 绊马脚用到的比较函数*/
		/** compare the knight leg */
		int knight_cmp(int x,int y);
		/** 由目标及起点获得绊马脚的位置坐标 */
		/**  get the knight_ leg from target and start postion */
		int get_knight_leg(int f_src,int f_dst)	;
		/** 清理棋盘及棋子数组*/
		/** clean all the chessman of board */
		void clean_board();
		/** 返回棋局走了多少步*/
		/** return how moves gone */
		int how_step(){ return fen_snapshots.empty()?0:fen_snapshots.size()-1;}
		/** 获取最后一个着法的镜像，悔棋用*/
		/** get the last snapshot for rue game */
		int get_last_move_from_snapshot(){return move_snapshots.back(); }
		const std::string& get_last_fen_from_snapshot(){ return fen_snapshots.back(); }
		/** 添加着法注释*/
		void add_comment(const std::string& str);
		/** 获取某着法的注释，如果有的话*/
		std::string* get_comment(int f_step);
		/** 返回将军标识，最好是在mate()调用后使用*/
		/** return the identify of checkby */
		inline bool get_checkby(){ return m_checked;}

		/**生成合法的步法着点集合*/
		void gen_which_can_move(std::vector<Gdk::Point>& points, int chess_, bool rev);
	private:
		/**
		 * @brief 从Fen串生成棋盘数组
		 *	  Fen array to board array
		 * @param szFen Fen串
		 */
		void from_fens(const char *szFen);
		/** 同步棋盘与反转棋盘*/
		/** sync revchessboard[] to chessboard[] */
		void sync_board();

	private:
		/** 当前局面的棋盘数组 current board array*/
		int chessboard[256];
		/** 反转的棋盘数组,黑棋在下 reverse board array,the black chess down*/
		int revchessboard[256];
		/** 当前局面的棋子数组，相应棋子的值为棋盘上的坐标,0表示被吃了
		 * 16-31表示红方棋子  32-47 表示黑方棋子
		 **/
		/** current chessman array, 16-31 is red chessman, 32-47 is black chessman */
		int chessmans[48];
		/** 用于保存所有历史局面的FEN串数组*/
		/**  save the all history station FEN array */
		std::vector<std::string> fen_snapshots;
		/** save the current FEN array for run chanju game*/
		std::string current_fen_snapshots;
		/** 用于保存所有的着法 */
		/** save all history moves */
		std::vector<int> move_snapshots;
		/** 着法的中文表达式*/
		/**  save all chinese moves */
		std::vector<Glib::ustring> move_chinese;
		/** 着法的注释*/
		/**  save all comment of moves */
		std::map<int,std::string> move_comment;
		/** 
		 * @brief 谁走子的信息
		 * 0 是红方走，1是黑方走
		 */
		bool black_player;
		bool m_checked;




};


#endif   /* ----- #ifndef ENGINE_FILE_HEADER_INC  ----- */

