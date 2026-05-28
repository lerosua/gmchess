/*
 * =====================================================================================
 * 
 *       Filename:  Pgnfile.h
 * 
 *    Description:  读取pgnfile文件
 * 
 *        Version:  1.0
 *        Created:  2009年02月24日 21时26分14秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 * 
 * =====================================================================================
 */

#ifndef  PGNFILE_FILE_HEADER_INC
#define  PGNFILE_FILE_HEADER_INC

#include <string>
#include <vector>

typedef struct _board_info{
		std::string event;
		std::string date;
		std::string site;
		std::string red;
		std::string black;
		std::string result;
		std::string ecco;
		std::string opening;
		std::string variation;
}Board_info;
class Engine;

class Pgnfile {
	public:
		Pgnfile(Engine& f_engine);
		~Pgnfile();
		char word_to_pos(const std::string& word);
		char word_to_digit(const std::string& word);
		char word_to_action(const std::string& word);
		char word_to_code(const std::string& word);
		int read(const std::string & filename);
		bool get_label(std::string& dst_str, const std::string& line_str, const std::string& name);
		const Board_info& get_board_info(){ return board_info ;}

	private:
		Engine& m_engine;
		Board_info board_info;

};



#endif   /* ----- #ifndef PGNFILE_FILE_HEADER_INC  ----- */
