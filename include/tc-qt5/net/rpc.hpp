#pragma once
#include <memory>
#include <QObject>
#include <boost/noncopyable.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <tinyrpc/rpc_websocket_service.hpp>
#include "chat.pb.h"

namespace tc_qt5
{
namespace net = boost::asio;
namespace sys = boost::system;

class rpc : public QObject
{
Q_OBJECT
public:
	
	using Stream = boost::beast::websocket::stream<net::ip::tcp::socket>;
	
	rpc(net::io_context &ioc, Stream stream) :
		ioc_(ioc),
		stream_(std::move(stream)),
		rpc_service_(stream_)
	{}
	
	void start_chat()
	{
		using request = chat::NotifyChatMessageRequest;
		using reply = chat::NotifyChatMessageReply;
		rpc_service_.rpc_bind<request,reply>(
			[this](const request &req,reply &rep)
			{
				message_received(req,rep);
			});
		spawn(ioc_,[this](net::yield_context yield){dispatch(yield);});
		spawn(ioc_,[this](net::yield_context yield){heartbeat(yield);});
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

signals:
	void message_receive(QString sender, QString message);
	void error(QString what);
public slots:
	void message_send(QString message)
	{
		net::spawn(ioc_,[this,message{std::move(message)}](net::yield_context yield) mutable
		{
			message_deliver(std::move(message),yield);
		});
	};
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
			emit error(QString("dispatch: ") + e.what());
		}
	}
	
	void heartbeat(net::yield_context yield)
	{
		net::steady_timer timer(ioc_);
		chat::VerifyRequest v_req;
		v_req.set_name(name_);
		v_req.set_token(token_);
		QString what;
		try
		{
			while (true)
			{
				timer.expires_after(std::chrono::minutes(1));
				timer.async_wait(yield);
				chat::VerifyReply v_reply;
				rpc_service_.async_call(v_req, v_reply, yield);
				if (!v_reply.ok())
				{
					what = "server replied not ok";
					break;
				}
			}
		}
		catch (sys::system_error &e)
		{
			what = QString("heartbeat: ") + e.what();
		}
		emit error(what);
	}
	
	void message_received(const chat::NotifyChatMessageRequest &req, chat::NotifyChatMessageReply &reply)
	{
		QString sender = QString::fromStdString(req.chat_message().sender());
		QString message = QString::fromStdString(req.chat_message().text());
		emit message_receive(sender, message);
	}
	
	void message_deliver(QString message, net::yield_context yield)
	{
		chat::ChatSendRequest v_req;
		{
			v_req.set_name(name_);
			v_req.set_token(token_);
			v_req.set_text(message.toStdString());
		}
		
		chat::ChatSendReply v_reply;
		
		rpc_service_.async_call(v_req, v_reply, yield);
		if (v_reply.result() != chat::ChatSendReply::ok)
		{
			emit error("message send failed");
		}
	}
	
	net::io_context &ioc_;
	Stream stream_;
	tinyrpc::rpc_websocket_service<Stream> rpc_service_;
	std::string name_;
	std::string token_;
};
	
}