#pragma once
#include <socket_wrapper.hpp>
#include <jansson.h>
#include <functional>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>

namespace crb_client //TODO プロジェクト名は要検討
{
const size_t JSON_SETTING = (JSON_COMPACT | JSON_ENCODE_ANY);

typedef std::function<void(json_t *)> SubCallback_t;

template <typename SOCKET_TYPE>
class ConnectionManager
{
public:
	ConnectionManager(SOCKET_TYPE *socket);
	int  advertise(const std::string topic_name, const std::string type);
	int  unadvertise(const std::string topic_name);

	void subscribe(const std::string topic_name, SubCallback_t cb);
	void publish(const std::string topic_name, json_t &data);
	void publish(int topic_id, json_t &data);
private:
	void _sockCallback(const std::string str);
	crb_sock::SockCallback_t _sockCallback_ref;

	SOCKET_TYPE *_socket;

	int _pub_id_counter;
	int _sub_id_counter;
	std::vector<SubCallback_t > _cb_vec;
	std::unordered_map<std::string, int> _pub_topic_name_to_id;
	std::unordered_map<std::string, int> _sub_topic_name_to_id;
};

template <typename SOCKET_TYPE>
ConnectionManager<SOCKET_TYPE>::ConnectionManager(SOCKET_TYPE *socket):
_socket(socket), _pub_id_counter(0), _sub_id_counter(0)
{
	_sockCallback_ref = [&](const std::string str){return this->_sockCallback(str);};
	_socket->setRecieveCb(_sockCallback_ref);
}

template <typename SOCKET_TYPE>
int ConnectionManager<SOCKET_TYPE>::advertise(const std::string topic_name, const std::string type)
{
	json_t *op_json;
	char *json_str;
	op_json = json_object();
	json_object_set_new(op_json, "op", json_string("advertise"));
	json_object_set_new(op_json, "topic", json_string(topic_name.c_str()));
	json_object_set_new(op_json, "type", json_string(type.c_str()));

	json_str = json_dumps(op_json, JSON_SETTING);
	_socket->sendStr(json_str);
	if(_pub_topic_name_to_id.find(topic_name) == _pub_topic_name_to_id.end())
	{
		_pub_topic_name_to_id[topic_name] = _pub_id_counter++;
	}

	json_decref(op_json);
	free(json_str);
}

template <typename SOCKET_TYPE>
int ConnectionManager<SOCKET_TYPE>::unadvertise(const std::string topic_name)
{
	json_t *op_json;
	char *json_str;
	op_json = json_object();
	json_object_set_new(op_json, "op", json_string("unadvertise"));
	json_object_set_new(op_json, "topic", json_string(topic_name.c_str()));

	json_str = json_dumps(op_json, JSON_SETTING);
	_socket->sendStr(json_str);
	json_decref(op_json);
	free(json_str);
}


template <typename SOCKET_TYPE>
void ConnectionManager<SOCKET_TYPE>::publish(const std::string topic_name, json_t &data)
{
	json_t *op_json;
	char *json_str;
	op_json = json_object();
	json_object_set_new(op_json, "op", json_string("publish"));
	json_object_set_new(op_json, "topic", json_string(topic_name.c_str()));
	json_object_set(op_json, "msg", &data);

	json_str = json_dumps(op_json, JSON_SETTING);
	_socket->sendStr(json_str);
	json_decref(op_json);
	free(json_str);
}
template <typename SOCKET_TYPE>
void ConnectionManager<SOCKET_TYPE>::subscribe(const std::string topic_name, SubCallback_t cb)
{
	json_t *op_json;
	char *json_str;
	op_json = json_object();
	json_object_set_new(op_json, "op", json_string("subscribe"));
	json_object_set_new(op_json, "topic", json_string(topic_name.c_str()));

	json_str = json_dumps(op_json, JSON_SETTING);
	if(_sub_topic_name_to_id.find(topic_name) == _sub_topic_name_to_id.end())
	{
		_sub_topic_name_to_id[topic_name] = _sub_id_counter++;
		//_cb_vec.push_back(&cb);
		_cb_vec.push_back(cb);
	}

	_socket->sendStr(json_str);
	json_decref(op_json);
	free(json_str);
}

/*
template <typename SOCKET_TYPE>
void ConnectionManager<SOCKET_TYPE>::publish(int topic_id, json_t &data)
{
}
// */
template <typename SOCKET_TYPE>
void ConnectionManager<SOCKET_TYPE>::_sockCallback(const std::string str)
{
	//std::cout << "called back with msg" << str << std::endl;
	//return;
	json_error_t err;
	json_t *msg_json;
	json_t *topicname_json;
	json_t *topicbody_json;
	int topic_id;
	msg_json = json_loads(str.c_str(), JSON_DECODE_ANY, &err);
	topicname_json = json_object_get(msg_json, "topic");
	std::string topic_name = json_string_value(topicname_json);
	//*
	if(_sub_topic_name_to_id.find(topic_name) != _sub_topic_name_to_id.end())
	{
		topicbody_json = json_object_get(msg_json, "msg");
		topic_id = _sub_topic_name_to_id[topic_name];
		(_cb_vec[topic_id])(topicbody_json);
		json_decref(topicbody_json);
	}
	// */
	json_decref(topicname_json);
	json_decref(msg_json);
}


/*
template <typename SOCKET_TYPE>
ConnectionManager<SOCKET_TYPE>::
*/

}//namespace crb_client
