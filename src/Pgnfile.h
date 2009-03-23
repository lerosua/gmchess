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

#include <gtkmm.h> 
#include <vector>

typedef struct _board_info{
		Glib::ustring event;
		Glib::ustring date;
		Glib::ustring site;
		Glib::ustring red;
		Glib::ustring black;
		Glib::ustring result;
		Glib::ustring ecco;
		Glib::ustring opening;
		Glib::ustring variation;
}Board_info;
class Engine;

class Pgnfile {
	public:
		Pgnfile(Engine& f_engine);
		~Pgnfile();
		char word_to_pos(const Glib::ustring& word);
		char word_to_digit(const Glib::ustring& word);
		char word_to_action(const Glib::ustring& word);
		char word_to_code(const Glib::ustring& word);
		int read(const std::string & filename);
		bool get_label(Glib::ustring& dst_str, const Glib::ustring& line_str, const Glib::ustring& name);
		const Board_info& get_board_info(){ return board_info ;}

	private:
		Engine& m_engine;
		Board_info board_info;

};



#endif   /* ----- #ifndef PGNFILE_FILE_HEADER_INC  ----- */

