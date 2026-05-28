#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <locale.h>
#include <fstream>

union Hanzi{
		char word[4];
		uint32_t digit;
	};

static std::string next_utf8_char(const std::string& text, size_t& pos)
{
	if(pos >= text.size())
		return std::string();

	const unsigned char c = static_cast<unsigned char>(text[pos]);
	size_t len = 1;
	if((c & 0x80) == 0)
		len = 1;
	else if((c & 0xe0) == 0xc0)
		len = 2;
	else if((c & 0xf0) == 0xe0)
		len = 3;
	else if((c & 0xf8) == 0xf0)
		len = 4;

	if(pos + len > text.size())
		len = text.size() - pos;

	std::string out = text.substr(pos, len);
	pos += len;
	return out;
}

char word_to_pos(const std::string& word)
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

char word_to_digit(const std::string& word)
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
char word_to_action(const std::string& word)
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


char word_to_code(const std::string& word)
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

bool get_label(std::string& dst_str, const std::string& line_str, const std::string& name)
{

	size_t pos = line_str.find(name);
	if(pos == std::string::npos)
		return false;
	pos = line_str.find_first_of("\"");
	if(pos == std::string::npos)
		return false;
	std::string tmp = line_str.substr(pos+1,std::string::npos);
	size_t end_pos = tmp.find_first_of("\"");
	dst_str = tmp.substr(0,end_pos);
	/*
	size_t end_pos = line_str.find_last_of("\"");
	if(end_pos == std::string::npos)
		return false;
	dst_str = line_str.substr(pos+1,end_pos);
	*/

	std::cout<<"get "<<name<<" = "<<dst_str<<std::endl;
	return true;
}


int fun(void)
{
	std::ifstream file("test.pgn");
	if(!file){
		printf("open file error\n");
		return -1;
	}

	std::string line;
	while(std::getline(file,line)){
		size_t pos = line.find_first_of("[");
		if(pos != std::string::npos){
			std::string dst_str;
			get_label(dst_str,line,"Site");
			get_label(dst_str,line,"Red ");
			get_label(dst_str,line,"RedTeam");
			get_label(dst_str,line,"Event");
			continue;
		}
#if 1
		std::cout<<line<<std::endl;

		size_t i=0;
		
		do{
			std::string word = next_utf8_char(line, i);
			int c = word_to_code(word);
			if(c == -1)
				continue;
			std::cout << word<<" ";
			std::string word2 = next_utf8_char(line, i);
			std::string word3 = next_utf8_char(line, i);
			std::string word4 = next_utf8_char(line, i);
			std::cout << word2<<" ";
			std::cout << word3<<" ";
			std::cout << word4<<" ";

			union Hanzi red;
			/** 如果读到的c是位置，则第二个则是棋子,否则是位置*/
			if(c>96){
				red.word[0] =word_to_code(word2);
				red.word[1] =c;
			}
			else
			{
				red.word[0] =c;
				red.word[1] =word_to_digit(word2);
			}
			red.word[2] =word_to_action(word3);
			red.word[3] =word_to_digit(word4);
			std::cout<<red.word[0]<<red.word[1]<<red.word[2]<<red.word[3]<<" == "<<red.digit<<std::endl;


		}while(i<line.length());
		
		std::cout<<std::endl;
#endif
	}

	file.close();
	return 0;
}

int main()
{
	 setlocale(LC_ALL, "zh_CN.UTF-8");
	std::string text = "炮二平四";
	size_t pos = 0;
	std::string word = next_utf8_char(text, pos);
	std::cout << word << std::endl;
	std::cout << "text.length()= "<<text.length()<<std::endl;
	if(word == "炮")
		std::cout<<"炮匹配了"<<std::endl;
	fun();
}
