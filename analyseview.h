#ifndef ANALYSEPLOT_H
#define ANALYSEPLOT_H

#include <QWidget>
#include "analyseplot.h"
#include "analysetable.h"
#include "operationtable.h"


//输出数据结构体，单位dB
typedef struct
{
        float L5;
        float L10;
        float L50;
        float L90;
        float L95;
        float SD;

        float graphDataDis[151]; //统计分布图数据  graphDataDis[0]表示0dB占了所有数据的百分比，后面的数据以此类推。   graphDataDis[150]为最大百分比。
        float graphDataCum[150]; //累积分布图数据  graphDataCum[0]表示大于0dB的数据占的百分比，后面的数据以此类推。

        int rate;  //数据率  //1000表示100.0%
        int validRate;  //数据有效率 //1000表示100.0%

}Statistics_OutTypeDef;


class AnalyseView : public QWidget
{
    Q_OBJECT

public:
    AnalyseView(QWidget *parent = nullptr);
    ~AnalyseView();

    void loadDataFile(const QString& filePath);
    bool isAnalyseEnable();
    bool isMostValueEnable();
    const QStringList& instDataNames();
    const QStringList& leqtDataNames();
    void setAnalyseDataName(QString instName, QString leqtName);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void calculateSelectAreaData();
    void calculateTotalData();

private:
    QJsonObject analyDataLine(bool isCrossDay,QString dataLine, int cow, int dataCow, QStringList &InstantDataName, QString& measureData);

    void calculateStaData(LeqStat_S *stat, std::vector<double> data);
    void calculateMostValue(LeqStat_S *stat, std::vector<double> data);
    void calculateLeqT(LeqStat_S *stat, std::vector<double> data);
    void calculateSEL(LeqStat_S *stat, float t);

private:
    AnalysePlot* m_plot = nullptr;
    AnalyseTable* m_table = nullptr;
    OperationTable* m_optTable = nullptr;
    QString m_strDataJson;// 瞬时数据 Json 字符串
    QStringList m_dataNameList; // 数据名称
    QStringList m_insDataName;// 瞬时数据名称列表
    QStringList m_leqtDataName;// Leqt数据名称列表
};
#endif // ANALYSEPLOT_H
