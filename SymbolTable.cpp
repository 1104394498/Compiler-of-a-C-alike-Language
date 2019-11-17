#include "SymbolTable.h"

void SymbolTable::pushStack() {
    table.push_back(new vector<Item *>);
}

void SymbolTable::pushStack(vector<Item *> *stack) {
    // only used for parameter table of functions
    bool nameDuplicate = false;
    for (auto iter1 = stack->begin(); iter1 != stack->end(); iter1++) {
        for (auto iter2 = iter1 + 1; iter2 != stack->end();) {
            if ((*iter1)->name == (*iter2)->name) {
                delete (*iter2);
                stack->erase(iter2);
                nameDuplicate = true;
            } else {
                iter2++;
            }
        }
    }

    // generate address
    for (auto &item : *stack) {
        // assert(item->info == nullptr && (item->type == intType || item->type == charType));
        item->info = new int;
        *static_cast<int *>(item->info) = addr;
        if (item->type == intType) {
            addr += 4;
        } else {
            addr += 1;
        }
    }

    table.push_back(stack);

    if (nameDuplicate) {
        throw DuplicateNameDefinition{};
    }
}

void SymbolTable::popStack() {
    vector<Item *> *p = table.back();
    table.pop_back();
    if (!p->empty()) {
        // recover addr
        for (auto &item : *p) {
            if (item->type == constInt || item->type == constChar || item->type == intType || item->type == charType) {
                addr = *static_cast<int *>(item->info);
                break;
            } else if (item->type == charArray || item->type == intArray) {
                addr = static_cast<ArrayInfo *>(item->info)->beginAddr;
                break;
            }
        }
    }
    for (Item *ptr_item : *p) {
        delete (ptr_item);
    }
    delete (p);
}

const Item* SymbolTable::searchName(const string &name) {
    auto iterator1 = table.rbegin();
    while (iterator1 != table.rend()) {
        auto iterator2 = (*iterator1)->begin();
        while (iterator2 != (*iterator1)->end()) {
            if (name == (*iterator2)->name) {
                return *iterator2;
            }
            ++iterator2;
        }
        ++iterator1;
    }
    return nullptr;
}

void SymbolTable::addName(const string &name, VariableType type, void *info) {
    for (Item *&item : *table.back()) {
        // only check current symbol stack
        if (item->name == name) {
            throw DuplicateNameDefinition{};
        }
    }

    if (type == intType || type == constInt) {
        info = new int;
        *static_cast<int *>(info) = addr;
        addr += 4;
    } else if (type == charType || type == constChar) {
        info = new int;
        *static_cast<int *>(info) = addr;
        addr += 1;  // maybe add += 4 due to align
    } else if (type == intArray) {
        auto *arrayInfo = static_cast<ArrayInfo *>(info);
        arrayInfo->beginAddr = addr;
        addr += (arrayInfo->len * 4);
    } else if (type == charArray) {
        auto *arrayInfo = static_cast<ArrayInfo *>(info);
        arrayInfo->beginAddr = addr;
        addr += arrayInfo->len;
    }

    table.back()->push_back(new Item{name, type, info});
}

SymbolTable::~SymbolTable() {   // memory leak
    while (!table.empty()) {
        popStack();
    }
}

void SymbolTable::checkFuncVariable(const string &name, const FuncInfo &funcInfo) {
    // request: name is a function name existing in symbol table
    auto iterator1 = table.rbegin();
    while (iterator1 != table.rend()) {
        auto iterator2 = (*iterator1)->begin();
        while (iterator2 != (*iterator1)->end()) {
            if (name == (*iterator2)->name) {
                auto realInfo = *(static_cast<FuncInfo *>((*iterator2)->info));
                realInfo.check(funcInfo);
                return;
            }
            ++iterator2;
        }
        ++iterator1;
    }
}


