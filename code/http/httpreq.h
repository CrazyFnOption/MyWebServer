/*
 * @Author       : wangshuxiao
 * @Date         : 2021-03-14
 * @copyleft Apache 2.0
 */

#ifndef HTTPREQ_H
#define HTTPREQ_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>
#include <mysql/mysql.h>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"


class HttpReq {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    HttpReq() {Init();}
    ~HttpReq() = default;

    void Init();
    bool Parse(Buffer& buffer);

    // analysis the request
    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;

private:
    // the way to Analysis
    // The Request is made up of requestline , header, body.
    bool ParseRequestLine_(const std::string& line);
    void ParseHeader_(const std::string& line);
    void ParseBody_(const std::string& line);

    // another parse
    void ParsePath_();
    void ParsePost_();
    void ParseFromUrlencoded_();

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);

    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);

};


#endif //HTTPREQ_H
