#include "analyseview.h"
#include "analyseplot.h"
#include "analysetable.h"

using namespace std;


AnalyseView::AnalyseView(QWidget *parent)
    : QWidget(parent)
{
    this->resize(1200, 700);

    QVBoxLayout* layout = new QVBoxLayout(this);
    this->setLayout(layout);

    // 曲线容器
    QWidget* plotContainer = new QWidget(this);
    plotContainer->setMinimumHeight(350);
    QVBoxLayout* plotLayout = new QVBoxLayout(plotContainer);
    plotLayout->setContentsMargins(0, 0, 0, 0);
    plotContainer->setLayout(plotLayout);

    // 曲线控件
    m_plot = new AnalysePlot(this);
    plotLayout->addWidget(m_plot);

    // 表格容器
    QWidget* tableContainer = new QWidget(this);
    QVBoxLayout* tableLayout = new QVBoxLayout(tableContainer);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableContainer->setLayout(tableLayout);

    // 表格控件
    AnalyseTable* table = new AnalyseTable();
    tableLayout->addWidget(table);

    layout->addWidget(plotContainer);
    layout->addWidget(tableContainer);
}

AnalyseView::~AnalyseView()
{
}

void AnalyseView::loadDataFile(const QString &filePath)
{
    m_strDataJson = "";
    m_dataNameList.clear();

    try
    {
        //装填固有数据 数据标签
        m_strDataJson.append("{\"activeType\": \"sys_back_lp\",\"list\": [");

        //一个时间点的数据列表
        QStringList InstantDataName;

        //读取文件
        QByteArray byteData;
        QString text;
        QFile file(filePath);
        QJsonArray dataListArray;
        QString measureData = "";
        QString day = "";
        bool isCrossDay = false;
        bool haveMeasurDate = false;
        int cow = 0,dataCow = 5000;
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            //文件只读开启失败
            return;
        }

        auto content = QString::fromLocal8Bit(file.readAll());
        file.seek(0);
        while(!file.atEnd())
        {
            byteData = file.readLine();
            //解码
            QString currentLine = QString::fromLocal8Bit(byteData);
            if(currentLine.contains("data") || currentLine.contains("Data"))
            {
                //记录数据开始行
                dataCow = cow;
            }

            QJsonObject oneLine;
            oneLine = analyDataLine(isCrossDay, currentLine,cow++,dataCow,InstantDataName,measureData);

            if("" != measureData && !haveMeasurDate)
            {
                //记录读取到的测量开始时间，用于做数据跨天处理
                day = measureData;
                haveMeasurDate = true;
            }
            else if(haveMeasurDate && !isCrossDay && day!=measureData)
            {
                //跨天标志位，跨天只进行一次
                isCrossDay = true;
            }

            if(!(oneLine.value("dateTime").toString() == ""))
            {
                m_strDataJson.append(QString(QJsonDocument(oneLine).toJson()) + ",");
            }

            currentLine.clear();
        }

        // 去掉多余的逗号
        int lastCommaIndex = m_strDataJson.lastIndexOf(',');
        if (lastCommaIndex != -1)
        {
            m_strDataJson.remove(lastCommaIndex, 1);
        }

        //声暴露级数据数字记录为DOSi时需要Y坐标单位改为% 此处type为1时修改坐标，其他则显示为dB
        int type = 2;
        if("DOSi" == InstantDataName[0])
            type = 1;
        m_strDataJson.append("],\"type\": " + QString::number(type) + "}");

        qDebug() << "json:" << InstantDataName;
        m_dataNameList = InstantDataName;
        return;
    }
    catch (exception& e)
    {
        qDebug() << "AWA6292Protocal::analyInstantFile exception";
        return;
    }

}

bool AnalyseView::isAnalyseEnable()
{
    bool enable = false;
    m_leqtDataName.clear();

    // 判断有没有 Leq,t 的数据，包括 LAeq,t LCeq,t LZeq,t
    for (QString name : m_dataNameList)
    {
        if (name.startsWith('L') && name.endsWith(",t"))
        {
            enable = true;
            m_leqtDataName.append(name);
        }
    }

    return enable;
}

bool AnalyseView::isMostValueEnable()
{
    bool enable = false;
    m_insDataName.clear();

    // 判断有没有瞬时数据，包括 LAFi LCFi LZFi LASi LCSi LZSi LAIi LCIi LZIi Linst
    for (QString name : m_dataNameList)
    {
        if ((name.startsWith('L') && name.endsWith("i")) || name.contains("Linst"))
        {
            enable = true;
            m_insDataName.append(name);
        }
    }

    return enable;
}

const QStringList &AnalyseView::instDataNames()
{
    return m_insDataName;
}

const QStringList &AnalyseView::leqtDataNames()
{
    return m_leqtDataName;
}

void AnalyseView::setAnalyseDataName(QString instName, QString leqtName)
{
    m_plot->setInstName(instName);
    m_plot->setLeqtNmae(leqtName);
}

void AnalyseView::showEvent(QShowEvent *event)
{
    m_plot->loadDataFromJson(m_strDataJson);

    QWidget::showEvent(event);
}

QJsonObject AnalyseView::analyDataLine(bool isCrossDay, QString dataLine, int cow, int dataCow, QStringList &InstantDataName, QString &measureDay)
{
    QJsonObject onePoint;

    try
    {
        //瞬时图数据用的json只需要数据记录即可,数据之前的信息不显示,只提取需要的测量方式，测量时间 ，
        if(cow <= dataCow)
        {
            QString dataName = dataLine.left(dataLine.indexOf("\t"));
            //去除空格，= 和 ：
            dataName.replace(QRegExp("\\s{1,}"), "");
            dataName.replace(":", "");

            QString data = dataLine.mid(dataLine.indexOf("\t") + 1 , dataLine.indexOf("\n") - dataLine.indexOf("\t") -1 );

            if(dataLine.contains("开始时间") || dataLine.contains("Start Time"))
            {
                QStringList day = dataLine.split("\t");

                //记录开始时间的日期 只取左边10位 YYYY-MM-DD
                measureDay = day[1].replace(QRegExp("\\s{1,}"), "").left(10);
            }

            //返回数据
            onePoint.insert(dataName, data);
            return onePoint;
        }

        if(cow == dataCow + 1)
        {
            //此处获取的是数据的名称行
            //以制表符分开数据块
            InstantDataName = dataLine.split("\t");
            InstantDataName.removeLast();

            for (int i = 0; i < InstantDataName.count(); i++)
            {
                //去除数据名称中的空格
                InstantDataName[i].replace(QRegExp("\\s{1,}"), "");
            }

            //返回空数据
            onePoint.insert(QString::number(cow), dataLine);
            return onePoint;
        }

        //开始数据的处理
        QJsonObject oneLineData;
        QStringList dataList = dataLine.split("\t");

        //根据数据格式，最后一个是时间戳，需要放在外面
        for (int i = 0; i < InstantDataName.count(); i++)
        {
            if(i >= dataList.count())
                continue ;

            double count = dataList[i].replace(QRegExp("\\s{1,}"), "").toDouble();
            //保留一位小数
            QString data = QString::number(count, 'f', 1);
            oneLineData.insert(InstantDataName[i],data);
        }

        //插入infoList数据点
        onePoint.insert("infoList",oneLineData);

        //插入时间戳 最后一个数据就是时间戳 20211122133326:015
        QString time = dataList[dataList.count()-1].replace(QRegExp("\\s{1,}"), "");
        QDateTime measureTimex = QDateTime::fromString(time,"yyyyMMddhhmmss:zzz");
        QString newStateTime = measureTimex.toString("yyyy-MM-dd hh:mm:ss:zzz");
        QDateTime day = QDateTime::fromString(measureDay,"yyyy-MM-dd");
        //如果是跨天的日期加一
        if("00:00:0" == time.left(7) && !isCrossDay)
        {
            measureDay = day.addDays(1).toString("yyyy-MM-dd");
        }
        if("" == newStateTime)
            onePoint.insert("dateTime",measureDay + " " + time);
        else
            onePoint.insert("dateTime",newStateTime);

        //返回单个节点数据
        return onePoint;

    } catch (exception& e)
    {
        onePoint.insert(QString::number(cow), dataLine);
        return onePoint;
    }
}

