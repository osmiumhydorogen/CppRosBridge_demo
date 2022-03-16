#include <jansson.h>
#include <crb_client.hpp>
#include <chrono>
#include <thread>
#include <wspp_wrapper.hpp>

json_t *buildTwistMsg(double omega)
{
	json_t *twist;
	json_t *linear, *angular;
	twist = json_object();
	linear = json_object();
	angular = json_object();
	json_object_set_new(linear, "x", json_real(0.));
	json_object_set_new(linear, "y", json_real(0.));
	json_object_set_new(linear, "z", json_real(0.));

	json_object_set_new(angular, "x", json_real(0.));
	json_object_set_new(angular, "y", json_real(0.));
	json_object_set_new(angular, "z", json_real(omega));

	json_object_set_new(twist, "linear", linear);
	json_object_set_new(twist, "angular", angular);
	return twist;
}

int main(int argc, char const *argv[]) {
	crb_sock::WsppWrapper sock;
	double t_last_adv = 0;
	std::cout <<"connect:"<< sock.connect("ws://0.0.0.0:9090") << std::endl;
	crb_client::ConnectionManager<crb_sock::WsppWrapper> cm(&sock);
	std::string vel_topic = "cmd_vel";

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	cm.advertise(vel_topic, "geometry_msgs/Twist");

	json_t *ccw_msg, *cw_msg;

	ccw_msg = buildTwistMsg( 1.);
	cw_msg  = buildTwistMsg(-1.);
	for(;;)
	{
		std::cout << "clock wise\n";
		cm.publish(vel_topic, *cw_msg);
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		std::cout << "counter clock wise\n";
		cm.publish(vel_topic, *ccw_msg);
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	}
	cm.unadvertise("testtopic");
	return 0;
}
