#ifndef ENUMCLASS_H
#define ENUMCLASS_H

// Enum class defining sections
enum class Section { kFinance, kSales, kTask, kNetwork, kProduct, kPurchase };

// Enum class defining transaction columns
enum class TransColumn { kID, kPostDate, kCode, kRatio, kDescription, kDocument, kState, kRelatedNode, kDebit, kCredit, kRemainder };

// Enum class defining search transaction columns
enum class SearchTransactionColumn {
    kID,
    kPostDate,
    kCode,
    kLhsNode,
    kLhsRatio,
    kLhsDebit,
    kLhsCredit,
    kDescription,
    kDocument,
    kState,
    kRhsNode,
    kRhsRatio,
    kRhsDebit,
    kRhsCredit,
};

// Enum class defining node columns
enum class NodeColumn { kName, kID, kCode, kDescription, kNote, kNodeRule, kBranch, kUnit, kTotal, kPlaceholder };

// Enum class defining check options
enum class Check { kAll, kNone, kReverse };

#endif // ENUMCLASS_H
