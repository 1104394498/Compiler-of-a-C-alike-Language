#ifndef MIPSCODE_OPTIMIZER_H
#define MIPSCODE_OPTIMIZER_H

#include "TypeDefine.h"
#include <utility>
#include <vector>

using namespace std;

class Optimizer {
public:
    explicit Optimizer(vector<IntermediateCmd> _intermediateCodes) : intermediateCodes{std::move(_intermediateCodes)} {
        optimize();
    }

    vector<IntermediateCmd> getIntermediateCodes() { return intermediateCodes; }

private:
    vector<IntermediateCmd> intermediateCodes;

    void optimize();
};


#endif //MIPSCODE_OPTIMIZER_H
