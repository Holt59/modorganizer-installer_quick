#include "installerquick.h"
#include "simpleinstalldialog.h"
#include <installationtester.h>

#include <QtPlugin>
#include <QDialog>


using namespace MOBase;


InstallerQuick::InstallerQuick()
  : m_MOInfo(nullptr)
{
}

bool InstallerQuick::init(IOrganizer *moInfo)
{
  m_MOInfo = moInfo;
  return true;
}

QString InstallerQuick::name() const
{
  return "Simple Installer";
}

QString InstallerQuick::author() const
{
  return "Tannin";
}

QString InstallerQuick::description() const
{
  return tr("Installer for very simple archives");
}

VersionInfo InstallerQuick::version() const
{
  return VersionInfo(1, 3, 0, VersionInfo::RELEASE_FINAL);
}

bool InstallerQuick::isActive() const
{
  return m_MOInfo->pluginSetting(name(), "enabled").toBool();
}

QList<PluginSetting> InstallerQuick::settings() const
{
  QList<PluginSetting> result;
  result.push_back(PluginSetting("enabled", "check to enable this plugin", QVariant(true)));
  result.push_back(PluginSetting("silent", "simple plugins will be installed without any user interaction", QVariant(false)));
  return result;
}

unsigned int InstallerQuick::priority() const
{
  return 50;
}


bool InstallerQuick::isManualInstaller() const
{
  return false;
}


bool InstallerQuick::isSimpleArchiveTopLayer(const DirectoryTree::Node *node) const
{
  // see if there is at least one directory that makes sense on the top level
  for (DirectoryTree::const_node_iterator iter = node->nodesBegin(); iter != node->nodesEnd(); ++iter) {
    if (InstallationTester::isTopLevelDirectory((*iter)->getData().name)) {
      qDebug("%s on the top level", (*iter)->getData().name.toUtf8().constData());
      return true;
    }
  }

  // see if there is a file that makes sense on the top level
  for (DirectoryTree::const_leaf_iterator iter = node->leafsBegin(); iter != node->leafsEnd(); ++iter) {
    if (InstallationTester::isTopLevelSuffix(iter->getName())) {
      return true;
    }
  }

  return false;
}


bool InstallerQuick::isDataTextArchiveTopLayer(const DirectoryTree::Node *node) const
{
  // a "DataText" archive is defined as having exactly one folder named data
  // and one or more text or PDF files (standard package from french modding site).
  if ((node->numNodes() == 1) &&
      (node->numLeafs() >= 1) &&
      ((*node->nodesBegin())->getData().name.toQString().toLower() == "data")) {
    for (DirectoryTree::const_leaf_iterator iter = node->leafsBegin(); iter != node->leafsEnd(); ++iter) {
      if (!(iter->getName().endsWith(".txt") || iter->getName().endsWith(".pdf")))
      {
        return false;
      }
    }
    return true;
  }
  return false;
}


const DirectoryTree::Node *InstallerQuick::getSimpleArchiveBase(const DirectoryTree &dataTree) const
{
  const DirectoryTree::Node *currentNode = &dataTree;

  while (true) {
    if (isSimpleArchiveTopLayer(currentNode) ||
        isDataTextArchiveTopLayer(currentNode)) {
      return currentNode;
    } else if ((currentNode->numLeafs() == 0) &&
               (currentNode->numNodes() == 1)) {
      currentNode = *currentNode->nodesBegin();
    } else {
      qDebug("not a simple archive");
      return nullptr;
    }
  }
}


bool InstallerQuick::isArchiveSupported(const DirectoryTree &tree) const
{
  const DirectoryTree::Node *baseNode = getSimpleArchiveBase(tree);
  return baseNode != nullptr;
}


IPluginInstaller::EInstallResult InstallerQuick::install(GuessedValue<QString> &modName, DirectoryTree &tree,
                                                         QString&, int&)
{
  const DirectoryTree::Node *baseNode = getSimpleArchiveBase(tree);
  if (baseNode != nullptr) {
    SimpleInstallDialog dialog(modName, parentWidget());
    if (m_MOInfo->pluginSetting(name(), "silent").toBool() || dialog.exec() == QDialog::Accepted) {
      modName.update(dialog.getName(), GUESS_USER);
      tree = *(baseNode->copy()); // need to make a copy because baseNode points inside tree
      if (isDataTextArchiveTopLayer(&tree)) {
        // move the text files to the data folder and set the data folder as the baseNode
        // guarenteed there is only one node at this time
        for (DirectoryTree::const_leaf_iterator iter = tree.leafsBegin(); iter != tree.leafsEnd(); ++iter) {
          (*tree.nodesBegin())->addLeaf(*iter);
        }
        tree = *((*tree.nodesBegin())->copy());
      }
      return RESULT_SUCCESS;
    } else {
      if (dialog.manualRequested()) {
        modName.update(dialog.getName(), GUESS_USER);
        return RESULT_MANUALREQUESTED;
      } else {
        return RESULT_CANCELED;
      }
    }
  } else {
    // install shouldn't even have even have been called
    qCritical("unsupported archive for quick installer");
    return RESULT_FAILED;
  }
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
Q_EXPORT_PLUGIN2(installerQuick, InstallerQuick)
#endif
