#include "optimizer.h"
#include "util.h"

#include <iostream>

using namespace hsql;

namespace mydb {

    Plan* Optimizer::createPlanTree(const SQLStatement* stmt) {
        switch (stmt->type()) {
            case kStmtSelect:
                return createSelectPlanTree(static_cast<const SelectStatement*>(stmt));
            case kStmtInsert:
                return createInsertPlanTree(static_cast<const InsertStatement*>(stmt));
            case kStmtUpdate:
                return createUpdatePlanTree(static_cast<const UpdateStatement*>(stmt));
            case kStmtDelete:
                return createDeletePlanTree(static_cast<const DeleteStatement*>(stmt));
            case kStmtCreate:
                return createCreatePlanTree(static_cast<const CreateStatement*>(stmt));
            case kStmtDrop:
                return createDropPlanTree(static_cast<const DropStatement*>(stmt));
            case kStmtTransaction:
                return createTrxPlanTree(static_cast<const TransactionStatement*>(stmt));
            case kStmtShow:
                return createShowPlanTree(static_cast<const ShowStatement*>(stmt));
            default:
                std::cout << "# ERROR: Statement type " << StmtTypeToString(stmt->type())
                          << " is not supported now." << std::endl;
        }
        return nullptr;
    }

    Plan* Optimizer::createCreatePlanTree(const CreateStatement* stmt) {
        CreatePlan* plan = new CreatePlan(stmt->type);
        plan->ifNotExists = stmt->ifNotExists;
        plan->type = stmt->type;
        plan->schema = stmt->schema;
        plan->tableName = stmt->tableName;
        plan->indexName = stmt->indexName;
        plan->columns = stmt->columns;
        plan->next = nullptr;

        if (plan->type == kCreateIndex) {
            Table* table = g_meta_data.getTable(plan->schema, plan->tableName);
            if (table == nullptr) {
                delete plan;
                return nullptr;
            }

            if (stmt->indexColumns != nullptr) {
                plan->indexColumns = new std::vector<ColumnDefinition*>;
            }

            for (auto col_name : *stmt->indexColumns) {
                ColumnDefinition* col_def = table->getColumn(col_name);
                if (col_def == nullptr) {
                    delete plan->indexColumns;
                    delete plan;
                    return nullptr;
                }
                plan->indexColumns->push_back(col_def);
            }
        }

        return plan;
    }

    Plan* Optimizer::createDropPlanTree(const DropStatement* stmt) {
        DropPlan* plan = new DropPlan();
        plan->type = stmt->type;
        plan->ifExists = stmt->ifExists;
        plan->schema = stmt->schema;
        plan->name = stmt->name;
        plan->indexName = stmt->indexName;
        plan->next = nullptr;
        return plan;
    }

    Plan* Optimizer::createInsertPlanTree(const InsertStatement* stmt) {
        InsertPlan* plan = new InsertPlan();
        return plan;
    }

    Plan* Optimizer::createUpdatePlanTree(const UpdateStatement* stmt) {
        UpdatePlan* plan = new UpdatePlan();
        plan->type = stmt->type;
        plan->table = g_meta_data.getTable(stmt->schema, stmt->tableName);
        plan->values = stmt->values;
        
        return plan;
    }

    Plan* Optimizer::createDeletePlanTree(const DeleteStatement* stmt) {
        DeletePlan* plan = new DeletePlan();
        return plan;
    }

    Plan* Optimizer::createSelectPlanTree(const SelectStatement* stmt) {
        SelectPlan* plan = new SelectPlan();
        return plan;
    }

    Plan* Optimizer::createTrxPlanTree(const TransactionStatement* stmt) {
        TrxPlan* plan = new TrxPlan();
        return plan;
    }

    Plan* Optimizer::createShowPlanTree(const ShowStatement* stmt) {
        ShowPlan* plan = new ShowPlan();
        plan->type = stmt->type;
        plan->schema = stmt->schema;
        plan->name = stmt->name;
        plan->next = nullptr;
        return plan;
    }

    Plan* Optimizer::createScanPlan() {
        ScanPlan* plan = new ScanPlan();
        return plan;
    }

    Plan* Optimizer::createFileterPlan() {
        FilterPlan* plan = new FilterPlan();
        return plan;
    }

    Plan* Optimizer::createProjdctionPlan() {
        ProjectionPlan* plan = new ProjectionPlan();
        return plan;
    }

    Plan* Optimizer::createSortPlan() {
        SortPlan* plan = new SortPlan();
        return plan;
    }

    Plan* Optimizer::createLimitPlan() {
        LimitPlan* plan = new LimitPlan();
        return plan;
    }

}