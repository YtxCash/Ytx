#include "global/signalstation.h"

SignalStation& SignalStation::Instance()
{
    static SignalStation instance;
    return instance;
}

void SignalStation::RegisterModel(Section section, int node_id, const AbstractTableModel* model)
{
    if (!model_hash_.contains(section))
        model_hash_[section] = QHash<int, const AbstractTableModel*>();

    model_hash_[section].insert(node_id, model);
}

void SignalStation::DeregisterModel(Section section, int node_id) { model_hash_[section].remove(node_id); }

void SignalStation::RAppendOne(Section section, CSPCTrans& trans)
{
    auto section_model_hash { model_hash_.value(section) };
    int related_node_id { *trans->related_node };

    auto model { section_model_hash.value(related_node_id, nullptr) };
    if (!model)
        return;

    connect(this, &SignalStation::SAppendOne, model, &AbstractTableModel::RAppendOne);
    emit SAppendOne(trans);
    disconnect(this, &SignalStation::SAppendOne, model, &AbstractTableModel::RAppendOne);
}

void SignalStation::RRetrieveOne(Section section, CSPTrans& trans)
{
    auto section_model_hash { model_hash_.value(section) };

    int related_node_id { *trans->related_node };
    auto related_model { section_model_hash.value(related_node_id, nullptr) };
    if (!related_model)
        return;

    connect(this, &SignalStation::SAppendOne, related_model, &AbstractTableModel::RAppendOne);
    emit SAppendOne(trans);
    disconnect(this, &SignalStation::SAppendOne, related_model, &AbstractTableModel::RAppendOne);

    int node { *trans->node };
    auto model { section_model_hash.value(node, nullptr) };
    if (!model)
        return;

    connect(this, &SignalStation::SRetrieveOne, model, &AbstractTableModel::RRetrieveOne);
    emit SRetrieveOne(trans);
    disconnect(this, &SignalStation::SRetrieveOne, model, &AbstractTableModel::RRetrieveOne);
}

void SignalStation::RRemoveOne(Section section, int node_id, int trans_id)
{
    auto section_model_hash { model_hash_.value(section) };

    auto model { section_model_hash.value(node_id, nullptr) };
    if (!model)
        return;

    connect(this, &SignalStation::SRemoveOne, model, &AbstractTableModel::RRemoveOne);
    emit SRemoveOne(node_id, trans_id);
    disconnect(this, &SignalStation::SRemoveOne, model, &AbstractTableModel::RRemoveOne);
}

void SignalStation::RUpdateBalance(Section section, int node_id, int trans_id)
{
    auto section_model_hash { model_hash_.value(section) };

    auto model { section_model_hash.value(node_id, nullptr) };
    if (!model)
        return;

    connect(this, &SignalStation::SUpdateBalance, model, &AbstractTableModel::RUpdateBalance);
    emit SUpdateBalance(node_id, trans_id);
    disconnect(this, &SignalStation::SUpdateBalance, model, &AbstractTableModel::RUpdateBalance);
}

void SignalStation::RMoveMulti(Section section, int old_node_id, int new_node_id, const QList<int>& trans_id_list)
{
    auto section_model_hash { model_hash_.value(section) };

    auto old_model { section_model_hash.value(old_node_id, nullptr) };
    if (old_model) {
        connect(this, &SignalStation::SMoveMulti, old_model, &AbstractTableModel::RMoveMulti);
        emit SMoveMulti(old_node_id, new_node_id, trans_id_list);
        disconnect(this, &SignalStation::SMoveMulti, old_model, &AbstractTableModel::RMoveMulti);
    }

    auto new_model { section_model_hash.value(new_node_id, nullptr) };
    if (new_model) {
        connect(this, &SignalStation::SMoveMulti, new_model, &AbstractTableModel::RMoveMulti);
        emit SMoveMulti(old_node_id, new_node_id, trans_id_list);
        disconnect(this, &SignalStation::SMoveMulti, new_model, &AbstractTableModel::RMoveMulti);
    }
}
