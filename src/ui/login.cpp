//
// Created by maxtorm on 2019/4/23.
//

#include <tinychat/ui/login.hpp>

namespace tinychat::qt5::ui
{

void login::handle_login_button_clicked(bool)
{
	auto serverText = ui_.serverEdit->text();
	auto portText = ui_.portEdit->text();
	auto usernameText = ui_.usernameEdit->text();
	auto passwordText = ui_.passwordEdit->text();
	
	Attribute attribute;
	{
		attribute.server = serverText.toStdString();
		attribute.port = portText.toStdString();
		attribute.username = usernameText.toStdString();
		attribute.password = passwordText.toStdString();
	}
	ui_.loginButton->setEnabled(false);
	emit login_start(attribute);
}

void login::handle_login_end(QString error)
{
	if (!error.isEmpty())
	{
		ui_.errorHintLabel->setText(error);
		ui_.loginButton->setEnabled(true);
	}
	else
	{
		close();
	}
}
}