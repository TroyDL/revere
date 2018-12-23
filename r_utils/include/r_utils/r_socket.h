
#ifndef r_utils_r_socket_h
#define r_utils_r_socket_h

#include "r_utils/interfaces/r_stream_io.h"
#include "r_utils/interfaces/r_socket_io.h"
#include "r_utils/interfaces/r_pollable.h"
#include "r_utils/r_socket_address.h"
#include "r_utils/r_exception.h"
#include <memory>
#include <map>
#include <mutex>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef int SOCKET;
#define SOCKET_SHUT_FLAGS SHUT_RDWR
#define SOCKET_SHUT_SEND_FLAGS SHUT_WR
#define SOCKET_SHUT_RECV_FLAGS SHUT_RD

class test_r_utils_r_socket;

namespace r_utils
{

class r_raw_socket : public r_socket_io, public r_pollable
{
    friend class ::test_r_utils_r_socket;

public:
    enum r_raw_socket_defaults
    {
        MAX_BACKLOG = 5
    };

    r_raw_socket();
    r_raw_socket( r_raw_socket&& obj ) noexcept;
    r_raw_socket( const r_raw_socket& ) = delete;
    virtual ~r_raw_socket() noexcept;

    r_raw_socket& operator = ( r_raw_socket&& obj ) noexcept;
    r_raw_socket& operator = ( const r_raw_socket& ) = delete;

    void create( int af );

    void connect( const std::string& host, int port );
    void listen( int backlog = MAX_BACKLOG );
    void bind( int port, const std::string& ip = "" );
    r_raw_socket accept();

    inline SOCKET get_sok_id() const { return _sok; }

    inline bool valid() const
	{
		return (_sok > 0) ? true : false;
	}

    virtual int raw_send( const void* buf, size_t len );
    virtual int raw_recv( void* buf, size_t len );

    void close();

    virtual bool wait_till_recv_wont_block( uint64_t& millis ) const;
    virtual bool wait_till_send_wont_block( uint64_t& millis ) const;

    std::string get_peer_ip() const;
    std::string get_local_ip() const;

protected:
    SOCKET _sok;
    r_socket_address _addr;
    std::string _host;

    static bool _sokSysStarted;
    static std::recursive_mutex _sokLock;
};

class r_socket : public r_stream_io, public r_pollable
{
    friend class ::test_r_utils_r_socket;

public:
    enum r_socket_defaults
    {
        MAX_BACKLOG = 5
    };

    inline r_socket() : _sok(), _ioTimeOut(10000) {}
    inline r_socket( r_socket&& obj ) noexcept :
        _sok( std::move(obj._sok) ),
        _ioTimeOut( std::move(obj._ioTimeOut ))
    {
    }

    r_socket( const r_socket& ) = delete;
    inline virtual ~r_socket() noexcept {}

    inline r_socket& operator = ( r_socket&& obj ) noexcept
    {
        if(valid())
            close();

        _sok = std::move(obj._sok);
        _ioTimeOut = std::move(obj._ioTimeOut);
        return *this;
    }

    r_socket& operator = ( const r_socket& ) = delete;

    void set_io_timeout( uint64_t ioTimeOut ) { _ioTimeOut = ioTimeOut; }

    inline void create( int af ) { _sok.create(af); }

    inline void connect( const std::string& host, int port ) { _sok.connect(host, port); }
    inline void listen( int backlog = MAX_BACKLOG ) { _sok.listen(backlog); }
    inline void bind( int port, const std::string& ip = "" ) { _sok.bind(port, ip ); }
    inline r_socket accept() { auto r = _sok.accept(); r_socket s; s._sok = std::move(r); return std::move(s); }

    inline SOCKET get_sok_id() const { return _sok.get_sok_id(); }

    virtual int raw_send( const void* buf, size_t len );

    virtual int raw_recv( void* buf, size_t len );

    inline virtual bool valid() const { return _sok.valid(); }

    virtual void send( const void* buf, size_t len );

    virtual void recv( void* buf, size_t len );

    inline void close() { _sok.close(); }

    inline virtual bool wait_till_recv_wont_block( uint64_t& millis ) const { return _sok.wait_till_recv_wont_block(millis); }
    inline virtual bool wait_till_send_wont_block( uint64_t& millis ) const { return _sok.wait_till_send_wont_block(millis); }

    inline std::string get_peer_ip() const { return _sok.get_peer_ip(); }
    inline std::string get_local_ip() const { return _sok.get_local_ip(); }

private:
    r_raw_socket _sok;
    uint64_t _ioTimeOut;
};

template<class SOK>
class r_buffered_socket : public r_stream_io, public r_pollable
{
public:
    friend class ::test_r_utils_r_socket;

public:
    enum r_buffered_socket_defaults
    {
        MAX_BACKLOG = 5
    };

    inline r_buffered_socket( size_t bufferSize = 4096 ) :
        _sok(),
        _buffer(),
        _bufferOff(0)
    {
        _buffer.reserve(bufferSize);
    }

    inline r_buffered_socket( r_buffered_socket&& obj ) noexcept :
        _sok( std::move(obj._sok) ),
        _buffer( std::move(obj._buffer) ),
        _bufferOff( std::move( obj._bufferOff ) )
    {
        obj._bufferOff = 0;
    }

    r_buffered_socket( const r_buffered_socket& ) = delete;

    inline virtual ~r_buffered_socket() noexcept {}

    inline r_buffered_socket& operator = ( r_buffered_socket&& obj ) noexcept
    {
        if(valid())
            close();

        _sok = std::move(obj._sok);
        _buffer = std::move(obj._buffer);
        _bufferOff = std::move(obj._bufferOff);
        obj._bufferOff = 0;
        return *this;
    }

    r_buffered_socket& operator = ( const r_buffered_socket& ) = delete;

    // Returns a reference to the underlying socket. This is especially useful if the underlying socket
    // type is SSL, in which case we don't have t a full API implemented.
    SOK& inner() { return _sok; }

    inline void create( int af ) { _sok.create(af); }

    inline void connect( const std::string& host, int port ) { _sok.connect(host, port); }
    inline void listen( int backlog = MAX_BACKLOG ) { _sok.listen(backlog); }
    inline void bind( int port, const std::string& ip = "" ) { _sok.bind(port, ip ); }
    inline r_buffered_socket accept() { r_buffered_socket bs(_buffer.capacity()); auto s = _sok.accept(); bs._sok = std::move(s); return std::move(bs); }

    inline SOCKET get_sok_id() const { return _sok.get_sok_id(); }

    bool buffer_recv()
    {
        if( _avail_in_buffer() > 0 )
            return true;

        size_t bufferCapacity = _buffer.capacity();

        _buffer.resize( bufferCapacity );

        int bytesRead = _sok.raw_recv( &_buffer[0], bufferCapacity );

        if( bytesRead >= 0 )
        {
            _buffer.resize( bytesRead );
            _bufferOff = 0;
        }

        return (bytesRead > 0) ? true : false;
    }

    inline virtual bool valid() const { return _sok.valid(); }

    virtual void send( const void* buf, size_t len )
    {
        _sok.send( buf, len );
    }

    virtual void recv( void* buf, size_t len )
    {
        size_t bytesToRecv = len;
        uint8_t* dst = (uint8_t*)buf;

        while( bytesToRecv > 0 )
        {
            size_t avail = _avail_in_buffer();

            if( avail == 0 )
            {
                buffer_recv();
                continue;
            }

            size_t ioSize = (bytesToRecv <= avail) ? bytesToRecv : avail;

            memcpy( dst, &_buffer[_bufferOff], ioSize );
            dst += ioSize;
            _bufferOff += ioSize;
            bytesToRecv -= ioSize;
        }
    }

    inline void close() { _sok.close(); }

    inline virtual bool wait_till_recv_wont_block( uint64_t& millis ) const { return _sok.wait_till_recv_wont_block(millis); }
    inline virtual bool wait_till_send_wont_block( uint64_t& millis ) const { return _sok.wait_till_send_wont_block(millis); }

    inline std::string get_peer_ip() const { return _sok.get_peer_ip(); }
    inline std::string get_local_ip() const { return _sok.get_local_ip(); }

    inline SOK& get_socket() { return _sok; }

private:
    inline size_t _avail_in_buffer() { return _buffer.size() - _bufferOff; }

    SOK _sok;
    std::vector<uint8_t> _buffer;
    size_t _bufferOff;
};

class r_socket_connect_exception : public r_exception
{
public:
    r_socket_connect_exception(const char* msg, ...);
    virtual ~r_socket_connect_exception() noexcept {}
};

class r_socket_exception : public r_exception
{
public:
    r_socket_exception(const char* msg, ...);
    virtual ~r_socket_exception() noexcept {}
};

namespace r_networking
{

std::vector<std::string> r_resolve( int type, const std::string& name );

std::map<std::string,std::vector<std::string>> r_get_interface_addresses( int af );

std::vector<uint8_t> r_get_hardware_address(const std::string& ifname);

std::string r_get_device_uuid(const std::string& ifname);

uint16_t r_ntohs(uint16_t x);

uint16_t r_htons(uint16_t x);

uint32_t r_ntohl(uint32_t x);

uint32_t r_htonl(uint32_t x);

uint64_t r_ntohll(uint64_t x);

uint64_t r_htonll(uint64_t x);

}

}

#endif
