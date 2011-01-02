/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.cc
 * Copyright (C) wind 2009 <xihels@gmail.com>
 * 
 */

//#include <libglademm/xml.h>
#include <iostream>
#include <gtkmm.h>
#include <gtkmm/main.h>
#include <glib/gi18n.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "MainWindow.h"
#include "ec_throw.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

TGMConf GMConf;
//检测单一实例
int singleon(const std::string& url)
{
	int sockfd;
	//struct hostent *he;
	struct sockaddr_in srvaddr;

	EC_THROW(-1 == (sockfd=socket(AF_INET,SOCK_STREAM,0)));
	memset(&srvaddr, 0, sizeof(srvaddr));
	srvaddr.sin_family=AF_INET;
	srvaddr.sin_port=htons(GMPORT);
	srvaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	int on = 1;
	EC_THROW( -1 == (setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) )));
	if(bind(sockfd,(struct sockaddr*)&srvaddr,
			sizeof(struct sockaddr))==-1){
		//printf("there has another gmchess running\n");
		if(url.empty())
			exit(0);
		else{
			//把参数发送给另一个gmchess再退出
			if( 0 == connect(sockfd,(struct sockaddr*)&srvaddr,sizeof(srvaddr))){
				write(sockfd,url.c_str(),url.size());
				close(sockfd);
				exit(0);

			}
			exit(1);
		}
	}
	EC_THROW(-1 == listen(sockfd,128));
	return sockfd;
}
int main (int argc, char *argv[])
{

	std::string url;

	if(2==argc){
		url = std::string(argv[1]);
	}
	else{
		url=std::string("");

	}
	int fd_io = singleon(url);
	bindtextdomain (GETTEXT_PACKAGE, GMCHESS_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	Gtk::Main kit(argc, argv);
	MainWindow win;
	win.watch_socket(fd_io);
	win.start_with(url);
	kit.run(win);
	return 0;
}

