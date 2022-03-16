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


json_t *floatVec2Json(const std::vector<float> &vec)
{
	json_t *data_json = json_array();
	for(auto &&val : vec)
	{
		json_array_append_new(data_json, json_real(val));
	}
	return data_json;
}

int main()
{
	int n_action = 8;
	int dim_input = 32 * 32;

//learning params
	float gamma = 0.99;
	float alpha_w = 3e-2;
	float alpha_z = 3e-2;

	float loop_freq_hz = 10;

	std::string weight_save_path = "ac_wall_weights.json";
	std::string weight_load_path = weight_save_path;

//weights
	std::vector<std::vector<float>> z_ij;
	std::vector<std::vector<float>> delta_z_ij;
	std::vector<float> w_i(dim_input);
	//std::vector<float> delta_w_i(dim_input);

	for(int i = 0; i < n_action; ++i)
	{
		z_ij.push_back(std::vector<float>(dim_input));
		delta_z_ij.push_back(std::vector<float>(dim_input));
	}


	/*---------- Save and Load Function ----------*/
	auto save_weights = [&](std::string save_path)
	{
		json_t *save_json, *zij_json;
		save_json = json_object();
		zij_json = json_array();
		for(auto &&v : z_ij)
		{
			json_array_append_new(zij_json, floatVec2Json(v));
		}
		json_object_set_new(save_json, "z_ij", zij_json);
		json_object_set_new(save_json, "w_i", floatVec2Json(w_i));

		json_dump_file(save_json, save_path.c_str(), JSON_INDENT(4) | JSON_COMPACT);
		json_decref(save_json);
	};

	auto load_weights = [&](std::string load_path)
	{
		json_t *load_json, *z_json, *w_json;
		json_error_t err;
		load_json = json_load_file(load_path.c_str(), JSON_DECODE_INT_AS_REAL, &err);
		z_json = json_object_get(load_json, "z_ij");
		w_json = json_object_get(load_json, "w_i");

		//load z_ij
		int i, j;
		json_t *zi_json;
		int n_ac, n_pc;
		n_ac = json_array_size(z_json);
		z_ij.resize(n_ac);
		std::printf("loading z_ij\n");
		json_array_foreach(z_json, i, zi_json)
		{
			json_t *zij_json;
			n_pc = json_array_size(zi_json);
			z_ij[i].resize(n_pc);
			std::printf("action %d n_pc %d\n", i, n_pc);
			json_array_foreach(zi_json, j, zij_json)
			{
				z_ij[i][j] = json_real_value(zij_json);
			}
		}

		//load w_i
		n_pc = json_array_size(w_json);
		w_i.resize(n_pc);
		json_t *wi_json;
		std::printf("loading w_i(len=%d)\n", n_pc);
		json_array_foreach(w_json, i, wi_json)
		{
			w_i[i] = json_real_value(wi_json);
		}
	};

	load_weights(weight_load_path);
	//other internal states
	bool is_first = true;

	float learning_stop_threshold = 0.05;
	bool waiting_for_restart = false;
	int stop_in_n_loop = 0;

	float reward = 0, prev_reward = 0;
	std::vector<float> input_vec(dim_input), latest_input_vec(dim_input), prev_input_vec(dim_input);

	std::vector<float> action(n_action);
	std::vector<float> p_j(n_action);
	float critic, prev_critic;

	std::random_device seed_gen;
	std::mt19937 rng_engine(seed_gen());

	int actual_action;
	float td_error;


	//*
	//auto reset_cb = [&](const std_msgs::Empty::ConstPtr &msg)
	auto reset_cb = [&](json_t *msg)
	{
		waiting_for_restart = false;
		is_first = true;
		std::printf("restarted actor critic\n");
	};
	// */

	/*callback lambda functions definision*/
	//auto input_cb = [&](const std_msgs::Float32MultiArray::ConstPtr& msg)
	auto input_cb = [&](json_t *msg)
	{
		/*
		char *str_c = json_dumps(msg, crb_client::JSON_SETTING);
		std::printf("recieved:\n%s\n",str_c);
		free(str_c);
		/// */

		F32Array2Vec(msg, &latest_input_vec);
		if(latest_input_vec.size() > dim_input)
		{
			dim_input = latest_input_vec.size();
			w_i.resize(dim_input);
			input_vec.resize(dim_input);
			prev_input_vec.resize(dim_input);

			for(int i = 0; i < n_action; ++i)
			{
				z_ij[i].resize(dim_input);
				delta_z_ij[i].resize(dim_input);
			}
			std::printf("Updated input size to %d\n", dim_input);
		}
	};

	auto reward_cb = [&](json_t* msg)
	{
		json_t *data = json_object_get(msg, "data");
		reward = json_real_value(data);
		json_decref(data);
		if(std::abs(reward) > learning_stop_threshold && !(stop_in_n_loop) && !waiting_for_restart)// && is_stop_status_by_edge)
		{
			stop_in_n_loop = 3;
		}
	};

	/*ROS Interfaces*/
	/*----------Setup Cpp ROS Bridge----------*/
	crb_sock::WsppWrapper sock;
	std::cout <<"connecting status:"<< sock.connect("ws://0.0.0.0:9090") << std::endl;
	crb_client::ConnectionManager<crb_sock::WsppWrapper> cm(&sock);
	//少し待機時間を入れないとなぜか上手くSubscribeしてくれない
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::string critic_topic = "critic";
	std::string action_vec_topic = "action_vec";
	std::string td_error_topic = "td_error";
	std::string action_id_topic = "action_id";

	std::string reward_topic = "/reward";
	std::string input_topic = "/ac_input";
	std::string reset_topic = "/reset";

	cm.advertise(critic_topic, "std_msgs/Float32");
	cm.advertise(td_error_topic, "std_msgs/Float32");
	cm.advertise(action_id_topic, "std_msgs/UInt32");
	cm.advertise(action_vec_topic, "std_msgs/Float32MultiArray");

	cm.subscribe(input_topic, input_cb);
	cm.subscribe(reward_topic, reward_cb);
	cm.subscribe(reset_topic, reset_cb);
	/*---------- ----------*/
	/*
	ros::Publisher critic_pub = n.advertise<std_msgs::Float32>(critic_topic, 1);
	ros::Publisher td_error_pub = n.advertise<std_msgs::Float32>(td_error_topic, 1);
	ros::Publisher action_id_pub = n.advertise<std_msgs::UInt32>(action_id_topic, 1);
	ros::Publisher action_vec_pub = n.advertise<std_msgs::Float32MultiArray>(action_vec_topic, 1);

	ros::Subscriber position_sub = n.subscribe<std_msgs::Float32MultiArray>(input_topic, 1, input_cb);
	ros::Subscriber reward_sub = n.subscribe<std_msgs::Float32>(reward_topic, 1, reward_cb);
	ros::Subscriber reset_sub = n.subscribe<std_msgs::Empty>(reset_topic, 1, reset_cb);
	// */
	int save_timer = 100;
	while(true)
	{
		/*
		if(waiting_for_restart)
		{
			loop_rate.sleep();
			continue;
		}
		// */

		critic = 0;
		input_vec = const_cast<const std::vector<float>&>(latest_input_vec);
		//#pragma omp parallel for
		for(int i = 0; i < dim_input; ++i)
			critic += w_i[i] * input_vec[i];

		if(is_first)
			is_first = false;
		else if(!waiting_for_restart)
		{
			//update weights using td-learning rule
			td_error = prev_reward + gamma * critic - prev_critic;
			#pragma omp parallel for
			for(int i = 0; i < dim_input; ++i)
			{
				w_i[i] += alpha_w * td_error * prev_input_vec[i];
				z_ij[actual_action][i] += alpha_z * td_error * prev_input_vec[i];
			}
			//if(std::abs(prev_reward) > learning_stop_threshold) is_stop_status_by_edge = true;
		}

		#pragma omp parallel for
		for(int j = 0; j < n_action; ++j)
		{
			action[j] = 0;
			for(int i = 0; i < dim_input; ++i)
			{
				action[j] += z_ij[j][i] * input_vec[i];
			}
			p_j[j] = std::exp(2. * action[j]);
		}

		actual_action = std::discrete_distribution<unsigned int>(p_j.begin(), p_j.end())(rng_engine);

		/*
		std_msgs::Float32 critic_msg;
		std_msgs::Float32MultiArray action_vec_msg;
		std_msgs::UInt32 action_id_msg;
		std_msgs::Float32 td_error_msg;

		action_id_msg.data = actual_action;
		action_vec_msg.data = action;
		critic_msg.data = critic;
		td_error_msg.data = td_error;

		action_id_pub.publish(action_id_msg);
		critic_pub.publish(critic_msg);
		action_vec_pub.publish(action_vec_msg);
		td_error_pub.publish(td_error_msg);
		// */
		json_t *tmp_msg;
		tmp_msg = json_pack("{s:i}", "data", actual_action);
		cm.publish(action_id_topic, *tmp_msg);
		json_decref(tmp_msg);

		tmp_msg = json_pack("{s:f}", "data", critic);
		cm.publish(critic_topic, *tmp_msg);
		json_decref(tmp_msg);

		tmp_msg = json_pack("{s:f}", "data", td_error);
		cm.publish(td_error_topic, *tmp_msg);
		json_decref(tmp_msg);

		tmp_msg = buildF32MultiArr(action);
		//std::printf(json_dumps(tmp_msg, crb_client::JSON_SETTING));
		cm.publish(action_vec_topic, *tmp_msg);
		json_decref(tmp_msg);

		prev_critic = critic;
		prev_reward = reward;
		prev_input_vec = const_cast<const std::vector<float>&>(input_vec);
		//if(stop_request > 0 &&)
		if(stop_in_n_loop > 0 && !(--stop_in_n_loop))
		{
			waiting_for_restart = true;
			is_first = true;
			//prev_reward = 0;
			//reward = 0;
			//stop_request = false;
			std::printf("stopping actor critic\n");
		}
		//loop_rate.sleep();
		if(!(--save_timer))
		{
			save_timer = 100;
			std::printf("saving weights\n");
			save_weights(weight_save_path);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
