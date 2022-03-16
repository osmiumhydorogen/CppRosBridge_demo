#include <wspp_wrapper.hpp>
//#include <websocketpp/common/memory.hpp>

namespace crb_sock
{

	WsppWrapper::WsppWrapper()
	{
		_client_ep.clear_access_channels(websocketpp::log::alevel::all);
		_client_ep.clear_error_channels(websocketpp::log::elevel::all);
		_client_ep.init_asio();
		_client_ep.start_perpetual();

		m_thread.reset(new websocketpp::lib::thread(&wspp_client_t::run, &_client_ep));
	}

	int WsppWrapper::connect(std::string const &uri)
	{
		websocketpp::lib::error_code ec;

		wspp_client_t::connection_ptr con = _client_ep.get_connection(uri, ec);
		_con = con;

		if (ec) {
			std::cout << "> Connect initialization error: " << ec.message() << std::endl;
			return -1;
		}
		/*
		_on_message = [&](websocketpp::connection_hdl hdl_, wspp_client_t::message_ptr msg_)
		{
			std::cout << "Something resieved." << std::endl;
			if (msg_->get_opcode() == websocketpp::frame::opcode::text)
			{
        this->_callbk(msg_->get_payload());
	    }
		};
		// */
		con->set_message_handler(websocketpp::lib::bind(
	    &WsppWrapper::_on_message,
	    this,
	    websocketpp::lib::placeholders::_1,
	    websocketpp::lib::placeholders::_2
		));
		hdl=con->get_handle();
		_client_ep.connect(con);
	}

	void WsppWrapper::_on_message(websocketpp::connection_hdl hdl_, wspp_client_t::message_ptr msg_)
	{
		//std::cout << "Something recieved." << std::endl;
		if (msg_->get_opcode() == websocketpp::frame::opcode::text)
		{
      this->_callbk(msg_->get_payload());
    }
	}

	int WsppWrapper::sendStr(const std::string &str)
	{
		websocketpp::lib::error_code ec;
		//std::cout <<"sending:" << str <<std::endl;
		_client_ep.send(hdl, str, websocketpp::frame::opcode::text, ec);
	}
	int WsppWrapper::setRecieveCb(SockCallback_t cb)
	{
		_callbk = cb;
	}

	WsppWrapper::~WsppWrapper()
	{
		/*
		websocketpp::lib::error_code ec;
		_client_ep.close(hdl, websocketpp::close::status::going_away, "", ec);
		if (ec) {
			std::cout << "> Error closing connection " << ": "
				<< ec.message() << std::endl;
		}
		m_thread->join();
		// */
	}
}
