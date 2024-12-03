#include <stdio.h>
#include <omnetpp.h>
#include "table.h"
#include "Message_m.h"

using namespace omnetpp;

class Host : public cSimpleModule
{
public:
private:
    //参数
    int Nodenum;
    int selfID;
    simtime_t HelloInterval;
    //消息
    Hello *HelloMsg;
    //自消息
    cMessage *timerHelloMsg;
    //表
    std::vector<NeighborT> Neighbor;
    NeighborT initialNeighbor;

    bool CheckMsg(std::string msgname,std::string head);
    void SendHelloMsg();
    void SendMessage(cMessage *msg);
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual Hello *createHelloMsg();
};

Define_Module(Host);

void initialNeighborT(NeighborT table){
    table.ID=-1;
    table.hop=-1;
    table.clusterNum=-1;
    table.posX=-1;
    table.posY=-1;
}

Hello *Host::createHelloMsg(){
    int hop = 0;
    int id = getIndex();
    int seq = 0;
    char msgname[20];
    snprintf(msgname,sizeof(msgname),"Hello from %d",&id);
    Hello *msg = new Hello(msgname);
    msg ->setHop(hop);
    msg -> setID(id);
    msg -> setSeq(seq);
    return msg;
}

bool Host::CheckMsg(std::string msgname,std::string head){
    if(sizeof(msgname) != sizeof(head)){
        return false;
    }else{
        for(int i=0;i<=sizeof(head);i++){
            if(msgname[i] != head[i]){
                return false;
            }
        }
        return true;
    }
}

void Host::SendHelloMsg(){
    HelloMsg = createHelloMsg();
    //sendMessage(HelloMsg);
    scheduleAt(simTime() + HelloInterval,timerHelloMsg);
}

/*
Host::~Host(){

}*/

void Host::initialize()
{
    Nodenum = par("Num").intValue();
    HelloInterval = par("hellointerval").doubleValue();
    selfID = getIndex();
    /*初始化邻居表*/
    initialNeighborT(initialNeighbor);
    Neighbor.resize(Nodenum,initialNeighbor);

    timerHelloMsg = new cMessage("Hello");


}

void Host::handleMessage(cMessage *msg)
{
    if(msg -> isSelfMessage()){
        if(CheckMsg(msg->getName(),"Hello") == true){
            SendHelloMsg();
        }
    }else{

    }
}
