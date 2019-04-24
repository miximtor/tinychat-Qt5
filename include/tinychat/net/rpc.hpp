#pragma once
#include <memory>
#include <QObject>
#include <boost/noncopyable.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <tinyrpc/rpc_websocket_service.hpp>
#include "chat.pb.h"

namespace tinychat::qt5::net
{

namespace net = boost::asio;
namespace sys = boost::system;
namespace websocket = boost::beast::websocket;

class rpc : public QObject
{
Q_OBJECT
public:
	using stream_type = boost::beast::websocket::stream<net::ip::tcp::socket>;
	rpc(net::io_context &ioc, stream_type stream) :
		ioc_(ioc),
		stream_(std::move(stream)),
		rpc_service_(stream_)
	{
		net::spawn(ioc_,
		           [this](net::yield_context yield)
		           { dispatch(yield); });
	}
	
	void start_chatting()
	{
		using request = chat::NotifyChatMessageRequest;
		using reply = chat::NotifyChatMessageReply;
		rpc_service_.rpc_bind<request, reply>(
			[this](const request &req, reply &rep)
			{
				message_received(req, rep);
			});
		net::spawn(ioc_,
		           [this](net::yield_context yield)
		           { heartbeat(yield); });
		
	}
	
	chat::LoginReply::statetype login(std::string username, std::string password, net::yield_context yield)
	{
		chat::LoginRequest req;
		req.set_name(std::move(username));
		req.set_auth(std::move(password));
		chat::LoginReply rep;
		rpc_service_.async_call(req, rep, yield);
		if (rep.state() == chat::LoginReply::ok)
		{
			token_ = rep.token();
			name_ = req.name();
		}
		return rep.state();
	}
	
	void deliver_message(QString message)
	{
		net::spawn(ioc_, [this, message{std::move(message)}](net::yield_context yield) mutable
		{
			chat::ChatSendRequest v_req;
			{
				v_req.set_name(name_);
				v_req.set_token(token_);
				v_req.set_text(message.toStdString());
			}
			
			chat::ChatSendReply v_reply;
			sys::error_code ec;
			rpc_service_.async_call(v_req, v_reply, yield[ec]);
			if (ec)
			{
				std::cerr << "message send: " << ec.message() << std::endl;
				emit error(QString::fromStdString(ec.message()));
				ws_terminate();
				return;
			}
			if (v_reply.result() != chat::ChatSendReply::ok)
			{
				emit error("message send failed");
				ws_terminate();
				return;
			}
		});
	}
signals:
	void message_receive(QString sender, QString message);
	void error(QString);
private:
	
	
	void dispatch(net::yield_context yield)
	{
		try
		{
			boost::beast::multi_buffer buf;
			while (true)
			{
				auto bytes = stream_.async_read(buf, yield);
				rpc_service_.dispatch(buf);
				buf.consume(bytes);
			}
		}
		catch (sys::system_error &e)
		{
			std::cerr << "dispatch: " << e.what() << std::endl;
			emit error(QString("dispatch: ") + e.what());
		}
		ws_terminate();
	}
	
	void heartbeat(net::yield_context yield)
	{
		QString what;
		try
		{
			net::steady_timer timer(ioc_);
			chat::VerifyRequest v_req;
			v_req.set_name(name_);
			v_req.set_token(token_);
			while (true)
			{
				timer.expires_after(std::chrono::minutes(1));
				timer.async_wait(yield);
				chat::VerifyReply v_reply;
				rpc_service_.async_call(v_req, v_reply, yield);
				if (!v_reply.ok())
				{
					what = "heartbeat: server replied not ok";
					break;
				}
			}
		}
		catch (sys::system_error &e)
		{
			what = QString("heartbeat: ") + e.what();
		}
		std::cerr << what.toStdString() << std::endl;
		emit error(what);
		ws_terminate();
	}
	
	void message_received(const chat::NotifyChatMessageRequest &req, chat::NotifyChatMessageReply &reply)
	{
		QString sender = QString::fromStdString(req.chat_message().sender());
		QString message = QString::fromStdString(req.chat_message().text());
		emit message_receive(sender, message);
	}
	
	void ws_terminate()
	{
		if (stream_.is_open())
		{
			stream_.close({});
		}
	}

private:
	net::io_context &ioc_;
	stream_type stream_;
	tinyrpc::rpc_websocket_service<stream_type> rpc_service_;
	
	std::string name_{};
	std::string token_{};
};

}