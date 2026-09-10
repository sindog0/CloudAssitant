#include "SigConnection.h"
#include "ConnectionManager.h"
#include <algorithm>

SigConnection::SigConnection(TaskScheduler *task_schduler, int sockfd)
    : TcpConnection(task_schduler, sockfd), state_(NONE)
{
    // 设置回调函数,处理读数据
    this->SetReadCallback([this](std::shared_ptr<TcpConnection> conn, BufferReader &buffer)
                          { return this->OnRead(buffer); });

    // 设置关闭回调，释放资源
    this->SetCloseCallback([this](std::shared_ptr<TcpConnection> conn)
                           { this->DisConnect(); });
}

SigConnection::~SigConnection()
{
    this->Clear();
}

void SigConnection::DisConnect()
{
    printf("SigConnection::DisConnect code=%s\r\n", code_.c_str());
    this->Clear();
}

void SigConnection::AddCustom(const std::string &code)
{
    // 添加客户,这个客户可能是拉流器可能是推流器
    for (const auto &it : objects_)
    {
        if (it == code) // 目标客户端已经添加
        {
            return;
        }
    }
    objects_.push_back(code);
}

void SigConnection::RemoveCustom(const std::string &code)
{
    if (objects_.empty())
    {
        return;
    }
    objects_.erase(std::remove(objects_.begin(), objects_.end(), code), objects_.end());
    if (objects_.empty())
    {
        state_ = IDLE; // 说明当前没有控制端，自己也不是控制端
    }
}

bool SigConnection::OnRead(BufferReader &buffer)
{
    while (buffer.ReadableBytes() > 0)
    {
        HandleMessage(buffer);
    }
    return true;
}

void SigConnection::HandleMessage(BufferReader &buffer)
{
    if (buffer.ReadableBytes() < sizeof(packet_head))
    {
        return;
    }

    packet_head *data = (packet_head *)buffer.Peek();
    if (buffer.ReadableBytes() < data->len)
    {
        return;
    }
    switch (data->cmd)
    {
    case JOIN:
        HandleJoin(data);
        break;
    case OBTAINSTREAM:
        HandleObtainStream(data);
        break;
    case CREATESTREAM:
        HandleCreateStream(data);
        break;
    case DELETESTREAM:
        HandleDeleteStream(data);
        break;
    case MOUSE:
    case MOUSEMOVE:
    case KEY:
    case WHEEL:
        HandleOtherMessage(data);
        break;
    default:
        break;
    }
    //更新缓冲区
    buffer.Retrieve(data->len);
}

void SigConnection::Clear()
{
    state_ = CLOSE;
    conn_ = nullptr;
    DeleteStream_body body; //需要通知所有关心者这个删除流通知
    for(auto it : objects_)
    {
        TcpConnection::Ptr conn = ConnectionManager::GetInstance()->QueryConn(it);
        if(conn){
            auto ctrCon = std::dynamic_pointer_cast<SigConnection>(conn);//转换为SigConnection
            if(ctrCon){
                ctrCon->RemoveCustom(code_);
            }
            body.SetStreamCount(ctrCon->objects_.size());//推流的时候，如果发现拉流数量为0,我们就需要停止推流，如果流数量不为0，就说明还有客户端连接，不能停止推流
            conn->Send((const char*)&body, body.len);//通知关心者删除流
        }
    }
}


void SigConnection::HandleJoin(const packet_head *data)
{
    //准备一个应答
    JoinReply_body reply_body;
    Join_body* body = (Join_body*)data;
    if(this->IsNoJoin())
    {
        std::string code = body->GetId();
        TcpConnection:Ptr ptr = ConnectionManager::GetInstance()->QueryConn(code);//是不是已经存在
        if(ptr)
        {
            reply_body.SetCode(ERROR);
            this->Send((const char*)&reply_body,reply_body.len);
            return;
        }
        code_ = code;
        state_ = IDLE;
        ConnectionManager::GetInstance()->AddConn(code,shared_from_this());
        printf("Jion count: %d\n",ConnectionManager::GetInstance()->Size());
        reply_body.SetCode(SUCCESSFUL);
        this->Send((const char*)&reply_body,reply_body.len);
        return;
    }
    //已经创建
    reply_body.SetCode(ERROR);
    this->Send((const char*)&reply_body,reply_body.len);
}

void SigConnection::HandleObtainStream(const packet_head *data)
{
    //获取流
    ObtainStream_body* body = (ObtainStream_body*)data;
    return this->DoObtainStream(body);
}

void SigConnection::HandleCreateStream(const packet_head *data)
{
    CreateStream_body* body = (CreateStream_body*)data;
    return this->DoCreateStream(body);
}

void SigConnection::HandleDeleteStream(const packet_head *data)
{
    if(this->IsBusy())
    {
        Clear();
    }   
}

void SigConnection::HandleOtherMessage(const packet_head *data)
{
    //鼠标，键盘消息 ,转发
    if(conn_ && state_ == PULLER) //说明当前是在拉流，
    {
    //  if(data->cmd == MOUSEMOVE)
    //  {
    //         MouseMove_body* mouse = (MouseMove_body*)data;
    //         printf("mouse xl: %d xr: %d yl: %d yr: %d\n",mouse->xl_ratio,mouse->xr_ratio,mouse->yl_ratio,mouse->yr_ratio);
    //  }
        conn_->Send((const char*)data,data->len);
    }
}

void SigConnection::DoObtainStream(const packet_head *data)
{
    ObtainStreamReply_body reply;
    CreateStream_body create_reply;
    std::string code = ((ObtainStream_body*)data)->GetId();
    TcpConnection::Ptr conn = ConnectionManager::GetInstance()->QueryConn(code);
    if(!conn)
    {
        printf("远程目标不存在\n");
        reply.SetCode(ERROR);
        this->Send((const char*)&reply,reply.len);
        return;
    }
    if(conn == shared_from_this()) //不能控制自己
    {
        printf("不能控制自己\n");
        reply.SetCode(ERROR);
        this->Send((const char*)&reply,reply.len);
        return; 
    }
    if(this->IsIdle())//本身是空闲就去获取流
    {
        auto con = std::dynamic_pointer_cast<SigConnection>(conn);
        switch (con->GetRoleState())
        {
        case IDLE://目标是空闲，我们就去通知他去推流
            printf("目标空闲\n");
            this->state_ = PULLER;
            this->AddCustom(code); //添加被控端
            con->AddCustom(code_);//被控端需要添加控制端
            reply.SetCode(SUCCESSFUL);
            conn_ = conn; //我们就可以通过这个目标(被控端)连接器来转发消息
            //通知被控端来创建流
            con->Send((const char*)&create_reply,create_reply.len);
            this->Send((const char*)&reply,reply.len);
            break;
        case NONE:
            printf("目标没上线\n");
            reply.SetCode(ERROR);
            this->Send((const char*)&reply,reply.len);
            break;
        case CLOSE:
            printf("目标离线\n");
            reply.SetCode(ERROR);
            this->Send((const char*)&reply,reply.len);
            break;
        case PULLER: //目标拉流(控制端) 控制端不能控制控制端
            printf("目标忙碌\n");
            reply.SetCode(ERROR);
            this->Send((const char*)&reply,reply.len);
            break;
        case PUSHER: //推流说明他是被控端，所以我们可以去拉流
            if(con->GetStreamAddress().empty()) //异常
            {
                printf("目标正在推流，但是流地址异常\n");
                reply.SetCode(ERROR);
                this->Send((const char*)&reply,reply.len);
            }
            else//在推流，而且流地址正常
            {
                printf("目标正在推流\n");
                this->state_ = PULLER;
                this->AddCustom(code);
                con->AddCustom(code_);
                //在推流 流已经存在，就不需要重新创建流，我们只需要播放流；
                PlayStream_body play_body;
                play_body.SetCode(SUCCESSFUL);
                play_body.SetstreamAddres(con->GetStreamAddress());
                this->Send((const char*)&play_body,play_body.len);
            }
            break;
        default:
            break;
        }
    }
    else //本身是忙碌
    {
        //返回错误
        reply.SetCode(ERROR);
        this->Send((const char*)&reply,reply.len);
    }
}

void SigConnection::DoCreateStream(const packet_head *data)
{
    PlayStream_body body;
    CreateStreamReply_body* reply = (CreateStreamReply_body*)data;
    printf("body size: %d,streamaddr: %s\n",reply->len,reply->GetstreamAddres().c_str());
    stream_address_ = reply->GetstreamAddres();
    //判断所有连接的状态 ,如果连接器状态十空闲，我们就去回应
    for(auto idefy : objects_)
    {
        TcpConnection::Ptr conn = ConnectionManager::GetInstance()->QueryConn(idefy);
        if(!conn)
        {
            this->RemoveCustom(idefy);
            continue;
        }
        auto con = std::dynamic_pointer_cast<SigConnection>(conn);
        if(stream_address_.empty())//流地址异常
        {
            printf("流地址异常\n");
            con->state_ = IDLE;
            body.SetCode(ERROR);
            con->Send((const char*)&body,body.len);
            continue;
        }
        switch (con->GetRoleState())
        {
        case NONE:
        case IDLE:
        case CLOSE:
        case PUSHER:
            body.SetCode(ERROR);
            this->RemoveCustom(con->GetCode());
            con->Send((const char*)&body,body.len);
            break;
        case PULLER:
        this->state_ = PUSHER;
        body.SetCode(SUCCESSFUL);
        body.SetstreamAddres(stream_address_);
        printf("streamadder: %s\n",stream_address_.c_str());
        conn->Send((const char*)&body,body.len);
        break;
        default:
            break;
        }
    }
}
