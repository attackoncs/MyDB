#pragma once

using namespace mydb {
enum UndoType { kInsert, kDelete, kUpdate };

struct Undo {
    Undo(UndoType t, tup_id_t id) : type(t), tup_id(id), data(nullptr) {}
    ~Undo() {
        if (data != nullptr) {
            free(data);
        }
    }
    UndoType type;
    tup_id_t tup_id;
    uchar* data;
};

}