/*
 * Radio.cc
 *
 *  Created on: 2024年12月4日
 *      Author: Hua
 */
#include <omnetpp.h>
#include <vector>
#include "Radio.h"
#include "Host.h"
#include "Table.h"
using namespace omnetpp;
Define_Module(Radio);


std::vector<RadioT> Radio::RadioTable;

std::vector<int> Radio::getRadioList(const int src){
    std::vector<int> RadioList;
    for(int dest : RadioTable[src].destID){
        RadioList.emplace_back(dest);
    }
    return RadioList;
}

double Radio::RadioDistance(int srcId, int destId){
    double x = pow(nodeList[destId].posX - nodeList[srcId].posX, 2);
    double y = pow(nodeList[destId].posY - nodeList[srcId].posY, 2);
    return std::sqrt(x + y);
}

void Radio::GetNodeList(){
    if(nodeList.size()==0 || Nodenum > sizeof(nodeList)/sizeof(nodeList[0])){
        EV << "nodeList is not initialized or too small";
    }

    for (int i = 0; i < Nodenum; i++) {
            cModule *host = getSystemModule()->getSubmodule("Host",i);
            if (host != nullptr) {
                double x = host->par("posX").doubleValue();
                double y = host->par("posY").doubleValue();
                int id = host->getIndex();
                nodeList[i].ID=id;
                nodeList[i].posX=x;
                nodeList[i].posY=y;
            }
        }
}

void Radio::GetRadioList(){
    for(int src=0; src < Nodenum; src++){
        for(int i=0;i < Nodenum; i++){
            comRange = getSystemModule()->getSubmodule("Host",src)->par("comRange").doubleValue();
            RadioTable[src].srcID=src;
            if(src != i && RadioDistance(src,i)<=comRange){
                RadioTable[src].destID.push_back(i);
            }
        }
    }
}

void Radio::initialize(){
    Nodenum = getSystemModule()->par("Num").intValue();
    RadioTable.resize(Nodenum);
    nodeList.resize(Nodenum,{-1,-1.0,-1.0});
    GetNodeList();
    GetRadioList();
}
