#ifndef MIPSCODE_OPTIMIZER_H
#define MIPSCODE_OPTIMIZER_H

#include "TypeDefine.h"
#include <utility>
#include <vector>
#include <map>
#include <stack>
#include <set>

using namespace std;

struct StreamBlock;

struct CodeBlock {
    CodeBlock() {
        pActiveVar = nullptr;
    }

    CodeBlock(const CodeBlock &aCodeBlock) {
        intermediateCodes = aCodeBlock.intermediateCodes;
        nextBlocks = aCodeBlock.nextBlocks;
        lastBlocks = aCodeBlock.lastBlocks;
        pActiveVar = nullptr;
    }

    vector<IntermediateCmd> intermediateCodes;
    vector<CodeBlock *> nextBlocks; // if nextBlocks is empty, it means that the codeBlock is an end block
    vector<CodeBlock *> lastBlocks; // if lastBlocks is emtpy, it means that the codeBlock is an beginning block
    StreamBlock *pActiveVar;
};

struct Node {
    explicit Node(bool _isOperate, OperatorType _operatorType, const string &_varName) : isOperate{_isOperate} {
        static int curNO = 0;
        nodeNO = curNO++;
        if (isOperate) {
            operatorType = _operatorType;
        } else {
            varName = _varName;
        }
    }

    void addChild(Node *toAddChild) {
        if (child1 == nullptr) {
            child1 = toAddChild;
        } else {
            child2 = toAddChild;
        }
    }

    bool isOperate;
    int nodeNO;
    OperatorType operatorType;  // isOperate = true
    string varName;             // isOperate = false
    vector<Node *> fatherNodes;
    Node *child1 = nullptr, *child2 = nullptr;
    bool removed = false;   // removed from DAG or not
};

struct DAG {
private:
    CodeBlock *ptrCodeBlock;
    map<string, int> vertexTable;
    map<int, Node *> nodes;
    TempVarGenerator *tempVarGenerator;

    int findNodeNO(const string &varName) {
        // find if varName exists in vertexTable
        const auto iter = vertexTable.find(varName);
        if (iter != vertexTable.end()) {
            return iter->second;
        }

        // create a new node (operatorType is meaningless)
        Node *newNode = new Node{false, Assign, varName};
        nodes[newNode->nodeNO] = newNode;
        vertexTable[varName] = newNode->nodeNO;
        return newNode->nodeNO;
    }

    void addCmd(const string &newVarName, OperatorType operatorType, int childNO1, int childNO2 = -1) {
        // deal with Add, Minus, Mul, Div, GetArrayValue, ArrayElemAssign, Neg
        // find if node exists
        for (const auto &pair : nodes) {
            Node *ptrNode = pair.second;
            if (ptrNode->isOperate && ptrNode->operatorType == operatorType) {
                if (operatorType == Add || operatorType == Mul) {
                    // two children can be swapped
                    if ((ptrNode->child1->nodeNO == childNO1 && ptrNode->child2->nodeNO == childNO2) ||
                        (ptrNode->child1->nodeNO == childNO2 && ptrNode->child2->nodeNO == childNO1)) {
                        vertexTable[newVarName] = ptrNode->nodeNO;
                        return;
                    }
                } else if (operatorType == Neg || operatorType == AssignRetValue) {
                    if (ptrNode->child1->nodeNO == childNO1) {
                        vertexTable[newVarName] = ptrNode->nodeNO;
                        return;
                    }
                } else if (ptrNode->child1->nodeNO == childNO1 && ptrNode->child2->nodeNO == childNO2) {
                    vertexTable[newVarName] = ptrNode->nodeNO;
                    return;
                }
            }
        }

        // node doesn't exist, create a new node
        Node *newNode = new Node{true, operatorType, ""};
        Node *child1 = nodes[childNO1];
        newNode->child1 = child1;
        child1->fatherNodes.push_back(newNode);
        if (childNO2 != -1) {
            Node *child2 = nodes[childNO2];
            newNode->child2 = child2;
            child2->fatherNodes.push_back(newNode);
        }
        nodes[newNode->nodeNO] = newNode;
        vertexTable[newVarName] = newNode->nodeNO;
    }

public:
    explicit DAG(TempVarGenerator *_tempVarGenerator1) : tempVarGenerator{_tempVarGenerator1} {
        ptrCodeBlock = new CodeBlock{};
    }

    static void DFS(Node *curNode, stack<Node *> &nodeStack) {
        nodeStack.push(curNode);
        curNode->removed = true;
        if (curNode->child1 != nullptr && !curNode->child1->removed) {
            bool allFatherRemoved = true;
            for (Node *fatherNode : curNode->child1->fatherNodes) {
                if (!fatherNode->removed) {
                    allFatherRemoved = false;
                    break;
                }
            }
            if (allFatherRemoved) {
                DFS(curNode->child1, nodeStack);
            }
        }

        if (curNode->child2 != nullptr && !curNode->child2->removed) {
            bool allFatherRemoved = true;
            for (Node *fatherNode : curNode->child2->fatherNodes) {
                if (!fatherNode->removed) {
                    allFatherRemoved = false;
                    break;
                }
            }
            if (allFatherRemoved) {
                DFS(curNode->child2, nodeStack);
            }
        }
    }

    void addIntermediateCmd(const IntermediateCmd &intermediateCmd) {
        OperatorType operatorType = intermediateCmd.getOperatorType();
        const vector<string> &operands = intermediateCmd.getOperands();

        if (operatorType == Add || operatorType == Minus || operatorType == Mul || operatorType == Div ||
            operatorType == GetArrayValue || operatorType == ArrayElemAssign) {
            const string &resultName = operands.at(0);
            const string &addName1 = operands.at(1);
            const string &addName2 = operands.at(2);

            int nodeNO1 = findNodeNO(addName1);
            int nodeNO2 = findNodeNO(addName2);

            addCmd(resultName, operatorType, nodeNO1, nodeNO2);
        } else if (operatorType == Neg) {
            const string &resultName = operands.at(0);
            const string &negName = operands.at(1);
            int nodeNO = findNodeNO(negName);
            addCmd(resultName, operatorType, nodeNO);
        } else if (operatorType == AssignRetValue) {
            const string &resultName = operands.at(0);
            const string retName = "RET";
            int nodeNO = findNodeNO(retName);
            addCmd(resultName, operatorType, nodeNO);
        } else if (operatorType == Assign) {
            const string &resultName = operands.at(0);
            const string &addName1 = operands.at(1);
            int nodeNO1 = findNodeNO(addName1);
            vertexTable[resultName] = nodeNO1;
        } else {
            // output DAG
            stack<Node *> nodeStack;
            while (true) {
                bool reachEnd = true;
                for (auto pair : nodes) {
                    Node *curNode = pair.second;
                    if (!curNode->removed) {
                        reachEnd = false;
                        bool allFatherRemoved = true;
                        for (Node *fatherNode : curNode->fatherNodes) {
                            if (!fatherNode->removed) {
                                allFatherRemoved = false;
                                break;
                            }
                        }
                        if (allFatherRemoved) {
                            DFS(curNode, nodeStack);
                        }
                    }
                }
                if (reachEnd) {
                    break;
                }
            }

            while (!nodeStack.empty()) {
                Node *curNode = nodeStack.top();
                nodeStack.pop();
                if (curNode->isOperate) {
                    vector<string> resultVarNames;
                    for (auto &pair : vertexTable) {
                        if (pair.second == curNode->nodeNO) {
                            resultVarNames.push_back(pair.first);
                        }
                    }
                    if (resultVarNames.empty()) {
                        // No variable is the value
                        resultVarNames.push_back(tempVarGenerator->getTempVar());
                    }
                    curNode->varName = resultVarNames.at(0);

                    if (curNode->child2 == nullptr) {
                        const string &varName = curNode->child1->varName;
                        for (const string &resultName : resultVarNames) {
                            IntermediateCmd cmd{operatorType};
                            cmd.addOperands(resultName);
                            cmd.addOperands(varName);
                            ptrCodeBlock->intermediateCodes.push_back(cmd);
                        }
                    } else {
                        const string &varName1 = curNode->child1->varName;
                        const string &varName2 = curNode->child2->varName;
                        for (const string &resultName : resultVarNames) {
                            IntermediateCmd cmd{operatorType};
                            cmd.addOperands(resultName);
                            cmd.addOperands(varName1);
                            cmd.addOperands(varName2);
                            ptrCodeBlock->intermediateCodes.push_back(cmd);
                        }
                    }
                } else {
                    string oldVarName = curNode->varName;
                    if (vertexTable[oldVarName] != curNode->nodeNO) {
                        string newVarName = tempVarGenerator->getTempVar();
                        // newVarName = oldVarName
                        IntermediateCmd assignCmd{Assign};
                        assignCmd.addOperands(newVarName);
                        assignCmd.addOperands(oldVarName);
                        ptrCodeBlock->intermediateCodes.push_back(assignCmd);
                        curNode->varName = newVarName;
                    }
                }
            }

            // clear DAG
            vertexTable.clear();
            nodes.clear();

            // output current cmd
            ptrCodeBlock->intermediateCodes.push_back(intermediateCmd);
        }
    }

    ~DAG() {
        for (auto &pair:nodes) {
            delete pair.second;
        }
        delete ptrCodeBlock;
    }

    CodeBlock getCodeBlock() {
        return *ptrCodeBlock;
    }

    void clearCodeBlock() {
        ptrCodeBlock->intermediateCodes.clear();
        ptrCodeBlock->nextBlocks.clear();
    }
};

struct StreamBlock {
    set<string> in;
    set<string> out;
    CodeBlock *pCodeBlock;
    vector<StreamBlock *> nextBlocks;
    vector<StreamBlock *> lastBlocks;
    set<string> kill;    // def in active variable analysis
    set<string> gen;     // used in active variable analysis
    set<string> used_in_block;
};

class Optimizer {
public:
    explicit Optimizer(vector<IntermediateCmd> _intermediateCodes, TempVarGenerator *_tempVarGenerator)
            : intermediateCodes{std::move(_intermediateCodes)}, tempVarGenerator{_tempVarGenerator} {
        optimize();
    }

    vector<IntermediateCmd> getIntermediateCodes() { return opt_intermediateCodes; }

    ~Optimizer() {
        for (CodeBlock *ptr : codeBlocks) {
            delete ptr;
        }
        for (StreamBlock *ptr : activeVars) {
            delete ptr;
        }
    }

private:
    vector<IntermediateCmd> intermediateCodes;
    vector<IntermediateCmd> opt_intermediateCodes;

    vector<CodeBlock *> codeBlocks;
    TempVarGenerator *tempVarGenerator;

    vector<StreamBlock *> activeVars;

    void optimize();

    void divideCodeBlocks();

    void DAG_optimize();

    void activeVarAnalysis();

    void constantPropagation();

    void optimize_redundant_assign();
};


#endif //MIPSCODE_OPTIMIZER_H
