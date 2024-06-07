#ifndef MYCHART_H
#define MYCHART_H
#include "qcustomplot.h"
#include <QMenu>
#include "crosslineplot.h"


struct SelectArea_S
{
    QString uid;                    // 区域 uid
    double startPos;                // 开始位置
    double endPos;                  // 结束位置
    QCPItemRect * area;             // 框选区域
    std::vector<double> keys;       // 框选区域包含的有效横坐标
    std::vector<double> values;     // 框选区域包含的有效纵坐标
    std::vector<double> instData;   // 框选区域包含的有效瞬时数据
};


class AnalysePlot : public QCustomPlot {
    Q_OBJECT
public:
    explicit AnalysePlot(QWidget *parent = nullptr);
    void addData(const QVector<double>& keys, const QVector<double>& values);
    void loadDataFromJson(const QString &jsonData);
    void setInstName(QString name);
    void setLeqtNmae(QString name);

public slots:
    void contextMenuRequest(QPoint pos);

private slots:
    void setupContextMenu();
    void preciseActionTriggered();
    void lowerPeekActionTriggered();
    void modifyActionTriggered();
    void deleteActionTriggered();
    void resetActionTriggered();
    void selectAreaBtnToggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initPlot();
    void createSelectArea();
    void setPlotInteraction(bool enable);
    void scanSelectedAreaData(const QString &uid);

private:
    QMenu *m_contextMenu;
    QMap<QString, QCPItemRect *> m_selectionRects;
    std::vector<SelectArea_S> m_areaList;
    QCPItemRect *m_currentRect;

    bool m_selectState = false;         // 是否是框选状态
    bool m_selecting;                   // 是否是框选中
    double m_selectionStart;
    double m_selectionEnd;
    QCPAxisTickerDateTime* m_xTicker;

    QAction *preciseAction;         // 精确选区
    QAction *lowerPeekAction;       // 下调峰值
    QAction *modifyAction;          // 修改数据
    QAction *deleteAction;          // 删除数据
    QAction *resetAction;           // 还原数据

    QString m_instName;
    QString m_leqtName;
    QVector<double> m_keys;
    QVector<double> m_leqValues;
    QMap<double, QJsonObject> m_data;   // 原始数据

    QString m_selectedAreaUid = "";     // 右键要操作的选区 uid

    CrossLinePlot* m_crossLine = nullptr;// 鼠标追踪十字线
};

#endif // MYCHART_H