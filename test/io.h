
#ifndef IO_H
#define IO_H

#include <vector>
#include <string>

#include "network.h"

//
// 会话, 非线程安全的
//

class IIOService;

class IIOSession
{
public :
    IIOSession()
        : m_Sid( 0 ),
          m_Layer( NULL ),
          m_IOContext( NULL )
    {}

    virtual ~IIOSession()
    {}

public :
    //
    // 网络事件
    // 多个网络线程中被触发
    //

    virtual int32_t onStart() { return 0; }
    virtual int32_t onProcess( const char * buffer, uint32_t nbytes ) { return 0; }
    virtual char *  onTransform( const char * buffer, uint32_t & nbytes ) { return const_cast<char *>(buffer); }
    virtual int32_t onTimeout() { return 0; }
    virtual int32_t onKeepalive() { return 0; }
    virtual int32_t onError( int32_t result ) { return 0; }
    virtual void    onShutdown( int32_t way ) {}

public :
    //
    // 在网络线程中对会话的操作
    //

    // 获取会话ID
    sid_t id() const { return m_Sid; }

    // 获取线程上下文参数
    void * iocontext() const { return m_IOContext; }

    // 设置超时/保活时间
    void setTimeout( int32_t seconds );
    void setKeepalive( int32_t seconds );

    // 发送数据
    int32_t send( const std::string & buffer );
    int32_t send( const char * buffer, uint32_t nbytes, bool isfree = false );

    // 关闭会话
    int32_t shutdown();

private :
    friend class IIOService;

    // 初始化会话
    void init( sid_t id, void * context, iolayer_t layer );

    // 内部回调函数
    static int32_t  onStartSession( void * context );
    static int32_t  onProcessSession( void * context, const char * buffer, uint32_t nbytes );
    static char *   onTransformSession( void * context, const char * buffer, uint32_t * nbytes );
    static int32_t  onTimeoutSession( void * context );
    static int32_t  onKeepaliveSession( void * context );
    static int32_t  onErrorSession( void * context, int32_t result );
    static void     onShutdownSession( void * context, int32_t way );

private :
    sid_t       m_Sid;
    iolayer_t   m_Layer;
    void *      m_IOContext;
};

//
// 网络通信层
//

class IIOService
{
public :
    IIOService( uint8_t nthreads, uint32_t nclients, bool realtime = false );
    virtual ~IIOService();

public :
    // 初始化/销毁IO上下文
    virtual void * initIOContext() { return NULL; }
    virtual void finalIOContext( void * context ) { return; }

    // 数据改造
    virtual char * onTransform( const char * buffer, uint32_t & nbytes ) { return const_cast<char *>(buffer); }

    // 接受/连接事件
    // 需要调用者自己实现
    // 有可能在IIOService的多个网络线程中被触发
    virtual IIOSession * onAccept( sid_t id, const char * host, uint16_t port ) { return NULL; }
    virtual IIOSession * onConnect( sid_t id, const char * host, uint16_t port ) { return NULL; }
    virtual bool onConnectError( int32_t result, const char * host, uint16_t port ) { return false; }

public :
    //
    // 线程安全的API
    //

    // 连接/监听
    bool listen( const char * host, uint16_t port );
    bool connect( const char * host, uint16_t port, int32_t seconds );

    // 停止服务
    void stop();

    // 发送数据
    int32_t send( sid_t id, const std::string & buffer );
    int32_t send( sid_t id, const char * buffer, uint32_t nbytes, bool isfree = false );

    // 广播数据
    int32_t broadcast( const std::string & buffer );
    int32_t broadcast( const std::vector<sid_t> & ids, const std::string & buffer );

    // 终止会话
    int32_t shutdown( sid_t id );
    int32_t shutdown( const std::vector<sid_t> & ids );

private :
    void attach( sid_t id, IIOSession * session, void * iocontext );

    static char * onTransformService( void * context, const char * buffer, uint32_t * nbytes );

    static int32_t onAcceptSession( void * context, void * iocontext, sid_t id, const char * host, uint16_t port );
    static int32_t onConnectSession( void * context, void * iocontext, int32_t result, const char * host, uint16_t port, sid_t id );

private :
    iolayer_t   m_IOLayer;
    uint8_t     m_ThreadsCount;
    uint32_t    m_SessionsCount;
    void **     m_IOContextGroup;
};

#endif
