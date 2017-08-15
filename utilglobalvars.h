#ifndef UTILGLOBALVARS_H
#define UTILGLOBALVARS_H

namespace GlobalVars{

static const QString AppName = "FRequest";
static const QString AppVersion = "1.0";
static const QString LastCompatibleVersion = "1.0";
static const QString AppConfigFileName = AppName + ".cfg";
static const QString AppLogFileName = AppName.toLower() + ".log";
static constexpr int AppRecentProjectsMaxSize=6;

}

#endif // UTILGLOBALVARS_H
