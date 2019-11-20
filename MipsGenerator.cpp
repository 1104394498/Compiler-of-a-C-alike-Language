#include "MipsGenerator.h"

void TempRegisterAllocator::useRegister(int NO) {
    auto iterator = supplyLine.begin();
    while (iterator != supplyLine.end()) {
        if (*iterator == NO) {
            supplyLine.erase(iterator);
            break;
        }
        iterator++;
    }
    supplyLine.push_back(NO);

}

int TempRegisterAllocator::allocateRegister() {
    int ret = supplyLine.at(0);
    supplyLine.push_back(ret);
    return ret;
}

Register::Register(RegisterType _type, int _NO) : type{_type}, NO{_NO} {
    /*
    switch (type) {
        case a:
            assert(NO >= 0 && NO <= 3);
            break;
        case t:
            assert(NO >= 0 && NO <= 9);
            break;
        case s:
            assert(NO >= 0 && NO <= 7);
            break;
        case sp:
        case ra:
        case hi:
        case lo:
        case zero:
            assert(NO == -1);
            break;
        case v0:
            break;
    }*/
}

string Register::print() {
    string result = "$";
    switch (type) {
        case a:
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
            result += "fp";
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
    if (type == t)
        TempRegisterAllocator::get_instance().useRegister(NO);
}


string MipsCmd::print() {
    string ret;
    switch (codeType) {
        case dot_text:
            ret += ".text";
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
            ret += "div ";
            ret += operands.at(0);
            ret += ", ";
            ret += operands.at(1);
            ret += ", ";
            ret += operands.at(2);
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
    }
    return ret;
}

Register MipsGenerator::getRegister(const string &varName) {
    // assume that varName is valid

    // find if the variable is in a register
    for (const auto &pair : functionFile->registerRecords) {
        if (pair.second == varName) {
            pair.first.useReg();
            return pair.first;
        }
    }

    // not in registers, then search memory
    const auto &iter1 = functionFile->memoryRecords.find(varName);
    int offset = -1;
    if (iter1 != functionFile->memoryRecords.end()) {
        offset = functionFile->memoryRecords[varName];
    }


    // load in a new register
    int newRegNO = TempRegisterAllocator::get_instance().allocateRegister();
    Register newReg{t, newRegNO};

    // find if the register is full
    const auto &iter = functionFile->registerRecords.find(newReg);
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
            mipsCmd.addRegister(newReg);
            mipsCmd.addInteger(oldOffset);
            mipsCmd.addRegister(Register{sp});
            mipsCodes->push_back(mipsCmd);
        } else {
            // otherwise
            functionFile->memoryRecords[varName] = functionFile->file_end;
            MipsCmd mipsCmd{sw};
            mipsCmd.addRegister(newReg);
            mipsCmd.addInteger(functionFile->file_end);
            functionFile->file_end += 4;
            mipsCmd.addRegister(Register{sp});
            mipsCodes->push_back(mipsCmd);
        }
    }

    // load the variable into newReg
    functionFile->registerRecords[newReg] = varName;
    if (offset != -1) {
        // lw newReg, offset($sp)
        MipsCmd mipsCmd1{lw};
        mipsCmd1.addRegister(newReg);
        mipsCmd1.addInteger(offset);
        mipsCmd1.addRegister(Register{sp});
        mipsCodes->push_back(mipsCmd1);
    } else {
        newReg.useReg();
    }
    return newReg;
}

Register MipsGenerator::getRegister() {
    // get a register
    int newRegNO = TempRegisterAllocator::get_instance().allocateRegister();
    Register newReg{t, newRegNO};

    // find if the register is full
    const auto &iter = functionFile->registerRecords.find(newReg);
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
            mipsCmd.addRegister(newReg);
            mipsCmd.addInteger(oldOffset);
            mipsCmd.addRegister(Register{sp});
            mipsCodes->push_back(mipsCmd);
        } else {
            // otherwise
            MipsCmd mipsCmd{sw};
            mipsCmd.addRegister(newReg);
            mipsCmd.addInteger(functionFile->file_end);
            functionFile->file_end += 4;
            mipsCmd.addRegister(Register{sp});
            mipsCodes->push_back(mipsCmd);
        }
    } else {
        newReg.useReg();
    }

    return newReg;
}

void MipsGenerator::dealSaveEnv() {
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
        } else {
            // variable hasn't been saved in memory

            // sw $t, stackEnd($sp)
            MipsCmd mipsCmd{sw};
            mipsCmd.addRegister(r);
            mipsCmd.addInteger(functionFile->file_end);
            functionFile->memoryRecords[varName] = functionFile->file_end;
            functionFile->file_end += 4;
            mipsCmd.addRegister(Register{sp});
        }
    }

    // deal with unhandled_push
    int para_num = 0;
    for (const auto &midCmd : unhandled_push) {
        const string &operand = midCmd.getOperands().at(0);
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
            } else if (isdigit(operand[0])) {
                // a integer
                int num = stoi(operand);
                MipsCmd mipsCmd{li};
                mipsCmd.addRegister(Register{a, para_num});
                mipsCmd.addInteger(num);
                mipsCodes->push_back(mipsCmd);
            } else {
                // a variable
                MipsCmd mipsCmd{mov};
                mipsCmd.addRegister(Register{a, para_num});
                // 写到这里了，要判断该variable存在哪里，考虑要写一个函数来查找某个变量的位置
                const Register &reg = getRegister(operand);
                mipsCmd.addRegister(reg);
                mipsCodes->push_back(mipsCmd);
            }
        } else {
            // store in memory
            int offset = functionFile->file_end + para_num * 4;
            if (operand[0] == '\'') {
                // a char
                // li r, char
                const Register &r = getRegister();
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
                const Register &r = getRegister();
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
                // sw reg, offset($sp)
                const Register &reg = getRegister(operand);
                MipsCmd mipsCmd{sw};
                mipsCmd.addRegister(reg);
                mipsCmd.addInteger(offset);
                mipsCmd.addRegister(Register{sp});
                mipsCodes->push_back(mipsCmd);
            }
        }
        para_num++;
    }
    unhandled_push.clear();
}

void MipsGenerator::dealRestoreEnv() {
    // restore $sp
    MipsCmd changeSp{subi};
    changeSp.addRegister(Register{sp});
    changeSp.addRegister(Register{sp});
    changeSp.addInteger(functionFile->file_end - functionFile->input_para_start);
    mipsCodes->push_back(changeSp);
    // restore other registers
    for (const auto &pair : functionFile->registerRecords) {
        const Register &reg = pair.first;
        const string &varName = pair.second;
        int offset = functionFile->memoryRecords[varName];
        // lw reg, offset($sp)
        MipsCmd mipsCmd{lw};
        mipsCmd.addRegister(reg);
        mipsCmd.addInteger(offset);
        mipsCmd.addRegister(Register{sp});
        mipsCodes->push_back(mipsCmd);
    }
}

void MipsGenerator::dealAssign(const IntermediateCmd &midCode) {
    const auto &operands = midCode.getOperands();
    const Register &r1 = getRegister(operands.at(0));
    const string &addNum = operands.at(1);
    if (isdigit(addNum[0])) {
        int num = stoi(addNum);
        MipsCmd liCmd{li};
        liCmd.addRegister(r1);
        liCmd.addInteger(num);
        mipsCodes->push_back(liCmd);
        functionFile->variableTypeRecords[operands.at(0)] = intType;
    } else if (addNum[0] == '\'') {
        char c = addNum[1];
        MipsCmd liCmd{li};
        liCmd.addRegister(r1);
        liCmd.addInteger(c);
        mipsCodes->push_back(liCmd);
        functionFile->variableTypeRecords[operands.at(0)] = charType;
    } else {
        const Register &r2 = getRegister(addNum);
        MipsCmd assignCmd{mov};
        assignCmd.addRegister(r1);
        assignCmd.addRegister(r2);
        mipsCodes->push_back(assignCmd);
        functionFile->variableTypeRecords[operands.at(0)] = functionFile->variableTypeRecords[addNum];
    }
}

void MipsGenerator::dealArithmetic(OperatorType operatorType, const IntermediateCmd &midCode) {
    // operatorType can only be add, minus, mul and division

    bool isConst1 = false, isConst2 = false;
    int addNum1 = 0, addNum2 = 0;
    // 需要判断加数是否为字符或整型常量
    const auto &operands = midCode.getOperands();
    const Register &r = getRegister(operands.at(0));

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
        mipsCmd->addRegister(r);
        mipsCmd->addRegister(getRegister(add1));
        mipsCmd->addInteger(addNum2);
        mipsCodes->push_back(*mipsCmd);
        delete mipsCmd;
    } else if (isConst1) { //&& !isConst2
        MipsCmd *mipsCmd;
        if (operatorType == Add) {
            mipsCmd = new MipsCmd{addi};
            mipsCmd->addRegister(r);
            mipsCmd->addRegister(getRegister(add2));
            mipsCmd->addInteger(addNum1);
            mipsCodes->push_back(*mipsCmd);
        } else if (operatorType == Minus) {
            // subi $r, $r_add2, addNum1
            mipsCmd = new MipsCmd{subi};
            mipsCmd->addRegister(r);
            mipsCmd->addRegister(getRegister(add2));
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
            mipsCmd->addRegister(r);
            mipsCmd->addRegister(getRegister(add2));
            mipsCmd->addInteger(addNum1);
            mipsCodes->push_back(*mipsCmd);
        } else {
            Register newReg = getRegister();
            // li newReg, addNum1
            MipsCmd liCmd{li};
            liCmd.addRegister(newReg);
            liCmd.addInteger(addNum1);
            mipsCodes->push_back(liCmd);
            // div $r, $newReg, $r_add2
            mipsCmd = new MipsCmd{divide};
            mipsCmd->addRegister(r);
            mipsCmd->addRegister(newReg);
            mipsCmd->addRegister(getRegister(add2));
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

        mipsCmd->addRegister(r);
        mipsCmd->addRegister(getRegister(add1));
        mipsCmd->addRegister(getRegister(add2));

        mipsCodes->push_back(*mipsCmd);
        delete mipsCmd;
    }

}

void MipsGenerator::dealNeg(const IntermediateCmd &midCode) {
    const string &varName1 = midCode.getOperands().at(0);
    const string &varName2 = midCode.getOperands().at(1);

    const Register &r = getRegister(varName1);

    bool isConst2 = false;
    int num2 = 0;
    if (isdigit(varName2[0])) {
        isConst2 = true;
        num2 = stoi(varName2);
    } else if (varName2[0] == '\'') {
        isConst2 = true;
        num2 = varName2[1];
    }

    if (isConst2) {
        // subi $r, $0, num2
        MipsCmd subiCmd{subi};
        subiCmd.addRegister(r);
        subiCmd.addRegister(Register{zero});
        subiCmd.addInteger(num2);
        mipsCodes->push_back(subiCmd);
    } else {
        // sub $r, $0, $r2
        MipsCmd subCmd{sub};
        subCmd.addRegister(r);
        subCmd.addRegister(Register{zero});
        subCmd.addRegister(getRegister(varName2));
        mipsCodes->push_back(subCmd);
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
    branch->addRegister(getRegister(lastMidCode->getOperands().at(0)));
    branch->addRegister(getRegister(lastMidCode->getOperands().at(1)));
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
    const string &varName = midCode.getOperands().at(0);
    const string &arrayName = midCode.getOperands().at(1);
    // cout << midCode.getOperands().at(2) << endl;

    const Register &addrReg = getRegister();
    // li $addrReg, arrayAddr
    const int arrayAddr = functionFile->memoryRecords[arrayName];
    MipsCmd liCmd{li};
    liCmd.addRegister(addrReg);
    liCmd.addInteger(arrayAddr);
    mipsCodes->push_back(liCmd);

    if (isdigit(midCode.getOperands().at(2)[0])) {
        const int pos = stoi(midCode.getOperands().at(2));
        // lw $r, pos ($addrReg)
        MipsCmd lwCmd{lw};
        const Register &r = getRegister(varName);
        lwCmd.addRegister(r);
        lwCmd.addInteger(pos * 4);
        lwCmd.addRegister(addrReg);
        mipsCodes->push_back(lwCmd);
    } else {
        const Register &posReg = getRegister(midCode.getOperands().at(2));
        const Register &offsetReg = getRegister();
        // mulo $offsetReg, $posReg, 4
        MipsCmd muloCmd{mulo};
        muloCmd.addRegister(offsetReg);
        muloCmd.addRegister(posReg);
        muloCmd.addInteger(4);
        mipsCodes->push_back(muloCmd);

        // add $addrReg, $addrReg, $offsetReg
        MipsCmd addCmd{add};
        addCmd.addRegister(addrReg);
        addCmd.addRegister(addrReg);
        addCmd.addRegister(offsetReg);
        mipsCodes->push_back(addCmd);

        // lw $r, 0($addrReg)
        MipsCmd lwCmd{lw};
        const Register &r = getRegister(varName);
        lwCmd.addRegister(r);
        lwCmd.addInteger(0);
        lwCmd.addRegister(addrReg);
        mipsCodes->push_back(lwCmd);
    }
}

void MipsGenerator::dealArrayElemAssign(const IntermediateCmd &midCode) {
    const string &varName = midCode.getOperands().at(2);
    const string &arrayName = midCode.getOperands().at(0);
    cout << midCode.getOperands().at(1) << endl;

    const Register &addrReg = getRegister();
    // li $addrReg, arrayAddr
    const int arrayAddr = functionFile->memoryRecords[arrayName];
    MipsCmd liCmd{li};
    liCmd.addRegister(addrReg);
    liCmd.addInteger(arrayAddr);
    mipsCodes->push_back(liCmd);

    if (isdigit(midCode.getOperands().at(1)[0])) {
        const int pos = stoi(midCode.getOperands().at(1)) * 4;

        // sw $r, pos($addrReg)
        const Register &r = getRegister(varName);
        MipsCmd swCmd{sw};
        swCmd.addRegister(r);
        swCmd.addInteger(pos);
        swCmd.addRegister(addrReg);
        mipsCodes->push_back(swCmd);
    } else {
        const Register &posReg = getRegister(midCode.getOperands().at(2));
        const Register &offsetReg = getRegister();
        // mulo $offsetReg, $posReg, 4
        MipsCmd muloCmd{mulo};
        muloCmd.addRegister(offsetReg);
        muloCmd.addRegister(posReg);
        muloCmd.addInteger(4);
        mipsCodes->push_back(muloCmd);

        // add $addrReg, $addrReg, $offsetReg
        MipsCmd addCmd{add};
        addCmd.addRegister(addrReg);
        addCmd.addRegister(addrReg);
        addCmd.addRegister(offsetReg);
        mipsCodes->push_back(addCmd);

        // sw $r, 0($addrReg)
        MipsCmd swCmd{sw};
        const Register &r = getRegister(varName);
        swCmd.addRegister(r);
        swCmd.addInteger(0);
        swCmd.addRegister(addrReg);
        mipsCodes->push_back(swCmd);
    }
}

void MipsGenerator::dealPrintf(const IntermediateCmd &midCode) {
    for (const string &op : midCode.getOperands()) {
        if (op[0] == '\"') {
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
            VariableType variableType = functionFile->variableTypeRecords[op];
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
            const Register &reg = getRegister(op);
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
}

void MipsGenerator::dealScanf(const IntermediateCmd &midCode) {
    for (const string &op : midCode.getOperands()) {
        VariableType variableType = functionFile->variableTypeRecords[op];
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
        const Register& reg = getRegister(op);
        MipsCmd moveCmd{mov};
        moveCmd.addRegister(reg);
        moveCmd.addRegister(Register{v0});
        mipsCodes->push_back(moveCmd);
    }
}

