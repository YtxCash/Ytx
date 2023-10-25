#ifndef SIGNALSTATION_H
#define SIGNALSTATION_H

#include "component/enumclass.h"
#include "table/tablemodel.h"
#include "table/transaction.h"

using CSPCTransaction = const QSharedPointer<const Transaction>;

class SignalStation : public QObject {
    Q_OBJECT

public:
    static SignalStation& Instance();
    void RegisterModel(Section section, int node_id, const TableModel* model);
    void DeregisterModel(Section section, int node_id);

signals:
    void SAppendOne(CSPCTrans& trans);
    void SDeleteOne(int node_id, int trans_id);
    void SUpdateBalance(int node_id, int trans_id);
    void SMoveMulti(int old_node_id, int new_node_id, const QList<int>& trans_id_list);

public slots:
    void RAppendOne(Section section, CSPCTrans& trans);
    void RDeleteOne(Section section, int node_id, int trans_id);
    void RUpdateBalance(Section section, int node_id, int trans_id);
    void RMoveMulti(Section section, int old_node_id, int new_node_id, const QList<int>& trans_id_list);

private:
    SignalStation() = default;
    ~SignalStation() {};

    SignalStation(const SignalStation&) = delete;
    SignalStation& operator=(const SignalStation&) = delete;
    SignalStation(SignalStation&&) = delete;
    SignalStation& operator=(SignalStation&&) = delete;

private:
    QHash<Section, QHash<int, const TableModel*>> model_hash_ {};
};

#endif // SIGNALSTATION_H
