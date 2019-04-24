//
// Created by maxtorm on 2019/4/23.
//
#pragma once
#include <QWidget>
#include "ui_Chat.h"
namespace tc_qt5
{

class Chat : public QWidget
{
	Q_OBJECT
public:
	explicit Chat(QWidget *parent = nullptr) : QWidget(parent)
	{
		ui_.setupUi(this);
		QWidget::setAttribute(Qt::WA_DeleteOnClose);
		connect(ui_.sendButton,&QPushButton::clicked,this,&Chat::handle_send_button_clicked);
		connect(this,&Chat::message_receive,this,&Chat::handle_message_receive);
	}
	
signals:
	
	void message_send(QString message);
	
public slots:
	void message_receive(QString sender,QString message)
	{
		handle_message_receive(sender,message);
	};

	void error(QString what)
	{
	
	}
private slots:
	
	void handle_send_button_clicked(bool)
	{
		auto message = ui_.msgInputEdit->toPlainText();
		ui_.msgInputEdit->setPlainText("");
		emit message_send(message);
	};

	void handle_message_receive(const QString &sender,const QString &message)
	{
		ui_.msgListWidget->addItem(sender+" says: "+message);
		ui_.msgListWidget->scrollToBottom();
	}

private:
	
	Ui::ChatWidget ui_{};
};

}
