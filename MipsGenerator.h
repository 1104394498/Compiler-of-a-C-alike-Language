#ifndef MIPSCODE_MIPSGENERATOR_H
#define MIPSCODE_MIPSGENERATOR_H

#include "TypeDefine.h"
#include <vector>
#include <map>
#include <stack>
#include <iostream>

using namespace std;

enum RegisterType {
    a, t, s, sp, ra, hi, lo, zero, v0
};

class TempRegisterAllocator {
    // Singleton; do round robin
private:
    int tempRegisterNum;
    vector<int> supplyLine;

    explicit TempRegisterAllocator(int _tempRegisterNum = 9) : tempRegisterNum{_tempRegisterNum} {
        for (int i = 0; i < tempRegisterNum; i++) {
            supplyLine.push_back(i);
        }
    }

public:
    static TempRegisterAllocator &get_instance() {
        static TempRegisterAllocator instance{};
        return instance;
    }

    void useRegister(int NO);

    int allocateRegister();
};

class Register {
private:
    RegisterType type;
    int NO;
public:
    explicit Register(RegisterType _type, int _NO = -1);

    string print();

    void useReg() const;

    bool operator<(const Register &aRegister) const;
};

enum MipsCodeType {
    dot_text,
    jal, jump, jr,
    add, sub, mul, divide,
    addi, subi, mulo,
    lw, sw,
    li,
    label,
    mov,
    beq, bne, bge, ble, bgt, blt,
    syscall
};

class MipsCmd {
private:
    MipsCodeType codeType;
    vector<string> operands;

public:
    explicit MipsCmd(MipsCodeType _codeType) : codeType{_codeType} {}

    void addRegister(Register aRegister) {
        operands.emplace_back(aRegister.print());
        aRegister.useReg();
    }

    void addLabel(const string &label) {
        operands.emplace_back(label);
    }

    void addInteger(int i) {
        operands.emplace_back(to_string(i));
    }

    string print();
};

class MipsGenerator {
public:
    explicit MipsGenerator(const vector<IntermediateCmd> &_intermediateCodes, vector<MipsCmd> *_mipsCodes)
            : intermediateCodes(&_intermediateCodes), mipsCodes{_mipsCodes} {
        generate();
    }


private:
    struct RunningFile {
        // Assume that input parameter region and $a0 - $a3 have been set before calling the function
        /* Structure of file
         * ------------------------------------    input_para_start   <--- $sp
         *      input parameter region
         * ------------------------------------    ra_loc
         *      ra region
         * ------------------------------------     temp_para_start
         *      temporary parameter region
         * ------------------------------------    file_end
         */

        int input_para_start = 0;           // offset of the input parameter region's start from $sp
        int ra_loc = 0;                     // offset of ra location from $sp
        int temp_para_start = 4;            // offset of parameter region's start from $sp
        int file_end = 4;                   // end of the file, pointing to the next position of the last parameter in the file

        map<Register, string> registerRecords;
        map<string, int> memoryRecords;     // key: variable name; value: offset from start of the file ($sp)
        map<string, VariableType> variableTypeRecords;  // the name of an array is assumed to have the same type as its elements
    };

    const vector<IntermediateCmd> *intermediateCodes;
    vector<MipsCmd> *mipsCodes;
    vector<IntermediateCmd> unhandled_push{};
    RunningFile *functionFile = nullptr;

    // get register
    Register getRegister(const string &varName);

    Register getRegister();

    /*
    Printf,         // printf
    Scanf,          // scanf
     */

    void generate() {
        mipsCodes->push_back(MipsCmd{dot_text});
        MipsCmd jalMain{jump};
        jalMain.addLabel("main");
        mipsCodes->push_back(jalMain);

        int input_para_count = 0;
        IntermediateCmd *lastMidCode = nullptr;

        for (const auto &midCode : *intermediateCodes) {
            OperatorType operatorType = midCode.getOperatorType();
            if (operatorType == FuncDefStart) {
                MipsCmd mipsCmd{MipsCodeType::label};
                mipsCmd.addLabel(midCode.getOperands().at(1));
                mipsCodes->push_back(mipsCmd);
                functionFile = new RunningFile{};
            } else if (operatorType == FuncPara) {
                const string &varName = midCode.getOperands().at(1);
                functionFile->ra_loc += 4;
                functionFile->temp_para_start = functionFile->ra_loc + 4;
                functionFile->file_end = functionFile->temp_para_start;

                input_para_count++;
                if (input_para_count <= 4)
                    functionFile->registerRecords[Register{a, input_para_count - 1}] = varName;
                functionFile->memoryRecords[varName] = (input_para_count - 1) * 4 + functionFile->input_para_start;
            } else if (operatorType == CallFuncPush) {
                unhandled_push.push_back(midCode);  // deal after saving environment
            } else if (operatorType == SaveEnv) {
                dealSaveEnv();
            } else if (operatorType == CallFunc) {
                MipsCmd changeSp{addi};
                changeSp.addRegister(Register{sp});
                changeSp.addRegister(Register{sp});
                changeSp.addInteger(functionFile->file_end - functionFile->input_para_start);
                mipsCodes->push_back(changeSp);

                MipsCmd Jal{jal};
                Jal.addLabel(midCode.getOperands().at(0));\
                mipsCodes->push_back(Jal);
            } else if (operatorType == Label) {
                MipsCmd mipsCmd{MipsCodeType::label};
                mipsCmd.addLabel(midCode.getOperands().at(0));
                mipsCodes->push_back(mipsCmd);
            } else if (operatorType == AssignRetValue) {
                const string &varName = midCode.getOperands().at(0);
                const Register &r = getRegister(varName);
                MipsCmd mipsCmd{mov};
                mipsCmd.addRegister(r);
                mipsCmd.addRegister(Register{v0});
                mipsCodes->push_back(mipsCmd);
                functionFile->variableTypeRecords[varName] =
            } else if (operatorType == FuncRetInDef) {
                // move $ra, [register of return variable]
                const Register &r = getRegister(midCode.getOperands().at(0));
                MipsCmd mipsCmd{mov};
                mipsCmd.addRegister(Register{v0});
                mipsCmd.addRegister(r);
                mipsCodes->push_back(mipsCmd);

                MipsCmd Jr{jr};
                Jr.addRegister(Register{ra});
                mipsCodes->push_back(Jr);
                delete functionFile;
            } else if (operatorType == RestoreEnv) {
                dealRestoreEnv();
            } else if (operatorType == Add || operatorType == Minus || operatorType == Mul || operatorType == Div) {
                dealArithmetic(operatorType, midCode);
            } else if (operatorType == Assign) {
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
                const VariableType constType = (midCode.getOperands().at(0) == "int") ? intType : charType;
                int num;
                if (constType == intType)
                    num = stoi(midCode.getOperands().at(2));
                else
                    num = midCode.getOperands().at(1)[0];

                const string &constName = midCode.getOperands().at(1);

                if (functionFile != nullptr) {
                    const Register &r = getRegister(constName);
                    MipsCmd liCmd{li};
                    liCmd.addRegister(r);
                    liCmd.addInteger(num);
                    mipsCodes->push_back(liCmd);
                    // functionFile->registerRecords[r] = constName;
                    functionFile->variableTypeRecords[constName] = constType;
                } else {
                    // global, handle later
                }
            } else if (operatorType == VarStatement) {
                const VariableType varType = (midCode.getOperands().at(0) == "int") ? intType : charType;
                const string &varName = midCode.getOperands().at(1);
                if (functionFile != nullptr) {
                    functionFile->variableTypeRecords[varName] = varType;
                    // functionFile->memoryRecords[varName] = functionFile->file_end;
                    // functionFile->file_end += 4;
                } else {
                    // global, handle later
                }
            } else if (operatorType == VarArrayStatement) {
                const string &arrayName = midCode.getOperands().at(1);
                // cout << midCode.getOperands().at(2) << endl;
                const int size = stoi(midCode.getOperands().at(2));
                const VariableType arrayType = (midCode.getOperands().at(0) == "int") ? intType : charType;
                if (functionFile != nullptr) {
                    functionFile->memoryRecords[arrayName] = functionFile->file_end;
                    functionFile->file_end += (size * 4);
                    functionFile->variableTypeRecords[arrayName] = arrayType;
                } else {
                    // global, handle later
                }
            } else if (operatorType == GetArrayValue) {
                dealGetArrayValue(midCode);
            } else if (operatorType == ArrayElemAssign) {
                dealArrayElemAssign(midCode);
            } else if (operatorType == Printf) {
                dealPrintf(midCode);
            } else if (operatorType == Scanf) {
                dealScanf(midCode);
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
};


#endif //MIPSCODE_MIPSGENERATOR_H
