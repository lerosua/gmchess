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


/**
 * @brief 处理棋盘数据结构
 * 暂时来说是保存FEN串为历史局面，把当前局面保存到棋盘数组并输出
 * 此处的x,y坐标相对于棋盘数组chessboard[256]来说，而这个数组中有效位置见
 * 棋盘区域表chessInBoard[256].
 * 获取的x，y坐标是数组序号的两个十六进制位，比如0xa3,x坐标是3,y坐标是a
 * 而它的9x10棋盘坐标就是0,7
 * 
 */
class Engine {
	public:
		Engine();
		~Engine();

		/**
		 * @brief 初始化棋局快照，设置开始的局面
		 * @param fen 局面FEN串
		 */
		void init_snapshot(const char* fen);
		/**
		 * @brief 得到某步时的快照
		 * @param num 某一步时
		 */
		void get_snapshot(int num);
		/** 添加中文表达着法 */
		void add_move_chinese(Glib::ustring f_line){ move_chinese.push_back(f_line); };
		/** 返回中文着法表达示快照集*/
		const std::vector<Glib::ustring>& get_move_chinese_snapshot() {return move_chinese;};
		const Glib::ustring& get_chinese_last_move(){ return move_chinese.back(); }
		
		/**
		 * @brief 棋盘数组生成Fen串 
		 * @param szFen 生成的Fen串
		 */
		void to_fens(char* szFen);
		/**
		 * @brief 添加棋子信息
		 * @param sq 棋盘数组的序号
		 * @param pc 棋子的类型
		 */
		void add_piece(int sq, int pc);
		/**
		 * @brief FEN串转成棋子标识数字，
		 * @param nArg FEN串字母
		 * @return 返回棋子类型标识
		 */
		int fen_to_piece(int nArg);
		/** 由棋盘数组的值获取iccs坐标，纵线(x轴)为a-i,横线(y轴)为9-0*/
		char get_iccs_x(int nArg);
		char get_iccs_y(int nArg);
		/** 数字转换成iccs坐标的横坐标 */
		char digit_to_alpha(int nArg);
		char alpha_to_digit(int nArg);
		/** 由x，y位置获得棋盘数组中的序号（它所在的位置）*/
		int get_coord(int x,int y){ return x+(y<<4);};
		/** 由棋子代号获取9x10棋盘坐标的x，y*/
		void get_xy_from_chess(int f_chess,int& x,int& y);
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
		bool in_board(int sq) {return  chessInBoard[sq] ; }
		/** 测试位置sq是否在九宫格内 */
		bool in_fort(int sq) { return chessInFort[sq]; }
		/** 交换走棋方*/
		void change_side(){black_player = 1-black_player;};
		/** 引擎重置 */
		void reset();
		/** @brief 返回x，y位置上的棋子 为8x9棋盘的坐标，
		 *  @param rx x 坐标
		 *  @param ry y 坐标
		 *  @return 返回棋子代号，如果0表示没有棋子
		 */
		int get_piece(int rx,int ry);
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
		/** 从棋盘9x10坐标(rx,ry)返回棋盘数组里的序号,通常用于获取着法的落点 */
		int get_dst_xy(int rx, int ry);
		/** 返回棋子所在棋盘数组里的坐标(序号) */
		inline int get_chessman_xy(int f_chess){return chessmans[f_chess] ; }

		/** 得到着法的起点 */
		int get_move_src(int mv){ return (int)mv & 255 ;}
		/** 得到着法的终点 */
		int get_move_dst(int mv){ return (int) (mv >>8)&255 ; }
		/** 把着法转成ICCS坐标格式，比如 h2e2（炮二平五)*/
		uint32_t move_to_iccs(int mv);
		std::string move_to_iccs_str(int mv);
		int iccs_str_to_move(const std::string& iccs_str);
		/** 把ICCS坐标格式转成着法*/
		int iccs_to_move(uint32_t iccs);

		/** ICCS坐标格式转成中文纵线表达方式 */
		uint32_t iccs_to_hanzi(uint32_t iccs);
		uint32_t hanzi_to_iccs(uint32_t hanzi);

		Glib::ustring hanzi_to_move_chinese(uint32_t hanzi);
		Glib::ustring digit_to_word(char digit);
		Glib::ustring action_to_word(char action);
		Glib::ustring code_to_word(char code);

		/** 执行着法 */
		int do_move(int mv);
		/** 检测着法是否合逻辑(合法）*/
		bool logic_move(int mv);
		/** 
		 * @brief 撤消此着法 
		 * @param mv 着法,已包含了被吃子信息
		 * */
		void undo_move(int mv);
		/**
		 * @brief 获取被吃的棋子
		 * @param mv 着法
		 * @return 棋子代号
		 */
		int get_move_eat(int mv){ return (int)(mv >>16)&255;  }
		/** 给着法加入被吃子信息*/
		int set_move_eat(int mv,int eated) { return mv + (eated <<16) ;}
		/** 由目标及起点获得绊相角的位置坐标 */
		int get_bishop_leg(int f_src,int f_dst)	{ return f_src + (f_dst-f_src)/2;}
		/** 绊马脚用到的比较函数*/
		int knight_cmp(int x,int y);
		/** 由目标及起点获得绊马脚的位置坐标 */
		int get_knight_leg(int f_src,int f_dst)	;
		/** 清理棋盘及棋子数组*/
		void clean_board();
		/** 返回棋局走了多少步*/
		int how_step(){ return fen_snapshots.empty()?0:fen_snapshots.size()-1;}
		/** 获取最后一个着法的镜像，悔棋用*/
		int get_last_move_from_snapshot(){return move_snapshots.back(); }
		const std::string& get_last_fen_from_snapshot(){ return fen_snapshots.back(); }
		/** 添加着法注释*/
		void add_comment(const std::string& str);
		/** 获取某着法的注释，如果有的话*/
		std::string* get_comment(int f_step);

	private:
		/**
		 * @brief 从Fen串生成棋盘数组
		 * @param szFen Fen串
		 */
		void from_fens(const char *szFen);

	private:
		/** 当前局面的棋盘数组*/
		int chessboard[256];
		/** 当前局面的棋子数组，相应棋子的值为棋盘上的坐标,0表示被吃了
		 * 16-31表示红方棋子  32-47 表示黑方棋子
		 **/
		int chessmans[48];
		/** 用于保存所有历史局面的FEN串数组*/
		std::vector<std::string> fen_snapshots;
		/** 用于保存所有的着法 */
		std::vector<int> move_snapshots;
		/** 着法的中文表达式*/
		std::vector<Glib::ustring> move_chinese;
		/** 着法的注释*/
		std::map<int,std::string> move_comment;
		/** 
		 * @brief 谁走子的信息
		 * 0 是红方先走，1是黑方先走
		 */
		bool black_player;




};


#endif   /* ----- #ifndef ENGINE_FILE_HEADER_INC  ----- */

