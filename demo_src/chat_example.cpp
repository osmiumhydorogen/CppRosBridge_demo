#include <jansson.h>
#include <crb_client.hpp>
#include <chrono>
#include <thread>
#include <wspp_wrapper.hpp>

int main(int argc, char const *argv[]) {
	//Websocket版
	crb_sock::WsppWrapper sock;
	double t_last_adv = 0;
	//sock.connect("127.0.0.1", 9090);
	std::cout <<"connect:"<< sock.connect("ws://0.0.0.0:9090") << std::endl;
	crb_client::ConnectionManager<crb_sock::WsppWrapper> cm(&sock);
	std::chrono::system_clock::time_point program_start, current_time;
	
	std::string pub_topic = "chatter";
	std::this_thread::sleep_for(std::chrono::seconds(1));
	cm.advertise(pub_topic, "std_msgs/String");

	json_t *op_json, *op_json2;
	//char *json_str;
	op_json = json_object();
	json_object_set_new(op_json, "data", json_string("Hello!\n"));

	for(;;)
	{
		double t_from_start;
		current_time = std::chrono::system_clock::now();
		t_from_start = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-program_start).count();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		//if(t_from_start > 100000) break;
		if(t_from_start - t_last_adv >= 100)
		{
			std::cout << "current_time" << t_from_start << ":";
			std::cout << std::endl;
			cm.publish(pub_topic, *op_json);
			t_last_adv = t_from_start;
		}
	}
	cm.unadvertise(pub_topic);
	return 0;
}
