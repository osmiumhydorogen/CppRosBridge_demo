#include "rosmsg_tool.hpp"

#include <jansson.h>
#include <crb_client.hpp>
#include <wspp_wrapper.hpp>

#include <unistd.h>
#include <chrono>
#include <string>
#include <iostream>
#include <cstdio>
#include <thread>

void callbk(json_t *msg)
{
	std::cout << json_dumps(msg, JSON_ENCODE_ANY) << std::endl;
}

int main()
{
	std::string sub_topic = "/chatter";
	/*----------Setup Cpp ROS Bridge----------*/
	crb_sock::WsppWrapper sock;
	std::cout <<"connecting status:"<< sock.connect("ws://0.0.0.0:9090") << std::endl;
	crb_client::ConnectionManager<crb_sock::WsppWrapper> cm(&sock);
	//少し待機時間を入れないとなぜか上手くSubscribeしてくれない
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	cm.subscribe(sub_topic, callbk);
	
	while(true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
