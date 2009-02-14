/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2009年02月14日 00时25分44秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 *
 * =====================================================================================
 */

#include <stdio.h>


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
//const char *const cszPieceBytes = "KABNRCP";
const char *const cszPieceBytes = "KABNRCPkabnrcp";

const int cnPieceTypes[48] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 6,
  7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 13, 13, 13
  //0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 6
};

// 判断棋子是否在棋盘中的数组
static const char ccInBoard[256] = {
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

static int Square[256]={0};
static int Pieces[48]={0};

// FEN串中棋子标识，注意这个函数只能识别大写字母，因此用小写字母时，首先必须转换为大写
int FenPiece(int nArg) {
  switch (nArg) {
  case 'K':
    return 0;
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
  default:
    return 7;
  }
}
//由x，y位置获得棋盘数组的位置
inline int COORD_XY(int x,int y){ return x+(y<<4);};
//获取y坐标
inline int RANK_Y(int sq) {
	  return sq >> 4;
}
//获取x坐标
inline int FILE_X(int sq) {
	  return sq & 15;
}

inline int SIDE_TAG(int sd) {
	  int pc = 16 + (sd << 4);
	    return pc;
}
inline char PIECE_BYTE(int pt) {
	  return cszPieceBytes[pt];
}
inline int PIECE_TYPE(int pc) {
	  return cnPieceTypes[pc];
}



//sq是棋盘数组的位置，pc是棋子种类
void AddPiece(int sq,int pc)
{
	Square[sq]=pc;
	Pieces[pc]=sq;

}

void FromFen(const char *szFen) {
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
        k = FenPiece(*lpFen);
        if (k < 7) {
          if (pcWhite[k] < 32) {
            //if (this->ucsqPieces[pcWhite[k]] == 0) {
              AddPiece(COORD_XY(j, i), pcWhite[k]);
              pcWhite[k] ++;
            //}
          }
        }
        j ++;
      }
    } else if (*lpFen >= 'a' && *lpFen <= 'z') {
      if (j <= FILE_RIGHT) {
        k = FenPiece(*lpFen + 'A' - 'a');
        if (k < 7) {
          if (pcBlack[k] < 48) {
            //if (this->ucsqPieces[pcBlack[k]] == 0) {
              AddPiece(COORD_XY(j, i), pcBlack[k]);
              pcBlack[k] ++;
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
//  if (this->sdPlayer == (*lpFen == 'b' ? 0 : 1)) {
//    ChangeSide();
//  }
//  // 4. 把局面设成“不可逆”
//  SetIrrev();
}

void ToFen(char *szFen)  {
  int i, j, k, pc;
  char *lpFen;

  lpFen = szFen;
  for (i = RANK_TOP; i <= RANK_BOTTOM; i ++) {
    k = 0;
    for (j = FILE_LEFT; j <= FILE_RIGHT; j ++) {
      pc = Square[COORD_XY(j, i)];
      if (pc != 0) {
        if (k > 0) {
          *lpFen = k + '0';
          lpFen ++;
          k = 0;
        }
        *lpFen = PIECE_BYTE(PIECE_TYPE(pc)) ;
        //*lpFen = PIECE_BYTE(PIECE_TYPE(pc)) + (pc < 32 ? 0 : 'a' - 'A');
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
  *lpFen = 'w';//(this->sdPlayer == 0 ? 'w' : 'b');
  lpFen ++;
  *lpFen = '\0';
}




int main(int argc,char *argv[])
{
 
	int i,j,k;
	char str[256];

	FromFen(cszStartFen );

	for(i=0;i<16;i++)
	{
		for(j=0;j<16;j++)
			printf(" %2d ",Square[i*16+j]);
		printf("\n");
	}
		printf("\n\n\n");
	for(i=0;i<3;i++)
	{
		for(j=0;j<16;j++)
			printf(" %x ",Pieces[i*16 + j]);
		printf("\n");
	}
	ToFen(str);
	printf("ToFen = %s\n",str);

	

	    return 0;
}
