#include "analyseview.h"
#include <QLibrary>
#include <iostream>


//创建函数指针与需加载的函数对应 typedef 返回值 (*函数指针名)(入参)
typedef char (*fun_read_cpuid)(char* out, int* outLen);//读cpuid
typedef char (*fun_read_license)(char* lic, int len);//读序列号
typedef int (*fun_statistic_init)(int windowNo, Statistics_OutTypeDef* statistics_out_var);//统计分析初始化
typedef void (*fun_statistic_analyze)(int windowNo, float lp);//进行统计
typedef void (*fun_getStatisticData)(int windowNo, Statistics_OutTypeDef* statistics_out_var);//获取统计结果

#define EXCUTE_SUSSESS 1

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
    connect(m_plot, &AnalysePlot::selectAreaFinish, this, &AnalyseView::calculateSelectAreaData);
    connect(m_plot, &AnalysePlot::plotDataLoadFinish, this, &AnalyseView::calculateTotalData);

    // 表格容器
    QWidget* tableContainer = new QWidget(this);
    QHBoxLayout* tableLayout = new QHBoxLayout(tableContainer);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->setSpacing(0);
    tableContainer->setLayout(tableLayout);

    // 表格控件
    m_table = new AnalyseTable();
    connect(m_table, &AnalyseTable::sigShowSpecifiedRowArea, m_plot, &AnalysePlot::slotShowSpecifiedRowArea);

    m_optTable = new OperationTable();
    m_optTable->setMaximumWidth(150);
    connect(m_optTable, &OperationTable::sigDeleteTableData, m_table, &AnalyseTable::slotDeleteTableData);
    connect(m_optTable, &OperationTable::sigDeleteTableData, m_plot, &AnalysePlot::slotDeleteArea);

    tableLayout->addWidget(m_table);
    tableLayout->addWidget(m_optTable);

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

void AnalyseView::calculateStaData(LeqStat_S* stat, std::vector<double> data)
{
    int n2 = 0;
    int data_num[1700] = {0};
    float value_la = 0;
    float sum = 0;
    float totalLa = 0;
    //辅助计算统计值用
    float lmax = 0;
    float lmin = 1000;
    int size = data.size();

    sum += pow(10, value_la * 0.1f);
    totalLa += value_la;

    /*
     * 计算lmax，lmin, leqT, SD(样本标准差), Lae(SEL)
    */
    for (int i = 0; i < size; i++)//totle改成num_end
    {
        value_la = data[i];
        data_num[int(value_la * 10)]++;//统计分析需要
        if (value_la > lmax)
        {
            lmax = value_la;
        }
        if (value_la < lmin)
        {
            lmin = value_la;
        }

        sum += pow(10, data[i] * 0.1f);
        totalLa += value_la;
        n2++;
    }

//    float leqT = 10 * log10(sum / n2);//长时间计权值

    //float leqT = 10*log10(sum / n2);//长时间计权值
    double avgLa = totalLa / n2;//平均声压级别

    float sd = 0;
    float la = 0;
    for (int i = 0; i < size; i++)//totle改成num_end
    {
        la = data[i];
        sd += pow(la - avgLa, 2);
    }

    sd = (float)sqrt(sd / (n2 - 1));//样本标准差

    qDebug() << "sd ======= " << sd;

    /*
     * 计算累计百分比
    */
    int jifenshijian2 = n2;
    int temp_L5 = jifenshijian2 * 95 / 100;
    int temp_L10 = jifenshijian2 * 9 / 10;
    int temp_L50 = jifenshijian2 / 2;
    int temp_L90 = jifenshijian2 / 10;
    int temp_L95 = jifenshijian2 * 5 / 100;
    int temp_count = 0;
    int maxLen = 1500;//步进0.1dB,0-170dB,一共1700个
    int i = 0;
    for (i = 0; i < maxLen; i++)
    {
        temp_count += data_num[i];
        if(temp_count > temp_L5 && stat->L5 <= 0)
        {
            stat->L5 = (float)i / 10;
        }
        if(temp_count >= temp_L10 && stat->L10 <= 0)
        {
            stat->L10 = (float)i / 10;
        }
        if(temp_count >= temp_L50 && stat->L50 <= 0)
        {
            stat->L50 = (float)i / 10;
        }
        if(temp_count >= temp_L90 && stat->L90 <= 0)
        {
            stat->L90 = (float)i / 10;
        }
        if(temp_count >= temp_L95 && stat->L95 <= 0)
        {
            stat->L95 = (float)i / 10;
        }
    }

    int ii = maxLen;
    int total = 0;
    while((100 * total) / temp_count < 5 && i > 0)
    {
        ii--;
        total += data_num[ii];
    }

    stat->L5 = (float)(ii) / 10;

    if(i < 1)
    {
        stat->L10 = stat->L50 = stat->L90 = stat->L95 = stat->L5;
    }

    while((100 * total) / temp_count < 10 && i > 0)
    {
        ii--;
        total += data_num[ii];

    }
    stat->L10 = (float)(ii) / 10;
    if(i < 1)
    {
        stat->L50 = stat->L90 = stat->L95 = stat->L10;
    }

    while((100 * total) / temp_count < 50 && i > 0)
    {
        ii--;
        total += data_num[ii];
    }

    stat->L50 = (float)(ii) / 10;
    if(i < 1)
    {
        stat->L90 = stat->L95 = stat->L50;
    }

    while((100*total) / temp_count < 90 && i > 0)
    {
        ii--;
        total += data_num[ii];

    }
    stat->L90 = (float)(ii) / 10;
    if(i < 1)
    {
        stat->L95 = stat->L90;
    }

    while((100 * total) / temp_count < 95 && i > 0)
    {
        ii--;
        total += data_num[ii];

    }

    stat->L95 = (float)(ii) / 10;
    stat->SD = sd;
    stat->Lmax = lmax;
    stat->Lmin = lmin;

    qDebug() << "Code L5:" << stat->L5;
    qDebug() << "Code L10:" << stat->L10;
    qDebug() << "Code L50:" << stat->L50;
    qDebug() << "Code L90:" << stat->L90;
    qDebug() << "Code L95:" << stat->L95;
    qDebug() << "Code SD:" << stat->SD;
}

void AnalyseView::calculateStaDataDLL(LeqStat_S *stat, std::vector<double> data)
{
    QString pathDll = QString("%1/%2").arg(qApp->applicationDirPath()).arg("MakeAWALib.dll");
    qDebug() << pathDll;

    QLibrary myLib(pathDll);
    //判断是否加载成功
    if(myLib.load())
    {
        char cpuid[100] = {0};
        char lic[100] = {0};
        int  len = 0;
        int  rescode = 0;//1成功, 0失败

        fun_read_cpuid read_cpuid = (fun_read_cpuid) myLib.resolve("read_cpuid");
        if (read_cpuid)
        {
            rescode = read_cpuid(cpuid, &len);
            if(rescode != EXCUTE_SUSSESS)
            {
                return;
            }
        }

        sprintf(lic, "license:%s","RciFHQLTGdBkxatH6EyAiC9FXv/vlGNbDTM0b6K0hhii4QMbfgCBh7WzzD07cBDcnlA+IDj/ggk=");//拿到授权软

        fun_read_license read_license = (fun_read_license) myLib.resolve("read_license");
        if (read_license)
        {
             rescode = read_license(lic, strlen(lic));//进行授权
             if(rescode != EXCUTE_SUSSESS)
             {
                 return;
             }
        }

        Statistics_OutTypeDef statistics_out_var;
        fun_statistic_init statistic_init = (fun_statistic_init) myLib.resolve("statistic_init");
        if (statistic_init)
        {
             rescode = statistic_init(0, &statistics_out_var);//进行授权
             if(rescode != EXCUTE_SUSSESS)
             {
                 return;
             }
        }

        fun_statistic_analyze statistic_analyze = (fun_statistic_analyze) myLib.resolve("statistic_analyze");
        if (statistic_analyze)
        {
            float laf = 0;
            int size = data.size();
            for (int i = 0; i < size; i++)//totle改成num_end
            {
                 laf = data[i];
                 statistic_analyze(0, laf);//进行分析

            }
        }

        fun_getStatisticData getStatisticData = (fun_getStatisticData) myLib.resolve("getStatisticData");
        if (getStatisticData)
        {
             getStatisticData(0, &statistics_out_var);//获取分析结果
        }

        stat->L5 = statistics_out_var.L5;
        stat->L10 = statistics_out_var.L10;
        stat->L50 = statistics_out_var.L50;
        stat->L90 = statistics_out_var.L90;
        stat->L95 = statistics_out_var.L95;
        stat->SD = statistics_out_var.SD;

        qDebug() << "Lib L5:" << stat->L5;
        qDebug() << "Lib L10:" << stat->L10;
        qDebug() << "Lib L50:" << stat->L50;
        qDebug() << "Lib L90:" << stat->L90;
        qDebug() << "Lib L95:" << stat->L95;
        qDebug() << "Lib SD:" << stat->SD;
        qDebug() << "Lib SEL:" << stat->SEL;

        myLib.unload();
    }
    else
    {
        qDebug() << "load DLL failed !";
    }
}

void AnalyseView::calculateMostValue(LeqStat_S *stat, std::vector<double> data)
{
    if (!data.empty())
    {
        stat->Lmax = data.front();
        stat->Lmin = data.front();
    }

    for (auto item : data)
    {
        stat->Lmin = stat->Lmin > item ? item : stat->Lmin;
        stat->Lmax = stat->Lmax < item ? item : stat->Lmax;
    }
}

void AnalyseView::calculateSelectAreaData()
{
    // 首先获取框选区域的数据
    auto area = m_plot->currentArea();

    // 开始计算数据
    LeqStat_S data;

    data.uid = area->uid;
    data.startTime = DateTime(area->startTime.toStdString(), "-", " ", ":");
    data.tm = area->tm;
    double interval = 0;

    if (!area->keys.empty())
    {
        interval = area->keys.at(1) - area->keys.at(0);
        interval *= 1000;
    }

    float t = interval * area->values.size();

    if (!area->instData.empty())
    {
        // 计算最值
        calculateMostValue(&data, area->instData);

        // 计算统计数据
        calculateStaData(&data, area->instData);
    }

    // 计算 Leq,T
    calculateLeqT(&data, area->values);

    // 计算 SEL
    calculateSEL(&data, t);

    // 计算完毕添加到表格中去
    m_table->addRow(data);

    m_optTable->addRow(area->uid);
}

void AnalyseView::calculateTotalData()
{
    // 开始计算数据
    LeqStat_S data;
    std::vector<double> instData = std::vector<double>(m_plot->instData().begin(), m_plot->instData().end());
    std::vector<double> leqtData = std::vector<double>(m_plot->leqtData().begin(), m_plot->leqtData().end());

    data.uid = m_plot->mainUid();
    data.startTime = DateTime(m_plot->startTime().toStdString(), "-", " ", ":");
    data.tm = m_plot->tm();
    double interval = 0;

    if (!m_plot->keyDataStd().empty())
    {
        interval = m_plot->keyDataStd()[1] - m_plot->keyDataStd()[0] + 0.0005;
        interval *= 1000;
    }

    float t = interval * m_plot->leqtDataStd().size();

    // 计算最值和统计相关指标
    if (!instData.empty())
    {
        // 计算最值
//        calculateMostValue(&data, instData);

        // 计算统计数据
//        calculateStaData(&data, m_plot->instDataStd());
        calculateStaDataDLL(&data, m_plot->instDataStd());
//        data.L5 = calculateQuantile(instData, 5);
//        data.L10 = calculateQuantile(instData, 10);
//        data.L50 = calculateQuantile(instData, 50);
//        data.L90 = calculateQuantile(instData, 90);
//        data.L95 = calculateQuantile(instData, 95);
//        data.SD = calculateSD(instData);
//        qDebug() << "sd ======= " << calculateSD(instData);
    }

    // 计算 Leq,T
    calculateLeqT(&data, leqtData);

    // 计算 SEL
    calculateSEL(&data, t);

    // 计算完毕添加到表格中去
    m_table->addRow(data);

    m_optTable->addRow(data.uid);
}

void AnalyseView::calculateLeqT(LeqStat_S *stat, std::vector<double> data)
{
    int n = data.size();
    double sum = 0.0;
    for(int i = 1; i <= n; ++i)
    {
        sum += pow(10.0, 0.1 * data[i]); // 注意这里的指数运算符需要使用双精度浮点数来确保准确性
    }
    stat->LeqT = 10.0 * log10(sum / n);

    qDebug() << "calculate LeqT = " << stat->LeqT;
}

void AnalyseView::calculateSEL(LeqStat_S *stat, float t)
{
    stat->SEL = stat->LeqT +  10.0 * log10(t);
}

float AnalyseView::calculateQuantile(std::vector<double> pressures, double percentile)
{
    if (pressures.empty())
    {
        return 0.0; // 或者返回错误代码
    }

    std::sort(pressures.begin(), pressures.end());
    size_t index = static_cast<size_t>(percentile / 100.0 * (pressures.size() - 1));
    return pressures[index];
}

float AnalyseView::calculateSD(const std::vector<double> &pressures)
{
    double mean = 0.0;
    for (double p : pressures)
    {
        mean += p;
    }
    mean /= pressures.size();

    double sumSquared = 0.0;
    for (double p : pressures)
    {
        sumSquared += (p - mean) * (p - mean);
    }

    return sqrt(sumSquared / pressures.size());
}

