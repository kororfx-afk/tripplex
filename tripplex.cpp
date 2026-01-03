#include "../include/tripplex_dll_api.h"
#include <string>
#include <mutex>
#include <thread>
#include <iostream>
#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

static std::string g_endpoint = "127.0.0.1:9000";
static std::mutex g_mutex;

static void cleanup_sockets()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

static bool parse_endpoint(const char* cfg, std::string &host, int &port)
{
    if (!cfg) return false;
    std::string s(cfg);
    // naive parse: look for last ':' and split host:port
    auto pos = s.find_last_of(':');
    if (pos == std::string::npos) return false;
    host = s.substr(0, pos);
    std::string port_s = s.substr(pos + 1);
    try {
        port = std::stoi(port_s);
    } catch(...) { return false; }
    return true;
}

static int connect_send_recv_plain(const std::string &host, int port, const std::string &data)
{
#ifdef _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) return -1;
#endif

    int sockfd = -1;
#ifdef _WIN32
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (sockfd < 0) { cleanup_sockets(); return -2; }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(static_cast<uint16_t>(port));
#ifdef _WIN32
    InetPtonA(AF_INET, host.c_str(), &serv_addr.sin_addr);
#else
    inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr);
#endif

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        cleanup_sockets();
        return -3;
    }

    // send data
    int total = 0;
    const char* buf = data.c_str();
    int len = static_cast<int>(data.size());
    while (total < len) {
#ifdef _WIN32
        int sent = send(sockfd, buf + total, len - total, 0);
#else
        int sent = ::send(sockfd, buf + total, len - total, 0);
#endif
        if (sent <= 0) break;
        total += sent;
    }

    // simple recv (echo)
    char rbuf[4096];
#ifdef _WIN32
    int r = recv(sockfd, rbuf, sizeof(rbuf)-1, 0);
#else
    int r = ::recv(sockfd, rbuf, sizeof(rbuf)-1, 0);
#endif
    if (r > 0) {
        rbuf[r] = '\0';
        std::lock_guard<std::mutex> lk(g_mutex);
        std::cerr << "tripplex dll: recv echo: " << rbuf << std::endl;
    }

#ifdef _WIN32
    closesocket(sockfd);
#else
    close(sockfd);
#endif
    cleanup_sockets();
    return 0;
}

#ifdef USE_OPENSSL
static bool openssl_init = false;

static void openssl_global_init()
{
    if (openssl_init) return;
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    openssl_init = true;
}

static int connect_send_recv_tls(const std::string &host, int port, const std::string &data)
{
    openssl_global_init();

    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) return -10;

    int sockfd = -1;
#ifdef _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) { SSL_CTX_free(ctx); return -11; }
#endif

#ifdef _WIN32
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (sockfd < 0) { SSL_CTX_free(ctx); cleanup_sockets(); return -12; }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(static_cast<uint16_t>(port));
#ifdef _WIN32
    InetPtonA(AF_INET, host.c_str(), &serv_addr.sin_addr);
#else
    inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr);
#endif

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        SSL_CTX_free(ctx);
        cleanup_sockets();
        return -13;
    }

    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        SSL_CTX_free(ctx);
        cleanup_sockets();
        return -14;
    }

    SSL_set_fd(ssl, (int)sockfd);
    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        SSL_CTX_free(ctx);
        cleanup_sockets();
        return -15;
    }

    // send
    int written = SSL_write(ssl, data.data(), (int)data.size());
    if (written <= 0) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        SSL_CTX_free(ctx);
        cleanup_sockets();
        return -16;
    }

    // read
    char rbuf[4096];
    int r = SSL_read(ssl, rbuf, sizeof(rbuf)-1);
    if (r > 0) {
        rbuf[r] = '\0';
        std::lock_guard<std::mutex> lk(g_mutex);
        std::cerr << "tripplex dll TLS: recv echo: " << rbuf << std::endl;
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
#ifdef _WIN32
    closesocket(sockfd);
#else
    close(sockfd);
#endif
    SSL_CTX_free(ctx);
    cleanup_sockets();
    return 0;
}
#endif

extern "C" {

TPX_API int TPX_Init(const char* config_json)
{
    std::lock_guard<std::mutex> lk(g_mutex);
    if (!config_json) return -1;
    // naive: accept either bare "host:port" or JSON with endpoint value
    std::string s(config_json);
    // if contains 'endpoint', try to extract after the last ':' occurrence
    size_t pos = s.find("endpoint");
    std::string ep = s;
    if (pos != std::string::npos) {
        // look for first quote after endpoint
        auto q = s.find('"', pos);
        if (q != std::string::npos) {
            auto q2 = s.find('"', q+1);
            if (q2 != std::string::npos) {
                ep = s.substr(q+1, q2-q-1);
            }
        }
    }
    g_endpoint = ep;
    return 0;
}

TPX_API int TPX_Auth(const char* auth_token)
{
    // prototype accepts all tokens
    (void)auth_token;
    return 0;
}

TPX_API int TPX_Subscribe(const char* symbol, int depth)
{
    (void)symbol; (void)depth;
    // no-op in prototype
    return 0;
}

TPX_API int TPX_Unsubscribe(const char* symbol)
{
    (void)symbol;
    return 0;
}

TPX_API int TPX_SendOrder(const char* order_payload, const char* client_order_id)
{
    if (!order_payload) return -1;
    std::string host;
    int port = 9000;
    if (!parse_endpoint(g_endpoint.c_str(), host, port)) {
        host = g_endpoint;
    }
    std::string data = "CLIENT_ORDER_ID:";
    if (client_order_id) data += client_order_id;
    data += "\n";
    data += order_payload;
#ifdef USE_OPENSSL
    // Use TLS when built with OpenSSL support
    return connect_send_recv_tls(host, port, data);
#else
    return connect_send_recv_plain(host, port, data);
#endif
}

TPX_API int TPX_ModifyOrder(const char* order_id, const char* update_payload)
{
    (void)order_id; (void)update_payload;
    return 0;
}

TPX_API int TPX_CancelOrder(const char* order_id)
{
    (void)order_id;
    return 0;
}

TPX_API int TPX_Query(const char* query_json, char* out_buf, int out_buf_len)
{
    (void)query_json;
    if (out_buf && out_buf_len>0) {
        const char* sample = "{\"positions\":[] }";
        strncpy(out_buf, sample, out_buf_len-1);
        out_buf[out_buf_len-1] = '\0';
        return 0;
    }
    return -1;
}

TPX_API int TPX_Heartbeat(uint64_t seq)
{
    (void)seq;
    return 0;
}

TPX_API void TPX_Shutdown()
{
    // nothing to cleanup in prototype
}

} // extern C
