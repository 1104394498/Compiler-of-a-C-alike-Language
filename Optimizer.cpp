#include "Optimizer.h"
#include <utility>
#include <vector>
#include <map>

void Optimizer::optimize() {
    // not implemented yet
    // should ensure that a[1] = b[1] ==> t1 = b[1]; a[1] = t1;
    divideCodeBlocks();
    // optimize_redundant_assign();

    /* in-block optimization */
    DAG_optimize();

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
                    break;
                }
            }
        } else if (operatorType == BZ || operatorType == BNZ || operatorType == DoWhileBNZ) {
            string targetLabel = lastCmd.getOperands().at(0);
            for (auto aCodeBlock : codeBlocks) {
                IntermediateCmd firstCmd = aCodeBlock->intermediateCodes.front();
                if (firstCmd.getOperatorType() == Label && firstCmd.getOperands().at(0) == targetLabel) {
                    codeBlock->nextBlocks.push_back(aCodeBlock);
                    break;
                }
            }

            codeBlock->nextBlocks.push_back(codeBlocks.at(i + 1));
        } else if (operatorType == CallFunc) {
            string targetFuncName = lastCmd.getOperands().at(0);
            for (auto aCodeBlock : codeBlocks) {
                IntermediateCmd firstCmd = aCodeBlock->intermediateCodes.front();
                if (firstCmd.getOperatorType() == FuncDefStart && firstCmd.getOperands().at(1) == targetFuncName) {
                    codeBlock->nextBlocks.push_back(aCodeBlock);
                    break;
                }
            }
        } else if (operatorType == FuncRetInDef) {
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
                }
            }
        } else {
            codeBlock->nextBlocks.push_back(codeBlocks.at(i + 1));
        }
    }
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
