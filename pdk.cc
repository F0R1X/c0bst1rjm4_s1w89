#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ev.h>

#include "daemonize.h"
#include "pdk.h"
#include "game.h"
#include "log.h"

static int parse_arg(int argc, char **argv);
static int init_conf();
static void dump_conf();
static int set_rlimit(int n);

PDK pdk;
Log log;

int main(int argc, char **argv)
{
	int ret;
	ret = parse_arg(argc, argv);
	if (ret < 0) {
		log.fatal("File: %s Func: %s Line: %d => parse_arg.\n",	__FILE__, __FUNCTION__, __LINE__);
		exit(1);
	}
	ret = init_conf();
	if (ret < 0) {
		log.fatal("File: %s Func: %s Line: %d => init_conf.\n",	__FILE__, __FUNCTION__, __LINE__);
		exit(1);
	}
	dump_conf();
	if (pdk.is_daemonize == 1)
		daemonize();
	signal(SIGPIPE, SIG_IGN);
	ret = single_instance_running(pdk.conf.get("pid_file", "conf/pdksvr.pid").asString().c_str());
	if (ret < 0) {
		log.fatal("File: %s Func: %s Line: %d => single_instance_running.\n", __FILE__, __FUNCTION__, __LINE__);
		exit(1);
	}
	log.start(pdk.conf["log"].get("log_file", "log/pdksvr.log").asString(),
				pdk.conf["log"].get("level", 4).asInt(),
				pdk.conf["log"].get("console", 0).asInt(),
				pdk.conf["log"].get("rotate", 1).asInt(),
				pdk.conf["log"].get("max_size", 1073741824).asInt(),
				pdk.conf["log"].get("max_file", 50).asInt());
	set_rlimit(10240);
	struct ev_loop *loop = ev_default_loop(0);
	pdk.loop = loop;
	pdk.game = new (std::nothrow) Game();
	pdk.game->start();
	ev_loop(loop, 0);
	return 0;
}

static int parse_arg(int argc, char **argv)
{
	int flag = 0;
	int oc; /* option chacb. */
	char ic; /* invalid chacb. */

	pdk.is_daemonize = 0;
	while((oc = getopt(argc, argv, "Df:")) != -1) {
		switch(oc) {
			case 'D':
				pdk.is_daemonize = 1;
				break;
			case 'f':
				flag = 1;
				pdk.conf_file = string(optarg);
				break;
			case '?':
				ic = (char)optopt;
				printf("invalid \'%c\'\n", ic);
				break;
			case ':':
				printf("lack option arg\n");
				break;
		}
	}

	if (flag == 0)
		return -1;

	return 0;
}

static int init_conf()
{
	std::ifstream in(pdk.conf_file.c_str(), std::ifstream::binary);
	if (!in) {
		std::cout << "init file no found." << endl;
		return -1;
	}

	Json::Reader reader;
	bool ret = reader.parse(in, pdk.conf);
	if (!ret) {
		in.close();
		std::cout << "init file parser." << endl;
		return -1;
	}
	in.close();
	pdk.room_conf["1001001"] = pdk.conf["room_conf"]["1001001"].asInt();
	pdk.room_conf["1001002"] = pdk.conf["room_conf"]["1001002"].asInt();
	pdk.room_conf["1002001"] = pdk.conf["room_conf"]["1002001"].asInt();
	pdk.room_conf["1002002"] = pdk.conf["room_conf"]["1002002"].asInt();
	pdk.room_conf["1003001"] = pdk.conf["room_conf"]["1003001"].asInt();
	pdk.room_conf["1003002"] = pdk.conf["room_conf"]["1003002"].asInt();
	pdk.room_conf["1004001"] = pdk.conf["room_conf"]["1004001"].asInt();
	pdk.room_conf["1004002"] = pdk.conf["room_conf"]["1004002"].asInt();
	pdk.room_conf["1005001"] = pdk.conf["room_conf"]["1005001"].asInt();
	pdk.room_conf["1005002"] = pdk.conf["room_conf"]["1005002"].asInt();
	pdk.room_conf["1005005"] = pdk.conf["room_conf"]["1005005"].asInt();
	pdk.room_conf["1005007"] = pdk.conf["room_conf"]["1005007"].asInt();
	pdk.room_conf["1015001"] = pdk.conf["room_conf"]["1015001"].asInt();
	pdk.room_conf["1015002"] = pdk.conf["room_conf"]["1015002"].asInt();
	pdk.room_conf["1016001"] = pdk.conf["room_conf"]["1016001"].asInt();
	pdk.room_conf["1016002"] = pdk.conf["room_conf"]["1016002"].asInt();
	pdk.room_conf["1017001"] = pdk.conf["room_conf"]["1017001"].asInt();
	pdk.room_conf["1017002"] = pdk.conf["room_conf"]["1017002"].asInt();
	pdk.room_conf["1018001"] = pdk.conf["room_conf"]["1018001"].asInt();
	pdk.room_conf["1018002"] = pdk.conf["room_conf"]["1018002"].asInt();
	pdk.room_conf["1018003"] = pdk.conf["room_conf"]["1018003"].asInt();
	pdk.room_conf["1018004"] = pdk.conf["room_conf"]["1018004"].asInt();
	pdk.room_conf["1019001"] = pdk.conf["room_conf"]["1019001"].asInt();
	pdk.room_conf["1020001"] = pdk.conf["room_conf"]["1020001"].asInt();
	pdk.room_conf["1021001"] = pdk.conf["room_conf"]["1021001"].asInt();
	pdk.room_conf["1021002"] = pdk.conf["room_conf"]["1021002"].asInt();
	return 0;
}

static void dump_conf()
{
	std::cout << "pid_file: "
		<< pdk.conf.get("pid_file", "conf/pdksvr.pid").asString() << endl;
	std::cout << "log:log_file: "
		<< pdk.conf["log"].get("log_file", "log/pdksvr.log").asString()
		<< endl;
	std::cout << "log:level: "
		<< pdk.conf["log"].get("level", 4).asInt() << endl;
	std::cout << "log:console: "
		<< pdk.conf["log"].get("console", 0).asInt() << endl;
	std::cout << "log:rotate: "
		<< pdk.conf["log"].get("rotate", 1).asInt() << endl;
	std::cout << "log:max_size: "
		<< pdk.conf["log"].get("max_size", 1073741824).asInt() << endl;
	std::cout << "log:max_file: "
		<< pdk.conf["log"].get("max_file", 50).asInt() << endl;

	std::cout << "game:host: "
		<< pdk.conf["game"]["host"].asString() << endl;
	std::cout << "game:port: "
		<< pdk.conf["game"]["port"].asInt() << endl;
	std::cout << "tables:begin: "
		<< pdk.conf["tables"]["begin"].asInt() << endl;
	std::cout << "tables:end: "
		<< pdk.conf["tables"]["end"].asInt() << endl;
	std::cout << "tables:vid: "
		<< pdk.conf["tables"]["vid"].asInt() << endl;
	std::cout << "tables:zid: "
		<< pdk.conf["tables"]["zid"].asInt() << endl;
	std::cout << "tables:base_money: "
		<< pdk.conf["tables"]["base_money"].asInt() << endl;
	std::cout << "tables:stand_money: "
		<< pdk.conf["tables"]["stand_money"].asInt() << endl;
	std::cout << "tables:type: "
		<< pdk.conf["tables"]["table_type"].asInt() << endl;
	std::cout << "tables:spring_ratio: "
		<< pdk.conf["tables"]["spring_ratio"].asInt()<<endl;
	std::cout << "tables:bomb_ratio: "
		<< pdk.conf["tables"]["bomb_ratio"].asInt()<<endl;
}

static int set_rlimit(int n)
{
	struct rlimit rt;
	rt.rlim_max = rt.rlim_cur = n;
	if (setrlimit(RLIMIT_NOFILE, &rt) == -1) {
		log.error("File: %s Func: %s Line: %d => setrlimit %s.\n",
							__FILE__, __FUNCTION__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;
}