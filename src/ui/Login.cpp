//
// Created by maxtorm on 2019/4/23.
//

#include <tc-qt5/ui/Login.hpp>

namespace tc_qt5
{

void Login::handle_login_button_clicked(bool)
{
	Attribute attribute;
	{
		attribute.server = ui_.serverEdit->text().toStdString();
		attribute.port =  ui_.portEdit->text().toStdString();
		attribute.username = ui_.usernameEdit->text().toStdString();
		attribute.password = ui_.passwordEdit->text().toStdString();
	}
	emit login_start(std::move(attribute));
}
}