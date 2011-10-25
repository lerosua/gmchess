/*
 * =====================================================================================
 *
 *       Filename:  robot.cc
 *
 *    Description:  the ai handle
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
#include "robot.h"
#include <gdk/gdkx.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include "ec_throw.h"
#include <cassert>
#include <string>
#include <unistd.h>
#include <fcntl.h>


void  set_signals()
{
	sigset_t set;
	struct sigaction act;

	EC_THROW( -1 == sigfillset(&set));
	EC_THROW( -1 == sigprocmask(SIG_SETMASK, &set, NULL));

	memset(&act, 0, sizeof(act));

	EC_THROW( sigfillset(&act.sa_mask));
	act.sa_handler = SIG_IGN;
	EC_THROW( sigaction(SIGHUP, &act, NULL));
	EC_THROW( sigaction(SIGINT, &act, NULL));
	EC_THROW( sigaction(SIGQUIT, &act, NULL));
	EC_THROW( sigaction(SIGPIPE, &act, NULL));

	EC_THROW( sigaction(SIGTERM, &act, NULL));
	EC_THROW( sigaction(SIGBUS, &act, NULL));
	EC_THROW( sigaction(SIGFPE, &act, NULL));
	EC_THROW( sigaction(SIGILL, &act, NULL));
	EC_THROW( sigaction(SIGSEGV, &act, NULL));
	EC_THROW( sigaction(SIGSYS, &act, NULL));
	EC_THROW( sigaction(SIGXCPU, &act, NULL));
	EC_THROW( sigaction(SIGXFSZ, &act, NULL));
	EC_THROW( sigaction(SIGHUP, &act, NULL));

	EC_THROW( sigemptyset(&set));
	EC_THROW( sigprocmask(SIG_SETMASK, &set, NULL));

}

void Robot::create_pipe()
{
	close_pipe();

	int std_pipe[2];
	EC_THROW(-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, std_pipe));
	child_tem = std_pipe[0];
	child_tem2 = std_pipe[1];
}

void Robot::close_pipe()
{
	if ( -1 != child_tem)
		close(child_tem);
	if ( -1 != child_tem2)
		close(child_tem2);
}

void Robot::set_s_pipe()
{
	close(child_tem); // 关闭父进程端.
	EC_THROW( -1 == dup2(child_tem2, STDIN_FILENO));
	EC_THROW( -1 == dup2(child_tem2, STDOUT_FILENO));
	//EC_THROW( -1 == dup2(child_tem2, STDERR_FILENO));
	close(child_tem2);
}

void Robot::set_m_pipe()
{
	close(child_tem2);
	child_tem2 = -1;

	fd_set fd_set_write;
	FD_ZERO(&fd_set_write);
	FD_SET(child_tem, &fd_set_write);

	EC_THROW( -1 == select (child_tem + 1, NULL, &fd_set_write, NULL, NULL));
	int flags = fcntl(child_tem, F_GETFL, 0);  
	fcntl(child_tem,  F_SETFL, flags | O_NONBLOCK);
}


////////////////////////////////////////////////////////////////

	Robot::Robot()
	:is_running(false)
	,is_pause(false)
	 ,child_pid(-1)
{
	engine_name = "eleeye_engine";
	child_tem = -1;
	child_tem2 = -1;
}


Robot::~Robot()
{
}

void Robot::wait_robot_exit(GPid pid, int)
{
	if (child_pid != -1) {
		ptm_conn.disconnect();

		waitpid(child_pid, NULL, 0);
		child_pid = -1;

		is_running = false;
		on_robot_exit();
		printf(" i have exit ======\n");
	}

}

void Robot::start()
{
	printf("%s:%d\n",__func__,__LINE__);
	signal_start().emit();
	const char* argv[2];
	//argv[0]="eleeye_engine";
	argv[0]=get_engine().c_str();
	argv[1]=NULL;
	my_system((char* const *)argv);

	printf("end %s:%d\n",__func__,__LINE__);
}

int Robot::my_system(char* const argv[])
{
	extern char **environ;
	create_pipe();
	assert(!out_slot.empty());
	printf("%s:%d\n",__func__,__LINE__);
	ptm_conn = Glib::signal_io().connect(
			out_slot,
			child_tem, Glib::IO_IN);

	printf("%s:%d\n",__func__,__LINE__);

	pid_t pid = fork();
	if (pid == -1)
		return -1;
	if (pid == 0) {

		set_signals();
		set_s_pipe();
		// 设置 这个子进程为进程组头，
		// 这样，只要杀掉这个进程，他的子进程也会退出
		EC_THROW(-1 == setpgid(0, 0));
		//sleep(4);
		execvp(argv[0], argv);

		perror("robot execvp:");
	} 
	child_pid = pid;

	set_m_pipe();


	wait_conn = Glib::signal_child_watch().connect(
			sigc::mem_fun(*this, &Robot::wait_robot_exit), pid, 0);

	is_running = true;
	return pid;
}

bool Robot::running() const
{
	return (is_running) && (child_pid != 0);
}


void Robot::send_ctrl_command(const char* c)
{
	if (running()) {
		assert(c != NULL);
		EC_THROW(-1 == write(child_tem, c, strlen (c)));
	}
}

void Robot::stop()
{
	if (child_pid != -1) {
		ptm_conn.disconnect();
		wait_conn.disconnect();

		for (;;) {
			kill(-child_pid, SIGKILL);
			int ret = (waitpid( -child_pid, NULL, WNOHANG));
			if (-1 == ret)
				break;
		}
		on_robot_exit();

		child_pid = -1;	
	}
	is_running = false;
}


void Robot::pause()
{
	is_pause = !is_pause;
	send_ctrl_command("pause\n");
}

