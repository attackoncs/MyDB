#include "storage.h"
#include "trx.h"
#include "util.h"

#include "sql/ColumnType.h"
#include "sql/Expr.h"

#include <cstdint>
#include <cstring>
#include <iostream>

using namespace hsql;

namespace mydb {

    TableStore::TableStore(std::vector<ColumnDefinition*>* columns)
            : colNum_(columns->size()), tupleSize_(0), columns_(columns) {
        colOffset_.push_back(0);

        // Add space for each columns
        for (auto col : *columns) {
            tupleSize_ += ColumnTypeSize(col->type);
            colOffset_.push_back(tupleSize_);
        }

        // Add space for null map
        tupleSize_ += colNum_;

        // Add space for header
        tupleSize_ += TUPLE_HEADER_SIZE;
    }

    TableStore::~TableStore() {
        for (auto tuple_group : tupleGroups_) {
            free(tuple_group);
        }
    }

    bool TableStore::insertTuple(std::vector<Expr*>* values) {
        if (freeList_.isEmpty()) {
            if (newTupleGroup()) {
                return true;
            }
        }

        Tuple* tup = freeList_.popHead();
        dataList_.addHead(tup);

        int idx = 0;
        for (auto expr : *values) {
            setColValue(tup, idx, expr);
            idx++;
        }

        if (g_transaction.inTransaction()) {
            g_transaction.addInsertUndo(this, tup);
        }

        return false;
    }

    bool TableStore::deleteTuple(Tuple* tup) {
        dataList_.delTuple(tup);
        freeList_.addHead(tup);
        if (g_transaction.inTransaction()) {
            g_transaction.addDeleteUndo(this, tup);
        }

        return true;
    }

    void TableStore::removeTuple(Tuple* tup) {
        dataList_.delTuple(tup);
        freeList_.addHead(tup);
    }

    void TableStore::recoverTuple(Tuple *tup) {
        dataList_.addHead(tup);
    }

    void TableStore::freeTuple(Tuple* tup) {
        freeList_.addHead(tup);
    }

    bool TableStore::updateTuple(Tuple* tup, std::vector<size_t>& idxs, std::vector<Expr*>& values) {
        if (g_transaction.inTransaction()) {
            g_transaction.addUpdateUndo(this, tup);
        }

        for (size_t i = 0; i < idxs.size(); i++) {
            size_t idx = idxs[i];
            Expr* expr = values[i];
            setColValue(tup, idx, expr);
        }

        return false;
    }

    Tuple* TableStore::seqScan(Tuple* tup) {
        if (tup == nullptr) {
            return dataList_.getHead();
        } else {
            return dataList_.getNext(tup);
        }
    }

    void TableStore::parseTuple(Tuple* tup, std::vector<Expr*>& values) {
        bool* is_null = reinterpret_cast<bool*>(&tup->data[0]);
        uchar* data = tup->data + columns_->size();

        for (size_t i = 0; i < columns_->size(); i++) {
            Expr* e = nullptr;
            if (is_null[i]) {
                e = Expr::makeNullLiteral();
                values.push_back(e);
                continue;
            }

            ColumnDefinition* col = (*columns_)[i];
            int offset = colOffset_[i];
            int size = colOffset_[i + 1] - colOffset_[i];
            switch (col->type.data_type) {
                case DataType::INT: {
                    int64_t val = *reinterpret_cast<int32_t*>(data + offset);
                    e = Expr::makeLiteral(val);
                    break;
                }
                case DataType::LONG: {
                    int64_t val = *reinterpret_cast<int64_t*>(data + offset);
                    e = Expr::makeLiteral(val);
                    break;
                }
                case DataType::CHAR:
                case DataType::VARCHAR: {
                    char* val = static_cast<char*>(malloc(size));
                    memcpy(val, (data + offset), size);
                    e = Expr::makeLiteral(val);
                    break;
                }
                default:
                    break;
            }
            values.push_back(e);
        }
    }

    bool TableStore::newTupleGroup() {
        Tuple* tuple_group =
                static_cast<Tuple*>(malloc(tupleSize_ * TUPLE_GROUP_SIZE));
        memset(tuple_group, 0, (tupleSize_ * TUPLE_GROUP_SIZE));
        if (tuple_group == nullptr) {
            std::cout << "[BYDB-Error]  Failed to malloc " << tupleSize_ * TUPLE_GROUP_SIZE
                      << " bytes";
            return true;
        }

        tupleGroups_.push_back(tuple_group);
        uchar* ptr = reinterpret_cast<uchar*>(tuple_group);
        for (int i = 0; i < TUPLE_GROUP_SIZE; i++) {
            Tuple* tup = reinterpret_cast<Tuple*>(ptr);
            freeList_.addHead(tup);
            ptr += tupleSize_;
        }

        return false;
    }

    void TableStore::setColValue(Tuple* tup, int idx, Expr* expr) {
        bool* is_null = reinterpret_cast<bool*>(&tup->data[0]);
        uchar* data = tup->data + colNum_;
        int offset = colOffset_[idx];
        int size = colOffset_[idx + 1] - colOffset_[idx];
        uchar* ptr = &data[offset];
        is_null[idx] = false;

        switch (expr->type) {
            case kExprLiteralInt: {
                if (size == 4) {
                    *reinterpret_cast<int32_t*>(ptr) = static_cast<int>(expr->ival);
                } else {
                    *reinterpret_cast<int64_t*>(ptr) = expr->ival;
                }
                break;
            }
            case kExprLiteralFloat: {
                if (size == 4) {
                    *reinterpret_cast<float*>(ptr) = static_cast<float>(expr->fval);
                } else {
                    *reinterpret_cast<double*>(ptr) = expr->fval;
                }
                break;
            }
            case kExprLiteralString: {
                int len = strlen(expr->name);
                memcpy(ptr, expr->name, len);
                ptr[len] = '\0';
                break;
            }
            case kExprLiteralNull:
                is_null[idx] = true;
                break;
            default:
                break;
        }
    }

}