/*
 * Radio.h
 *
 *  Created on: 2024年12月5日
 *      Author: Hua
 */

#ifndef RADIO_H_
#define RADIO_H_

#include <omnetpp.h>
#include "Table.h"

using namespace omnetpp;

class Radio : public cSimpleModule
{
public:
    static std::vector<int> getRadioList(const int src);
    double comRange;
private:
    int Nodenum;
    std::vector<NodeList> nodeList;
    static std::vector<RadioT> RadioTable;
    //static std::vector<int> RadioList;
    double RadioDistance(int srcId, int distId);
protected:
    void GetNodeList();
    void GetRadioList();
    virtual void initialize() override;

};

#endif /* RADIO_H_ */
