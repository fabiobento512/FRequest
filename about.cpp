/*
 *
Copyright (C) 2017  Fábio Bento (random-guy)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose,true ); //destroy itself once finished.

    ui->lbAbout->setText("<html>"
                         "<p style='font-size:x-large;'><b>" + GlobalVars::AppName + " " + GlobalVars::AppVersion + "</b></p>"
                         "<p style='font-size:large;line-height: 18px;'>"
                         "Written by Fábio Bento <a href='https://github.com/random-guy'>(random-guy)</a><br /><br/>"
                         "Build Date " + __DATE__ + " " + __TIME__ + "<br /><br />"
                         "Thanks to:<br/>"
                         "Arseny Kapoulkine and the remaining contributors for pugixml library<br />"
                         "Andrzej Krzemienski and the remaining contributors for C++14 optional library<br />"
                         "Sergey Podobry and the remaining contributors for plog library<br />"
                         "Murakumon for project folder icon (found in findicons.com)<br />"
                         "Woothemes for application icon (found in findicons.com)<br />"
                         "ana nirwana for send request icon (found in iconfinder.com)<br />"
                         "Omercetin for clipboard icon (found in iconfinder.com) <a href='https://creativecommons.org/licenses/by/3.0/'>(license)</a>"
                         + R"(<hr/>
                         This program is free software: you can redistribute it and/or modify
                         it under the terms of the GNU General Public License as published by
                         the Free Software Foundation, either version 3 of the License, or
                         (at your option) any later version.

                         This program is distributed in the hope that it will be useful,
                         but WITHOUT ANY WARRANTY; without even the implied warranty of
                         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
                         GNU General Public License for more details.

                         You should have received a copy of the GNU General Public License
                         along with this program.  If not, see <a href='http://www.gnu.org/licenses/'>http://www.gnu.org/licenses/</a>.
                         )" +
                         "</p>"
                         "</html>"); // Don't use rich text in qtdesigner because it generates platform dependent code

    ui->lbCommunity->setText("<html>"
                                "<p style='font-size:large;'>"
                                "<center>"
                                "Visit FRequest <a href='https://random-guy.github.io/FRequest/'>website</a><br />"
                                "or the <a href='https://github.com/random-guy/FRequest'>github project</a><br />"
                                "</center>"
                                "</p>"
                                "</html>"
                                );

}

About::~About()
{
    delete ui;
}

void About::on_pushButton_clicked()
{
    this->close();
}
