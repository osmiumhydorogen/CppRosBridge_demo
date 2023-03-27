#include <jansson.h>
#include <crb_client.hpp>
#include <chrono>
#include <thread>
#include <wspp_wrapper.hpp>

int main(int argc, char const *argv[]) {
	crb_sock::WsppWrapper sock;
	double t_last_adv = 0;
	//sock.connect("127.0.0.1", 9090);
	std::cout <<"connect:"<< sock.connect("ws://127.0.0.1:9090") << std::endl;
	crb_client::ConnectionManager<crb_sock::WsppWrapper> cm(&sock);
	std::chrono::system_clock::time_point program_start, current_time;
	/*
	program_start = std::chrono::system_clock::now();
	for(;;)
	{
		double t_from_start;
		current_time = std::chrono::system_clock::now();
		t_from_start = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-program_start).count();

		if(t_from_start > 1000) break;
	}
	// */
	std::this_thread::sleep_for(std::chrono::seconds(1));
	cm.advertise("testtopic", "std_msgs/Int64");
	cm.advertise("testtopic2", "std_msgs/Float32");

	json_t *op_json, *op_json2;
	//char *json_str;
	op_json = json_object();
	json_object_set_new(op_json, "data", json_integer(100));

	op_json2 = json_object();
	json_object_set_new(op_json2, "data", json_real(1.5));

	for(int kuriyama = 0; kuriyama < 10; kuriyama++)
	{
		double t_from_start;
		current_time = std::chrono::system_clock::now();
		t_from_start = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-program_start).count();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		//if(t_from_start > 100000) break;
		if(true)//t_from_start - t_last_adv >= 100)
		{
			std::cout << t_from_start << ":" << std::endl;
			//cm.advertise("testtopic", "std_msgs/Int64");
			cm.publish("testtopic", *op_json);
			cm.publish("testtopic2", *op_json2);
			t_last_adv = t_from_start;
		}
	}
	cm.unadvertise("testtopic");
	return 0;
}
