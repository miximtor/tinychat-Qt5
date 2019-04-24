//
// Created by maxtorm on 2019/4/23.
//
#pragma once
#include <optional>

#include <QWidget>
#include "ui_Login.h"

namespace tc_qt5
{

class Login : public QWidget
{
	Q_OBJECT
public:
	struct Attribute
	{
		std::string server = "127.0.0.1";
		std::string port = "8000";
		std::string username ;
		std::string password;
	};
	explicit Login(QWidget* parent = nullptr) : QWidget(parent)
	{
		ui_.setupUi(this);
		QWidget::setAttribute(Qt::WA_DeleteOnClose);
		QObject::connect(ui_.loginButton, &QPushButton::clicked, this, &Login::handle_login_button_clicked);
	}
signals:
	void login_start(Attribute);
private slots:
	void handle_login_button_clicked(bool);
private:
	Ui::TinyChatLoginWidget ui_{};
};

}
