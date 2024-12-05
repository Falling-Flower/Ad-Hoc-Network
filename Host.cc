/*
 * Host.cc
 *
 *  Created on: 2024年12月4日
 *      Author: Hua
 */
#include <omnetpp.h>
#include <Message_m.h>
#include "Host.h"
#include "Radio.h"
#include "Table.h"
using namespace omnetpp;
Define_Module(Host);


void Host::initialize(){
    Nodenum = getSystemModule()->par("Num").intValue();
    HelloInterval = getSystemModule()->par("hellointerval").doubleValue();
    selfID = getIndex();
    startNet = false;
    seq = 0;
    waitStart = 2.5*HelloInterval;
    /*初始化邻居表*/
    initialNeighborT(initialNeighbor);
    Neighbor.resize(Nodenum,initialNeighbor);

    timerHelloMsg = new cMessage("Hello");
    timerStartNet = new cMessage("Start");
    scheduleAt(simTime(),timerHelloMsg);
}

void Host::handleMessage(cMessage *msg)
{
    if(msg -> isSelfMessage()){
        if(CheckMsg(msg->getName(),"Hello") == true){
            SendHelloMsg();
        }
        if(CheckMsg(msg->getName(),"Start") == true){
            StartNet();
        }
    }else{
        if(CheckMsg(msg->getName(),"Hello from")==true){
            handleHelloMsg(msg);
        }
    }
}

void Host::initialNeighborT(NeighborT table){
    table.ID=-1;
    table.Hop=-1;
    table.ClusterNum=-1;
//    table.posX=-1;
//    table.posY=-1;
}

Hello *Host::createHelloMsg(){
    int hop = 0;
    int clusternum=0;
    char msgname[20];
    snprintf(msgname,sizeof(msgname),"Hello from %d",selfID);
    Hello *msg = new Hello(msgname);
    msg ->setHop(hop);
    msg -> setID(selfID);
    msg -> setSeq(seq);
    msg -> setClusterNum(clusternum);
    seq++;
    return msg;
}

bool Host::CheckMsg(std::string msgname,std::string head){
//    if(sizeof(msgname) != sizeof(head)){
//        return false;
//    }else{
        for(int i=0;i<=sizeof(head);i++){
            if(msgname[i] != head[i]){
                return false;
            }
        }
        return true;

}

void Host::SendMessage(cMessage *msg){
    std::vector<int> srcRadioList = Radio::getRadioList(selfID);
    cMessage* msgCopy = msg->dup();
    for(int dest : srcRadioList){
        sendDirect(msgCopy,getSystemModule()->getSubmodule("Host", dest)->gate("radioIn"));
    }
}

void Host::SendHelloMsg(){
    HelloMsg = createHelloMsg();
    SendMessage(HelloMsg);
    scheduleAt(simTime() + HelloInterval,timerHelloMsg);
}

void Host::handleHelloMsg(cMessage *msg){
    Hello *RecvHello = check_and_cast<Hello *>(msg);
    int Node = RecvHello -> getID();
    Neighbor[Node].ID = RecvHello -> getID();
    Neighbor[Node].Hop = RecvHello -> getHop();
    Neighbor[Node].ClusterNum = RecvHello -> getClusterNum();
    Neighbor[Node].Seq = RecvHello -> getSeq();
    if(!startNet && !timerStartNet->isScheduled() ){
        scheduleAt(simTime()+waitStart,timerStartNet);
    }
    delete msg;

}

void Host::StartNet(){
    //
}
