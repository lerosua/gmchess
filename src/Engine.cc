/*
 * =====================================================================================
 *
 *       Filename:  Engine.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2009年02月14日 16时58分36秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 *
 * =====================================================================================
 */

#include "Engine.h"
#include <string.h>
Engine::Engine()
{
	memset(chessboard,0,256);
	memset(chessmans,0,48);
	now_player = 0;
	count=0;


}

Engine::~Engine()
{}

void Engine::reset()
{
	memset(chessboard,0,256);
	memset(chessmans,0,48);
	now_player = 0;
	count=0;
	fen_snapshots.clear();
}

void Engine::add_piece(int sq,int pc)
{
	chessboard[sq]=pc;
	chessmans[pc]=sq;
}

void Engine::from_fen(const char *szFen) {
  int i, j, k;
  int pcWhite[7];
  int pcBlack[7];
  const char *lpFen;
  // FEN串的识别包括以下几个步骤：
  // 1. 初始化，清空棋盘
  pcWhite[0] = SIDE_TAG(0) + KING_FROM;
  pcWhite[1] = SIDE_TAG(0) + ADVISOR_FROM;
  pcWhite[2] = SIDE_TAG(0) + BISHOP_FROM;
  pcWhite[3] = SIDE_TAG(0) + KNIGHT_FROM;
  pcWhite[4] = SIDE_TAG(0) + ROOK_FROM;
  pcWhite[5] = SIDE_TAG(0) + CANNON_FROM;
  pcWhite[6] = SIDE_TAG(0) + PAWN_FROM;
  for (i = 0; i < 7; i ++) {
    pcBlack[i] = pcWhite[i] + 16;
  }
  /* 数组"pcWhite[7]"和"pcBlack[7]"分别代表红方和黑方每个兵种即将占有的序号，
   * 以"pcWhite[7]"为例，由于棋子16到31依次代表“帅仕仕相相马马车车炮炮兵兵兵兵兵”，
   * 所以最初应该是"pcWhite[7] = {16, 17, 19, 21, 23, 25, 27}"，每添加一个棋子，该项就增加1，
   * 这种做法允许添加多余的棋子(例如添加第二个帅，就变成仕了)，但添加前要做边界检测
   */
//  ClearBoard();
  lpFen = szFen;
  if (*lpFen == '\0') {
 //   SetIrrev();
    return;
  }
  // 2. 读取棋盘上的棋子
  i = RANK_TOP;
  j = FILE_LEFT;
  while (*lpFen != ' ') {
    if (*lpFen == '/') {
      j = FILE_LEFT;
      i ++;
      if (i > RANK_BOTTOM) {
        break;
      }
    } else if (*lpFen >= '1' && *lpFen <= '9') {
      for (k = 0; k < (*lpFen - '0'); k ++) {
        if (j >= FILE_RIGHT) {
          break;
        }
        j ++;
      }
    } else if (*lpFen >= 'A' && *lpFen <= 'Z') {
      if (j <= FILE_RIGHT) {
        k = fen_to_piece(*lpFen);
        if (k < 7) {
          if (pcWhite[k] < 32) {
            //if (this->ucsqchessmans[pcWhite[k]] == 0) {
              add_piece(get_coord(j, i), pcWhite[k]);
              pcWhite[k] ++;
            //}
          }
        }
        j ++;
      }
    } else if (*lpFen >= 'a' && *lpFen <= 'z') {
      if (j <= FILE_RIGHT) {
        k = fen_to_piece(*lpFen);
        if (6<k < 14) {
          if (pcBlack[k-7] < 48) {
            //if (this->ucsqchessmans[pcBlack[k]] == 0) {
              add_piece(get_coord(j, i), pcBlack[k-7]);
              pcBlack[k-7] ++;
            //}
          }
        }
        j ++;
      }
    }
    lpFen ++;
    if (*lpFen == '\0') {
      //SetIrrev();
      return;
    }
  }
  lpFen ++;
  // 3. 确定轮到哪方走
  if (this->now_player == (*lpFen == 'b' ? 0 : 1)) {
    change_side();
  }
//  // 4. 把局面设成“不可逆”
//  SetIrrev();
}

void Engine::to_fen(char *szFen)  {
  int i, j, k, pc;
  char *lpFen;

  lpFen = szFen;
  for (i = RANK_TOP; i <= RANK_BOTTOM; i ++) {
    k = 0;
    for (j = FILE_LEFT; j <= FILE_RIGHT; j ++) {
      pc = chessboard[get_coord(j, i)];
      if (pc != 0) {
        if (k > 0) {
          *lpFen = k + '0';
          lpFen ++;
          k = 0;
        }
        *lpFen = piece_to_fen(get_chessman_type(pc));
        lpFen ++;
      } else {
        k ++;
      }
    }
    if (k > 0) {
      *lpFen = k + '0';
      lpFen ++;
    }
    *lpFen = '/';
    lpFen ++;
  }
  *(lpFen - 1) = ' '; // 把最后一个'/'替换成' '
  *lpFen = (this->now_player == 0 ? 'w' : 'b');
  lpFen ++;
  *lpFen = '\0';
}

void Engine::add_snapshot(const char* fen)
{
	from_fen(fen);
	fen_snapshots.push_back(std::string(fen));
}

void Engine::get_snapshot(int num)
{

}
int Engine::get_piece(int rx,int ry)
{
	int site=0;
	site = get_coord(rx + 3,ry + 3);
	return chessboard[site];

}


/** 
 * FEN串中棋子标识  */
int Engine::fen_to_piece(int nArg) {
  switch (nArg) {
  case 'K':
    return 0;
  case 'G':
  case 'A':
    return 1;
  case 'B':
  case 'E':
    return 2;
  case 'N':
  case 'H':
    return 3;
  case 'R':
    return 4;
  case 'C':
    return 5;
  case 'P':
    return 6;
    case 'k':
        return 7;
    case 'g':
    case 'a':
        return 8;
    case 'b':
    case 'e':
        return 9;
    case 'h':
    case 'n':
        return 10;
    case 'r':
        return 11;
    case 'c':
        return 12;
    case 'p':
        return 13;
  default:
    return 14;
  }
}

/**
 * @brief 
 * 执行了mv着法，将生成的棋盘数组转成FEN串添加到FEN快照里
 * 界面棋盘将根据棋盘数组更新
 * 另外生成着法的中文字符/ICCS 表示
 * @param mv 着法
 * @return 被吃的棋子
 */
int Engine::do_move(int mv)
{
	int src = get_move_src(mv);
	int dst = get_move_dst(mv);
	int eated = chessboard[dst];

	/**
	 * 如果dst的位置上有棋子，即是出现被吃子现象，
	 * 则要将这个棋子的位置值置0,表示被吃掉，不再出现在棋盘上
	 */
	if(eated !=0){
		DLOG(" %d has been eat\n",chessboard[dst]);
		chessmans[eated]=0;
	}
	chessboard[dst] = chessboard[src];
	chessboard[src] = 0;
	/** 被移动的棋子的位置值变成目标地点 */
	chessmans[chessboard[dst]] = dst ;

	/** 交换走子方 */
	change_side();

	return eated;
}

void Engine::undo_move(int mv , int eated)
{
	int src = get_move_src(mv);
	int dst = get_move_dst(mv);

	chessboard[src] = chessboard[dst];
	chessboard[dst] = eated;

	chessmans[chessboard[src]]=src;
	chessmans[eated]=dst ;

	/** 交换走子方 */
	change_side();



}
uint32_t Engine::move_to_iccs(int mv)
{
	union{
		char c[4];
		uint32_t dw;
	}Ret;
	Ret.c[0] = RANK_X(get_move_src(mv)) - FILE_LEFT +'a';
	Ret.c[1] = '9' - RANK_Y(get_move_src(mv)) - RANK_TOP;
	Ret.c[2] = RANK_X(get_move_dst(mv)) - FILE_LEFT + 'a';
	Ret.c[3] = '0' - RANK_Y(get_move_dst(mv)) - RANK_TOP;

	return Ret.dw;

}
int Engine::iccs_to_move(uint32_t iccs)
{
	int src,dst;
	char *p;
	p = (char*)&iccs;
	src = get_coord( p[0] - 'a' + FILE_LEFT, '9'-p[1] +RANK_TOP);
	dst = get_coord( p[2] - 'a' + FILE_LEFT, '9'-p[3] +RANK_TOP);

	return (in_board(src) && in_board(dst) ? get_move(src,dst):0);
}



uint64_t Engine::iccs_to_hanzi(uint32_t iccs)
{
	int pos;
	uint16_t *arg;




}

uint32_t Engine::hanzi_to_iccs(uint64_t hanzi)
{





}
