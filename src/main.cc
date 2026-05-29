/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @file main.cc
 * @author wind
 * @brief Main entry of gmchess executable
 * @details Copyright (C) wind 2009 <xihels@gmail.com>
 */

#include <iostream>
#include <adwaita.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "MainWindow.h"
#include "ec_throw.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

TGMConf GMConf;


/**
 * @brief 检测单一实例
 * @todo Consider implementing this function with D-Bus
 */
static int singleon(const std::string& url)
{
	int sockfd;
	//struct hostent *he;
	struct sockaddr_in srvaddr;

	EC_THROW(-1 == (sockfd=socket(AF_INET,SOCK_STREAM,0)));
	EC_THROW(-1 == fcntl(sockfd, F_SETFD, FD_CLOEXEC));
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
				ssize_t written = write(sockfd,url.c_str(),url.size());
				(void)written;
				close(sockfd);
				exit(0);

			}
			exit(1);
		}
	}
	EC_THROW(-1 == listen(sockfd,128));
	return sockfd;
}


static void activate_cb(GtkApplication* app, gpointer user_data)
{
    const std::string* url = static_cast<const std::string*>(user_data);
    int fd_io = singleon(*url);
    MainWindow* win = new MainWindow(app);
    win->watch_socket(fd_io);
    win->start_with(*url);
}

int main (int argc, char *argv[])
{
    std::string url;

    if (argc == 2) {
        url = std::string(argv[1]);
    } else {
        url = std::string("");
    }

    bindtextdomain (GETTEXT_PACKAGE, GMCHESS_LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    GtkApplication* app = GTK_APPLICATION(adw_application_new("org.gmchess.GMChess", G_APPLICATION_DEFAULT_FLAGS));
    g_signal_connect(app, "activate", G_CALLBACK(activate_cb), &url);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
