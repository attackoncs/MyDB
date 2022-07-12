#pragma once
#include "SQLParser.h"
#include "SQLParserResult.h"
#include "util/sqlhelper.h"

using namespace hsql;

namespace mydb {
    class Parser {
    public:
        Parser();
        ~Parser();

        bool parseStatement(std::string query);

        SQLParserResult* getResult() { return result_; }

    private:
        bool checkStmtsMeta();

        bool checkMeta(const SQLStatement* stmt);

        bool checkSelectStmt(const SelectStatement* stmt);

        bool checkInsertStmt(const InsertStatement* stmt);

        bool checkUpdateStmt(const UpdateStatement* stmt);

        bool checkDeleteStmt(const DeleteStatement* stmt);

        bool checkCreateStmt(const CreateStatement* stmt);

        bool checkDropStmt(const DropStatement* stmt);

        bool checkCreateIndexStmt(const CreateStatement* stmt);

        bool checkCreateTableStmt(const CreateStatement* stmt);

        Table* getTable(TableRef* table_ref);

        bool checkColumn(Table* table, char* col_name);

        bool checkExpr(Table* table, Expr* expr);

        bool checkValues(std::vector<ColumnDefinition*>* columns, std::vector<Expr*>* values);

        SQLParserResult* result_;
    };

}