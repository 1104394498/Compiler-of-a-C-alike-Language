#include "Optimizer.h"
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <iostream>

using namespace std;

void Optimizer::optimize() {
    // not implemented yet
    // should ensure that a[1] = b[1] ==> t1 = b[1]; a[1] = t1;
    divideCodeBlocks();
    // optimize_redundant_assign();
    activeVarAnalysis();

    constantPropagation();

    /* in-block optimization */
    // DAG_optimize();


    for (const CodeBlock *codeBlock : codeBlocks) {
        for (const IntermediateCmd &cmd : codeBlock->intermediateCodes) {
            opt_intermediateCodes.push_back(cmd);
        }
    }

}

void Optimizer::optimize_redundant_assign() {
    bool has_change;
    do {
        has_change = false;
        vector<IntermediateCmd>::iterator iter1, iter2;
        iter1 = intermediateCodes.begin();
        while (iter1 != intermediateCodes.end()) {
            if (iter1->getOperatorType() == Assign) {
                string varName = iter1->getOperands().at(0);
                string subVarName = iter1->getOperands().at(1);
                iter2 = iter1 + 1;
                while (iter2 != intermediateCodes.end()) {
                    if (iter2->getOperands().size() < 2) {
                        iter2++;
                        continue;
                    }
                    string varName0 = iter2->getOperands().at(0);
                    string varName1 = iter2->getOperands().at(1);
                    OperatorType type2 = iter2->getOperatorType();
                    if (type2 == Add || type2 == Minus || type2 == Mul || type2 == Div || type2 == Neg ||
                        type2 == Assign || type2 == AssignRetValue || type2 == GetArrayValue) {
                        if (varName == varName0) {
                            break;
                        }
                    }

                    if (type2 == Assign) {
                        if (varName1 == varName) {
                            IntermediateCmd assignCmd{Assign};
                            assignCmd.addOperands(varName0);
                            assignCmd.addOperands(subVarName);
                            *iter2 = assignCmd;
                            has_change = true;
                        }
                    } else if (type2 == Add) {

                    }
                    iter2++;
                }
            }
            if (has_change) {
                intermediateCodes.erase(iter1);
                break;
            }
            iter1++;
        }
    } while (has_change);
}

void Optimizer::divideCodeBlocks() {
    // First, find all entrance sentences
    vector<int> entranceIdx;
    // add the first
    entranceIdx.push_back(0);
    for (int lineNO = 1; lineNO < (int) intermediateCodes.size(); lineNO++) {
        OperatorType operatorType = intermediateCodes.at(lineNO).getOperatorType();
        if (operatorType == FuncDefStart || operatorType == Label) {
            if (entranceIdx.back() != lineNO)
                entranceIdx.push_back(lineNO);
        } else if (operatorType == Goto || operatorType == BZ || operatorType == BNZ || operatorType == CallFunc ||
                   operatorType == FuncRetInDef || operatorType == DoWhileBNZ) {
            if (lineNO + 1 < intermediateCodes.size())
                entranceIdx.push_back(lineNO + 1);
        }
    }


    // Second, create all code blocks
    for (int i = 0; i < (int) entranceIdx.size(); i++) {
        auto *pCodeBlock = new CodeBlock{};
        if (i == entranceIdx.size() - 1) {
            for (int j = entranceIdx.at(i); j < (int) intermediateCodes.size(); j++) {
                pCodeBlock->intermediateCodes.push_back(intermediateCodes.at(j));
            }
        } else {
            for (int j = entranceIdx.at(i); j < entranceIdx.at(i + 1); j++) {
                pCodeBlock->intermediateCodes.push_back(intermediateCodes.at(j));
            }
        }
        codeBlocks.push_back(pCodeBlock);
    }
    codeBlocks.push_back(new CodeBlock{});  // end block, nextBlocks is empty

    // Finally, deal with the connectivity among the code blocks
    for (int i = 0; i < (int) codeBlocks.size() - 1; i++) { // i < size - 1 to skip the end block
        CodeBlock *codeBlock = codeBlocks.at(i);
        const IntermediateCmd &lastCmd = codeBlock->intermediateCodes.back();
        OperatorType operatorType = lastCmd.getOperatorType();
        if (operatorType == Goto) {
            string targetLabel = lastCmd.getOperands().at(0);
            for (auto aCodeBlock : codeBlocks) {
                IntermediateCmd firstCmd = aCodeBlock->intermediateCodes.front();
                if (firstCmd.getOperatorType() == Label && firstCmd.getOperands().at(0) == targetLabel) {
                    codeBlock->nextBlocks.push_back(aCodeBlock);
                    aCodeBlock->lastBlocks.push_back(codeBlock);
                    break;
                }
            }
        } else if (operatorType == BZ || operatorType == BNZ || operatorType == DoWhileBNZ) {
            string targetLabel = lastCmd.getOperands().at(0);
            for (auto aCodeBlock : codeBlocks) {
                IntermediateCmd firstCmd = aCodeBlock->intermediateCodes.front();
                if (firstCmd.getOperatorType() == Label && firstCmd.getOperands().at(0) == targetLabel) {
                    codeBlock->nextBlocks.push_back(aCodeBlock);
                    aCodeBlock->lastBlocks.push_back(codeBlock);
                    break;
                }
            }

            codeBlock->nextBlocks.push_back(codeBlocks.at(i + 1));
            codeBlocks.at(i + 1)->lastBlocks.push_back(codeBlock);
        } else if (operatorType == CallFunc) {
            /*
              string targetFuncName = lastCmd.getOperands().at(0);
            for (auto aCodeBlock : codeBlocks) {
                IntermediateCmd firstCmd = aCodeBlock->intermediateCodes.front();
                if (firstCmd.getOperatorType() == FuncDefStart && firstCmd.getOperands().at(1) == targetFuncName) {
                    codeBlock->nextBlocks.push_back(aCodeBlock);
                    aCodeBlock->lastBlocks.push_back(codeBlock);
                    break;
                }
            }
            */
            codeBlock->nextBlocks.push_back(codeBlocks.at(i + 1));
            codeBlocks.at(i + 1)->lastBlocks.push_back(codeBlock);
        } else if (operatorType == FuncRetInDef) {
            /*
            // find the function name
            string funcName;
            for (int j = i; j >= 0; j--) {
                CodeBlock *curBlock = codeBlocks.at(j);
                const IntermediateCmd &firstCmd = curBlock->intermediateCodes.front();
                if (firstCmd.getOperatorType() == FuncDefStart) {
                    funcName = firstCmd.getOperands().at(1);
                    break;
                }
            }
            // debug
            if (funcName.empty()) {
                printf("In divide code blocks: cannot find function name\n");
                exit(1);
            }

            // find call funcName
            for (int j = 0; j < (int) codeBlocks.size() - 1; j++) {
                const IntermediateCmd &aLastCmd = codeBlocks.at(j)->intermediateCodes.back();
                if (aLastCmd.getOperatorType() == CallFunc && aLastCmd.getOperands().at(0) == funcName) {
                    codeBlock->nextBlocks.push_back(codeBlocks.at(j + 1));
                    codeBlocks.at(j + 1)->lastBlocks.push_back(codeBlock);
                }
            }
             */
            codeBlock->nextBlocks.push_back(codeBlocks.back());
            codeBlocks.back()->lastBlocks.push_back(codeBlock);
        } else {
            int nextBlockIdx = i + 1;
            while (true) {
                const auto &nextIntermidiateCmds = codeBlocks.at(nextBlockIdx)->intermediateCodes;
                if (nextIntermidiateCmds.empty())
                    break;
                const IntermediateCmd &firstCmd = nextIntermidiateCmds.at(0);
                if (firstCmd.getOperatorType() == FuncDefStart) {
                    if (firstCmd.getOperands().at(1) != "main") {
                        nextBlockIdx++;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
            codeBlock->nextBlocks.push_back(codeBlocks.at(nextBlockIdx));
            codeBlocks.at(nextBlockIdx)->lastBlocks.push_back(codeBlock);
        }
    }
    /*
    int count = 0;
    for (const auto &codeBlock : codeBlocks) {
        cout << "code block " << ++count << endl;
        for (const auto &cmd : codeBlock->intermediateCodes) {
            cout << cmd.print() << endl;
        }
        cout << endl;
        cout << "next blocks:" << endl;
        for (const auto &nextCodeBlock : codeBlock->nextBlocks) {
            if (nextCodeBlock->intermediateCodes.empty())
                cout << "empty" << endl;
            else
                cout << nextCodeBlock->intermediateCodes.at(0).print() << endl;
        }
        cout << endl;
    }
     */
}

void Optimizer::DAG_optimize() {
    DAG dag{tempVarGenerator};
    for (auto &i : codeBlocks) {
        CodeBlock *codeBlock = i;
        for (const IntermediateCmd &intermediateCmd : codeBlock->intermediateCodes) {
            dag.addIntermediateCmd(intermediateCmd);
        }
        auto *newCodeBlock = new CodeBlock{dag.getCodeBlock()};
        dag.clearCodeBlock();
        i = newCodeBlock;
        delete codeBlock;
    }
}


void Optimizer::activeVarAnalysis() {
    // create a same structure of active variable analysis as code blocks
    for (CodeBlock *codeBlock : codeBlocks) {
        auto *streamBlock = new StreamBlock{};
        streamBlock->pCodeBlock = codeBlock;
        codeBlock->pActiveVar = streamBlock;
        activeVars.push_back(streamBlock);
    }

    for (CodeBlock *codeBlock : codeBlocks) {
        for (CodeBlock *nextCodeBlock : codeBlock->nextBlocks) {
            codeBlock->pActiveVar->nextBlocks.push_back(nextCodeBlock->pActiveVar);
            nextCodeBlock->pActiveVar->lastBlocks.push_back(codeBlock->pActiveVar);
        }
    }

    // Construct kill(def) and gen(used) set for each code block
    for (CodeBlock *codeBlock : codeBlocks) {
        auto &def = codeBlock->pActiveVar->kill;
        auto &used = codeBlock->pActiveVar->gen;
        auto &used_in_block = codeBlock->pActiveVar->used_in_block;

        for (const IntermediateCmd &cmd : codeBlock->intermediateCodes) {
            const vector<string> &operands = cmd.getOperands();
            switch (cmd.getOperatorType()) {
                case FuncPara:
                    def.insert(operands.at(1));
                    break;
                case CallFuncPush:
                    if (def.count(operands.at(0)) == 0)
                        used.insert(operands.at(0));
                    used_in_block.insert(operands.at(0));
                    break;
                case CallFunc:
                    break;
                case FuncRetInDef:
                    if (!operands.empty()) {
                        if (def.count(operands.at(0)) == 0)
                            used.insert(operands.at(0));
                        used_in_block.insert(operands.at(0));
                    }
                    def.insert("RET");
                    break;
                case Add:
                case Minus:
                case Mul:
                case Div:
                    if (!isdigit(operands.at(1).at(0)) && operands.at(1).at(0) != '\'') {
                        if (def.count(operands.at(1)) == 0)
                            used.insert(operands.at(1));
                        used_in_block.insert(operands.at(1));
                    }

                    if (!isdigit(operands.at(2).at(0)) && operands.at(2).at(0) != '\'') {
                        if (def.count(operands.at(2)) == 0)
                            used.insert(operands.at(2));
                        used_in_block.insert(operands.at(2));
                    }
                    def.insert(operands.at(0));
                    break;
                case Neg:
                case Assign:
                    if (!isdigit(operands.at(1).at(0)) && operands.at(1).at(0) != '\'') {
                        if (def.count(operands.at(1)) == 0)
                            used.insert(operands.at(1));
                        used_in_block.insert(operands.at(1));
                    }
                    def.insert(operands.at(0));
                    break;
                case IsEqual:
                case IsLess:
                case IsLeq:
                case IsGreater:
                case IsGeq:
                case IsNeq:
                    if (def.count(operands.at(0)) == 0)
                        used.insert(operands.at(0));
                    used_in_block.insert(operands.at(0));
                    if (def.count(operands.at(1)) == 0)
                        used.insert(operands.at(1));
                    used_in_block.insert(operands.at(1));
                    break;
                case AssignRetValue:
                    used.insert("RET");
                    used_in_block.insert("RET");
                    def.insert(operands.at(0));
                    break;
                case GetArrayValue:
                    def.insert(operands.at(0));
                    used.insert(operands.at(1)); // name of array. Array name won't be in def
                    used_in_block.insert(operands.at(1));
                    if (!isdigit(operands.at(2).at(0))) {
                        if (def.count(operands.at(2)) == 0)
                            used.insert(operands.at(2));     // index is a variable
                        used_in_block.insert(operands.at(2));
                    }
                    break;
                case Printf:
                    for (auto &s : operands) {
                        if (s[0] != '\"' && s[0] != '\'' && !isdigit(s[0])) {
                            if (def.count(s) == 0)
                                used.insert(s);
                            used_in_block.insert(s);
                        }
                    }
                    break;
                case Scanf:
                    for (auto &s : operands) {
                        def.insert(s);
                    }
                    break;
                case ArrayElemAssign:
                    // Don't add the name of array into def
                    if (!isdigit(operands.at(1).at(0))) {
                        if (def.count(operands.at(1)) == 0)
                            used.insert(operands.at(1));
                        used_in_block.insert(operands.at(1));

                    }
                    if (!isdigit(operands.at(2).at(0)) && operands.at(2).at(0) != '\'') {
                        if (def.count(operands.at(2)) == 0)
                            used.insert(operands.at(2));
                        used_in_block.insert(operands.at(2));
                    }
                    break;
                case ConstDef:
                case VarStatement:
                case VarArrayStatement:
                    def.insert(operands.at(1));
                    break;
                case LoopSaveRegStatus:
                case LoopRestoreRegStatus:
                case DoWhileBNZ:
                case FuncDefStart:
                case Goto:
                case BNZ:
                case BZ:
                case Label:
                case SaveEnv:
                case RestoreEnv:
                case Exit:
                    break;
            }
        }
    }

    // add global non-constant variables and arrays into use set of end block
    auto &used_endBlock = activeVars.back()->gen;
    const auto &firstCodeBlock = codeBlocks.front();
    if (firstCodeBlock->intermediateCodes.front().getOperatorType() != FuncDefStart) {
        // global initialization
        for (const IntermediateCmd &intermediateCmd : firstCodeBlock->intermediateCodes) {
            OperatorType type = intermediateCmd.getOperatorType();
            if (type == VarStatement || type == VarArrayStatement) {
                const string &varName = intermediateCmd.getOperands().at(1);
                used_endBlock.insert(varName);
            }
        }
    }

    // from bottom to top, construct out and in for every activeVar block
    bool noChange = true;
    set<StreamBlock *> vis;
    queue<StreamBlock *> toDeal;
    while (true) {
        noChange = true;
        vis.clear();
        // add the end block into queue
        toDeal.push(activeVars.back());
        while (!toDeal.empty()) {
            StreamBlock *curBlock = toDeal.front();
            toDeal.pop();
            if (vis.count(curBlock) == 0) {
                vis.insert(curBlock);
                for (StreamBlock *lastBlock : curBlock->lastBlocks) {
                    if (vis.count(lastBlock) == 0) {
                        toDeal.push(lastBlock);
                    }
                }

                auto &def = curBlock->kill;
                auto &used = curBlock->gen;
                auto &out = curBlock->out;
                auto &in = curBlock->in;

                // update out
                set<string> new_out = out;
                for (StreamBlock *nextBlock : curBlock->nextBlocks) {
                    auto &next_in = nextBlock->in;
                    set_union(new_out.begin(), new_out.end(), next_in.cbegin(), next_in.cend(),
                              inserter(new_out, new_out.begin()));
                }
                if (out != new_out) {
                    noChange = false;
                    out = new_out;
                }

                // update in
                set<string> out_minus_def;
                set_difference(out.cbegin(), out.cend(), def.cbegin(), def.cend(),
                               inserter(out_minus_def, out_minus_def.begin()));

                set<string> new_in;
                set_union(out_minus_def.cbegin(), out_minus_def.cend(), used.cbegin(), used.cend(),
                          inserter(new_in, new_in.begin()));

                if (in != new_in) {
                    in = new_in;
                    noChange = false;
                }
            }
        }
        if (noChange)
            break;
    }


    /*
    int count = 0;
    for (StreamBlock *block : activeVars) {
        std::cout << "NO. " << ++count << ": " << std::endl;
        std::cout << "IN: " << std::endl;
        for (const string &s : block->in) {
            std::cout << s << std::endl;
        }
        std::cout << "OUT: " << endl;
        for (const string &s : block->out) {
            std::cout << s << std::endl;
        }
        std::cout << std::endl;
    }
     */

}

void Optimizer::constantPropagation() {
    OperatorType operatorType;

    /* 涉及到被替换的数：
     * CallFuncPush
     * Add, Mul, Div, Minus
     * Neg
     * IsEqual, IsLess,IsLeq, IsGreater, IsGeq, IsNeq
     * Assign
     * FuncRetInDef
     * GetArrayValue (索引数)（索引不能直接为整数！！）
     * printf
     * ArrayElemAssign (索引数和赋值的数)（索引数不能直接为整数！！）
     */

    /* 涉及到替换的数：
     * Assign
     */

    for (CodeBlock *codeBlock : codeBlocks) {
        /*
         * 思路：在一个代码块内，首先找Assign，将左值(不在出口活跃变量里)用右值替换，直到左值出现在第一类语句的左边为止。并且删除这个Assign
         * */

        bool noChange;
        do {
            noChange = true;
            for (auto iter = codeBlock->intermediateCodes.begin(); iter != codeBlock->intermediateCodes.end(); iter++) {
                if (iter->getOperatorType() == Assign) { // find Assign command
                    auto &operands = iter->getOperands();
                    const string &leftVar = operands.at(0);
                    const string &rightVar = operands.at(1);
                    if (leftVar == rightVar) {
                        codeBlock->intermediateCodes.erase(iter);
                        iter--;
                    }
                    if (codeBlock->pActiveVar->out.count(leftVar) != 0) // ensure leftVar is not in out
                        continue;

                    for (auto iter1 = iter + 1; iter1 != codeBlock->intermediateCodes.end(); iter1++) {
                        bool leftBeChanged = false;
                        OperatorType type = iter1->getOperatorType();
                        auto &operands1 = iter1->getOperands();
                        switch (type) {
                            case CallFuncPush:
                                if (operands1.at(0) == leftVar) {
                                    IntermediateCmd newCmd{CallFuncPush};
                                    newCmd.addOperands(rightVar);
                                    *iter1 = newCmd;
                                }
                                break;
                            case Add:
                            case Mul:
                            case Div:
                            case Minus: {
                                IntermediateCmd newCmd{type};
                                newCmd.addOperands(operands1.at(0));
                                if (operands1.at(1) == leftVar) {
                                    newCmd.addOperands(rightVar);
                                } else {
                                    newCmd.addOperands(operands1.at(1));
                                }
                                if (operands1.at(2) == leftVar) {
                                    newCmd.addOperands(rightVar);
                                } else {
                                    newCmd.addOperands(operands1.at(2));
                                }
                                *iter1 = newCmd;
                                if (operands1.at(0) == leftVar || operands1.at(0) == rightVar) {
                                    leftBeChanged = true;
                                }
                            }
                                break;
                            case Neg:
                                if (operands1.at(1) == leftVar) {
                                    IntermediateCmd newCmd{Neg};
                                    newCmd.addOperands(operands1.at(0));
                                    newCmd.addOperands(rightVar);
                                    *iter1 = newCmd;
                                }
                                if (operands1.at(0) == leftVar || operands1.at(0) == rightVar) {
                                    leftBeChanged = true;
                                }
                                break;
                            case IsEqual:
                            case IsLess:
                            case IsLeq:
                            case IsGeq:
                            case IsGreater:
                            case IsNeq: {
                                IntermediateCmd newCmd{type};
                                if (operands1.at(0) == leftVar) {
                                    newCmd.addOperands(rightVar);
                                } else {
                                    newCmd.addOperands(operands1.at(0));
                                }
                                if (operands1.at(1) == leftVar) {
                                    newCmd.addOperands(rightVar);
                                } else {
                                    newCmd.addOperands(operands1.at(1));
                                }
                                *iter1 = newCmd;
                            }
                                break;
                            case Assign:
                                if (operands1.at(1) == leftVar) {
                                    IntermediateCmd newCmd{Assign};
                                    newCmd.addOperands(operands1.at(0));
                                    newCmd.addOperands(rightVar);
                                    *iter1 = newCmd;
                                }
                                if (operands1.at(0) == leftVar || operands1.at(0) == rightVar) {
                                    leftBeChanged = true;
                                }
                                break;
                            case FuncRetInDef:
                                if (!operands1.empty() && operands1.at(0) == leftVar) {
                                    IntermediateCmd newCmd{FuncRetInDef};
                                    newCmd.addOperands(rightVar);
                                    *iter1 = newCmd;
                                }
                                break;
                            case GetArrayValue:
                                /* if (!isdigit(rightVar[0]) &&
                                    operands1.at(2) == leftVar) {   // exclude the case that index is an integer */
                                if (operands1.at(2) == leftVar) {
                                    IntermediateCmd newCmd{GetArrayValue};
                                    newCmd.addOperands(operands1.at(0));
                                    newCmd.addOperands(operands1.at(1));
                                    newCmd.addOperands(rightVar);
                                    *iter1 = newCmd;
                                }
                                if (operands1.at(0) == leftVar || operands1.at(0) == rightVar) {
                                    leftBeChanged = true;
                                }
                                break;
                            case Printf: {
                                IntermediateCmd newCmd{Printf};
                                for (const string &s : operands1) {
                                    if (s == leftVar) {
                                        newCmd.addOperands(rightVar);
                                    } else {
                                        newCmd.addOperands(s);
                                    }
                                }
                                *iter1 = newCmd;
                            }
                                break;
                            case ArrayElemAssign: {
                                IntermediateCmd newCmd{ArrayElemAssign};
                                newCmd.addOperands(operands1.at(0));
                                // if (!isdigit(rightVar[0]) && operands1.at(1) == leftVar) {
                                if (operands1.at(1) == leftVar) {
                                    newCmd.addOperands(rightVar);
                                } else {
                                    newCmd.addOperands(operands1.at(1));
                                }
                                if (operands1.at(2) == leftVar) {
                                    newCmd.addOperands(rightVar);
                                } else {
                                    newCmd.addOperands(operands1.at(2));
                                }
                                *iter1 = newCmd;
                            }
                                break;
                            default:
                                break;
                        }

                        if (leftBeChanged)
                            break;
                    }


                    // delete Assign
                    bool canErase = true;
                    bool leftBeChanged = false;
                    for (auto iter1 = iter + 1; iter1 != codeBlock->intermediateCodes.end(); iter1++) {
                        OperatorType type = iter1->getOperatorType();
                        auto &operands1 = iter1->getOperands();
                        switch (type) {
                            case CallFuncPush:
                                if (operands1.at(0) == leftVar) {
                                    canErase = false;
                                }
                                break;
                            case Add:
                            case Mul:
                            case Div:
                            case Minus:
                                if (operands1.at(1) == leftVar) {
                                    canErase = false;
                                }
                                if (operands1.at(2) == leftVar) {
                                    canErase = false;
                                }
                                if (operands1.at(0) == leftVar) {
                                    leftBeChanged = true;
                                }
                                break;
                            case Neg:
                                if (operands1.at(1) == leftVar) {
                                    canErase = false;
                                }
                                if (operands1.at(0) == leftVar) {
                                    leftBeChanged = true;
                                }
                                break;
                            case IsEqual:
                            case IsLess:
                            case IsLeq:
                            case IsGeq:
                            case IsGreater:
                            case IsNeq:
                                if (operands1.at(0) == leftVar) {
                                    canErase = false;
                                }
                                if (operands1.at(1) == leftVar) {
                                    canErase = false;
                                }
                                break;
                            case Assign:
                                if (operands1.at(1) == leftVar) {
                                    canErase = false;
                                }
                                if (operands1.at(0) == leftVar) {
                                    leftBeChanged = true;
                                }
                                break;
                            case FuncRetInDef:
                                if (!operands1.empty() && operands1.at(0) == leftVar) {
                                    canErase = false;
                                }
                                break;
                            case GetArrayValue:
                                if (!isdigit(rightVar[0]) &&
                                    operands1.at(2) == leftVar) {   // exclude the case that index is an integer
                                    canErase = false;
                                }
                                if (operands1.at(0) == leftVar) {
                                    leftBeChanged = true;
                                }
                                break;
                            case Printf: {
                                for (const string &s : operands1) {
                                    if (s == leftVar) {
                                        canErase = false;
                                    }
                                }
                            }
                                break;
                            case ArrayElemAssign:
                                if (!isdigit(rightVar[0]) && operands1.at(1) == leftVar) {
                                    canErase = false;
                                }
                                if (operands1.at(2) == leftVar) {
                                    canErase = false;
                                }
                                break;
                            default:
                                break;
                        }
                        if (!canErase || leftBeChanged)
                            break;
                    }

                    // 这里有bug
                    if (canErase) {
                        codeBlock->intermediateCodes.erase(iter);
                        iter--;
                    }
                    noChange = false;


                    // 有bug
                    /*
                    codeBlock->intermediateCodes.erase(iter);
                    iter--;
                     */
                    /*
                    for (IntermediateCmd &cmd : codeBlock->intermediateCodes) {
                        cout << cmd.print() << endl;
                    }
                    cout << endl;
                     */
                } else if (iter->getOperatorType() == AssignRetValue) {
                    auto iter1 = iter + 1;
                    string assignVar = iter->getOperands().at(0);
                    if (iter1 != codeBlock->intermediateCodes.end() && iter1->getOperatorType() == Assign) {
                        string leftVar = iter1->getOperands().at(0);
                        string rightVar = iter1->getOperands().at(1);
                        if (rightVar == assignVar && codeBlock->pActiveVar->out.count(assignVar) == 0) {
                            bool canSubstitute = true;
                            for (auto iter2 = iter + 2; iter2 != codeBlock->intermediateCodes.end(); iter2++) {
                                for (const string &var : iter2->getOperands()) {
                                    if (var == assignVar) {
                                        canSubstitute = false;
                                        break;
                                    }
                                }
                                if (!canSubstitute)
                                    break;
                            }

                            if (canSubstitute) {
                                IntermediateCmd newCmd{AssignRetValue};
                                newCmd.addOperands(leftVar);
                                *iter1 = newCmd;
                                codeBlock->intermediateCodes.erase(iter);
                                iter--;
                            }
                        }
                    }
                } else if (iter->getOperatorType() == Add || iter->getOperatorType() == Minus ||
                           iter->getOperatorType() == Mul || iter->getOperatorType() == Div) {
                    string assignVar = iter->getOperands().at(0);
                    auto iter1 = iter + 1;
                    if (iter1 != codeBlock->intermediateCodes.end() && iter1->getOperatorType() == Assign) {
                        string leftVar = iter1->getOperands().at(0);
                        string rightVar = iter1->getOperands().at(1);
                        if (rightVar == assignVar && codeBlock->pActiveVar->out.count(assignVar) == 0) {
                            bool canSubstitute = true;
                            for (auto iter2 = iter + 2; iter2 != codeBlock->intermediateCodes.end(); iter2++) {
                                for (const string &var : iter2->getOperands()) {
                                    if (var == assignVar) {
                                        canSubstitute = false;
                                        break;
                                    }
                                }
                                if (!canSubstitute)
                                    break;
                            }

                            if (canSubstitute) {
                                IntermediateCmd newCmd{iter->getOperatorType()};
                                newCmd.addOperands(leftVar);
                                newCmd.addOperands(iter->getOperands().at(1));
                                newCmd.addOperands(iter->getOperands().at(2));
                                *iter1 = newCmd;
                                codeBlock->intermediateCodes.erase(iter);
                                // 这里要好好检查一下
                                iter--;
                            }
                        }
                    }
                }
            }

        } while (!noChange);
    }
}
