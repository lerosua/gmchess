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
#include <stdlib.h>
Engine::Engine():count(0),black_player(0)
{
	int i,j;
	for(i=0;i<16;i++)
		for(j=0;j<16;j++)
			chessboard[i*16+j] =0;

	for(i=0;i<3;i++)
		for(j=0;j<16;j++)
			chessmans[i*16 + j]=0;
}

Engine::~Engine()
{}

void Engine::reset()
{
	int i,j;
	for(i=0;i<16;i++)
		for(j=0;j<16;j++)
			chessboard[i*16+j] =0;

	for(i=0;i<3;i++)
		for(j=0;j<16;j++)
			chessmans[i*16 + j]=0;


	black_player = 0;
	count=0;
	fen_snapshots.clear();
	move_snapshots.clear();
}

void Engine::add_piece(int sq,int pc)
{
	chessboard[sq]=pc;
	chessmans[pc]=sq;
}

void Engine::from_fens(const char *szFen) {
  int i, j, k;
  int pcRed[7];
  int pcBlack[7];
  const char *lpFen;
  // FEN串的识别包括以下几个步骤：
  // 1. 初始化，清空棋盘
  pcRed[0] = SIDE_TAG(0) + KING_FROM;
  pcRed[1] = SIDE_TAG(0) + ADVISOR_FROM;
  pcRed[2] = SIDE_TAG(0) + BISHOP_FROM;
  pcRed[3] = SIDE_TAG(0) + KNIGHT_FROM;
  pcRed[4] = SIDE_TAG(0) + ROOK_FROM;
  pcRed[5] = SIDE_TAG(0) + CANNON_FROM;
  pcRed[6] = SIDE_TAG(0) + PAWN_FROM;
  for (i = 0; i < 7; i ++) {
    pcBlack[i] = pcRed[i] + 16;
  }
  /* 数组"pcRed[7]"和"pcBlack[7]"分别代表红方和黑方每个兵种即将占有的序号，
   * 以"pcRed[7]"为例，由于棋子16到31依次代表“帅仕仕相相马马车车炮炮兵兵兵兵兵”，
   * 所以最初应该是"pcRed[7] = {16, 17, 19, 21, 23, 25, 27}"，每添加一个棋子，该项就增加1，
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
          if (pcRed[k] < 32) {
            //if (this->ucsqchessmans[pcRed[k]] == 0) {
              add_piece(get_coord(j, i), pcRed[k]);
              pcRed[k] ++;
            //}
          }
        }
        j ++;
      }
    } else if (*lpFen >= 'a' && *lpFen <= 'z') {
      if (j <= FILE_RIGHT) {
        k = fen_to_piece(*lpFen);
        if ((k>6)&&(k < 14)) {
          if (pcBlack[k-7] < 48) {
		  //printf("pcBlack[%d-7] = %d \n",k,pcBlack[k-7]);
            if (chessmans[pcBlack[k-7]] == 0) {
              add_piece(get_coord(j, i), pcBlack[k-7]);
              pcBlack[k-7] ++;
            }
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
  //if (this->black_player == (*lpFen == 'b' ? 0 : 1)) {
  if (this->black_player == (*lpFen == 'b' ? 1 : 0)) {
    change_side();
  }
//  // 4. 把局面设成“不可逆”
//  SetIrrev();
}

void Engine::to_fens(char *szFen)  {
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
  *lpFen = (this->black_player == 0 ? 'w' : 'b');
  lpFen ++;
  *lpFen = '\0';
}

void Engine::init_snapshot(const char* fen)
{
	from_fens(fen);
	fen_snapshots.push_back(std::string(fen));
	move_snapshots.push_back(0);



	int i,j;
	for(i=0;i<16;i++)
	{
		for(j=0;j<16;j++)
			printf(" %2d ",chessboard[i*16+j]);
		printf("\n");
	}
}

void Engine::get_snapshot(int num)
{
	std::string fens = fen_snapshots[num];
	from_fens(fens.c_str());
	count = num;

}
int Engine::get_piece(int rx,int ry)
{
	int site=0;
	site = get_coord(rx + 3,ry + 3);
	return chessboard[site];

}
int Engine::get_dst_xy(int rx, int ry)
{
	return get_coord(rx+3,ry+3);
}

char Engine::get_iccs_y(int nArg)
{
	int tmp = (nArg&240)>>4;
	switch(tmp){
	case 3:
		return '9';
	case 4:
		return '8';
	case 5:
		return '7';
	case 6:
		return '6';
	case 7:
		return '5';
	case 8:
		return '4';
	case 9:
		return '3';
	case 10:
		return '2';
	case 11:
		return '1';
	case 12:
		return '0';
	default:
		return -1;
		};
}
char Engine::get_iccs_x(int nArg)
{
	return (nArg&15+94);
}

char Engine::digit_to_alpha(int nArg)
{
	switch(nArg){
		case '1':
			return 'i';
		case '2':
			return 'h';
		case '3':
			return 'g';
		case '4':
			return 'f';
		case '5':
			return 'e';
		case '6':
			return 'd';
		case '7':
			return 'c';
		case '8':
			return 'b';
		case '9':
			return 'a';
		default:
			return -1;
	}
}

/** 
 * FEN串中棋子类型标识  */
int Engine::fen_to_piece(int nArg) 
{
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
 * @return 0表示执行，-1表示执行失败
 */
int Engine::do_move(int mv)
{
	int i,j;
	/*
	for(i=0;i<16;i++)
	{
		for(j=0;j<16;j++)
			printf(" %2d ",chessboard[i*16+j]);
		printf("\n");
	}
	*/

	int src = get_move_src(mv);
	int dst = get_move_dst(mv);
	int eated = get_move_eat(mv);
	printf(" src = %x dst = %x mv = %d\n chessboard[src]= %d , chessboard[dst] = %d,eated = %d\n",src,dst,mv,chessboard[src],chessboard[dst],eated);

	if(eated != chessboard[dst])
		return -1;
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

	/** 着法添加进快照 */
	move_snapshots.push_back(mv);
	char str_fen[128];
	to_fens(str_fen);
	fen_snapshots.push_back(std::string(str_fen));

		printf("\n ==after do move ==\n");
	for(i=0;i<16;i++)
	{
		for(j=0;j<16;j++)
			printf(" %2d ",chessboard[i*16+j]);
		printf("\n");
	}
	
	return 0;
}

void Engine::undo_move(int mv)
{
	int src = get_move_src(mv);
	int dst = get_move_dst(mv);
	int eated = get_move_eat(mv);

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



uint32_t Engine::iccs_to_hanzi(uint32_t iccs)
{
	int pos;
	uint16_t *arg;
	char *p;
	p=(char*) &iccs;




}

uint32_t Engine::hanzi_to_iccs(uint32_t f_hanzi)
{
	/** hazi中文纵线表示方式中，hanzi的四个字符依次是: 炮二平五(C2.5) */

	union Hanzi c_hanzi;
	union Hanzi c_iccs;

	c_hanzi.digit = f_hanzi;
	/** cman_type 是棋子类型*/
	int cman_type = fen_to_piece(c_hanzi.word[0]);
	int src_x,src_y,dst_x,dst_y;
	int num;

	if(black_player)
		num = 16;
	else
		num= 0;

	if(0 == cman_type ){
		/** 处理帅将的源地址*/
		src_x = get_iccs_x(chessmans[16+num]);  
		src_y = get_iccs_y(chessmans[16+num]);  
		c_iccs.word[0] = src_x;
		c_iccs.word[1] = src_y;
	}
	else if(1 == cman_type){
		/** 处理士的源及目标地址*/
		/** 士与相没有前后之分,能进则是后面那个，能退则是前面那个*/
		src_x = digit_to_alpha(c_hanzi.word[1]);
		c_iccs.word[0] = src_x;

		dst_x = digit_to_alpha(c_hanzi.word[3]);
		int a1_x = get_iccs_x(chessmans[17+num]);
		int a2_x = get_iccs_x(chessmans[18+num]);
		if(a1_x == a2_x){
			/** 士在同一纵线上*/
			int a1_y = get_iccs_y(chessmans[17+num]);
			int a2_y = get_iccs_y(chessmans[18+num]);
			
			if(c_hanzi.word[2] == '+'){
				src_y = a1_y<a2_y?a1_y:a2_y;
				dst_y = src_y+1;

			}
			else if(c_hanzi.word[2] == '-'){
				src_y = a1_y>a2_y?a1_y:a2_y;
				dst_y = src_y-1;
			}

			c_iccs.word[1] =src_y;
			c_iccs.word[2] =dst_x;
			c_iccs.word[3] =dst_y;

		}
		else{
			if(a1_x == src_x){
				src_y = get_iccs_y(chessmans[17+num]);
			}
			else if(a2_x == src_x){
				src_y = get_iccs_y(chessmans[18+num]);
			}
			else{
				DLOG("士的匹配位置出现问题\n");
				goto error_out;
			}

			if(c_hanzi.word[2] == '+'){
				dst_y = src_y+1;
			}
			else if(c_hanzi.word[2] == '-'){
				dst_y = src_y-1;
			}
			c_iccs.word[1]=src_y;
			c_iccs.word[2] =dst_x;
			c_iccs.word[3] =dst_y;

		}
			
		return c_iccs.digit;
	}
	else if(2 == cman_type){
		/** 相/象的处理*/
		src_x = digit_to_alpha(c_hanzi.word[1]);
		c_iccs.word[0] = src_x;

		dst_x = digit_to_alpha(c_hanzi.word[3]);
		int a1_x = get_iccs_x(chessmans[19+num]);
		int a2_x = get_iccs_x(chessmans[20+num]);
		if(a1_x == a2_x){
			/** 象在同一纵线上*/
			int a1_y = get_iccs_y(chessmans[19+num]);
			int a2_y = get_iccs_y(chessmans[20+num]);
			
			if(c_hanzi.word[2] == '+'){
				src_y = a1_y<a2_y?a1_y:a2_y;
				dst_y = src_y+2;

			}
			else if(c_hanzi.word[2] == '-'){
				src_y = a1_y>a2_y?a1_y:a2_y;
				dst_y = src_y-2;
			}

			c_iccs.word[1] =src_y;
			c_iccs.word[2] =dst_x;
			c_iccs.word[3] =dst_y;

		}
		else{
			if(a1_x == src_x){
				src_y = get_iccs_y(chessmans[19+num]);
			}
			else if(a2_x == src_x){
				src_y = get_iccs_y(chessmans[20+num]);
			}
			else{
				DLOG("象的匹配位置出现问题\n");
				goto error_out;
			}

			if(c_hanzi.word[2] == '+'){
				dst_y = src_y+2;
			}
			else if(c_hanzi.word[2] == '-'){
				dst_y = src_y-2;
			}
			c_iccs.word[1]=src_y;
			c_iccs.word[2] =dst_x;
			c_iccs.word[3] =dst_y;

		}


		return c_iccs.digit;
	}
	else if(3 == cman_type){
		/** 马的处理*/
		src_x = digit_to_alpha(c_hanzi.word[1]);
		//c_iccs.word[0] = src_x;

		dst_x = digit_to_alpha(c_hanzi.word[3]);

		int a1_x = get_iccs_x(chessmans[19+num]);
		int a2_x = get_iccs_x(chessmans[20+num]);
		if(src_x<0){
			/** 同一纵线上有两匹马 */
			int a1_y = get_iccs_y(chessmans[21+num]);
			int a2_y = get_iccs_y(chessmans[22+num]);
			if(c_hanzi.word[1] == 'a')
				src_y = a1_y>a2_y?a1_y:a2_y;
			else
				src_y = a1_y<a2_y?a1_y:a2_y;
			if(c_hanzi.word[2] == '+'){
				if(abs(dst_x-src_x)== 1)
					dst_y = src_y +2;
				else
					dst_y = src_y +1;
			}

			c_iccs.word[0] = a1_x;
			c_iccs.word[1] =src_y;
			c_iccs.word[2] =dst_x;
			c_iccs.word[3] =dst_y;
		}
		else{
			if(a1_x == src_x){
				src_y = get_iccs_y(chessmans[21+num]);
			}
			else if(a2_x == src_x){
				src_y = get_iccs_y(chessmans[22+num]);
			}
			else{
				DLOG("马的匹配位置出现问题\n");
				goto error_out;
			}

			if(c_hanzi.word[2] == '+'){
				if(abs(dst_x-src_x)== 1)
					dst_y = src_y +2;
				else
					dst_y = src_y +1;
			}
			else if(c_hanzi.word[2] == '-'){
				if(abs(dst_x-src_x)== 1)
					dst_y = src_y -2;
				else
					dst_y = src_y -1;
			}
			c_iccs.word[0]=src_x;
			c_iccs.word[1]=src_y;
			c_iccs.word[2] =dst_x;
			c_iccs.word[3] =dst_y;


		}

		return c_iccs.digit;
	}
	else if(4==cman_type||5==cman_type){
		/** 车跑的目标坐标生成*/

		src_x = digit_to_alpha(c_hanzi.word[1]);

		int a1_x = get_iccs_x(chessmans[19+num]);
		int a2_x = get_iccs_x(chessmans[20+num]);
		if(src_x<0){
			/** 同一纵线上有两个车或跑 */
			int a1_y = get_iccs_y(chessmans[21+num]);
			int a2_y = get_iccs_y(chessmans[22+num]);
			if(c_hanzi.word[1] == 'a'){
				src_y = a1_y>a2_y?a1_y:a2_y;
			}
			else{
				src_y = a1_y>a2_y?a1_y:a2_y;
			}
			src_x= a1_x;
		}
		else {
			int type_num;
			if(cman_type==4)
				type_num = 23;
			else if(5==cman_type)
				type_num=25;

			if(a1_x == src_x){
				src_y = get_iccs_y(chessmans[type_num+num]);
			}
			else if(a2_x == src_x){
				src_y = get_iccs_y(chessmans[type_num+1+num]);
			}
			else{
				DLOG("车炮的匹配位置出现问题\n");
				goto error_out;
			}
		}

		if(c_hanzi.word[2] == '+'){
			char c= c_hanzi.word[3];
			dst_x = src_x;
			dst_y = src_y + atoi(&c);
		}
		else if(c_hanzi.word[2] == '-'){
			char c= c_hanzi.word[3];
			dst_x = src_x;
			dst_y = src_y - atoi(&c);
		}
		else if(c_hanzi.word[2] == '.'){
			dst_x = digit_to_alpha(c_hanzi.word[3]);
			dst_y = src_y;

		}
		c_iccs.word[0]=src_x;
		c_iccs.word[1]=src_y;
		c_iccs.word[2] =dst_x;
		c_iccs.word[3] =dst_y;

		return c_iccs.digit;
	}
	else if(6==cman_type){

		src_x = digit_to_alpha(c_hanzi.word[1]);

		int a1_x = get_iccs_x(chessmans[27+num]);
		int a2_x = get_iccs_x(chessmans[28+num]);
		int a3_x = get_iccs_x(chessmans[29+num]);
		int a4_x = get_iccs_x(chessmans[30+num]);
		int a5_x = get_iccs_x(chessmans[31+num]);
		if(src_x<0){
			/** 处理纵线上多兵的问题，未解决*/
			if(c_hanzi.word[1] == 'a'){


			}
			else if(c_hanzi.word[1] == 'b'){


			}
			else if(c_hanzi.word[1] == 'c'){
			}
			else if(c_hanzi.word[1] == 'd'){
			}
			else if(c_hanzi.word[1] == 'e'){
			}


		}
		else{
			if(a1_x == src_x)
				src_y = get_iccs_y(chessmans[27+num]);
			else if(a2_x == src_x)
				src_y = get_iccs_y(chessmans[28+num]);
			else if(a3_x == src_x)
				src_y = get_iccs_y(chessmans[29+num]);
			else if(a4_x == src_x)
				src_y = get_iccs_y(chessmans[30+num]);
			else if(a5_x == src_x)
				src_y = get_iccs_y(chessmans[31+num]);

		}

		if(c_hanzi.word[2] == '+'){
			char c= c_hanzi.word[3];
			dst_x = src_x;
			dst_y = src_y + atoi(&c);
		}
		else if(c_hanzi.word[2] == '-'){
			char c= c_hanzi.word[3];
			dst_x = src_x;
			dst_y = src_y - atoi(&c);
		}
		else if(c_hanzi.word[2] == '.'){
			dst_x = digit_to_alpha(c_hanzi.word[3]);
			dst_y = src_y;

		}


		c_iccs.word[0]=src_x;
		c_iccs.word[1]=src_y;
		c_iccs.word[2] =dst_x;
		c_iccs.word[3] =dst_y;

		return c_iccs.digit;
	}
error_out:
	printf(" %s error\n",__FUNCTION__);
	return 0;
}

#if 0
uint32_t Engine::hanzi_to_iccs(uint32_t f_hanzi)
{

	union Hanzi c_hanzi;
	union Hanzi c_iccs;

	c_hanzi.digit = f_hanzi;
	/** cman 是棋子类型*/
	int cman = fen_to_piece(c_hanzi.word[0]);
	int pos_t;
	if(c_hanzi.word[1]>96){
	/** 这里处理前中后的棋子关系,比如前兵进一中的“前兵” */

		pos_t = c_hanzi.word[1];
	}
	else if(cman ==1||cman==2)
	{
		/** 士相的前后位置也要另外处理*/
		/** 仕(士)和相(象)如果在同一纵线上，不用“前”和“后”区别，
		  * 因为能退的一定在前，能进的一定在后*/


	}
	else{
		/**处理一般的棋子位置，比如炮二平五，中的"炮二"*/
		pos_t = get_iccs_x(c_hanzi.word[1]);
		c_iccs.word[0]=pos_t;

		/** 跟着确定棋子的纵坐标*/
		/** 一般棋子都有两个以上，要逐个查找在此横坐标上的棋子,
		 * 还好有棋子数组，它里面保存着棋子在棋盘上的坐标,现在
		 * 问题就是从棋子类型和一个横坐标得到它的纵坐标   */

		
		/* 以下是方便从棋子类型取棋子代号*/
		  int i, j, k;
		  int pcRed[7];
		  int pcBlack[7];
		  pcRed[0] = SIDE_TAG(0) + KING_FROM;
		  pcRed[1] = SIDE_TAG(0) + ADVISOR_FROM;
		  pcRed[2] = SIDE_TAG(0) + BISHOP_FROM;
		  pcRed[3] = SIDE_TAG(0) + KNIGHT_FROM;
		  pcRed[4] = SIDE_TAG(0) + ROOK_FROM;
		  pcRed[5] = SIDE_TAG(0) + CANNON_FROM;
		  pcRed[6] = SIDE_TAG(0) + PAWN_FROM;
		  for (i = 0; i < 7; i ++) {
			    pcBlack[i] = pcRed[i] + 16;
		  }

		  int man_start;
		if(black_player){
			/** 黑方处理*/
			man_start = pcBlack[cman];
			}
		else{
			/** 红方处理*/
			man_start= pcRed[cman];
		}	
			while(man_start<48){
				int tmp = chessmans[man_start];
				if(cman<7){
					if(man_start>tmp);
						goto error_out;
				}
						

				if( ((tmp&15)+94) == pos_t){
					/**终于匹配了*/
					int tmp_y=((tmp&240)>>4);
					int result_y;
					switch(tmp_y){
						case 3:
							return '9';
							break;
						case 4:
							return '8';
							break;
						case 5:
							return '7';
							break;
						case 6:
							return '6';
							break;
						case 7:
							return '5';
							break;
						case 8:
							return '4';
							break;
						case 9:
							return '3';
							break;
						case 10:
							return '2';
							break;
						case 11:
							return '1';
							break;
						case 12:
							return '0';
							break;
						default:
							return -1;
							break;
					};
					if(result_y<0){
						/**错误处理*/
						return -1;
					}
					c_iccs.word[1]=result_y;

					}
					man_start++;

			}
			

	}

	/** 生成车，跑，兵，帅，的直线走法的目标*/
	if(cman == 0 || cman == 4||cman == 5||cman == 6){

	if(c_hanzi.word[2] == '.'){
		c_iccs.word[3] = c_iccs.word[1];
		c_iccs.word[2] = get_iccs_x(c_hanzi.word[3]);
		
	}
	else if(c_hanzi.word[2] == '+'){
		c_iccs.word[2]=c_iccs.word[0];
		char c = get_iccs_x(c_hanzi.word[3]);
		int num = atoi(&c);
		c_iccs.word[3]= c_iccs.word[1] +num;

	}
	else if(c_hanzi.word[2] == '-'){
		c_iccs.word[2]=c_iccs.word[0];
		char c = get_iccs_x(c_hanzi.word[3]);
		int num = atoi(&c);
		c_iccs.word[3]= c_iccs.word[1] -num;

	}
		else{
			goto error_out;
		}
	}
	else if(cman ==1){
		/** 士的走法目标*/

	}
	else if(cman == 2){
		/** 相的走法目标*/

	}
	else if(cman == 3){
		/** 马的走法目标*/


	}



	/** 前面已经把中文表示转化成符号表示，比如炮二平五, C2.5 */

	return c_iccs.digit;


error_out:
	printf(" %s error\n",__FUNCTION__);
	return 0;



}

#endif
