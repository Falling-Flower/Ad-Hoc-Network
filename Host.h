/*
 * Host.h
 *
 *  Created on: 2024年12月4日
 *      Author: Hua
 */

#ifndef HOST_H_
#define HOST_H_
#include <omnetpp.h>
#include <Message_m.h>
#include "Table.h"
#include "Radio.h"

using namespace omnetpp;

class Host : public cSimpleModule{
public:

private:
       //参数
       int Nodenum;
       int selfID;
       int seq;
       bool startNet;   //组网标志，在第一次收到Hello消息时启动，此后不再启动
       simtime_t HelloInterval;
       simtime_t waitStart;
       //消息
       Hello *HelloMsg;
       //自消息
       cMessage *timerHelloMsg;
       cMessage *timerStartNet;
       //表
       std::vector<NeighborT> Neighbor;
       NeighborT initialNeighbor;
       //函数
       void SendMessage(cMessage *msg);
       bool CheckMsg(std::string msgname,std::string head);
       void initialNeighborT(NeighborT table);
       void SendHelloMsg();
       void handleHelloMsg(cMessage *msg);
       void StartNet();
       Hello *createHelloMsg();
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

};


#endif /* HOST_H_ */
