//
// Created by maxtorm on 2019/4/23.
//
#pragma once
#include <optional>

#include <QWidget>
#include "ui_Login.h"

namespace tinychat::qt5::ui
{
class login : public QWidget
{
Q_OBJECT
public:
	struct Attribute
	{
		std::string server = "127.0.0.1";
		std::string port = "8000";
		std::string username;
		std::string password;
	};
	
	explicit login(QWidget *parent = nullptr) : QWidget(parent)
	{
		ui_.setupUi(this);
		QWidget::setAttribute(Qt::WA_DeleteOnClose);
		connect(ui_.loginButton, &QPushButton::clicked, this, &login::handle_login_button_clicked);
		connect(this,&login::login_end,this,&login::handle_login_end);
	}
	
signals:
	void login_start(Attribute);
	void login_end(QString);
private slots:
	void handle_login_end(QString error);
	void handle_login_button_clicked(bool);
private:
	Ui::TinyChatLoginWidget ui_{};
};

}
