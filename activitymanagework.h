﻿#pragma once
#pragma execution_character_set("utf-8")

#include <QObject>
#include "querymodel.h"
#include "service.h"

class ActivityManageWork : public QObject
{
	Q_OBJECT

public:
	ActivityManageWork(QObject *parent = nullptr);
	~ActivityManageWork();
	void working();
	void homeWorking();

	QSqlDatabase getDB();
	void submitAll();
	void setModel(QSqlTableModel* model);
	void setMemberModel(QSqlTableModel* model);
	void apply(const QString aid, const QString& uid);
	void cancel(const QString aid, const QString& uid);
	void delActivity(const QString aid);
	void m_approve(const QString actm_id);
	void m_reject(const QString actm_id);
	void m_delete(const QString actm_id);
	void setType(int t);

private:
	int type;
	service db_service;
	QSqlDatabase DB;
	QSqlTableModel* tabModel, *memberTabModel;
signals:
	void activityManageWorkFinished(int type);
	void actHomeWorkFinished();
	void submitAllFinished(bool);
	void operateFinished(QString error);
	void manageOperateFinished(QString error);
};
