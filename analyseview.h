#ifndef ANALYSEPLOT_H
#define ANALYSEPLOT_H

#include <QWidget>

class AnalysePlot;

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

private:
    QJsonObject analyDataLine(bool isCrossDay,QString dataLine, int cow, int dataCow, QStringList &InstantDataName, QString& measureData);

private:
    AnalysePlot* m_plot = nullptr;
    QString m_strDataJson;// 瞬时数据 Json 字符串
    QStringList m_dataNameList; // 数据名称
    QStringList m_insDataName;// 瞬时数据名称列表
    QStringList m_leqtDataName;// Leqt数据名称列表
};
#endif // ANALYSEPLOT_H
