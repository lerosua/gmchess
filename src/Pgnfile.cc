#include <iostream> 
#include <string> 
#include <locale.h> 
#include <fstream> 
#include "gmchess.h" 
#include "Pgnfile.h"
#include "Engine.h"

Pgnfile::Pgnfile(Engine& f_engine):m_engine(f_engine)
{

}

Pgnfile::~Pgnfile()
{

}

char Pgnfile::word_to_pos(const Glib::ustring& word)
{
	if(false){
	}
	else if(word == "一" || word == "前")
		return 'a';
	else if(word == "二" || word == "中")
		return 'b';
	else if(word == "三" || word == "后")
		return 'c';
	else if(word == "四" )
		return 'd';
	else if(word == "五" )
		return 'e';
	else
		return -1;
}

char Pgnfile::word_to_digit(const Glib::ustring& word)
{
	if(false){
	}
	else if(word == "一" || word == "1")
		return '1';
	else if(word == "二" || word == "2")
		return '2';
	else if(word == "三" || word == "3")
		return '3';
	else if(word == "四" || word == "4")
		return '4';
	else if(word == "五" || word == "5")
		return '5';
	else if(word == "六" || word == "6")
		return '6';
	else if(word == "七" || word == "7")
		return '7';
	else if(word == "八" || word == "8")
		return '8';
	else if(word == "九" || word == "9")
		return '9';
	else
		return -1;



}
char Pgnfile::word_to_action(const Glib::ustring& word)
{
	if(false){
	}
	else if(word == "平")
		return '.';
	else if(word == "进")
		return '+';
	else if(word == "退")
		return '-';
	else
		return -1;
}


char Pgnfile::word_to_code(const Glib::ustring& word)
{
	if(false){
	}
	else if(word == "帅"|| word == "将")
		return 'K'; 
	else if(word == "仕" || word == "士")
		return 'A';
	else if(word == "相"  || word == "象")
		return 'B';
	else if(word == "马")
		return 'N';
	else if(word == "车")
		return 'R';
	else if(word == "炮")
		return 'C';
	else if(word == "兵" || word == "卒")
		return 'P';
	else if(word == "一" || word == "前")
		return 'a';
	else if(word == "二" || word == "中")
		return 'b';
	else if(word == "三" || word == "后")
		return 'c';
	else if(word == "四" )
		return 'd';
	else if(word == "五" )
		return 'e';
	else
		return -1;
}



int Pgnfile::read(void)
{
	std::ifstream file("test.pgn");
	if(!file){
		printf("open file error\n");
		return -1;
	}

	std::string line;
	while(std::getline(file,line)){
		size_t pos = line.find_first_of("[");
		if(pos != std::string::npos)
			continue;
		pos = line.find_first_of("===");
		if(pos != std::string::npos)
			continue;
		 Glib::ustring uline(line);
		//std::cout<<uline<<std::endl;
		/** 保存着法的中文表达*/
		//move_chinese.push_back(uline);
		m_engine.add_move_chinese(uline);

		int i=0;
		
		do{
			Glib::ustring word;
			word.assign(1,uline[i]);
			i++;
			int c = word_to_code(word);
			if(c == -1)
				continue;
			Glib::ustring word2;
			Glib::ustring word3;
			Glib::ustring word4;
			word2.assign(1,uline[i++]);
			word3.assign(1,uline[i++]);
			word4.assign(1,uline[i++]);
			std::cout << word<<" ";
			std::cout << word2<<" ";
			std::cout << word3<<" ";
			std::cout << word4<<" ";


			union Hanzi c_hanzi;
			/** 如果读到的c是位置，则第二个则是棋子,否则是位置*/
			if(c>96){
				c_hanzi.word[0] =word_to_code(word2);
				c_hanzi.word[1] =c;
			}
			else
			{
				c_hanzi.word[0] =c;
				c_hanzi.word[1] =word_to_digit(word2);
			}
			c_hanzi.word[2] =word_to_action(word3);
			c_hanzi.word[3] =word_to_digit(word4);
			std::cout<<c_hanzi.word[0]<<c_hanzi.word[1]<<c_hanzi.word[2]<<c_hanzi.word[3]<<" == "<<c_hanzi.digit<<std::endl;

			uint32_t iccs = m_engine.hanzi_to_iccs(c_hanzi.digit);
			int move = m_engine.iccs_to_move(iccs);
			m_engine.do_move(move);


		}while(i<uline.length());
		
	}

	file.close();
	return 0;
}

