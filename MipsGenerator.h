#ifndef MIPSCODE_MIPSGENERATOR_H
#define MIPSCODE_MIPSGENERATOR_H

#include "TypeDefine.h"
#include <vector>
#include <map>
#include <stack>
#include <iostream>

using namespace std;

enum RegisterType {
    a, t, s, sp, ra, hi, lo, zero, v0, v1, k0, k1, fp
};

class Register {
private:
    RegisterType type;
    int NO;
public:
    explicit Register(RegisterType _type, int _NO = -1);

    Register(const Register &r) {
        type = r.type;
        NO = r.NO;
    }

    string print() const;

    void useReg() const;

    bool operator<(const Register &aRegister) const;

    bool operator==(const Register &aRegister) const;

    bool operator!=(const Register &aRegister) const;
};


class TempRegisterAllocator {
    // Singleton; do round robin
private:
    int tempRegisterNum;
    int sRegisterNum;
    vector<Register> supplyLine;

    explicit TempRegisterAllocator(int _sRegisterNum = 8, int _tempRegisterNum = 9) : sRegisterNum{_sRegisterNum},
                                                                                      tempRegisterNum{
                                                                                              _tempRegisterNum} {
        for (int i = 0; i < tempRegisterNum; i++) {
            supplyLine.emplace_back(t, i);
        }
        /*
        for (int i = 0; i < sRegisterNum; i++) {
            supplyLine.emplace_back(s, i);
        }
         */
    }

public:
    static TempRegisterAllocator &get_instance() {
        static TempRegisterAllocator instance{};
        return instance;
    }

    void useRegister(const Register &r);

    Register allocateRegister();
};

enum MipsCodeType {
    dot_data, dot_text, dot_word, dot_asciiz,
    jal, jump, jr,
    add, sub, mul, divide,
    addi, subi, mulo,
    lw, sw,
    li,
    label,
    mov,
    beq, bne, bge, ble, bgt, blt,
    syscall,
    la,
    sll,
    comment // for debug
};

class MipsCmd {
private:
    MipsCodeType codeType;
    vector<string> operands;

public:
    explicit MipsCmd(MipsCodeType _codeType) : codeType{_codeType} {}

    void addRegister(const Register &aRegister) {
        operands.push_back(aRegister.print());
        aRegister.useReg();
    }

    void addLabel(const string &label) {
        operands.push_back(label);
    }

    void addInteger(int i) {
        operands.push_back(to_string(i));
    }

    string print() const;
};

class MipsGenerator {
public:
    explicit MipsGenerator(const vector<IntermediateCmd> &_intermediateCodes, vector<MipsCmd> *_mipsCodes)
            : intermediateCodes(&_intermediateCodes), mipsCodes{_mipsCodes} {
        generate();
    }

    ~MipsGenerator() {
        delete functionFile;
    }


private:
    struct RunningFile {
        // Assume that input parameter region and $a0 - $a3 have been set before calling the function
        /* Structure of file
         * ------------------------------------    input_para_start   <--- $sp          HIGH address
         *      input parameter region
         * ------------------------------------    ra_loc
         *      ra region
         * ------------------------------------     temp_para_start
         *      temporary parameter region
         * ------------------------------------    file_end                             LOW address
         */

        int input_para_start = 0;            // offset of the input parameter region's start from $sp
        int ra_loc = 0;                      // offset of ra location from $sp
        int temp_para_start = -4;            // offset of parameter region's start from $sp
        int file_end = -4;                   // end of the file, pointing to the next position of the last parameter in the file

        map<Register, string> registerRecords;
        map<string, int> memoryRecords;     // key: variable name; value: offset from start of the file ($sp)
        map<string, VariableType> variableTypeRecords;  // the name of an array is assumed to have the same type as its elements
        map<Register, string> savedRegisterRecords; // used for save registers records upon finishing saving environment
    };

    map<string, VariableType> globalNameTypeRecords;   // record the types of global variances, constants and functions
    const vector<IntermediateCmd> *intermediateCodes;
    vector<MipsCmd> *mipsCodes;
    vector<IntermediateCmd> unhandled_push{};
    RunningFile *functionFile = nullptr;
    stack<VariableType> lastRetTypes;

    stack<map<Register, string>> storedSavedRegisterRecords;

    // get register
    // Register getRegister(const string &varName);

    // Register getRegister();

    vector<Register> getRegisters(const vector<string> &varNames, int requireNum = 0);

    static Register *findDiffReg(const vector<Register> &ret);

    void writeRegValueBack(const Register *newReg);

    void loadValueInReg(const string &varName, const Register *newReg);

    void generate() {
        // .data
        mipsCodes->push_back(MipsCmd{dot_data});

        int input_para_count = 0;
        IntermediateCmd *lastMidCode = nullptr;

        for (const auto &midCode : *intermediateCodes) {
            // cout << midCode.print() << endl;
            if (debug_info && midCode.getOperatorType() != CallFuncPush) {
                MipsCmd commentCmd{comment};
                commentCmd.addLabel(midCode.print());
                mipsCodes->push_back(commentCmd);
            }

            OperatorType operatorType = midCode.getOperatorType();
            if (operatorType == FuncDefStart) {
                dealFunDefStart(midCode, input_para_count);
            } else if (operatorType == FuncPara) {
                dealFunPara(midCode, input_para_count); // 没问题
            } else if (operatorType == CallFuncPush) {
                unhandled_push.push_back(midCode);  // deal after saving environment    // 没问题
            } else if (operatorType == SaveEnv) {
                dealSaveEnv();  // 没问题
            } else if (operatorType == CallFunc) {
                dealCallFunc(midCode);  // 没问题
            } else if (operatorType == Label) {
                dealLabel(midCode); // 没问题
            } else if (operatorType == AssignRetValue) {    // 要改
                dealAssignRetValue(midCode);
            } else if (operatorType == FuncRetInDef) {
                dealFunRetInDef(midCode);
            } else if (operatorType == RestoreEnv) {
                dealRestoreEnv();
            } else if (operatorType == Add || operatorType == Minus || operatorType == Mul || operatorType == Div) {
                dealArithmetic(operatorType, midCode);
            } else if (operatorType == Assign) {    //要改
                dealAssign(midCode);
            } else if (operatorType == Goto) {
                MipsCmd jumpCmd{jump};
                jumpCmd.addLabel(midCode.getOperands().at(0));
                mipsCodes->push_back(jumpCmd);
            } else if (operatorType == Neg) {
                dealNeg(midCode);
            } else if (operatorType == IsEqual || operatorType == IsNeq || operatorType == IsGeq ||
                       operatorType == IsGreater || operatorType == IsLess || operatorType == IsLeq) {
                lastMidCode = new IntermediateCmd{operatorType}; // left to handle in branch operation
                for (const string &op : midCode.getOperands())
                    lastMidCode->addOperands(op);
            } else if (operatorType == BNZ || operatorType == BZ) {
                dealBranch(lastMidCode, midCode);
            } else if (operatorType == Exit) {
                dealExit();
            } else if (operatorType == ConstDef) {
                dealConstDef(midCode);
            } else if (operatorType == VarStatement) {
                dealVarStatement(midCode);
            } else if (operatorType == VarArrayStatement) {
                dealVarArrayStatement(midCode);
            } else if (operatorType == GetArrayValue) { //要改
                dealGetArrayValue(midCode);     // 这里还需要改，让全局变量也不出bug
            } else if (operatorType == ArrayElemAssign) {
                dealArrayElemAssign(midCode);   // 这里还需要改，让全局变量也不出bug
            } else if (operatorType == Printf) {
                dealPrintf(midCode);
            } else if (operatorType == Scanf) {
                dealScanf(midCode);
            } else if (operatorType == DoWhileBNZ) {
                dealDoWhileBNZ(lastMidCode, midCode);
            } else if (operatorType == LoopSaveRegStatus || operatorType == IfBegin) {
                dealSaveRegStatus();
            } else if (operatorType == LoopRestoreRegStatus) {
                dealRestoreRegStatus();
            } else if (operatorType == LoopEnd) {
                //
                //functionFile->registerRecords = storedSavedRegisterRecords.top();
                storedSavedRegisterRecords.pop();
            } else if (operatorType == ElseBegin) {
                // pop the register status in if begin
                auto ifBeginRegStatus = storedSavedRegisterRecords.top();
                storedSavedRegisterRecords.pop();
                // push the register status in if end
                storedSavedRegisterRecords.push(functionFile->registerRecords);
                // change the register status to the same as if begin
                functionFile->registerRecords = ifBeginRegStatus;
            } else if (operatorType == ElseEnd) {
                // restore the register status to the same as if end
                dealRestoreRegStatus();
                // pop if end register status
                auto ifEndRegStatus = storedSavedRegisterRecords.top();
                storedSavedRegisterRecords.pop();
                // change the register status to the same as if end
                functionFile->registerRecords = ifEndRegStatus;
            } else if (operatorType == IfEnd) {
                dealRestoreRegStatus();
                //
                //functionFile->registerRecords = storedSavedRegisterRecords.top();
                storedSavedRegisterRecords.pop();
            } else {
                // debug
                exit(1);
            }
        }
    }

    /********* deal with commands *************/
    void dealSaveEnv();

    void dealRestoreEnv();

    void dealAssign(const IntermediateCmd &midCode);

    void dealArithmetic(OperatorType operatorType, const IntermediateCmd &midCode);

    void dealNeg(const IntermediateCmd &midCode);

    void dealBranch(IntermediateCmd *lastMidCode, const IntermediateCmd &midCode);

    void dealExit();

    void dealGetArrayValue(const IntermediateCmd &midCode);

    void dealArrayElemAssign(const IntermediateCmd &midCode);

    void dealPrintf(const IntermediateCmd &midCode);

    void dealScanf(const IntermediateCmd &midCode);

    void dealVarArrayStatement(const IntermediateCmd &midCode);

    void dealVarStatement(const IntermediateCmd &midCode);

    void dealConstDef(const IntermediateCmd &midCode);

    void dealFunRetInDef(const IntermediateCmd &midCode);

    void dealFunDefStart(const IntermediateCmd &midCode, int &paraCount);

    void dealFunPara(const IntermediateCmd &midCode, int &input_para_count);

    void dealCallFunc(const IntermediateCmd &midCode);

    void dealLabel(const IntermediateCmd &midCode);

    void dealAssignRetValue(const IntermediateCmd &midCode);

    void dealDoWhileBNZ(IntermediateCmd *lastMidCode, const IntermediateCmd &midCode);

    void dealSaveRegStatus();

    void dealRestoreRegStatus();
};


#endif //MIPSCODE_MIPSGENERATOR_H
