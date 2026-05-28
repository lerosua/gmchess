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

#include <cstdlib>
#include <functional>
#include <glib.h>
#include <iostream>
#include <string>
#include <unistd.h>

class Robot {
	public:
		typedef std::function<bool(GIOCondition)> OutputCallback;
		typedef std::function<void()> EventCallback;

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
		void set_out_callback(const OutputCallback& callback)
		{ out_callback = callback; }
		void set_start_callback(const EventCallback& callback)
		{ start_callback = callback; }
		void set_stop_callback(const EventCallback& callback)
		{ stop_callback = callback; }

		ssize_t get_robot_log(char* buf, size_t count) 
		{ return read(child_tem, buf, count); }
	protected:
		int my_system(char* const argv[]);
	private:
		static gboolean robot_io_cb(GIOChannel* source, GIOCondition condition, gpointer data);
		static void robot_child_watch_cb(GPid pid, gint status, gpointer data);

		void wait_robot_exit(GPid, gint);
		virtual void on_robot_exit() {};
		void set_s_pipe();
		void set_m_pipe();
		void create_pipe();
		void close_pipe();

		OutputCallback	out_callback;
		EventCallback	stop_callback;
		EventCallback	start_callback;
		guint		ptm_source_id;
		guint		wait_source_id;
		int		child_tem; 	// 主进程和子进程的连接管道的主进程则
		int 	child_tem2; 	// 子进程端的标准输入，输出
		int		child_pid;	/* robot's pid (internal)*/
		bool	is_running;	
		bool	is_pause;	
		std::string  engine_name;  //the ucci engine execute file's name
};

#endif
