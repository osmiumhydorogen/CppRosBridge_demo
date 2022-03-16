#pragma once
#include <string>
#include <functional>
namespace crb_sock
{
typedef std::function<void(const std::string)> SockCallback_t;

class SocketWrapper
{
public:
	//TODO 返り値の方針の決定
	// 案1:では成功したら0以上の値を返す
	int sendStr(const std::string &str){}
	int setRecieveCb(SockCallback_t cb){}
};

} //namespace crb_sock
