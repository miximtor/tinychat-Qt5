//
// Created by maxtorm on 2019/4/23.
//
#pragma once
#include <QWidget>
#include "ui_Chat.h"

namespace tinychat::qt5::ui
{

class chat : public QWidget
{
Q_OBJECT
public:
	explicit chat(QWidget *parent = nullptr) : QWidget(parent)
	{
		ui_.setupUi(this);
		QWidget::setAttribute(Qt::WA_DeleteOnClose);
		connect(ui_.sendButton, &QPushButton::clicked, this, &chat::handle_send_button_clicked);
		connect(this, SIGNAL(show_wd()), this, SLOT(show_window()));
	}

signals:
	
	void message_send(QString message);
	
	void show_wd();

public slots:
	
	void message_receive(QString sender, QString message)
	{
		handle_message_receive(sender, message);
	};
	
	void error(QString what)
	{
	
	}
private slots:
	void show_window()
	{
		show();
	};
	
	void handle_send_button_clicked(bool)
	{
		auto message = ui_.msgInputEdit->toPlainText();
		ui_.msgInputEdit->setPlainText("");
		emit message_send(message);
	};
	
	void handle_message_receive(const QString &sender, const QString &message)
	{
		ui_.msgListWidget->addItem(sender + " says: " + message);
		ui_.msgListWidget->scrollToBottom();
	}

private:
	
	Ui::ChatWidget ui_{};
};

}
