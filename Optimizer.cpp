#include "Optimizer.h"
#include <vector>

void Optimizer::optimize() {
    // not implemented yet
    // should ensure that a[1] = b[1] ==> t1 = b[1]; a[1] = t1;
    optimize_redundant_assign();
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
