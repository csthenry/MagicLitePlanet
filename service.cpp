﻿/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

#include "service.h"
#include <QDebug>

service::service()
{
    /*****************请在此处完善数据库信息*****************/

    dataBaseType = "QMYSQL";    //available drivers: QSQLITE QMYSQL QMYSQL3 QODBC QODBC3 QPSQL QPSQL7
    hostName = "api.bytecho.net";
    dataBasePort = 3306;
    dataBaseName = "magic";
    dataBaseUserName = "magic";
    dataBasePassword = "***************";

    /*****************请在此处完善数据库信息*****************/
}

//网络授时 https://www.freesion.com/article/3807754024/
qint32 service::getWebTime()
{
    QUdpSocket udpSocket;
    udpSocket.connectToHost("time.windows.com", 123);
    if (udpSocket.waitForConnected(3000)) {
        qint8 LI = 0;
        qint8 VN = 3;
        qint8 MODE = 3;
        qint8 STRATUM = 0;
        qint8 POLL = 4;
        qint8 PREC = -6;
        QDateTime epoch(QDate(1900, 1, 1));
        qint32 second = quint32(epoch.secsTo(QDateTime::currentDateTime()));
        qint32 temp = 0;
        QByteArray timeRequest(48, 0);
        timeRequest[0] = (LI << 6) | (VN << 3) | (MODE);
        timeRequest[1] = STRATUM;
        timeRequest[2] = POLL;
        timeRequest[3] = PREC & 0xff;
        timeRequest[5] = 1;
        timeRequest[9] = 1;
        timeRequest[40] = (temp = (second & 0xff000000) >> 24);
        temp = 0;
        timeRequest[41] = (temp = (second & 0x00ff0000) >> 16);
        temp = 0;
        timeRequest[42] = (temp = (second & 0x0000ff00) >> 8);
        temp = 0;
        timeRequest[43] = ((second & 0x000000ff));
        udpSocket.flush();
        udpSocket.write(timeRequest);
        udpSocket.flush();
        if (udpSocket.waitForReadyRead(3000)) {
            QByteArray newTime;
            QDateTime epoch(QDate(1900, 1, 1));
            QDateTime unixStart(QDate(1970, 1, 1));
            do
            {
                newTime.resize(udpSocket.pendingDatagramSize());
                udpSocket.read(newTime.data(), newTime.size());
            } while (udpSocket.hasPendingDatagrams());
            QByteArray TransmitTimeStamp;
            TransmitTimeStamp = newTime.right(8);
            quint32 seconds = TransmitTimeStamp[0];
            quint8 temp = 0;
            for (int j = 1; j <= 3; j++)
            {
                seconds = seconds << 8;
                temp = TransmitTimeStamp[j];
                seconds = seconds + temp;
            }
            quint32 t = seconds - epoch.secsTo(unixStart);
            qDebug() << "网络时间戳：" << t;
            return t;
            //time.setTime_t(seconds-epoch.secsTo(unixStart));
        }
    }
    return -1;
}

QString service::pwdEncrypt(const QString &str) //字符串MD5算法加密
{
    QByteArray btArray;
    btArray.append(str);//加入原始字符串
    //::CryptGenRandom()// 加盐（随机数）函数 #include <Windows.h>
    //QCryptographicHash hash_sha512(QCryptographicHash::Sha512);  //SHA512加密算法
    QCryptographicHash hash_md5(QCryptographicHash::Md5);  //Md5加密算法
    hash_md5.addData(btArray);  //添加数据到加密哈希值
    QByteArray resultArray = hash_md5.result();  //返回最终的哈希值
    QString md5 = resultArray.toHex();  //转换为16进制字符串
    return md5;
}

void service::connectDatabase(QSqlDatabase& db)
{
    //如果存在连接名为qt_sql_default_connection（默认连接名）的连接，则不需要addDatabase
    //否则会报出QSqlDatabasePrivate::addDatabase: duplicate connection name 'qt_sql_default_connect...
    
    if(QSqlDatabase::contains("qt_sql_default_connection"))
        db = QSqlDatabase::database("qt_sql_default_connection");
    else
        db = QSqlDatabase::addDatabase(dataBaseType);
    db.setHostName(hostName);
    db.setPort(dataBasePort);
    db.setDatabaseName(dataBaseName);
    db.setUserName(dataBaseUserName);
    db.setPassword(dataBasePassword);
}

void service::addDatabase(QSqlDatabase& db, const QString &flag)
{
    if (QSqlDatabase::contains(flag))
        db = QSqlDatabase::database(flag);
    else
        db = QSqlDatabase::addDatabase(dataBaseType, flag);
    db.setHostName(hostName);
    db.setPort(dataBasePort);
    db.setDatabaseName(dataBaseName);
    db.setUserName(dataBaseUserName);
    db.setPassword(dataBasePassword);
}

bool service::initDatabaseTables(QSqlDatabase db)
{
    qDebug() << "initDatabaseTables()...";
    bool res = true;
    QSqlQuery query(db);
    //创建magic_users表
    //初始化root用户数据，登录名100000，密码123456
    QString creatTableStr = "CREATE TABLE IF NOT EXISTS magic_users "
        "(                                                     "
        "uid           int(10)      NOT NULL AUTO_INCREMENT,   "
        "password      varchar(64)  NOT NULL ,"
        "name          varchar(32)  NOT NULL ,"
        "gender        tinytext     NULL ,"
        "telephone     varchar(64)  NULL ,"
        "mail          varchar(128) NULL ,"
        "user_group    int(10) NOT  NULL ,"
        "user_dpt      int(10) NOT  NULL ,"
        "user_avatar   varchar(256) NULL ,"
        "score         float(5,2)   NUll ,"
        "user_status   tinyint(1)   NOT NULL ,"
        "last_login    datetime     NULL ,"
        "PRIMARY KEY (uid)                "
        ")ENGINE=InnoDB;                  "
        "INSERT IGNORE INTO magic_users          "
        "(uid, password, name, user_group, user_dpt, score, user_status)"
        "VALUES(1, 'kH9bV0rP5dF8oW7g', '系统', 2, 1, 0, 0);"
        "INSERT IGNORE INTO magic_users          "
        "(uid, password, name, user_group, user_dpt, score, user_status)"
        "VALUES                           "
        "(                                "
        "100000, '" + service::pwdEncrypt("123456") + "', 'admin', 1, 1, 0, 1);";
    res = query.exec(creatTableStr);

    //用户组权限分配表，默认有管理员和普通用户
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_group"
        "(group_id        int(10)     NOT NULL    AUTO_INCREMENT,"
        "group_name      varchar(32)  NOT NULL,"
        "users_manage     tinyint(1)  NOT NUll,"
        "attend_manage    tinyint(1)  NOT NUll,"
        "apply_manage     tinyint(1)  NOT NUll,"
        "applyItem_manage tinyint(1)  NOT NUll,"
        "group_manage     tinyint(1)  NOT NUll,"
        "activity_manage  tinyint(1)  NOT NUll,"
        "send_message     tinyint(1)  NOT NUll,"
        "notice_manage    tinyint(1)  NOT NUll,"
        "PRIMARY KEY (group_id)) ENGINE=InnoDB;"
        "INSERT IGNORE INTO magic_group"
        "(group_id, group_name, users_manage, attend_manage, apply_manage, applyItem_manage, group_manage, activity_manage, send_message, notice_manage)"
        "VALUES(1, '超级管理员', 1, 1, 1, 1, 1, 1, 1, 1);"
        "INSERT IGNORE INTO magic_group"
        "(group_id, group_name, users_manage, attend_manage, apply_manage, applyItem_manage, group_manage, activity_manage, send_message, notice_manage)"
        "VALUES(2, '普通用户', 0, 0, 0, 0, 0, 0, 1, 0);";
    if(res)
        res = query.exec(creatTableStr);

    //组织架构表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_department"
        "(dpt_id      int(10)      NOT NULL    AUTO_INCREMENT,"
        "dpt_name     varchar(32)  NOT NUll,"
        "PRIMARY KEY (dpt_id))ENGINE=InnoDB;"
        "INSERT IGNORE INTO magic_department"
        "(dpt_id, dpt_name)"
        "VALUES(1, '默认部门');";
    if (res)
        res = query.exec(creatTableStr);

    //考勤表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_attendance"
        "(num       int(10)      NOT NULL    AUTO_INCREMENT,"
        "a_uid      int(10)      NOT NUll,"
        "begin_date datetime,"
        "end_date   datetime,"
        "today      date,"
        "isSupply   tinytext     NOT NUll,"
        "operator   int(10),"
        "PRIMARY KEY (num, a_uid)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);

    //申请表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_apply"
        "(apply_id       int(10)      NOT NULL    AUTO_INCREMENT,"
        "uid             int(10)      NOT NUll,"
        "item_id         int(10)      NOT NUll,"
        "options         text,"
        "status          tinyint(1)   NOT NUll,"
        "operate_time    datetime     NOT NUll,"
        "token           varchar(32),"
        "PRIMARY KEY (apply_id)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);

    //审批流程项目表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_applyItems"
        "(item_id       int(10)      NOT NULL   AUTO_INCREMENT,"
        "title          varchar(32)  NOT NUll,"
        "options        text,"
        "publisher      int(10), "
        "auditor_list   varchar(64),"
        "isHide         tinyint(1)   NOT NUll,"
        "PRIMARY KEY (item_id)"
        ")ENGINE=InnoDB;"
        "INSERT IGNORE INTO magic_applyItems"
		"(item_id, title, options, publisher, auditor_list, isHide)"
        "VALUES (1, '修改用户名', '新的用户名$申请理由$', 1, '100000;', 0);"
        "INSERT IGNORE INTO magic_applyItems"
        "(item_id, title, options, publisher, auditor_list, isHide)"
        "VALUES (2, '变更组织架构', '用户组$部门$', 1, '100000;', 0);";

    if (res)
        res = query.exec(creatTableStr);
    
	//审批流程结果表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_applyProcess"
        "(apply_id       int(10)     NOT NULL,"
        "auditor         int(10)     NOT NUll,"
        "result          tinyint(1),"
        "result_text     text,"
        "operate_time    datetime    NOT NUll"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);
    
    //用户认证信息表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_verify"
        "(v_uid          int(10)      NOT NULL    AUTO_INCREMENT,"
        "vid             int(10)  NOT NUll,"
        "info            varchar(64),"
        "PRIMARY KEY (v_uid)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);

    //用户认证类型表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_verifyList"
        "(v_id           int(10)      NOT NULL    AUTO_INCREMENT,"
        "verify_name     varchar(32)  NOT NUll,"
        "icon            tinyint(1)   NOT NUll,"
        "PRIMARY KEY (v_id)"
        ")ENGINE=InnoDB;"
        "INSERT IGNORE INTO magic_verifyList"
        "(v_id, verify_name, icon)"
        "VALUES (1, '个人认证', 0);"
        "INSERT IGNORE INTO magic_verifyList"
        "(v_id, verify_name, icon)"
        "VALUES (2, '机构认证', 1);";
    if (res)
        res = query.exec(creatTableStr);

    //活动表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_activity"
        "(act_id      int(10)      NOT NULL    AUTO_INCREMENT,"
        "act_name     varchar(32)  NOT NUll,"
        "act_des      text         NUll,"
        "joinDate     datetime     NOT NUll,"
        "beginDate    datetime     NOT NUll,"
        "endDate      datetime     NOT NUll,"
        "editUid      int(10)      NOT NUll,"
        "act_score    float(5,2)   NOT NUll,"
        "PRIMARY KEY (act_id)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);

    //活动成员表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_activityMembers"
        "(actm_id      int(10)      NOT NULL    AUTO_INCREMENT,"
        "act_id        int(10)      NOT NUll,"
        "actm_uid      int(10)      NOT NUll,"
        "actm_joinDate datetime     NOT NUll,"
        "status        tinytext     NOT NUll,"
        "PRIMARY KEY (actm_id)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);
    //通知动态表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_contents"
        "(c_id         int(10)      NOT NULL    AUTO_INCREMENT,"
        "title         varchar(64)  NUll,"
        "text          mediumtext   NUll,"
        "created       datetime     NOT NUll,"
        "modified      datetime     NOT NUll,"
        "c_type        tinytext     NOT NUll,"
        "isHide        tinytext     NOT NUll,"
        "author_id     int(10)      NOT NUll,"
        "PRIMARY KEY (c_id)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);
    //统计数据表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_statistics"
        "(date         date      NOT NULL DEFAULT 0,"
        "login_cnt     int(10)   NOT NULL DEFAULT 0,"
        "register_cnt  int(10)   NOT NULL DEFAULT 0,"
        "get_cnt       int(10)   NOT NULL DEFAULT 0,"
        "activity_cnt  int(10)   NOT NULL DEFAULT 0,"
        "dynamics_cnt  int(10)   NOT NULL DEFAULT 0)"
        "ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);
    //系统数据表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_system"
        "(sys_name     varchar(64)    NOT NULL,"
        "field_1       varchar(128)   NULL,"
        "field_2       varchar(256)   NULL,"
        "field_3       varchar(128)   NULL,"
        "field_4       varchar(128)   NULL,"
        "field_5       varchar(128)   NULL,"
        "PRIMARY KEY (sys_name)"
        ")ENGINE=InnoDB;"
        "INSERT IGNORE INTO magic_system"
        "(sys_name, field_3)"
        "VALUES ('announcement', 0);"
        "INSERT IGNORE INTO magic_system"
        "(sys_name, field_1)"
        "VALUES ('debug', 0);"
        "INSERT IGNORE INTO magic_system"
        "(sys_name, field_1)"
        "VALUES ('openChat', 1);";
    
    if (res)
        res = query.exec(creatTableStr);
    //好友关系表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_relation"
        "(user_id      int(10)        NOT NULL,"
        "friend_id     int(10)        NOT NULL,"
        "extra         varchar(128)   NULL,"
        "status        tinyint(1)     NOT NULL)"
        "ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);
    //聊天信息表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_message"
        "(id            bigint(20)   NOT NULL AUTO_INCREMENT,"
        "from_uid       int(10)      NOT NULL,"
        "to_uid         int(10)      NOT NULL,"
        "text           mediumtext   NOT NULL,"
        "send_time      datetime     NOT NULL,"
        "PRIMARY KEY (id))"
        "ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);
    query.clear();
    
    return res;
}

int service::authAccount(QSqlDatabase& db, QString& uid, const long long account, const QString& pwd)
{
    QSqlQuery query(db), statistics(db);

    if (account == 0 || QString::number(account).isEmpty() || pwd.isEmpty())
        return 403;
    //验证UID
    bool net = query.exec("SELECT password, user_status FROM magic_users WHERE uid = " + QString::number(account));
    if (!net)
        return 500;
    //统计登录请求量
    statistics.exec("SELECT * FROM magic_statistics WHERE date='" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "'");
    if(statistics.next())
        statistics.exec("UPDATE magic_statistics SET login_cnt=login_cnt+1 WHERE date='" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "'");
    else
        statistics.exec("INSERT INTO magic_statistics (date, login_cnt) VALUES ('" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "', 1)");
    statistics.clear();
    if(query.next() && pwd == query.value("password").toString())
    {
        uid = QString::number(account);
        qDebug() << uid << "登录方式：账号登录";
        if (query.value("user_status").toInt() == 0)    //账号状态检测
            return 400;
		query.exec("UPDATE magic_users SET last_login = '" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "' WHERE uid = " + QString::number(account));   //更新最后登录时间
        return 200;
    }
    else
    {
        //验证手机号，可能有重复的手机号，所以用while
        net = query.exec("SELECT uid, password, user_status FROM magic_users WHERE telephone = " + QString::number(account));
        if (!net)
            return 500;
        while(query.next())
            if(pwd == query.value("password").toString())
            {
                uid = query.value("uid").toString();
                qDebug() << uid << "登录方式：手机号登录";
                if (query.value("user_status").toInt() == 0)    //账号状态检测
                    return 400;
                query.exec("UPDATE magic_users SET last_login = '" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "' WHERE uid = " + uid);   //更新最后登录时间
                return 200;
            }
        return 403;
    }
}

bool service::setAuthority(QSqlDatabase& db, const QString &uid, const QVector<QAction*>& vector)
{
    QSqlQuery query(db);
    query.exec("SELECT user_group FROM magic_users WHERE uid='" + uid + "';");
    query.next();
    QString groupId = query.value(0).toString();
    query.exec("SELECT * FROM magic_group WHERE group_id='" + groupId + "';");

    if(query.next())
    {
        if(query.value("send_message").toString() == '0')
        {
            vector[0]->setEnabled(false);
            vector[0]->setVisible(false);
        }
        else{
            vector[0]->setEnabled(true);
            vector[0]->setVisible(true);
        }
        if(query.value("users_manage").toString() == '0')
        {
            vector[1]->setEnabled(false);
            vector[1]->setVisible(false);

        }
        else{
            vector[1]->setEnabled(true);
            vector[1]->setVisible(true);
        }
        if(query.value("attend_manage").toString() == '0')
        {
            vector[2]->setEnabled(false);
            vector[2]->setVisible(false);

        }
        else{
            vector[2]->setEnabled(true);
            vector[2]->setVisible(true);
        }
        if(query.value("activity_manage").toString() == '0')
        {
            vector[3]->setEnabled(false);
            vector[3]->setVisible(false);

        }
        else{
            vector[3]->setEnabled(true);
            vector[3]->setVisible(true);
        }
        if(query.value("apply_manage").toString() == '0')
        {
            vector[4]->setEnabled(false);
            vector[4]->setVisible(false);

        }
        else{
            vector[4]->setEnabled(true);
            vector[4]->setVisible(true);
        }
        if(query.value("applyItem_manage").toString() == '0')
        {
            vector[5]->setEnabled(false);
            vector[5]->setVisible(false);

        }
        else{
            vector[5]->setEnabled(true);
            vector[5]->setVisible(true);
        }
        if(query.value("group_manage").toString() == '0')
        {
            vector[6]->setEnabled(false);
            vector[6]->setVisible(false);

        }
        else{
            vector[6]->setEnabled(true);
            vector[6]->setVisible(true);
        }
    }else
    {
        return false;
    }
    return true;
}

//https://stackoverflow.com/questions/6326237/qnetworkaccessmanager-crashes-on-delete
QPixmap service::getAvatar(const QString& url)
{
    QUrl picUrl(url);
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply = manager.get(QNetworkRequest(picUrl));    //可能产生潜在的栈溢出？
    //请求结束并下载完成后，退出子事件循环
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    //开启子事件循环
    loop.exec();
    QByteArray jpegData = reply->readAll();
    QPixmap pixmap;
    pixmap.loadFromData(jpegData);
    
    reply->deleteLater();
    return pixmap;
}

QPixmap service::setAvatarStyle(QPixmap avatar)
{
    QPixmap pixmapa(avatar), pixmap(120,120);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, 120, 120);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, 120, 120, pixmapa);
    return pixmap;
}

QString service::getGroup(QSqlDatabase& db, const QString& uid)
{
    QSqlQuery query(db);
    query.exec("SELECT user_group FROM magic_users WHERE uid = " + uid);
    if(!query.next())
        return "--";
    query.exec("SELECT group_name FROM magic_group WHERE group_id = " + query.value("user_group").toString());
    if(!query.next())
        return "--";
    return query.value("group_name").toString();
}

QString service::getDepartment(QSqlDatabase& db, const QString& uid)
{
    QSqlQuery query(db);
    query.exec("SELECT user_dpt FROM magic_users WHERE uid = " + uid);
    if(!query.next())
        return "--";
    query.exec("SELECT dpt_name FROM magic_department WHERE dpt_id = " + query.value("user_dpt").toString());
    if(!query.next())
        return "--";
    return query.value("dpt_name").toString();
}
//函数已废弃
/*
void service::buildAttendChart(QChartView *chartView_attend, const QWidget *parent, const QFont &font, int data_1, int data_2, int data_3, int data_4)
{
    QChart* attendChart = new QChart();
	QChart* oldChart = chartView_attend->chart();

    chartView_attend->setChart(attendChart);
    chartView_attend->setRenderHint(QPainter::Antialiasing);

    if (oldChart != nullptr)
    {
        delete oldChart;
        oldChart = nullptr;
    }

    attendChart->setTitle("工作时长统计（作息时间很不错~）");
    attendChart->setAnimationOptions(QChart::SeriesAnimations);
    QPieSeries *series = new QPieSeries(attendChart); //创建饼图序列
    series->setHoleSize(0.1);   //饼图空心大小
    //添加分块数据 6- 6-8 8-10 10+
    series->append("4h以下", data_1); //添加一个饼图分块数据,标签，数值
    series->append("4~6h", data_2);
    series->append("6~8h", data_3);
    series->append("8h以上", data_4);

	//设置每个分块的标签文字
    for(int i = 0; i < 4; i++)
    {
        QPieSlice* slice = series->slices().at(i);  //获取分块
        slice->setLabel(slice->label() + QString::asprintf(":%.0f次", slice->value()));    //设置分块的标签
        //信号与槽函数关联，鼠标落在某个分块上时，此分块弹出
        slice->setLabelFont(font);
        slice->setLabelVisible(true);
        if(slice->value() < 1)
            slice->setLabelVisible(false);  //过小就不显示label
        parent->connect(slice, SIGNAL(hovered(bool)), parent, SLOT(on_PieSliceHighlight(bool)));
    }
    //series->setLabelsVisible(true); //只影响当前的slices，必须添加完slice之后再设置
    attendChart->addSeries(series); //添加饼图序列

    if(series->slices().at(0)->percentage() > (series->slices().at(1)->percentage() + series->slices().at(2)->percentage()))
        attendChart->setTitle("工作时长统计（需要再勤奋一点哦！）");
    if(series->slices().at(3)->percentage() > (series->slices().at(1)->percentage() + series->slices().at(2)->percentage()))
        attendChart->setTitle("工作时长统计（要注意休息哦，身体最重要~）");
    attendChart->legend()->setFont(font);
    attendChart->setTitleFont(font);
    attendChart->legend()->setVisible(true); //图例
    attendChart->legend()->setAlignment(Qt::AlignBottom);
}
*/
