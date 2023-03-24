# CppRosBridge_demo
ROSBridgeを使って通信を行うC++用ライブラリのデモ
# 構成
## inc/crb_client.hpp
ROSのトピック管理などを行うライブラリ本体
## inc/socket_wrapper.hpp
ソケット通信のラッパーインターフェース
## inc/wspp_wrapper.hpp, inc/wspp_wrapper.cpp
websocket通信ライブラリのラッパー  
中身はwebsocketppを使用

# 依存ライブラリ
- [Jansson](https://jansson.readthedocs.io/en/2.11/index.html)
- [websocketpp](https://github.com/zaphoyd/websocketpp)
 