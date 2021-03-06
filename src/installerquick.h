#ifndef INSTALLERQUICK_H
#define INSTALLERQUICK_H

#include <iplugininstallersimple.h>

class InstallerQuick : public MOBase::IPluginInstallerSimple
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginInstaller MOBase::IPluginInstallerSimple)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  Q_PLUGIN_METADATA(IID "org.tannin.InstallerQuick" FILE "installerquick.json")
#endif

public:

  InstallerQuick();

  virtual bool init(MOBase::IOrganizer *moInfo);
  virtual QString name() const;
  virtual QString author() const;
  virtual QString description() const;
  virtual MOBase::VersionInfo version() const;
  virtual bool isActive() const;
  virtual QList<MOBase::PluginSetting> settings() const;

  virtual unsigned int priority() const;
  virtual bool isManualInstaller() const;

  virtual bool isArchiveSupported(const MOBase::DirectoryTree &tree) const;
  virtual EInstallResult install(MOBase::GuessedValue<QString> &modName, MOBase::DirectoryTree &tree,
                                 QString &version, int &modID);

private:

  bool isSimpleArchiveTopLayer(const MOBase::DirectoryTree::Node *node) const;
  bool isDataTextArchiveTopLayer(const MOBase::DirectoryTree::Node *node) const;
  const MOBase::DirectoryTree::Node *getSimpleArchiveBase(const MOBase::DirectoryTree &dataTree) const;

private:

  const MOBase::IOrganizer *m_MOInfo;

};

#endif // INSTALLERQUICK_H
