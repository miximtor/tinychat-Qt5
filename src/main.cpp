#include <memory>

#include <QApplication>

#include <thread>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>

#include <tc-qt5/ui/Login.hpp>
#include <tc-qt5/ui/Chat.hpp>
#include <tc-qt5/net/rpc.hpp>

#include <iostream>

namespace tc = tc_qt5;
namespace net = boost::asio;
namespace sys = boost::system;

using rpc_type = tc::rpc;
using std::placeholders::_1;

tc::Login * loginInterface;
tc::Chat * chatInterface;
rpc_type* rpc;


void login_start(net::io_context &ioc,tc::Login::Attribute attribute);

int main(int argc, char* argv[])
{
	QApplication application(argc, argv);
	net::io_context ioc;
	auto work_guard = net::make_work_guard(ioc);
	std::thread io_thread([&ioc]() { ioc.run(); });

	loginInterface = new tc::Login;
	chatInterface = new tc::Chat;
	
	QObject::connect(loginInterface,&tc::Login::login_start,std::bind(&login_start,std::ref(ioc),_1));
	
	loginInterface->show();
	auto error_code = QApplication::exec();
	return error_code;
}

void login_start(net::io_context &ioc,tc::Login::Attribute attribute)
{
	spawn(ioc,
		[&ioc,attribute{std::move(attribute)}](net::yield_context yield)
		{
			rpc_type::Stream stream(ioc);
			net::ip::tcp::resolver resolver(ioc);
			auto endpoints = resolver.async_resolve(attribute.server,attribute.port,yield);
			net::async_connect(stream.next_layer(),endpoints.begin(),endpoints.end(),yield);
			stream.async_handshake(attribute.server,"/tinychat",yield);
			stream.binary(true);
			rpc = new rpc_type(ioc,std::move(stream));
			auto reply = rpc->login(attribute.username,attribute.password,yield);
			
			switch (reply)
			{
				case (chat::LoginReply::ok):
					break;
				case (chat::LoginReply::not_registered):
					break;
				case (chat::LoginReply::auth_failed):
					break;
				case (chat::LoginReply::duplicate_login):
					break;
				case (chat::LoginReply::banned):
					break;
				default:
					break;
			}
			
			QObject::connect(rpc,&rpc_type::message_receive,chatInterface,&tc::Chat::message_receive);
			QObject::connect(rpc,&rpc_type::error,chatInterface,&tc::Chat::error);
			QObject::connect(chatInterface,&tc::Chat::message_send,rpc,&rpc_type::message_send);
			
			chatInterface->show();
			loginInterface->close();
			
			rpc->start_chat();
		});
}



