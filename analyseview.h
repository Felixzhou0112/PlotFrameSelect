#ifndef ANALYSEPLOT_H
#define ANALYSEPLOT_H

#include <QWidget>
#include "analyseplot.h"
#include "analysetable.h"

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

private:
    QJsonObject analyDataLine(bool isCrossDay,QString dataLine, int cow, int dataCow, QStringList &InstantDataName, QString& measureData);

    float calculateLmax(std::vector<double> data);
    float calculateLmin(std::vector<double> data);
    float calculateLeqT(std::vector<double> data);
    float calculateL5(std::vector<double> data);
    float calculateL10(std::vector<double> data);
    float calculateL50(std::vector<double> data);
    float calculateL90(std::vector<double> data);
    float calculateL95(std::vector<double> data);
    float calculateSD(std::vector<double> data);
    float calculateSEL(std::vector<double> data);

private:
    AnalysePlot* m_plot = nullptr;
    AnalyseTable* m_table = nullptr;
    QString m_strDataJson;// 瞬时数据 Json 字符串
    QStringList m_dataNameList; // 数据名称
    QStringList m_insDataName;// 瞬时数据名称列表
    QStringList m_leqtDataName;// Leqt数据名称列表
};
#endif // ANALYSEPLOT_H
