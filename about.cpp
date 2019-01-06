/*
 *
Copyright (C) 2017-2019  Fábio Bento (fabiobento512)

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
                         "Written by Fábio Bento <a href='https://github.com/fabiobento512'>(fabiobento512)</a><br /><br/>"
                         "Build Date " + __DATE__ + " " + __TIME__ + "<br /><br />"
                         "Thanks to:<br/><br/>"
                         "Arseny Kapoulkine and the remaining contributors for pugixml library<br />"
                         "Andrzej Krzemienski and the remaining contributors for C++14 optional library<br />"
                         "Sergey Podobry and the remaining contributors for plog library<br />"
                         "Jane G. (isomoar) for the JSON syntax highlighter (from SchemaBasedJSONEditor)<br />"
                         "Dmitry Ivanov for the XML syntax highlighter (from SchemaBasedJSONEditor)<br />"
						 "Jürgen Skrotzky (Jorgen-VikingGod) for the Dark Theme<br />"
                         "<br/>"
                         "Murakumon for project folder icon (found in findicons.com)<br />"
                         "Woothemes for application icon (found in findicons.com)<br />"
                         "ana nirwana for send request icon (found in iconfinder.com)<br />"
                         "Omercetin for clipboard icon (found in iconfinder.com) <a href='https://creativecommons.org/licenses/by/3.0/'>(license)</a><br />"
                         "Icons8 for clone icon (found in <a href='https://icons8.com/'>here</a>) <a href='https://creativecommons.org/licenses/by-nd/3.0/'>(license)</a><br />"
                         "Aha-Soft Team for abort icon (found in findicons.com)<br />"
                         "FatCow Web Hosting for warning icon (found in iconfinder.com) <a href='http://creativecommons.org/licenses/by/3.0/us/'>(license)</a><br />"
						 "Icons8 for delete icon (found in <a href='https://icons8.com/'>here</a>) <a href='https://creativecommons.org/licenses/by-nd/3.0/'>(license)</a>"
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
                                "Visit FRequest <a href='https://fabiobento512.github.io/FRequest/'>website</a><br />"
                                "or the <a href='https://github.com/fabiobento512/FRequest'>github project</a><br />"
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
