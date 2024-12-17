/*
 * Table.h
 *
 *  Created on: 2024年12月5日
 *      Author: Hua
 */

#ifndef TABLE_H_
#define TABLE_H_

#include <omnetpp.h>
#include <string.h>
using namespace omnetpp;

typedef struct{
    int ID;
    int Hop;
    int ClusterNum;
    int Seq;
}NeighborT;

typedef struct{
    int srcID;
    std::vector<int> destID;
}RadioT;//发射表：源节点可以通信的目的节点

typedef struct{
    int ID;
    double posX;
    double posY;
}NodeList;//节点表：存储网络中所有节点的位置信息

enum State{
    Idle = 0,
    BuildNet,
    Wait,
    InCluster,
    Wander
};

#endif /* TABLE_H_ */
