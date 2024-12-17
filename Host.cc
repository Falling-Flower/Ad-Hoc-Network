/*
 * Host.cc
 *
 *  Created on: 2024年12月4日
 *      Author: Hua
 */
#include <omnetpp.h>
#include <algorithm>
#include <Message_m.h>
#include "Host.h"
#include "Radio.h"
#include "Table.h"
using namespace omnetpp;
Define_Module(Host);

std::vector<int> clusternum;

const char* color[] = {
    "red",
    "yellow",
    "blue",
    "green",
    "black"
};

Host::~Host(){
    cancelAndDelete(timerHelloMsg);
    cancelAndDelete(timerBuildNet);
}

void Host::ChangeColor(){
    cModule *host = getSystemModule()->getSubmodule("Host", selfID);
    cDisplayString& dispStr = host->getDisplayString();
    dispStr.setTagArg("i", 2, color[clusterNum]);
}

void Host::initialize(){
    Nodenum = getSystemModule()->par("Num").intValue();
    // HelloInterval = getSystemModule()->par("hellointerval").doubleValue();
    HelloInterval = 0.2;    //test
    selfID = getIndex();
    clustercnt = false;
    slot = static_cast<double>((selfID*HelloInterval)/Nodenum);
    delay = static_cast<double>((HelloInterval*Nodenum));
    // delay = 0.5;
    setState(Idle); //设置初始状态
    /*初始化本节点属性*/
    clusterNum = -1;
    hop = -1;
    seq = 0;
    hellocount = 0;
    waitStart = 2.5*delay;
    /*初始化邻居表*/
    Neighbor.reserve(Nodenum);
    initialNeighborT(initialNeighbor);
    Neighbor.resize(Nodenum,initialNeighbor);
    //WATCH_VECTOR(Neighbor);
    WATCH(timerHelloMsg);
    WATCH(timerWander);
    WATCH(timerBuildNet);
    WATCH(currentState);
    WATCH(clusterNum);

    timerHelloMsg = new cMessage("Hello");
    timerWander = new cMessage("Wander");
    timerSend = new cMessage("Send");
    timerBuildNet = new cMessage("Build");

    scheduleAt(simTime() + delay + slot ,timerSend);
    scheduleAt(simTime() + delay ,timerHelloMsg);
    scheduleAt(simTime() + 5.0*delay,timerWander);
    
}

void Host::handleMessage(cMessage *msg)
{
    if(msg == nullptr) return;
    if(msg -> isSelfMessage()){
        if(CheckMsg(msg->getName(),"Hello") == true){
            SendHelloMsg();
        }
        if(CheckMsg(msg->getName(),"Build") == true){
            StartNet();
        }
        if(CheckMsg(msg->getName(),"Wander") == true){
            setState(Wander);
            cancelAndDelete(timerWander);
            // cancelAndDelete(timerHelloMsg); 
        }
        if(CheckMsg(msg->getName(),"Send") == true){
            SendMsgQueue();
            scheduleAt(simTime() + delay ,timerSend);
        }
    }else if (msg && dynamic_cast<cMessage *>(msg))
    {
        if(CheckMsg(msg->getName(),"HelloMsg")==true){
            handleHelloMsg(msg);
            delete msg;
        }
        if(CheckMsg(msg->getName(),"RequestHeadMsg")==true){
            handleRequestHeadMsg(msg);
        }
        if(CheckMsg(msg->getName(),"AckHeadMsg")==true){
            handleAckHeadMsg(msg);
        }
        if(CheckMsg(msg->getName(),"ClusterMsg")==true){
            handleClusterMsg(msg);
        }
    }
}

void Host::initialNeighborT(NeighborT& table){
    table.ID = -1;
    table.Hop = -1;
    table.ClusterNum = -1;
    table.Seq = -1;
}

int Host::createClusterNum(){
    int num;
    while(true){
        num = intuniform(0,10); //随机生成簇号
        auto it = std::find(clusternum.begin(),clusternum.end(),num);
        if(it == clusternum.end()){
            clusternum.push_back(num);
            break;
        }
    }
    return num;
}

Hello *Host::createHelloMsg(){
    char msgname[20];
    snprintf(msgname,sizeof(msgname),"HelloMsg");
    Hello *msg = new Hello(msgname);
    msg ->setHop(hop);
    msg -> setID(selfID);
    msg -> setSeq(seq++);
    msg -> setClusterNum(clusterNum);
    return msg;
}

RequestHead *Host::createRequestHeadMsg(){
    char msgname[20];
    snprintf(msgname,sizeof(msgname),"RequestHeadMsg");
    RequestHead *msg = new RequestHead(msgname);
    msg -> setID(selfID);
    msg -> setSeq(seq++);
    msg -> setHop(hop);
    msg -> setClusterNum(clusterNum);
    return msg;
}

AckHead *Host::createAckHeadMsg(int dest, bool approve){
    char msgname[20];
    snprintf(msgname,sizeof(msgname),"AckHeadMsg");
    AckHead *msg = new AckHead(msgname);
    msg -> setID(selfID);
    msg -> setSeq(seq++);
    msg -> setHop(hop);
    msg -> setClusterNum(clusterNum);
    msg -> setDest(dest);
    msg -> setApprove(approve);
    return msg;
}

Cluster *Host::createClusterMsg(){
    char msgname[20];
    snprintf(msgname,sizeof(msgname),"ClusterMsg");
    Cluster *msg = new Cluster(msgname);
    int newhop=hop;
    msg -> setID(selfID);
    msg -> setSeq(seq++);
    msg -> setHop(hop);
    msg -> setClusterNum(clusterNum);
    msg -> setNewClusterNum(clusterNum);
    msg -> setNewHop(++newhop);
    return msg;
}

void Host::SendMessage(cMessage *msg){
    std::vector<int> srcRadioList = Radio::getRadioList(selfID);
    for(int dest : srcRadioList){
        sendDirect(msg->dup(),getSystemModule()->getSubmodule("Host", dest)->gate("radioIn"));
        bubble("Send Success");
        EV<<"Send to"<<"Host["<<dest<<"]"<<endl;
    }
}

void Host::SendMsgQueue(){
    while (!MsgQueue.empty()) {
        cMessage *msg = MsgQueue.front();
        SendMessage(msg); 
        MsgQueue.pop();
        delete msg;
    }
}

void Host::SendHelloMsg(){
    HelloMsg = createHelloMsg();
    AddMsgQueue(HelloMsg);
    scheduleAt(simTime() + delay,timerHelloMsg);
}

void Host::SendRequestHeadMsg(){
    RequestHeadMsg = createRequestHeadMsg();
    AddMsgQueue(RequestHeadMsg);
}

void Host::SendAckHeadMsg(int dest, bool approve){
    AckHeadMsg = createAckHeadMsg(dest, approve);
    AddMsgQueue(AckHeadMsg);
}

void Host::SendClusterMsg(){
    ClusterMsg = createClusterMsg();
    AddMsgQueue(ClusterMsg);
}

void Host::handleHelloMsg(cMessage *msg){
    Hello *Recv = check_and_cast<Hello *>(msg);
    if(Recv == nullptr){
        EV << "Error: Recieved invalid Hello message" << endl;
        return;
    }
    int Node = Recv -> getID();
    if (Node < 0 || Node >= Neighbor.size()) {  // 检查ID是否越界
    EV << "Error: Node ID " << Node << " is out of range!" << endl;
    return;
    }  
    Neighbor[Node].ID = Recv -> getID();
    Neighbor[Node].Hop = Recv -> getHop();
    Neighbor[Node].ClusterNum = Recv -> getClusterNum();
    Neighbor[Node].Seq = Recv -> getSeq();
    if(currentState==Idle){
        if(!timerBuildNet->isScheduled()){
            scheduleAt(simTime(),timerBuildNet);
        }
        if(timerWander && timerWander->isScheduled()){
            cancelEvent(timerWander); 
        }
        setState(BuildNet);
    }

    if(currentState==Wander){
        if(!timerBuildNet->isScheduled()){
            scheduleAt(simTime(),timerBuildNet);
        }
    } 
}

void Host::handleRequestHeadMsg(cMessage *msg){
    if(currentState==BuildNet){
        // if(timerBuildNet -> isScheduled()){
        //     cancelEvent(timerBuildNet);
        // }
        RequestHead *Recv = static_cast<RequestHead *>(msg);
        int Node = Recv -> getID();
        if( Neighbor[Node].Seq < Recv->getSeq()){
            Neighbor[Node].ID = Recv -> getID();
            Neighbor[Node].Hop = Recv -> getHop();
            Neighbor[Node].ClusterNum = Recv -> getClusterNum();
            Neighbor[Node].Seq = Recv -> getSeq();
            bool flag = true;
            for(auto const neighbor:Neighbor ){
                if(neighbor.ID > Node){
                    flag = false; 
                }
            }
            SendAckHeadMsg(Node,flag);
            setState(Wait);
        }
    }
}

void Host::handleAckHeadMsg(cMessage *msg){
    AckHead *Recv = static_cast<AckHead *>(msg);
    int Node = Recv -> getID();
    Neighbor[Node].ID = Recv -> getID();
    Neighbor[Node].Hop = Recv -> getHop();
    Neighbor[Node].ClusterNum = Recv -> getClusterNum();
    Neighbor[Node].Seq = Recv -> getSeq();
    if(currentState==Wait){
        if(Recv->getDest()==selfID && Recv->getApprove()==true){
            hop = 0;
            clusterNum = createClusterNum();
            SendClusterMsg();
            if(hasGUI()){
                ChangeColor();
            }
            setState(InCluster);
        }
    }
    if(currentState==Wander){
        hellocount=0;
        setState(BuildNet);
    }
    // delete msg;
}

void Host::handleClusterMsg(cMessage *msg){
    Cluster *Recv = static_cast<Cluster *>(msg);
    int Node = Recv -> getID();
    Neighbor[Node].ID = Recv -> getID();
    Neighbor[Node].Hop = Recv -> getHop();
    Neighbor[Node].ClusterNum = Recv -> getClusterNum();
    Neighbor[Node].Seq = Recv -> getSeq();

    if(currentState==Idle || currentState==Wait){
        clusterNum = Recv -> getNewClusterNum();
        hop = Recv -> getNewHop();
        if(hop < 2){
            SendClusterMsg();
        }else{
            //SendQuikCLuster();
        }
        if(hasGUI()){
            ChangeColor();
        }
        setState(InCluster);
    }
    // if(!clustercnt){
    //     clustercnt = true;
    // }
}

void Host::StartNet(){
    bool flag = true;
    for(auto const neighbor:Neighbor ){
        if(neighbor.ClusterNum == -1){
            flag = false; 
        }
    }
    if(flag){
        bool flaghop = true;
        for(auto const neighbor:Neighbor ){
            if(neighbor.Hop < 2){
                flaghop = false;
            }
        }
        if(flaghop){
            clusterNum = createClusterNum();
            hop = 0;
            SendClusterMsg();
            if(hasGUI()){
                ChangeColor();
            }
            setState(InCluster);
        }else{
            setState(Wait);
        }
    }else{
        bool flagid = true;
        for(auto const neighbor:Neighbor ){
            if(neighbor.ID > selfID){
                flagid = false;
            }
        }
        if(flagid){
            SendRequestHeadMsg();
            // scheduleAt(simTime(),timerRequestHead);
            setState(Wait);
        }else{
            setState(Wait);
        }
    }
}

bool Host::CheckMsg(std::string msgname,std::string head){
    if (msgname.size() < head.size()) {
        return false;
    } else {
        for (int i = 0; i < head.size(); i++) {
            if (msgname[i] != head[i]) {
                return false;
            }
        }
        return true;
    }
}

bool Host::compareNeighborT(){
    bool ifLarger = true;
    for(int i = 0; i < Nodenum; ++i){
        if(Neighbor[i].ID >= selfID){
            ifLarger = false;
            break;
        }
    }
    return ifLarger;
}

void Host::printNeighborT(){//打印邻居表
    EV<<selfID<<" 's neighbor table: ["<<endl;
    for(int i = 0; i < Nodenum; ++i){
        EV<<Neighbor[i].ID<<" ; ";
    }
    EV<<"]"<<endl;
}

void Host::AddMsgQueue(cMessage *msg){
    MsgQueue.push(msg);
}



