#pragma once
#include <socket_wrapper.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>

namespace crb_sock
{
typedef websocketpp::client<websocketpp::config::asio_client> wspp_client_t;
class WsppWrapper : public SocketWrapper
{
public:
	WsppWrapper();
	~WsppWrapper();
	int connect(std::string const &uri);

	int sendStr(const std::string &str);
	int setRecieveCb(SockCallback_t cb);

private:
	SockCallback_t _callbk;
	//std::function<void(websocketpp::connection_hdl, wspp_client_t::message_ptr)> _on_message;
	void _on_message(websocketpp::connection_hdl hdl_, wspp_client_t::message_ptr msg_);
	wspp_client_t _client_ep;
	wspp_client_t::connection_ptr _con;
	websocketpp::connection_hdl hdl;
	websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
};

}
