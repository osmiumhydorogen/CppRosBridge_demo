#include <wspp_wrapper.hpp>

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
        //////////////////////////////////////////////// kuriyama
        con->set_open_handler(websocketpp::lib::bind(
            &WsppWrapper::_on_open,
            this,
            &_client_ep,
            websocketpp::lib::placeholders::_1
        ));
        con->set_fail_handler(websocketpp::lib::bind(
            &WsppWrapper::_on_fail,
            this,
            &_client_ep,
            websocketpp::lib::placeholders::_1
        ));
        ///////////////////////////////////////////////////
		
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
		if (msg_->get_opcode() == websocketpp::frame::opcode::text)
		{
            this->_callbk(msg_->get_payload());
        }
	}

    ///////////////////////////////////////////// kuriyama
	void WsppWrapper::_on_fail(wspp_client_t *c, websocketpp::connection_hdl hdl_)
	{
        std::cout << "> ERROR: Connection Failed" << std::endl;
        wspp_client_t::connection_ptr con = c->get_con_from_hdl(hdl_);
        std::cout << con->get_ec().message() << std::endl;

        m_status = "Failed";
	}
	void WsppWrapper::_on_open(wspp_client_t *c, websocketpp::connection_hdl hdl_)
	{
        std::cout << "> Status: Connection Succeeded" << std::endl;
        m_status = "Open";
	}

    void WsppWrapper::close(){
        websocketpp::lib::error_code ec;
        _client_ep.close(hdl, websocketpp::close::status::normal, "", ec);
	    if (ec) {
	    	std::cout << "> Error closing connection " << ": " << ec.message() << std::endl;
	    }else{
            std::cout << "> Status: Connection Closed." << std::endl;
            m_status = "Closed";
        }
    }
    ///////////////////////////////////////////


	int WsppWrapper::sendStr(const std::string &str)
	{
		websocketpp::lib::error_code ec;
		_client_ep.send(hdl, str, websocketpp::frame::opcode::text, ec);
	}
	int WsppWrapper::setRecieveCb(SockCallback_t cb)
	{
		_callbk = cb;
	}

	WsppWrapper::~WsppWrapper()
	{
        _client_ep.stop_perpetual();
        if (m_status == "Open"){
    		websocketpp::lib::error_code ec;
            _client_ep.close(hdl, websocketpp::close::status::going_away, "", ec);
		    if (ec) {
		    	std::cout << "> Error closing connection " << ": " << ec.message() << std::endl;
		    }else{
                std::cout << "> Status: Connection Closed by destructor." << std::endl;
            }
        }
        m_thread->join();
	}
}
