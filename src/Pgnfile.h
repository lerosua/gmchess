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

class Engine;

class Pgnfile {
	public:
		Pgnfile(Engine& f_engine);
		~Pgnfile();
		char word_to_pos(const Glib::ustring& word);
		char word_to_digit(const Glib::ustring& word);
		char word_to_action(const Glib::ustring& word);
		char word_to_code(const Glib::ustring& word);
		int read(void);

	private:
		//std::vector<Glib::ustring> move_chinese;
		Engine& m_engine;




};



#endif   /* ----- #ifndef PGNFILE_FILE_HEADER_INC  ----- */

