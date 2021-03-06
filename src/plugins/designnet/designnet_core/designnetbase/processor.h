﻿#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include <QList>

#include "../designnet_core_global.h"
#include "../property/propertyowner.h"
#include "Utils/XML/xmldeserializer.h"
#include "port.h"
#include <QWaitCondition>
#include <QMutex>
#include <QFutureInterface>
#include <QObject>
#include <QIcon>
#include <QReadWriteLock>
QT_BEGIN_NAMESPACE
class QThread;
QT_END_NAMESPACE

namespace DesignNet{

class IData;
class DesignNetSpace;
class Processor;
class ProcessorWorker : public QObject{
	Q_OBJECT
public:
	ProcessorWorker(Processor *processor);
	bool isWorking();
	Processor* m_processor;
public slots:
	void run();
	void stopped();
	void started();
protected:
	bool			m_bWorking;				//!< 是否正在执行
	QMutex			m_mutex;
};

class DESIGNNET_CORE_EXPORT Processor : public QObject, public PropertyOwner
{
    friend class Port;
	friend class ProcessorWorker;
	Q_OBJECT
public:
    explicit Processor(DesignNetSpace *space = 0, QObject* parent = 0);
    virtual ~Processor();
	virtual void init() {};
    virtual Processor* create(DesignNetSpace *space = 0) const = 0;  //!< 创建Processor

    const QList<Port*> & getInputPorts() const;
    const QList<Port *> &getOutputPorts() const;

    Port*	getPort(const QString &name) const; //!< 得到指定名称的Port
	void    addPort(Port* port);			//!< 添加端口

	void    removePort(Port* port = 0);     //!< 删除端口

	void waitForFinish();
	QIcon icon() const;
	void setIcon(const QString &str);
    
    void    setName(const QString& name);//!< set name
    virtual QString name() const;        //!< get name
    void    setID(const int &id);       //!< 在一个DesignNetSpace中的唯一ID
    int     id() const;                 //!< getter

    virtual Core::Id typeID() const;			//!< 返回类型ID
    virtual QString category() const;			//!< 返回种类
    virtual void pushData(IData *data, const QString& portname );
    int indegree(QList<Processor*> exclusions = QList<Processor*>()) const; //!< 计算入度
    
	DesignNetSpace *space() const{ return m_space; }
	void setSpace(DesignNetSpace *space) ;

	virtual bool isDataReady() const;				//!< 数据是否已经准备好了
	void setDataReady(const bool &bReady = true);	//!< 设置数据是否准备好了
	void setRepickData(const bool &repick = true);	//!< 下次执行是否重新去数据

	virtual void serialize(Utils::XmlSerializer& s) const;
	virtual void deserialize(Utils::XmlDeserializer& s) ;
	virtual QString serializableType() const;
	virtual Utils::XmlSerializable* createSerializable() const;
	bool isPortCountResizable() const;
	void setPortCountResizable(const bool &resizable);
	bool isRunning();
	QReadWriteLock m_workingLock;
signals:
	void logout(QString log);
public slots:
	void run();							//!< 线程执行该函数
	void onPropertyChanged_internal();
protected:
	virtual void propertyChanged(Property *prop);
	
	virtual void propertyAdded(Property* prop);
	virtual bool process() = 0;			//!< 正式处理
	virtual bool beforeProcess();		//!< 处理之前的准备,这里会确保数据已经准备好了
	virtual void afterProcess(bool status = true);		//!< 完成处理
	virtual void showConfig();			//!< 显示config
    virtual void stateChanged(Port* port);  //!< 端口状态改变
    virtual void dataArrived(Port* port);  //!< 数据到达
	virtual bool connectionTest(Port* src, Port* target);
	virtual bool checkDataReady();			//!< 检查数据是否准备好了

    QList<Port*>	m_inputPorts;			//!< 输入端口列表
    QList<Port*>	m_outputPorts;			//!< 输出端口列表
    QIcon			m_icon;
	QString			m_title;				//!< 种类title
    DesignNetSpace* m_space;				//!< DesignNetSpace
    int				m_id;
	QWaitCondition	m_dataReadyWait;			//!< 等待数据到来
	QMutex			m_dataReadyMutex;
	bool			m_portCountResizable;	//!< 端口可变
	mutable QMutex	m_mutexReady;			//!< 数据操作锁
	bool			m_bReady;
	bool			m_bRepickData;			//!< 下次执行是否重新取数据
	ProcessorWorker m_worker;
	QThread*  m_thread;
};
}

#endif // PROCESSOR_H
