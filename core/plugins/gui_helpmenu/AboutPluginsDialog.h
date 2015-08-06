#pragma once

#include <QDialog>
#include <QIcon>
#include <QTextEdit>
#include <QSpacerItem>

class QLabel;
class QPushButton;
class QStringList;
class QTreeWidget;
class QTreeWidgetItem;
class QGroupBox;

class PluginDialog : public QDialog
{
    Q_OBJECT

public:
  PluginDialog(const QString &path, const QStringList &fileNames,QWidget *parent = 0);

private:
  void populateTreeWidget(const QString &path, const QStringList &fileNames);
  void addItems(QTreeWidgetItem *pluginItem, const QStringList &features);

  QLabel *label;
  QTreeWidget *treeWidget;
  QLabel * labelInfo;
	QPushButton *okButton;
  QIcon interfaceIcon;
	QIcon featureIcon;
	QSpacerItem *spacerItem;
	QGroupBox *groupBox;
//	QGroupBox *groupBox;
	QString pathDirectory;
public slots:
	void displayInfo(QTreeWidgetItem* item,int ncolumn);

};
