/*
 * =====================================================================================
 *
 *       Filename:  robot.h
 *
 *    Description:  
 *
 *        Version:  0.30
 *        Created:  2009年03月31日 13时55分18秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  lerosua@gmail.com
 *        Company:  cyclone
 *
 * =====================================================================================
 */
#ifndef GMROBOT_H_
#define GMROBOT_H_

#include <stdlib.h>
#include <iostream>
#include <gtkmm.h>

class Robot {
	public:
		Robot();
		~Robot();
		void send_ctrl_command(const char* c);
		void pause(); 
		void stop();
		void start();
		void set_engine(const std::string&  name){ engine_name = name; }
		std::string get_engine() { return engine_name ;}

		bool running() const;
		bool pausing() const { return is_pause; }
		void set_out_slot(const sigc::slot<bool, Glib::IOCondition>& slot)
		{ out_slot = slot; }

		ssize_t get_robot_log(char* buf, size_t count) 
		{ return read(child_tem, buf, count); }

		typedef const sigc::signal<void> type_signal_stop;
		type_signal_stop signal_stop() const
		{ return signal_stop_; }

		typedef const sigc::signal<void> type_signal_start;
		type_signal_start signal_start() const
		{ return signal_start_; }	
	protected:
		int my_system(char* const argv[]);
	private:
		void wait_robot_exit(GPid, int);
		virtual void on_robot_exit() {};
		void set_s_pipe();
		void set_m_pipe();
		void create_pipe();
		void close_pipe();

		sigc::slot<bool, Glib::IOCondition> 	out_slot;
		sigc::connection 		ptm_conn;
		sigc::connection 		wait_conn;
		type_signal_stop  		signal_stop_;
		type_signal_start 		signal_start_;
		int		child_tem; 	// 主进程和子进程的连接管道的主进程则
		int 	child_tem2; 	// 子进程端的标准输入，输出
		int		child_pid;	/* robot's pid (internal)*/
		bool	is_running;	
		bool	is_pause;	
		std::string  engine_name;  //the ucci engine execute file's name
};

#endif
