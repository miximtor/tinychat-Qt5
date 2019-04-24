#include <memory>

#include <QApplication>

#include <thread>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>

#include <tinychat/ui/login.hpp>
#include <tinychat/ui/chat.hpp>
#include <tinychat/net/rpc.hpp>

namespace ui = tinychat::qt5::ui;
namespace net = boost::asio;
namespace sys = boost::system;

using rpc = tinychat::qt5::net::rpc;

ui::login *loginInterface;
ui::chat *chatInterface;
rpc *session;

void login_start(net::io_context &ioc, ui::login::Attribute attribute);

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	
	net::io_context ioc;
	auto work_guard = net::make_work_guard(ioc);
	
	std::thread io_thread(
		[&ioc]()
		{ ioc.run(); }
	);
	
	loginInterface = new ui::login;
	chatInterface = new ui::chat;
	
	QObject::connect(
		loginInterface, &ui::login::login_start,
		[&ioc](ui::login::Attribute attribute)
		{
			login_start(ioc, std::move(attribute));
		});
	
	loginInterface->show();
	auto error_code = QApplication::exec();
	return error_code;
}

void login_start(net::io_context &ioc, ui::login::Attribute attribute)
{
	spawn(
		ioc,
		[&ioc, attribute](net::yield_context yield)
		{
			rpc::stream_type stream(ioc);
			net::ip::tcp::resolver resolver(ioc);
			auto endpoints = resolver.async_resolve(attribute.server, attribute.port, yield);
			net::async_connect(stream.next_layer(), endpoints.begin(), endpoints.end(), yield);
			stream.async_handshake(attribute.server, "/tinychat", yield);
			stream.binary(true);
			session = new rpc(ioc, std::move(stream));
			
			auto reply = session->login(attribute.username, attribute.password, yield);
			
			switch (reply)
			{
				case (chat::LoginReply::ok): emit loginInterface->login_end({});
					emit chatInterface->show_wd();
					break;
				case (chat::LoginReply::not_registered): emit loginInterface->login_end({"not registered"});
					return;
				case (chat::LoginReply::auth_failed): emit loginInterface->login_end({"auth failed"});
					return;
				case (chat::LoginReply::duplicate_login): emit loginInterface->login_end({"duplicate_login"});
					return;
				case (chat::LoginReply::banned): emit loginInterface->login_end({"banned"});
					return;
				default: break;
			}
			QObject::connect(session, &rpc::message_receive, chatInterface, &ui::chat::message_receive);
			QObject::connect(session, &rpc::error, chatInterface, &ui::chat::error);
			QObject::connect(
				chatInterface, &ui::chat::message_send,
				[](QString message)
				{
					session->deliver_message(message);
				});
			session->start_chatting();
		});
}



