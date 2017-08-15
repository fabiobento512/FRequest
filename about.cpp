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
                         "Omercetin for clipboard icon (found in iconfinder.com) <a href='https://creativecommons.org/licenses/by/3.0/'>(license)</a><br />"
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
