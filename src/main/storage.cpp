#include "storage.h"
#include "util.h"

#include "sql/ColumnType.h"

#include <cstdint>
#include <cstring>
#include <iostream>

using namespace hsql;

namespace mydb {

TableStore::TableStore(std::vector<ColumnDefinition*>* columns)
    : colNum_(columns->size()), tupleSize_(0), rowCount_(0), columns_(columns) {
  colOffset_.push_back(0);

  // Add space for each columns
  for (auto col : *columns) {
    tupleSize_ += ColumnTypeSize(col->type);
    colOffset_.push_back(tupleSize_);
  }

  // Add space for null map
  tupleSize_ += colNum_;

  // Add space for header
  tupleSize_ += sizeof(bool);
  tupleSize_ += sizeof(trx_id_t);
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

  tup->is_free = false;
  tup->trx_id = 0;
  int idx = 0;
  for(auto expr:*values){
    setColValues(tup,idx,expr);
    idx++;
  }

  rowCount_++;
  return false;
}

bool TableStore::deleteTuple(Tuple* tup){
    memset(tup,0,tupleSize_);
    tup->is_free = true;
    dataList_.delTuple(tup);
    freeList_.addHead(tup);
    return true;
}

bool TableStore::updateTuple(Tuple* tup, std::vector<UpdateClause*>* updates) {
  //tup->trx_id = NewTrxId();

  for (auto upd : *updates) {
    size_t idx = 0;
    for (auto col : *columns_) {
      if (strcmp(upd->column, col->name) == 0) {
        break;
      }
      idx++;
    }

    setColValue(tup,idx,upd->value);
  }

  return false;
}

bool TableStore::newTupleGroup() {
  Tuple* tuple_group =
      static_cast<Tuple*>(malloc(tupleSize_ * TUPLE_GROUP_SIZE));
  memset(tuple_group, 0, (tupleSize_ * TUPLE_GROUP_SIZE));
  if (tuple_group == nullptr) {
    std::cout << "# ERROR: Failed to malloc " << tupleSize_ * TUPLE_GROUP_SIZE
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