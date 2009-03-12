#include <gtkmm.h>
#include <iostream>
#include <string>
#include <locale.h>
#include <fstream>

union Hanzi{
		char word[4];
		uint32_t digit;
	};

char word_to_pos(const Glib::ustring& word)
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

char word_to_digit(const Glib::ustring& word)
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
char word_to_action(const Glib::ustring& word)
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


char word_to_code(const Glib::ustring& word)
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

bool get_label(Glib::ustring& dst_str, const Glib::ustring& line_str, const Glib::ustring& name)
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
			Glib::ustring dst_str;
			get_label(dst_str,line,"Site");
			get_label(dst_str,line,"Red ");
			get_label(dst_str,line,"RedTeam");
			get_label(dst_str,line,"Event");
			continue;
		}
#if 1
		 Glib::ustring uline(line);
		std::cout<<uline<<std::endl;

		int i=0;
		
		do{
			Glib::ustring word;
			word.assign(1,uline[i]);
			i++;
			int c = word_to_code(word);
			if(c == -1)
				continue;
			std::cout << word<<" ";
			Glib::ustring word2;
			Glib::ustring word3;
			Glib::ustring word4;
			word2.assign(1,uline[i++]);
			word3.assign(1,uline[i++]);
			word4.assign(1,uline[i++]);
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


		}while(i<uline.length());
		
		std::cout<<std::endl;
#endif
	}

	file.close();
	return 0;
}

int main()
{
	 setlocale(LC_ALL, "zh_CN.UTF-8");
	Glib::ustring text = "炮二平四";
	Glib::ustring word;
	word.assign(1,text[0]);
	std::cout << word << std::endl;
	std::cout << "text.length()= "<<text.length()<<std::endl;
	if(word == "炮")
		std::cout<<"炮匹配了"<<std::endl;
	fun();
}
