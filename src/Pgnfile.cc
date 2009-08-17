/*
 * Pgnfile.h
 * Copyright (C) lerosua 2009 <lerosua@gmail.com>
 * 
 */
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


bool Pgnfile::get_label(Glib::ustring& dst_str, const Glib::ustring& line_str, const Glib::ustring& name)
{

	size_t pos = line_str.find(name);
	if(pos == std::string::npos)
		return false;
	pos = line_str.find_first_of("\"");
	if(pos == std::string::npos)
		return false;
	Glib::ustring tmp = line_str.substr(pos+1,std::string::npos);
	size_t end_pos = tmp.find_first_of("\"");
	dst_str = tmp.substr(0,end_pos);

	return true;

}

int Pgnfile::read(const std::string & filename)
{
	std::fstream file;
	file.open(filename.c_str(),std::ios_base::in);
	if(!file){
		DLOG("open %s file error\n",filename.c_str());
		return -1;
	}

	m_engine.init_snapshot(start_fen);

	std::string line;
	Glib::ustring startFen;
	bool comment=false;
	std::string comment_str;
	while(std::getline(file,line)){
		size_t pos = line.find_first_of("[");
		if(pos != std::string::npos){
			get_label(board_info.event,line,"Event");
			get_label(board_info.date ,line,"Date");
			get_label(board_info.red  ,line,"Red ");
			get_label(board_info.black,line,"Black ");
			get_label(board_info.site, line, "Site");
			get_label(board_info.result,line,"Result");
			get_label(board_info.ecco,  line, "ECCO");
			get_label(board_info.opening,line, "Opening");
			get_label(board_info.variation,line, "Variation");
			if(get_label(startFen,line, "FEN")){
				DLOG("get FEN %s\n",startFen.c_str());
				m_engine.init_snapshot(startFen.c_str());
			}

			continue;
		}
		/**暂时过滤象眼的信息*/
		pos = line.find_first_of("=");
		if(pos != std::string::npos)
			continue;
		/**暂时过滤棋步注释*/
		pos = line.find_first_of("{");
		if(pos != std::string::npos){
			comment = true;

			/** 注释在一行内结束*/
			/** the comment will end in one line*/
			size_t s_pos = line.find_last_of("}");
			if(s_pos != std::string::npos){
				comment_str += line;
				m_engine.add_comment(comment_str);
				comment = false;
				comment_str.clear();
				continue;
			}

			/** 注释有多行*/
			/** the comment has multi line*/
			comment_str += line;
			continue;
		}

		pos = line.find_last_of("}");
		if(pos != std::string::npos){
			comment_str += line;
			m_engine.add_comment(comment_str);
			comment = false;
			comment_str.clear();
			continue;
		}

		if(comment){
			comment_str += line;
			continue;
		}

		 Glib::ustring uline(line);
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
			if(m_engine.logic_move(move)){
				m_engine.do_move(move);
			}


		}while(i<uline.length());
		
	}

	file.close();
	return 0;
}

