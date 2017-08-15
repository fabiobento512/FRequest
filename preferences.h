#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QSettings>
#include <QMessageBox>
#include <QShowEvent>

#include "util.h"
#include "configfilefrequest.h"

namespace Ui {
class Preferences;
}

class Preferences : public QDialog
{
    Q_OBJECT
    
public:
    Preferences(QWidget *parent, ConfigFileFRequest::Settings &currentSettings);
    ~Preferences();

public slots:
    void accept ();
    
private slots:

    void on_buttonBox_rejected();

    void on_cbUseDefaultHeaders_toggled(bool checked);

    void on_cbRequestType_currentIndexChanged(const QString &arg1);

    void on_tbRequestBodyKeyValueAdd_clicked();

    void on_tbRequestBodyKeyValueRemove_clicked();

    void on_cbRequestBodyType_currentIndexChanged(const QString &arg1);
    void preferencesHaveLoaded();

    void on_cbProxyUseProxy_toggled(bool checked);

    void on_cbProxyType_currentIndexChanged(const QString &arg1);

signals:
    void signalPreferencesAreLoaded();
    void saveSettings();

private:
    Ui::Preferences *ui;
    ConfigFileFRequest::Settings &currentSettings;
    QString previousRequestType = "GET"; // by default
    QString previousRequestBodyType = "raw";
    bool preferencesAreFullyLoaded = false;

private:
    void showEvent(QShowEvent *e);
    void loadExistingSettings();
    std::experimental::optional<QVector<UtilFRequest::HttpHeader> > getRequestHeaders();
    void loadCurrentDefaultHeaders();
    void updateCurrentDefaultHeaders();
};

#endif // PREFERENCES_H
