/*
 * Host.h
 *
 *  Created on: 2024年12月4日
 *      Author: Hua
 */

#ifndef HOST_H_
#define HOST_H_
#include <omnetpp.h>
#include <queue>
#include <Message_m.h>
#include "Table.h"
#include "Radio.h"

using namespace omnetpp;

class Host : public cSimpleModule{
public:
    ~Host();
    void setState(State newState){
        currentState = newState;
    }
    State getState() const{
        return currentState;
    }
private:
    //参数
    int Nodenum;
    int selfID;
    int clusterNum; //自身簇号
    int seq;
    int hop;
    bool clustercnt;

    State currentState;//节点状态：Idle：0, BuildNet, Wait, InCluster, Wander
    int hellocount; //Hello计数器用于游离态是控制Hello消息发送频率
    double delay;//发送时延
    double slot;
    double HelloInterval;
    double waitStart;
    //消息
    Hello *HelloMsg;
    RequestHead *RequestHeadMsg;
    AckHead *AckHeadMsg;
    Cluster *ClusterMsg;
    
    //自消息
    cMessage *timerHelloMsg;
    cMessage *timerBuildNet;
    cMessage *timerRequestHead;
    cMessage *timerWander;
    cMessage *timerWait;
    cMessage *timerSend;
    // cMessage *timererror;

    //表
    std::vector<NeighborT> Neighbor;
    std::queue<cMessage*> MsgQueue;
    NeighborT initialNeighbor;
    //函数
    void SendMessage(cMessage *msg);
    void SendMessagetest(cMessage *msg);
    bool CheckMsg(std::string msgname,std::string head);
    void initialNeighborT(NeighborT& table);
    bool compareNeighborT();
    void printNeighborT();
    void ChangeColor();

    void SendMsgQueue();
    void SendHelloMsg();
    void SendRequestHeadMsg();
    void SendAckHeadMsg(int dest, bool approve);
    void SendClusterMsg();
    void handleHelloMsg(cMessage *msg);
    void handleRequestHeadMsg(cMessage *msg);
    void handleAckHeadMsg(cMessage *msg);
    void handleClusterMsg(cMessage *msg);

    void AddMsgQueue(cMessage *msg);
    void StartNet();
    int createClusterNum();
    void wantClusterHead();
    
    RequestHead *createRequestHeadMsg();
    Hello *createHelloMsg();
    AckHead *createAckHeadMsg(int dest, bool approve);
    Cluster *createClusterMsg();
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

};


#endif /* HOST_H_ */
