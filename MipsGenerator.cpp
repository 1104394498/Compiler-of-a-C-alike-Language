#include "MipsGenerator.h"
#include <set>

void TempRegisterAllocator::useRegister(const Register &r) {
    auto iterator = supplyLine.begin();
    while (iterator != supplyLine.end()) {
        if (*iterator == r) {
            supplyLine.erase(iterator);
            break;
        }
        iterator++;
    }
    supplyLine.push_back(r);

}

Register TempRegisterAllocator::allocateRegister() {
    Register ret = supplyLine.at(0);
    supplyLine.erase(supplyLine.begin());
    supplyLine.push_back(ret);
    return ret;
}

Register::Register(RegisterType _type, int _NO) : type{_type}, NO{_NO} {}

string Register::print() const {
    string result = "$";
    switch (type) {
        case a:
            // cout << NO << endl;
            result += ("a" + to_string(NO));
            break;
        case t:
            result += ("t" + to_string(NO));
            // TempRegisterAllocator::get_instance().useRegister(NO);
            break;
        case s:
            result += ("s" + to_string(NO));
            break;
        case sp:
            result += "sp";
            break;
        case ra:
            result += "ra";
            break;
        case hi:
            result += "hi";
            break;
        case lo:
            result += "lo";
            break;
        case zero:
            result += "0";
            break;
        case v0:
            result += "v0";
            break;
        case v1:
            result += "v1";
            break;
        case k0:
            result += "k0";
            break;
        case k1:
            result += "k1";
            break;
        case fp:
            result += "fp";
            break;
    }
    return result;
}

bool Register::operator<(const Register &aRegister) const {
    if (this->type != aRegister.type) {
        return this->type < aRegister.type;
    }
    return this->NO < aRegister.NO;
}

void Register::useReg() const {
    if (type == t || type == s)
        TempRegisterAllocator::get_instance().useRegister(*this);
}

bool Register::operator==(const Register &aRegister) const {
    return (this->type == aRegister.type) && (this->NO == aRegister.NO);
}

bool Register::operator!=(const Register &aRegister) const {
    return !(*this == aRegister);
}


string MipsCmd::print() const {
    string ret;
    switch (codeType) {
        case dot_data:
            ret += ".data";
            break;
        case dot_text:
            ret += ".text";
            break;
        case dot_word:
            ret += operands.at(0);
            ret += ": .word ";
            if (operands.size() >= 2) {
                ret += operands.at(1);  // initial value
                if (operands.size() >= 3) {
                    ret += ":";
                    ret += operands.at(2);  // byte number
                }
            }
            break;
        case dot_asciiz:
            ret += operands.at(0);
            ret += ": .asciiz ";
            ret += operands.at(1);
            break;
        case jal:
            ret += "jal ";
            ret += operands.at(0);
            break;
        case add:
            ret += "add ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case sub:
            ret += "sub ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case mul:
            ret += "mul ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case divide:
            if (isdigit(operands.at(2).at(0))) {
                ret += "div ";
                ret += operands.at(0);
                ret += ", ";
                ret += operands.at(1);
                ret += ", ";
                ret += operands.at(2);
            } else {
                // div $r1, $r2
                ret += "div ";
                ret += operands.at(1);
                ret += ", ";
                ret += operands.at(2);

                ret += '\n';
                // mflo $r0
                ret += "mflo ";
                ret += operands.at(0);
            }
            break;
        case lw:
            ret += "lw ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += "(";
            ret += operands.at(2);
            ret += ")";
            break;
        case sw:
            ret += "sw ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += "(";
            ret += operands.at(2);
            ret += ")";
            break;
        case li:
            ret += "li ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            break;
        case label:
            ret += operands.at(0);
            ret += ":";
            break;
        case mov:
            ret += "move ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            break;
        case jump:
            ret += "j ";
            ret += operands.at(0);
            break;
        case addi:
            ret += "addi ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case subi:
            ret += "subi ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case jr:
            ret += "jr ";
            ret += operands.at(0);
            break;
        case mulo:
            ret += "mulo ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case beq:
            ret += "beq ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case bne:
            ret += "bne ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case bge:
            ret += "bge ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case ble:
            ret += "ble ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case bgt:
            ret += "bgt ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case blt:
            ret += "blt ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case syscall:
            ret += "syscall";
            break;
        case la:
            ret += "la ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            break;
        case sll:
            ret += "sll ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
            break;
        case comment:
            ret += "\n# ";
            ret += operands.at(0);
            break;
    }
    return ret;
}

// assistant function of getRegisters
Register *MipsGenerator::findDiffReg(const vector<Register> &ret) {
    // find a different register
    Register *newReg = nullptr;
    while (true) {
        delete newReg;
        newReg = new Register{TempRegisterAllocator::get_instance().allocateRegister()};
        bool isDiff = true;
        for (const Register &r : ret) {
            if (r == *newReg) {
                isDiff = false;
                break;
            }
        }
        if (isDiff)
            break;
    }
    return newReg;
}

// assistant function of getRegisters
void MipsGenerator::writeRegValueBack(const Register *newReg) {
    // if there is a value in the register, return the value back to the memory
    // find if the register is full
    const auto iter = functionFile->registerRecords.find(*newReg);
    if (iter != functionFile->registerRecords.end()) {
        // the register isn't empty. store the oldVar into memory
        const string &oldVarName = iter->second;
        const auto &oldIter = functionFile->memoryRecords.find(oldVarName);
        if (oldIter != functionFile->memoryRecords.end()) {
            // the old variable has its own memory space
            int oldOffset = oldIter->second;
            // may be optimized
            // sw newReg, oldOffset($sp)
            MipsCmd mipsCmd{sw};
            mipsCmd.addRegister(*newReg);
            mipsCmd.addInteger(oldOffset);
            mipsCmd.addRegister(Register{sp});
            mipsCodes->push_back(mipsCmd);
        } else if (globalNameTypeRecords.count(oldVarName) > 0) {
            // global
            // la $v1, oldVarName
            MipsCmd laCmd{la};
            laCmd.addRegister(Register{v1});
            laCmd.addLabel(oldVarName);
            mipsCodes->push_back(laCmd);
            // sw $newReg, 0($v1)
            MipsCmd swCmd{sw};
            swCmd.addRegister(*newReg);
            swCmd.addInteger(0);
            swCmd.addRegister(Register{v1});
            mipsCodes->push_back(swCmd);
        } else {
            // otherwise
            functionFile->memoryRecords[oldVarName] = functionFile->file_end;
            MipsCmd mipsCmd{sw};
            mipsCmd.addRegister(*newReg);
            mipsCmd.addInteger(functionFile->file_end);
            functionFile->file_end -= 4;
            mipsCmd.addRegister(Register{sp});
            mipsCodes->push_back(mipsCmd);
        }
        functionFile->registerRecords.erase(iter);
    }
}

void MipsGenerator::loadValueInReg(const string &varName, const Register *newReg) {
    // search registers
    for (const auto &pair : functionFile->registerRecords) {
        const Register &r = pair.first;
        const string &regVarName = pair.second;
        if (varName == regVarName) {
            // the value of the variable is in this reg
            if (r != *newReg) {
                // move $newReg, $r
                MipsCmd moveCmd{mov};
                moveCmd.addRegister(*newReg);
                moveCmd.addRegister(r);
                mipsCodes->push_back(moveCmd);
                functionFile->registerRecords[*newReg] = varName;
            }
            return;
        }
    }

    bool inGlobal = false;
    // not in registers, then search memory
    const auto iter1 = functionFile->memoryRecords.find(varName);
    int offset = 1;
    if (iter1 != functionFile->memoryRecords.end()) {
        offset = functionFile->memoryRecords[varName];
    } else if (globalNameTypeRecords.count(varName) > 0) {
        // not in the function scope, then search in the global scope
        inGlobal = true;
    } // else the variable is a new one

    // load the variable into newReg
    functionFile->registerRecords[*newReg] = varName;
    if (offset != 1) {
        // lw newReg, offset($sp)
        MipsCmd mipsCmd1{lw};
        mipsCmd1.addRegister(*newReg);
        mipsCmd1.addInteger(offset);
        mipsCmd1.addRegister(Register{sp});
        mipsCodes->push_back(mipsCmd1);
    } else if (inGlobal) {
        // la $newReg, varName
        MipsCmd laCmd{la};
        laCmd.addRegister(*newReg);
        laCmd.addLabel(varName);
        mipsCodes->push_back(laCmd);
        // lw $newReg, 0($newReg)
        MipsCmd lwCmd{lw};
        lwCmd.addRegister(*newReg);
        lwCmd.addInteger(0);
        lwCmd.addRegister(*newReg);
        mipsCodes->push_back(lwCmd);
    } else {
        newReg->useReg();
    }
}

vector<Register> MipsGenerator::getRegisters(const vector<string> &varNames, int requireNum) {
    vector<Register> ret{};
    for (const string &varName : varNames) {
        bool findReg = false;
        for (const auto &pair : functionFile->registerRecords) {
            if (pair.second == varName) {
                const Register &r = pair.first;
                ret.push_back(r);
                findReg = true;
                break;
            }
        }
        if (findReg)
            continue;

        // find a different register
        Register *newReg = findDiffReg(ret);
        writeRegValueBack(newReg);
        loadValueInReg(varName, newReg);

        ret.push_back(*newReg);
        delete newReg;
    }

    for (int i = 0; i < requireNum; i++) {
        // find a different register
        Register *newReg = findDiffReg(ret);
        writeRegValueBack(newReg);

        ret.push_back(*newReg);
        delete newReg;
    }
    return ret;
}

void MipsGenerator::dealSaveEnv() {
    // save $ra
    // sw $ra, ra_loc($sp)
    MipsCmd saveRa{sw};
    saveRa.addRegister(Register{ra});
    saveRa.addInteger(functionFile->ra_loc);
    saveRa.addRegister(Register{sp});
    mipsCodes->push_back(saveRa);

    for (const auto &pair : functionFile->registerRecords) {    // include $a0-$a3, $t_
        const Register &r = pair.first;
        const string &varName = pair.second;
        const auto &iter = functionFile->memoryRecords.find(varName);
        if (iter != functionFile->memoryRecords.end()) {
            // variable has been saved in memory
            // can be optimized: don't write back to memory if not change

            // sw $t, offset($sp)
            MipsCmd mipsCmd{sw};
            mipsCmd.addRegister(r);
            mipsCmd.addInteger(iter->second);
            mipsCmd.addRegister(Register{sp});
            mipsCodes->push_back(mipsCmd);
        } else {
            const auto &iter1 = globalNameTypeRecords.find(varName);
            if (iter1 != globalNameTypeRecords.end()) {
                // la $v1, varName
                MipsCmd mipsCmd{la};
                mipsCmd.addRegister(Register{v1});
                mipsCmd.addLabel(varName);
                mipsCodes->push_back(mipsCmd);

                // sw r, 0($v1)
                MipsCmd mipsCmd1{sw};
                mipsCmd1.addRegister(r);
                mipsCmd1.addInteger(0);
                mipsCmd1.addRegister(Register{v1});

                mipsCodes->push_back(mipsCmd1);
            } else {
                // variable hasn't been saved in memory

                // sw $t, stackEnd($sp)
                MipsCmd mipsCmd{sw};
                mipsCmd.addRegister(r);
                mipsCmd.addInteger(functionFile->file_end);
                functionFile->memoryRecords[varName] = functionFile->file_end;
                functionFile->file_end -= 4;
                mipsCmd.addRegister(Register{sp});
                mipsCodes->push_back(mipsCmd);
            }
        }
    }

    // save the current register records
    functionFile->savedRegisterRecords = functionFile->registerRecords;

    // deal with unhandled_push

    int para_num = 0;
    set<Register> revised_a_regs;
    for (const auto &midCmd : unhandled_push) {
        if (debug_info) {
            MipsCmd commentCmd{comment};
            commentCmd.addLabel(midCmd.print());
            mipsCodes->push_back(commentCmd);
        }
        const string operand = midCmd.getOperands().at(0);
        if (para_num < 4) {
            // store in $a_
            if (operand[0] == '\'') {
                // a char
                // li $a_, ascii(char)
                char c = operand[1];
                MipsCmd mipsCmd{li};
                mipsCmd.addRegister(Register{a, para_num});
                mipsCmd.addInteger(c);
                mipsCodes->push_back(mipsCmd);
                revised_a_regs.emplace(a, para_num);
            } else if (isdigit(operand[0])) {
                // a integer
                int num = stoi(operand);
                MipsCmd mipsCmd{li};
                mipsCmd.addRegister(Register{a, para_num});
                mipsCmd.addInteger(num);
                mipsCodes->push_back(mipsCmd);
                revised_a_regs.emplace(a, para_num);
            } else {
                // a variable
                const Register reg = getRegisters(vector<string>{operand}).at(0);
                if (revised_a_regs.count(reg) > 0) {
                    // Because $a_ containing a value are all saved in memory
                    int offset = functionFile->memoryRecords[operand];
                    // lw $a_{para_num}, offset($sp)
                    MipsCmd lwCmd{lw};
                    lwCmd.addRegister(Register(a, para_num));
                    lwCmd.addInteger(offset);
                    lwCmd.addRegister(Register(sp));
                    mipsCodes->push_back(lwCmd);
                } else {
                    MipsCmd mipsCmd{mov};
                    mipsCmd.addRegister(Register{a, para_num});
                    mipsCmd.addRegister(reg);
                    mipsCodes->push_back(mipsCmd);
                }
                if (reg != Register{a, para_num})
                    revised_a_regs.emplace(a, para_num);
            }
        } else {
            // store in memory
            int offset = functionFile->file_end - para_num * 4;
            if (operand[0] == '\'') {
                // a char
                // li r, char
                const Register r = getRegisters(vector<string>{}, 1).at(0);
                char c = operand[1];
                MipsCmd mipsCmd{li};
                mipsCmd.addRegister(r);
                mipsCmd.addInteger(c);
                mipsCodes->push_back(mipsCmd);
                // sw r, offset($sp)
                MipsCmd mipsCmd1{sw};
                mipsCmd1.addRegister(r);
                mipsCmd1.addInteger(offset);
                mipsCmd1.addRegister(Register{sp});
                mipsCodes->push_back(mipsCmd1);
            } else if (isdigit(operand[0])) {
                // a integer
                int num = stoi(operand);
                // li r, num
                const Register r = getRegisters(vector<string>{}, 1).at(0);
                MipsCmd mipsCmd{li};
                mipsCmd.addRegister(r);
                mipsCmd.addInteger(num);
                mipsCodes->push_back(mipsCmd);
                // sw r, offset($sp)
                MipsCmd mipsCmd1{sw};
                mipsCmd1.addRegister(r);
                mipsCmd1.addInteger(offset);
                mipsCmd1.addRegister(Register{sp});
                mipsCodes->push_back(mipsCmd1);
            } else {
                // a variable
                const Register reg = getRegisters(vector<string>{operand}).at(0);
                if (revised_a_regs.count(reg) > 0) {
                    // lw $v1, bias($sp)
                    int bias = functionFile->memoryRecords[operand];
                    MipsCmd lwCmd{lw};
                    lwCmd.addRegister(Register(v1));
                    lwCmd.addInteger(bias);
                    lwCmd.addRegister(Register(sp));
                    mipsCodes->push_back(lwCmd);
                    // sw $v1, offset($sp)
                    MipsCmd swCmd{sw};
                    swCmd.addRegister(Register{v1});
                    swCmd.addInteger(offset);
                    swCmd.addRegister(Register{sp});
                    mipsCodes->push_back(swCmd);
                } else {
                    // sw reg, offset($sp)
                    MipsCmd mipsCmd{sw};
                    mipsCmd.addRegister(reg);
                    mipsCmd.addInteger(offset);
                    mipsCmd.addRegister(Register{sp});
                    mipsCodes->push_back(mipsCmd);
                }
            }
        }
        para_num++;
    }
    unhandled_push.clear();
}

void MipsGenerator::dealRestoreEnv() {
    // restore register records
    functionFile->registerRecords = functionFile->savedRegisterRecords;
    functionFile->savedRegisterRecords.clear();
    // restore $sp
    MipsCmd changeSp{addi};
    changeSp.addRegister(Register{sp});
    changeSp.addRegister(Register{sp});
    changeSp.addInteger(functionFile->input_para_start - functionFile->file_end);
    mipsCodes->push_back(changeSp);

    // restore $ra
    // lw $ra, ra_loc($sp)
    MipsCmd restoreRa{lw};
    restoreRa.addRegister(Register{ra});
    restoreRa.addInteger(functionFile->ra_loc);
    restoreRa.addRegister(Register{sp});
    mipsCodes->push_back(restoreRa);

    // restore other registers
    for (const auto &pair : functionFile->registerRecords) {
        const Register reg = pair.first;
        const string varName = pair.second;
        // cout << functionFile->memoryRecords.count(varName) << endl;
        if (functionFile->memoryRecords.count(varName) > 0) {
            int offset = functionFile->memoryRecords[varName];
            // lw reg, offset($sp)
            MipsCmd mipsCmd{lw};
            mipsCmd.addRegister(reg);
            mipsCmd.addInteger(offset);
            mipsCmd.addRegister(Register{sp});
            mipsCodes->push_back(mipsCmd);
        } else {
            // la $v1, varName
            MipsCmd mipsCmd{la};
            // const Register tempReg = getRegisters(vector<string>{}, 1).at(0);
            Register tempReg{v1};
            mipsCmd.addRegister(tempReg),
                    mipsCmd.addLabel(varName);
            mipsCodes->push_back(mipsCmd);

            // lw reg, 0($v1)
            MipsCmd mipsCmd1{lw};
            mipsCmd1.addRegister(reg);
            mipsCmd1.addInteger(0);
            mipsCmd1.addRegister(tempReg);

            mipsCodes->push_back(mipsCmd1);
        }
    }
}

void MipsGenerator::dealAssign(const IntermediateCmd &midCode) {
    const auto &operands = midCode.getOperands();

    const string addNum = operands.at(1);
    if (isdigit(addNum[0])) {
        const Register r1 = getRegisters(vector<string>{operands.at(0)}).at(0);
        int num = stoi(addNum);
        MipsCmd liCmd{li};
        liCmd.addRegister(r1);
        liCmd.addInteger(num);
        mipsCodes->push_back(liCmd);
        if (functionFile->variableTypeRecords.count(operands.at(0)) == 0)
            functionFile->variableTypeRecords[operands.at(0)] = intType;
        // cout << liCmd.print() << endl;
    } else if (addNum[0] == '\'') {
        const Register r1 = getRegisters(vector<string>{operands.at(0)}).at(0);
        char c = addNum[1];
        MipsCmd liCmd{li};
        liCmd.addRegister(r1);
        liCmd.addInteger(c);
        mipsCodes->push_back(liCmd);
        if (functionFile->variableTypeRecords.count(operands.at(0)) == 0)
            functionFile->variableTypeRecords[operands.at(0)] = charType;
    } else {
        const vector<Register> regs = getRegisters(vector<string>{operands.at(0), addNum});
        const Register r1 = regs.at(0);
        const Register r2 = regs.at(1);
        MipsCmd assignCmd{mov};
        assignCmd.addRegister(r1);
        assignCmd.addRegister(r2);
        mipsCodes->push_back(assignCmd);

        if (globalNameTypeRecords.count(operands.at(0)) == 0) {
            // if varName1 is a global variable, we needn't update its value
            if (functionFile->variableTypeRecords.count(addNum) > 0) {
                functionFile->variableTypeRecords[operands.at(0)] = functionFile->variableTypeRecords[addNum];
            } else if (globalNameTypeRecords.count(addNum) > 0) {
                functionFile->variableTypeRecords[operands.at(0)] = globalNameTypeRecords[addNum];
            } else {
                printf("error in dealAssign: addNum type doesn't exist\n");
                exit(1);
            }
        }
    }
}

void MipsGenerator::dealArithmetic(OperatorType operatorType, const IntermediateCmd &midCode) {
    // operatorType can only be add, minus, mul and division
    bool isConst1 = false, isConst2 = false;
    int addNum1 = 0, addNum2 = 0;
    // 需要判断加数是否为字符或整型常量
    const auto &operands = midCode.getOperands();
    // const Register r = getRegister(operands.at(0));

    string add1 = operands.at(1);
    if (isdigit(add1[0])) {
        addNum1 = stoi(add1);
        isConst1 = true;
    } else if (add1[0] == '\'') {
        addNum1 = add1[1];
        isConst1 = true;
    }

    string add2 = operands.at(2);
    if (isdigit(add2[0])) {
        addNum2 = stoi(add2);
        isConst2 = true;
    } else if (add2[0] == '\'') {
        addNum2 = add2[1];
        isConst2 = true;
    }

    if (isConst1 && isConst2) {
        int num;
        if (operatorType == Add)
            num = addNum1 + addNum2;
        else if (operatorType == Minus)
            num = addNum1 - addNum2;
        else if (operatorType == Mul)
            num = addNum1 * addNum2;
        else
            num = addNum1 / addNum2;
        MipsCmd liCmd{li};
        const Register r = getRegisters(vector<string>{operands.at(0)}).at(0);
        liCmd.addRegister(r);
        liCmd.addInteger(num);
        mipsCodes->push_back(liCmd);
    } else if (!isConst1 && isConst2) {
        MipsCmd *mipsCmd;
        if (operatorType == Add)
            mipsCmd = new MipsCmd{addi};
        else if (operatorType == Minus)
            mipsCmd = new MipsCmd{subi};
        else if (operatorType == Mul)
            mipsCmd = new MipsCmd{mulo};
        else
            mipsCmd = new MipsCmd{divide};
        const vector<Register> regs = getRegisters(vector<string>{operands.at(0), add1});
        const Register r = regs.at(0);
        const Register r1 = regs.at(1);
        mipsCmd->addRegister(r);
        mipsCmd->addRegister(r1);
        mipsCmd->addInteger(addNum2);
        mipsCodes->push_back(*mipsCmd);
        delete mipsCmd;
    } else if (isConst1) { //&& !isConst2
        MipsCmd *mipsCmd;
        if (operatorType == Add) {
            mipsCmd = new MipsCmd{addi};
            const vector<Register> regs = getRegisters(vector<string>{operands.at(0), add2});
            const Register r = regs.at(0);
            const Register r2 = regs.at(1);
            mipsCmd->addRegister(r);
            mipsCmd->addRegister(r2);
            mipsCmd->addInteger(addNum1);
            mipsCodes->push_back(*mipsCmd);
        } else if (operatorType == Minus) {
            // subi $r, $r_add2, addNum1
            mipsCmd = new MipsCmd{subi};
            const vector<Register> regs = getRegisters(vector<string>{operands.at(0), add2});
            const Register r = regs.at(0);
            const Register r2 = regs.at(1);
            mipsCmd->addRegister(r);
            mipsCmd->addRegister(r2);
            mipsCmd->addInteger(addNum1);
            mipsCodes->push_back(*mipsCmd);
            // sub $r, $0, $r
            delete mipsCmd;
            mipsCmd = new MipsCmd{sub};
            mipsCmd->addRegister(r);
            mipsCmd->addRegister(Register{zero});
            mipsCmd->addRegister(r);
            mipsCodes->push_back(*mipsCmd);
        } else if (operatorType == Mul) {
            mipsCmd = new MipsCmd{mulo};
            const vector<Register> regs = getRegisters(vector<string>{operands.at(0), add2});
            const Register r = regs.at(0);
            const Register r2 = regs.at(1);
            mipsCmd->addRegister(r);
            mipsCmd->addRegister(r2);
            mipsCmd->addInteger(addNum1);
            mipsCodes->push_back(*mipsCmd);
        } else {
            const vector<Register> regs = getRegisters(vector<string>{operands.at(0), add2}, 1);
            const Register r = regs.at(0);
            const Register r2 = regs.at(1);
            Register newReg = regs.at(2);
            // li newReg, addNum1
            MipsCmd liCmd{li};
            liCmd.addRegister(newReg);
            liCmd.addInteger(addNum1);
            mipsCodes->push_back(liCmd);
            // div $r, $newReg, $r_add2
            mipsCmd = new MipsCmd{divide};
            mipsCmd->addRegister(r);
            mipsCmd->addRegister(newReg);
            mipsCmd->addRegister(r2);
            mipsCodes->push_back(*mipsCmd);
        }

        delete mipsCmd;
    } else {
        MipsCmd *mipsCmd;
        if (operatorType == Add)
            mipsCmd = new MipsCmd{add};
        else if (operatorType == Minus)
            mipsCmd = new MipsCmd{sub};
        else if (operatorType == Mul)
            mipsCmd = new MipsCmd{mul};
        else
            mipsCmd = new MipsCmd{divide};

        const vector<Register> regs = getRegisters(vector<string>{operands.at(0), add1, add2});
        const Register r = regs.at(0);
        const Register r1 = regs.at(1);
        const Register r2 = regs.at(2);

        mipsCmd->addRegister(r);
        mipsCmd->addRegister(r1);
        mipsCmd->addRegister(r2);

        mipsCodes->push_back(*mipsCmd);
        delete mipsCmd;
    }
    functionFile->variableTypeRecords[operands.at(0)] = intType;
}

void MipsGenerator::dealNeg(const IntermediateCmd &midCode) {
    const string varName1 = midCode.getOperands().at(0);
    const string varName2 = midCode.getOperands().at(1);

    // const Register r = getRegister(varName1);

    bool isConst2 = false;
    int num2 = 0;
    if (isdigit(varName2[0])) {
        isConst2 = true;
        num2 = stoi(varName2);
        functionFile->variableTypeRecords[varName1] = intType;
    } else if (varName2[0] == '\'') {
        isConst2 = true;
        num2 = varName2[1];
        functionFile->variableTypeRecords[varName1] = charType;
    }

    if (isConst2) {
        // subi $r, $0, num2
        const Register r = getRegisters(vector<string>{varName1}).at(0);
        MipsCmd subiCmd{subi};
        subiCmd.addRegister(r);
        subiCmd.addRegister(Register{zero});
        subiCmd.addInteger(num2);
        mipsCodes->push_back(subiCmd);
    } else {
        const vector<Register> regs = getRegisters(vector<string>{varName1, varName2});
        const Register r = regs.at(0);
        const Register r2 = regs.at(1);
        // sub $r, $0, $r2
        MipsCmd subCmd{sub};
        subCmd.addRegister(r);
        subCmd.addRegister(Register{zero});
        subCmd.addRegister(r2);
        mipsCodes->push_back(subCmd);
        if (globalNameTypeRecords.count(varName1) ==
            0) {   // if varName1 is a global variable, we needn't update its value
            if (functionFile->variableTypeRecords.count(varName2) > 0) {
                functionFile->variableTypeRecords[varName1] = functionFile->variableTypeRecords[varName2];
            } else if (globalNameTypeRecords.count(varName2) > 0) {
                functionFile->variableTypeRecords[varName1] = globalNameTypeRecords[varName2];
            } else {
                printf("error in dealNeg: varName2 type doesn't exist\n");
                exit(1);
            }
        }
    }
}

void MipsGenerator::dealBranch(IntermediateCmd *lastMidCode, const IntermediateCmd &midCode) {
    OperatorType lastCodeType = lastMidCode->getOperatorType();
    OperatorType operatorType = midCode.getOperatorType();
    MipsCmd *branch = nullptr;
    if (operatorType == BNZ) {
        if (lastCodeType == IsEqual)
            // beq $r1, $r2, label
            branch = new MipsCmd{beq};
        else if (lastCodeType == IsNeq)
            branch = new MipsCmd{bne};
        else if (lastCodeType == IsGeq)
            branch = new MipsCmd{bge};
        else if (lastCodeType == IsGreater)
            branch = new MipsCmd{bgt};
        else if (lastCodeType == IsLeq)
            branch = new MipsCmd{ble};
        else    // lastCodeType == IsLess
            branch = new MipsCmd{blt};
    } else {
        if (lastCodeType == IsNeq)
            // beq $r1, $r2, label
            branch = new MipsCmd{beq};
        else if (lastCodeType == IsEqual)
            branch = new MipsCmd{bne};
        else if (lastCodeType == IsLess)
            branch = new MipsCmd{bge};
        else if (lastCodeType == IsLeq)
            branch = new MipsCmd{bgt};
        else if (lastCodeType == IsGreater)
            branch = new MipsCmd{ble};
        else    // lastCodeType == IsGeq
            branch = new MipsCmd{blt};
    }

    const string &op0 = lastMidCode->getOperands().at(0);
    const string &op1 = lastMidCode->getOperands().at(1);

    vector<Register> regs;
    if (isdigit(op1[0])) {
        if (isdigit(op0[0])) {
            regs = getRegisters(vector<string>{}, 1);
            MipsCmd liCmd{li};
            liCmd.addRegister(regs[0]);
            liCmd.addInteger(atoi(op0.c_str()));
            mipsCodes->push_back(liCmd);
        } else {
            regs = getRegisters(vector<string>{op0});
        }
        branch->addRegister(regs[0]);
        branch->addInteger(atoi(op1.c_str()));
    } else {
        if (isdigit(op0[0])) {
            regs = getRegisters(vector<string>{op1}, 1);
            MipsCmd liCmd{li};
            liCmd.addRegister(regs[1]);
            liCmd.addInteger(atoi(op0.c_str()));
            mipsCodes->push_back(liCmd);
        } else {
            regs = getRegisters(vector<string>{op1, op0});
        }
        branch->addRegister(regs[1]);
        branch->addRegister(regs[0]);
    }

    branch->addLabel(midCode.getOperands().at(0));
    mipsCodes->push_back(*branch);
    delete branch;
    delete lastMidCode;
}

void MipsGenerator::dealDoWhileBNZ(IntermediateCmd *lastMidCode, const IntermediateCmd &midCode) {
    OperatorType lastCodeType = lastMidCode->getOperatorType();
    OperatorType operatorType = midCode.getOperatorType();
    MipsCmd *branch = nullptr;

    if (lastCodeType == IsEqual)
        // beq $r1, $r2, label
        branch = new MipsCmd{beq};
    else if (lastCodeType == IsNeq)
        branch = new MipsCmd{bne};
    else if (lastCodeType == IsGeq)
        branch = new MipsCmd{bge};
    else if (lastCodeType == IsGreater)
        branch = new MipsCmd{bgt};
    else if (lastCodeType == IsLeq)
        branch = new MipsCmd{ble};
    else    // lastCodeType == IsLess
        branch = new MipsCmd{blt};

    const vector<Register> regs = getRegisters(
            vector<string>{lastMidCode->getOperands().at(0), lastMidCode->getOperands().at(1)});

    // move $k0, $r0
    MipsCmd moveCmd1{mov};
    moveCmd1.addRegister(Register{k0});
    moveCmd1.addRegister(Register{regs.at(0)});
    mipsCodes->push_back(moveCmd1);

    // move $k1, $r1
    MipsCmd moveCmd2{mov};
    moveCmd2.addRegister(Register{k1});
    moveCmd2.addRegister(Register{regs.at(1)});
    mipsCodes->push_back(moveCmd2);

    // deal restore reg
    dealRestoreRegStatus();

    branch->addRegister(Register{k0});
    branch->addRegister(Register{k1});
    branch->addLabel(midCode.getOperands().at(0));
    mipsCodes->push_back(*branch);
    delete branch;
    delete lastMidCode;
}


void MipsGenerator::dealExit() {
    // li v0, 10
    MipsCmd setV0{li};
    setV0.addRegister(Register{v0});
    setV0.addInteger(10);
    mipsCodes->push_back(setV0);
    // syscall
    mipsCodes->push_back(MipsCmd{syscall});
}

void MipsGenerator::dealGetArrayValue(const IntermediateCmd &midCode) {
    const string varName = midCode.getOperands().at(0);
    const string arrayName = midCode.getOperands().at(1);
    // cout << midCode.getOperands().at(2) << endl;

    // const Register $addrReg = getRegister();

    if (isdigit(midCode.getOperands().at(2)[0])) {
        // local array
        vector<Register> regs;
        if (functionFile->variableTypeRecords.count(varName) > 0) {
            regs = getRegisters(vector<string>{varName}, 1);
        } else {
            regs = getRegisters(vector<string>{}, 2);
            functionFile->registerRecords[regs.at(0)] = varName;
            if (functionFile->variableTypeRecords.count(arrayName) > 0) {
                functionFile->variableTypeRecords[varName] = functionFile->variableTypeRecords[arrayName];
            } else {
                functionFile->variableTypeRecords[varName] = globalNameTypeRecords[arrayName];
            }
        }
        const Register &addrReg = regs.at(1);
        const Register &r = regs.at(0);
        if (functionFile->memoryRecords.count(arrayName) > 0) {
            // local array
            // addi $addrReg, $sp, arrayAddr
            const int arrayAddr = functionFile->memoryRecords[arrayName];
            MipsCmd addiCmd{addi};
            addiCmd.addRegister(addrReg);
            addiCmd.addRegister(Register(sp));
            addiCmd.addInteger(arrayAddr);
            mipsCodes->push_back(addiCmd);

            const int pos = -stoi(midCode.getOperands().at(2)) * 4;
            // lw $r, pos ($addrReg)
            MipsCmd lwCmd{lw};
            lwCmd.addRegister(r);
            lwCmd.addInteger(pos);
            lwCmd.addRegister(addrReg);
            mipsCodes->push_back(lwCmd);
        } else {
            // global array
            // la $addrReg, arrayName
            MipsCmd laCmd{la};
            laCmd.addRegister(addrReg);
            laCmd.addLabel(arrayName);
            mipsCodes->push_back(laCmd);

            const int pos = stoi(midCode.getOperands().at(2)) * 4;
            // lw $r, pos ($addrReg)
            MipsCmd lwCmd{lw};
            lwCmd.addRegister(r);
            lwCmd.addInteger(pos);
            lwCmd.addRegister(addrReg);
            mipsCodes->push_back(lwCmd);
        }

        if (functionFile->variableTypeRecords.count(varName) == 0) {
            // la $v1, varName
            MipsCmd laCmd{la};
            laCmd.addRegister(Register{v1});
            laCmd.addLabel(varName);
            mipsCodes->push_back(laCmd);
            // sw $r, 0($v1)
            MipsCmd swCmd{sw};
            swCmd.addRegister(r);
            swCmd.addInteger(0);
            swCmd.addRegister(Register{v1});
            mipsCodes->push_back(swCmd);
        }
    } else {
        vector<Register> regs;
        if (functionFile->variableTypeRecords.count(varName) > 0) {
            regs = getRegisters(vector<string>{midCode.getOperands().at(2), varName}, 2);
        } else {
            regs = getRegisters(vector<string>{midCode.getOperands().at(2)}, 3);
            functionFile->registerRecords[regs.at(1)] = varName;
            if (functionFile->variableTypeRecords.count(arrayName) > 0) {
                functionFile->variableTypeRecords[varName] = functionFile->variableTypeRecords[arrayName];
            } else {
                functionFile->variableTypeRecords[varName] = globalNameTypeRecords[arrayName];
            }
        }

        const Register &addrReg = regs.at(2);
        const Register &varReg = regs.at(1);
        const Register &posReg = regs.at(0);
        const Register &offsetReg = regs.at(3);

        if (functionFile->memoryRecords.count(arrayName) > 0) {
            // local
            // addi $addrReg, $sp, arrayAddr
            const int arrayAddr = functionFile->memoryRecords[arrayName];
            MipsCmd addiCmd{addi};
            addiCmd.
                    addRegister(addrReg);
            addiCmd.
                    addRegister(Register(sp)
            );
            addiCmd.
                    addInteger(arrayAddr);
            mipsCodes->
                    push_back(addiCmd);

            // sll $offsetReg, $posReg, 2
            MipsCmd sllCmd{sll};
            sllCmd.
                    addRegister(offsetReg);
            sllCmd.
                    addRegister(posReg);
            sllCmd.addInteger(2);
            mipsCodes->push_back(sllCmd);
            // sub $addrReg, $addrReg, $offsetReg
            MipsCmd subCmd{sub};
            subCmd.addRegister(addrReg);
            subCmd.addRegister(addrReg);
            subCmd.addRegister(offsetReg);
            mipsCodes->push_back(subCmd);
            // lw $varReg, 0($addrReg)
            MipsCmd lwCmd{lw};
            lwCmd.addRegister(varReg);
            lwCmd.addInteger(0);
            lwCmd.addRegister(addrReg);
            mipsCodes->push_back(lwCmd);
        } else {
            // global
            // la $addrReg, arrayName
            MipsCmd laCmd{la};
            laCmd.addRegister(addrReg);
            laCmd.addLabel(arrayName);
            mipsCodes->push_back(laCmd);
            // sll $offsetReg, $posReg, 2
            MipsCmd sllCmd{sll};
            sllCmd.addRegister(offsetReg);
            sllCmd.addRegister(posReg);
            sllCmd.addInteger(2);
            mipsCodes->push_back(sllCmd);
            // add $addrReg, $addrReg, $offsetReg
            MipsCmd addCmd{add};
            addCmd.addRegister(addrReg);
            addCmd.addRegister(addrReg);
            addCmd.addRegister(offsetReg);
            mipsCodes->push_back(addCmd);
            // lw $varReg, 0($addrReg)
            MipsCmd lwCmd{lw};
            lwCmd.addRegister(varReg);
            lwCmd.addInteger(0);
            lwCmd.addRegister(addrReg);
            mipsCodes->push_back(lwCmd);
        }

        if (functionFile->variableTypeRecords.count(varName) == 0) {
            // la $v1, varName
            MipsCmd laCmd{la};
            laCmd.addRegister(Register{v1});
            laCmd.addLabel(varName);
            mipsCodes->push_back(laCmd);
            // sw $varReg, 0($v1)
            MipsCmd swCmd{sw};
            swCmd.addRegister(varReg);
            swCmd.addInteger(0);
            swCmd.addRegister(Register{v1});
            mipsCodes->push_back(swCmd);
        }
    }

    if (globalNameTypeRecords.count(varName) == 0) {   // if varName1 is a global variable, we needn't update its value
        if (functionFile->variableTypeRecords.count(arrayName) > 0) {
            functionFile->variableTypeRecords[varName] = functionFile->variableTypeRecords[arrayName];
        } else if (globalNameTypeRecords.count(arrayName) > 0) {
            functionFile->variableTypeRecords[varName] = globalNameTypeRecords[arrayName];
        } else {
            printf("error in dealGetArrayValue: arrayName type doesn't exist\n");
            exit(1);
        }
    }
}

void MipsGenerator::dealArrayElemAssign(const IntermediateCmd &midCode) {
    const string varName = midCode.getOperands().at(2);
    const string arrayName = midCode.getOperands().at(0);
    // cout << midCode.getOperands().at(1) << endl;

    if (isdigit(midCode.getOperands().at(1)[0])) {
        vector<Register> regs;
        int varNum = 0;
        if (isdigit(varName[0])) {
            // var is an integer
            varNum = stoi(varName);
            regs = getRegisters(vector<string>{}, 1);
        } else if (varName[0] == '\'') {
            // var is a char
            varNum = varName[1];
            regs = getRegisters(vector<string>{}, 1);
        } else {
            // var is a variable
            regs = getRegisters(vector<string>{varName}, 1);
        }


        const Register addrReg = regs.back();

        if (functionFile->memoryRecords.count(arrayName) > 0) {
            // addi $addrReg, $sp, offset
            const int offset = functionFile->memoryRecords[arrayName];
            MipsCmd addiCmd{addi};
            addiCmd.addRegister(addrReg);
            addiCmd.addRegister(Register(sp));
            addiCmd.addInteger(offset);
            mipsCodes->push_back(addiCmd);

            const int pos = -stoi(midCode.getOperands().at(1)) * 4;

            if (regs.size() == 2) {
                // sw $r, pos($addrReg)
                const Register r = regs.at(0);
                MipsCmd swCmd{sw};
                swCmd.addRegister(r);
                swCmd.addInteger(pos);
                swCmd.addRegister(addrReg);
                mipsCodes->push_back(swCmd);
            } else {
                // var is an integer or char
                // li $v1, varNum
                MipsCmd liCmd{li};
                liCmd.addRegister(Register{v1});
                liCmd.addInteger(varNum);
                mipsCodes->push_back(liCmd);
                // sw $v1, pos($addrReg)
                MipsCmd swCmd{sw};
                swCmd.addRegister(Register(v1));
                swCmd.addInteger(pos);
                swCmd.addRegister(addrReg);
                mipsCodes->push_back(swCmd);
            }

        } else {
            // global
            // la $addrReg, arrayName
            MipsCmd laCmd{la};
            laCmd.addRegister(addrReg);
            laCmd.addLabel(arrayName);
            mipsCodes->push_back(laCmd);

            const int pos = stoi(midCode.getOperands().at(1)) * 4;
            if (regs.size() == 2) {
                // sw $r, pos($addrReg)
                const Register r = regs.at(0);
                MipsCmd swCmd{sw};
                swCmd.addRegister(r);
                swCmd.addInteger(pos);
                swCmd.addRegister(addrReg);
                mipsCodes->push_back(swCmd);
            } else {
                // var is an integer or char
                // li $v1, varNum
                MipsCmd liCmd{li};
                liCmd.addRegister(Register{v1});
                liCmd.addInteger(varNum);
                mipsCodes->push_back(liCmd);
                // sw $v1, pos($addrReg)
                MipsCmd swCmd{sw};
                swCmd.addRegister(Register(v1));
                swCmd.addInteger(pos);
                swCmd.addRegister(addrReg);
                mipsCodes->push_back(swCmd);
            }
        }
    } else {
        // array[n] = k
        // 我把下一句的2改为了1
        const string posVar = midCode.getOperands().at(1);
        vector<Register> regs;
        int varNum = 0;
        if (isdigit(varName[0])) {
            // var is an integer
            varNum = stoi(varName);
            regs = getRegisters(vector<string>{posVar}, 2);
        } else if (varName[0] == '\'') {
            // var is a char
            varNum = varName[1];
            regs = getRegisters(vector<string>{posVar}, 2);
        } else {
            // var is a variable
            regs = getRegisters(vector<string>{posVar, varName}, 2);
        }
        Register posReg{v1}, varReg{v1}, addrReg{v1}, offsetReg{v1};
        if (regs.size() == 4) {
            posReg = regs.at(0);
            varReg = regs.at(1);
            addrReg = regs.at(2);
            offsetReg = regs.at(3);
        } else {
            posReg = regs.at(0);
            addrReg = regs.at(1);
            offsetReg = regs.at(2);
        }

        if (functionFile->memoryRecords.count(arrayName) > 0) {
            // local
            const int offset = functionFile->memoryRecords[arrayName];
            // addi $addrReg, $sp, offset
            MipsCmd addiCmd{addi};
            addiCmd.addRegister(addrReg);
            addiCmd.addRegister(Register(sp));
            addiCmd.addInteger(offset);
            mipsCodes->push_back(addiCmd);
            // sll $offsetReg, $posReg, 2
            MipsCmd sllCmd{sll};
            sllCmd.addRegister(offsetReg);
            sllCmd.addRegister(posReg);
            sllCmd.addInteger(2);
            mipsCodes->push_back(sllCmd);
            // sub $addrReg, $addrReg, $offsetReg
            MipsCmd subCmd{sub};
            subCmd.addRegister(addrReg);
            subCmd.addRegister(addrReg);
            subCmd.addRegister(offsetReg);
            mipsCodes->push_back(subCmd);

            if (regs.size() != 4) {
                // li $varReg, varNum
                MipsCmd liCmd{li};
                liCmd.addRegister(varReg);
                liCmd.addInteger(varNum);
                mipsCodes->push_back(liCmd);
            }

            // sw $varReg, 0($addrReg)
            MipsCmd swCmd{sw};
            swCmd.addRegister(varReg);
            swCmd.addInteger(0);
            swCmd.addRegister(addrReg);
            mipsCodes->push_back(swCmd);
        } else {
            // global
            // la $addrReg, arrayName
            MipsCmd laCmd{la};
            laCmd.addRegister(addrReg);
            laCmd.addLabel(arrayName);
            mipsCodes->push_back(laCmd);
            // sll $offsetReg, $posReg, 2
            MipsCmd sllCmd{sll};
            sllCmd.addRegister(offsetReg);
            sllCmd.addRegister(posReg);
            sllCmd.addInteger(2);
            mipsCodes->push_back(sllCmd);
            // add $addrReg, $addrReg, $offsetReg
            MipsCmd addCmd{add};
            addCmd.addRegister(addrReg);
            addCmd.addRegister(addrReg);
            addCmd.addRegister(offsetReg);
            mipsCodes->push_back(addCmd);

            if (regs.size() != 4) {
                // li $varReg, varNum
                MipsCmd liCmd{li};
                liCmd.addRegister(varReg);
                liCmd.addInteger(varNum);
                mipsCodes->push_back(liCmd);
            }

            // sw $varReg, 0($addrReg)
            MipsCmd swCmd{sw};
            swCmd.addRegister(varReg);
            swCmd.addInteger(0);
            swCmd.addRegister(addrReg);
            mipsCodes->push_back(swCmd);
        }
    }
}

void MipsGenerator::dealPrintf(const IntermediateCmd &midCode) {
    // save $a0 value
    bool save_a0 = false;
    if (functionFile->registerRecords.count(Register{a, 0}) > 0) {
        save_a0 = true;
    }

    if (save_a0) {
        // move $v1, $a0
        MipsCmd movCmd{mov};
        movCmd.addRegister(Register{v1});
        movCmd.addRegister(Register{a, 0});
        mipsCodes->push_back(movCmd);
    }

    for (string op : midCode.getOperands()) {
        if (op[0] == '\"') {
            /*
            // a string
            // li $v0, 11  (print char)
            MipsCmd liCmd{li};
            liCmd.addRegister(Register{v0});
            liCmd.addInteger(11);
            mipsCodes->push_back(liCmd);

            for (const char &c : op) {
                if (c == '\"')
                    continue;

                // li $a0, c
                MipsCmd liCmd1{li};
                liCmd1.addRegister(Register{a, 0});
                liCmd1.addInteger(c);
                mipsCodes->push_back(liCmd1);
                // syscall
                mipsCodes->push_back(MipsCmd{syscall});
            }
             */
            static int printStrNO = 0;
            string strHead = "PrintString" + to_string(printStrNO);
            printStrNO++;

            // strHead: .asciiz "xxxx"
            // deal with \n, \t, etc
            auto iter = op.begin();
            while (iter != op.end()) {
                if (*iter == '\\') {
                    op.insert(iter, '\\');
                    iter++;
                }
                iter++;
            }

            MipsCmd stringDef{dot_asciiz};
            stringDef.addLabel(strHead);
            stringDef.addLabel(op);
            mipsCodes->insert(mipsCodes->begin() + 1, stringDef);

            // la $a0, strHead
            MipsCmd laCmd{la};
            laCmd.addRegister(Register(a, 0));
            laCmd.addLabel(strHead);
            mipsCodes->push_back(laCmd);

            // li $v0, 4
            MipsCmd liCmd{li};
            liCmd.addRegister(Register{v0});
            liCmd.addInteger(4);
            mipsCodes->push_back(liCmd);

            // syscall
            mipsCodes->push_back(MipsCmd{syscall});

        } else if (op[0] == '\'') {
            // a char
            MipsCmd liCmd{li};
            liCmd.addRegister(Register{v0});
            liCmd.addInteger(11);
            mipsCodes->push_back(liCmd);

            MipsCmd liCmd1{li};
            liCmd1.addRegister(Register{a, 0});
            liCmd1.addInteger(op[1]);
            mipsCodes->push_back(liCmd1);
            // syscall
            mipsCodes->push_back(MipsCmd{syscall});
        } else if (isdigit(op[0])) {
            // an int
            int num = stoi(op);
            // li $v0, 1  (print int)
            MipsCmd liCmd{li};
            liCmd.addRegister(Register{v0});
            liCmd.addInteger(1);
            mipsCodes->push_back(liCmd);

            // li $a0, num
            MipsCmd liCmd1{li};
            liCmd1.addRegister(Register{a, 0});
            liCmd1.addInteger(num);
            mipsCodes->push_back(liCmd1);
            // syscall
            mipsCodes->push_back(MipsCmd{syscall});
        } else {
            // a (const) variable
            VariableType variableType;
            if (functionFile->variableTypeRecords.count(op) > 0)
                variableType = functionFile->variableTypeRecords[op];
            else
                variableType = globalNameTypeRecords[op];

            if (variableType == intType) {
                // an int variable
                // li $v0, 1
                MipsCmd liCmd{li};
                liCmd.addRegister(Register{v0});
                liCmd.addInteger(1);
                mipsCodes->push_back(liCmd);
            } else {
                // a char variable
                // li $v0, 11
                MipsCmd liCmd{li};
                liCmd.addRegister(Register{v0});
                liCmd.addInteger(11);
                mipsCodes->push_back(liCmd);
            }

            // move $a0, $reg
            Register reg = getRegisters(vector<string>{op}).at(0);
            if (reg == Register{a, 0})
                reg = Register{v1};
            MipsCmd moveCmd{mov};
            moveCmd.addRegister(Register{a, 0});
            moveCmd.addRegister(reg);
            mipsCodes->push_back(moveCmd);
            // syscall
            mipsCodes->push_back(MipsCmd{syscall});
        }
    }

    // print \n
    // li $v0, 11  (print char)
    MipsCmd liCmd{li};
    liCmd.addRegister(Register{v0});
    liCmd.addInteger(11);
    mipsCodes->push_back(liCmd);
    // li $a0, '\n'
    MipsCmd liCmd1{li};
    liCmd1.addRegister(Register{a, 0});
    liCmd1.addInteger('\n');
    mipsCodes->push_back(liCmd1);
    // syscall
    mipsCodes->push_back(MipsCmd{syscall});

    // restore $a0 value
    if (save_a0) {
        // move $v1, $a0
        MipsCmd movCmd{mov};
        movCmd.addRegister(Register{a, 0});
        movCmd.addRegister(Register{v1});
        mipsCodes->push_back(movCmd);
    }
}

void MipsGenerator::dealScanf(const IntermediateCmd &midCode) {
    // save $a0 value
    bool save_a0 = false;
    if (functionFile->registerRecords.count(Register{a, 0}) > 0) {
        save_a0 = true;
    }
    if (save_a0) {
        // move $v1, $a0
        MipsCmd movCmd{mov};
        movCmd.addRegister(Register{v1});
        movCmd.addRegister(Register{a, 0});
        mipsCodes->push_back(movCmd);
    }

    VariableType variableType;
    for (const string &op : midCode.getOperands()) {
        if (functionFile->variableTypeRecords.count(op) > 0) {
            variableType = functionFile->variableTypeRecords[op];
        } else {
            variableType = globalNameTypeRecords[op];
        }

        if (variableType == intType) {
            // li $v0, 5
            MipsCmd liCmd{li};
            liCmd.addRegister(Register{v0});
            liCmd.addInteger(5);
            mipsCodes->push_back(liCmd);
        } else {
            // li $v0, 12
            MipsCmd liCmd{li};
            liCmd.addRegister(Register{v0});
            liCmd.addInteger(12);
            mipsCodes->push_back(liCmd);
        }
        // syscall
        mipsCodes->push_back(MipsCmd{syscall});

        // move $reg, $v0
        Register reg = getRegisters(vector<string>{op}).at(0);
        if (reg == Register{a, 0})
            reg = Register{v1};
        MipsCmd moveCmd{mov};
        moveCmd.addRegister(reg);
        moveCmd.addRegister(Register{v0});
        mipsCodes->push_back(moveCmd);
    }

    // restore $a0 value
    if (save_a0) {
        // move $a0, $v1
        MipsCmd movCmd{mov};
        movCmd.addRegister(Register{a, 0});
        movCmd.addRegister(Register{v1});
        mipsCodes->push_back(movCmd);
    }
}

void MipsGenerator::dealVarArrayStatement(const IntermediateCmd &midCode) {
    const string arrayName = midCode.getOperands().at(1);
    // cout << midCode.getOperands().at(2) << endl;
    const int size = stoi(midCode.getOperands().at(2));
    const VariableType arrayType = (midCode.getOperands().at(0) == "int") ? intType : charType;
    if (functionFile != nullptr) {
        functionFile->memoryRecords[arrayName] = functionFile->file_end;
        functionFile->file_end -= (size * 4);
        functionFile->variableTypeRecords[arrayName] = arrayType;
    } else {
        // global, handle later
        MipsCmd dotWordCmd{dot_word};
        dotWordCmd.addLabel(arrayName);
        dotWordCmd.addInteger(0);
        dotWordCmd.addInteger(size);
        mipsCodes->push_back(dotWordCmd);
        globalNameTypeRecords[arrayName] = arrayType;
    }
}

void MipsGenerator::dealVarStatement(const IntermediateCmd &midCode) {
    const VariableType varType = (midCode.getOperands().at(0) == "int") ? intType : charType;
    const string varName = midCode.getOperands().at(1);
    if (functionFile != nullptr) {
        functionFile->variableTypeRecords[varName] = varType;
        // functionFile->memoryRecords[varName] = functionFile->file_end;
        // functionFile->file_end -= 4;
    } else {
        // global, handle later
        MipsCmd dotWordCmd{dot_word};
        dotWordCmd.addLabel(varName);
        mipsCodes->push_back(dotWordCmd);
        globalNameTypeRecords[varName] = varType;
    }
}

void MipsGenerator::dealConstDef(const IntermediateCmd &midCode) {
    const VariableType constType = (midCode.getOperands().at(0) == "int") ? intType : charType;
    int num;
    if (constType == intType)
        num = stoi(midCode.getOperands().at(2));
    else
        num = midCode.getOperands().at(2)[1];

    const string constName = midCode.getOperands().at(1);

    if (functionFile != nullptr) {
        const Register r = getRegisters(vector<string>{constName}).at(0);
        MipsCmd liCmd{li};
        liCmd.addRegister(r);
        liCmd.addInteger(num);
        mipsCodes->push_back(liCmd);
        // functionFile->registerRecords[r] = constName;
        functionFile->variableTypeRecords[constName] = constType;
    } else {
        // global, handle later
        MipsCmd dotWordCmd{dot_word};
        dotWordCmd.addLabel(constName);
        dotWordCmd.addInteger(num);
        mipsCodes->push_back(dotWordCmd);
        globalNameTypeRecords[constName] = constType;
    }
}

void MipsGenerator::dealFunRetInDef(const IntermediateCmd &midCode) {
    // move $ra, [register of return variable]
    if (!midCode.getOperands().empty()) {
        if (isdigit(midCode.getOperands().at(0).at(0))) {
            int retNum = stoi(midCode.getOperands().at(0).c_str());
            // li $v0, retNum
            MipsCmd liCmd{li};
            liCmd.addRegister(Register(v0));
            liCmd.addInteger(retNum);
            mipsCodes->push_back(liCmd);
        } else if (midCode.getOperands().at(0).at(0) == '\'') {
            int retAscii = midCode.getOperands().at(0).at(1);
            // li $v0, retAscii
            MipsCmd liCmd{li};
            liCmd.addRegister(Register(v0));
            liCmd.addInteger(retAscii);
            mipsCodes->push_back(liCmd);
        } else {
            const Register r = getRegisters(vector<string>{midCode.getOperands().at(0)}).at(0);
            MipsCmd mipsCmd{mov};
            mipsCmd.addRegister(Register{v0});
            mipsCmd.addRegister(r);
            mipsCodes->push_back(mipsCmd);
        }

    }

    for (auto &pair : functionFile->registerRecords) {
        const Register &reg = pair.first;
        const string &varName = pair.second;
        if (!functionFile->variableTypeRecords.count(varName) && globalNameTypeRecords.count(varName)) {
            // la $v1, varName
            MipsCmd laCmd{la};
            laCmd.addRegister(Register(v1));
            laCmd.addLabel(varName);
            mipsCodes->push_back(laCmd);
            // sw $reg, 0($v1)
            MipsCmd swCmd{sw};
            swCmd.addRegister(reg);
            swCmd.addInteger(0);
            swCmd.addRegister(Register(v1));
            mipsCodes->push_back(swCmd);
        }
    }

    MipsCmd Jr{jr};
    Jr.addRegister(Register{ra});
    mipsCodes->push_back(Jr);
    //delete functionFile;
    //functionFile = nullptr;
}

void MipsGenerator::dealFunDefStart(const IntermediateCmd &midCode, int &paraCount) {
    static bool defFirst = true;
    if (defFirst) {
        mipsCodes->push_back(MipsCmd{dot_text});
        MipsCmd subSp{subi};
        subSp.addRegister(Register{sp});
        MipsCmd jalMain{jump};
        jalMain.addLabel("main");
        mipsCodes->push_back(jalMain);
        defFirst = false;
    }

    paraCount = 0;

    const string funcName = midCode.getOperands().at(1);

    MipsCmd mipsCmd{MipsCodeType::label};
    mipsCmd.addLabel(funcName);
    mipsCodes->push_back(mipsCmd);

    // there is no problem if funcFile == nullptr
    delete functionFile;
    functionFile = new RunningFile{};

    if (midCode.getOperands().at(0) == "int") {
        globalNameTypeRecords[funcName] = intType;
    } else if (midCode.getOperands().at(0) == "char") {
        globalNameTypeRecords[funcName] = charType;
    }
}

void MipsGenerator::dealFunPara(const IntermediateCmd &midCode, int &input_para_count) {
    const string varName = midCode.getOperands().at(1);
    functionFile->ra_loc -= 4;
    functionFile->temp_para_start = functionFile->ra_loc - 4;
    functionFile->file_end = functionFile->temp_para_start;

    input_para_count++;
    if (input_para_count <= 4)
        functionFile->registerRecords[Register{a, input_para_count - 1}] = varName;
    functionFile->memoryRecords[varName] = -(input_para_count - 1) * 4 + functionFile->input_para_start;
    if (midCode.getOperands().at(0) == "int") {
        functionFile->variableTypeRecords[varName] = intType;
    } else {
        functionFile->variableTypeRecords[varName] = charType;
    }

}

void MipsGenerator::dealCallFunc(const IntermediateCmd &midCode) {
    MipsCmd changeSp{subi};
    changeSp.addRegister(Register{sp});
    changeSp.addRegister(Register{sp});
    changeSp.addInteger(functionFile->input_para_start - functionFile->file_end);
    mipsCodes->push_back(changeSp);

    MipsCmd Jal{jal};
    Jal.addLabel(midCode.getOperands().at(0));
    mipsCodes->push_back(Jal);
    lastRetTypes.push(globalNameTypeRecords[midCode.getOperands().at(0)]);
}

void MipsGenerator::dealLabel(const IntermediateCmd &midCode) {
    MipsCmd mipsCmd{MipsCodeType::label};
    mipsCmd.addLabel(midCode.getOperands().at(0));
    mipsCodes->push_back(mipsCmd);
}

void MipsGenerator::dealAssignRetValue(const IntermediateCmd &midCode) {
    const string varName = midCode.getOperands().at(0);
    const Register r = getRegisters(vector<string>{varName}).at(0);
    MipsCmd mipsCmd{mov};
    mipsCmd.addRegister(r);
    mipsCmd.addRegister(Register{v0});
    mipsCodes->push_back(mipsCmd);
    functionFile->variableTypeRecords[varName] = lastRetTypes.top();
    lastRetTypes.pop();
}

void MipsGenerator::dealSaveRegStatus() {
    // functionFile cannot be nullptr
    storedSavedRegisterRecords.push(functionFile->registerRecords);
}

void MipsGenerator::dealRestoreRegStatus() {
    for (const auto &pair : storedSavedRegisterRecords.top()) {
        const Register r = pair.first;
        const string oldVarName = pair.second;
        if (functionFile->registerRecords.count(r) > 0) {
            writeRegValueBack(&r);
            loadValueInReg(oldVarName, &r);
        } else {
            loadValueInReg(oldVarName, &r);
            functionFile->registerRecords[r] = oldVarName;
        }
    }
    /*
    for (const auto &pair : functionFile->registerRecords) {
        const Register &r = pair.first;
        const string &curVarName = pair.second;
        const string &preVarName = storedSavedRegisterRecords.top()[r];
        if (curVarName != preVarName) {
            // change curVar into preVar for $r
            writeRegValueBack(&r);
            loadValueInReg(preVarName, &r);
        }
    }
     */
}
