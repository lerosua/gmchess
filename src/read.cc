#include <gtkmm.h>
#include <iostream>
#include <string>
#include <locale.h>
#include <fstream>

char word_to_code(const Glib::ustring& word)
{
	if(false){
	}
	else if(word == "帅")
		return 'k'; 
	else if(word == "仕" || word == "士")
		return 's';
	else if(word == "相  || word == ""象")
		return 'b';
	else if(word == "马")
		return 'n';
	else if(word == "车")
		return 'j';
	else if(word == "炮")
		return 'c';
	else if(word == "兵" || word == "卒")
		return 'p';
	else if(word == "一" || word == "1")
		return 1;
	else if(word == "二" || word == "2")
		return 2;
	else if(word == "三" || word == "3")
		return 3;
	else if(word == "四" || word == "4")
		return 4;
	else if(word == "五" || word == "5")
		return 5;
	else if(word == "六" || word == "6")
		return 6;
	else if(word == "七" || word == "7")
		return 7;
	else
		return -1;
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
		if(pos != std::string::npos)
			continue;
		 Glib::ustring uline(line);
		std::cout<<uline<<std::endl;

		int i;
		for (i = 0; i< uline.length(); i++)
		{
			Glib::ustring word;
			word.assign(1,uline[i]);
			int c = word_to_code(word);
			std::cout << word<<"-"<<c<<"   ";

		}
		std::cout<<std::endl;
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
