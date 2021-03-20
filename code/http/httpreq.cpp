/*
 * @Author       : wangshuxiao
 * @Date         : 2021-03-14
 * @copyleft Apache 2.0
 */
#include "httpreq.h"

using namespace std;

// the html page
const unordered_set<string> HttpReq::DEFAULT_HTML {
        "/index", "/register", "/login",
        "/welcome", "/video", "/picture"
};

const unordered_map<string, int>HttpReq::DEFAULT_HTML_TAG {
        {"/register.html", 0}, {"/login.html", 1},
};

void HttpReq::Init() {
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool HttpReq::IsKeepAlive() const {
    if (header_.count("Connection") == 1) {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

bool HttpReq::Parse(Buffer &buffer) {
    const char CRLF[] = "\r\n";
    if (buffer.ReadableBytes() <= 0) return false;
    while (buffer.ReadableBytes() && state_ != FINISH) {
        const char* lineEnd = search(buffer.Peek(), buffer.BeginWriteConst(), CRLF, CRLF + 2);
        string line(buffer.Peek(), lineEnd);
        switch(state_)
        {
            case REQUEST_LINE:
                if (!ParseRequestLine_(line)) return false;
                ParsePath_();
                break;
            case HEADERS:
                ParseHeader_(line);
                if (buffer.ReadableBytes() <= 2) state_ = FINISH;
                break;
            case BODY:
                ParseBody_(line);
                break;
            default:
                break;
        }
        if (lineEnd == buffer.BeginWrite()) break;
        // read buffer until lineEnd + \r\n
        buffer.TakeUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

void HttpReq::ParsePath_() {
    // this can change to /home.html
    if (path_ == "/") {
        path_ = "/index.html";
    }
    else {
        for (auto &item: DEFAULT_HTML) {
            if (item == path_){
                path_ += ".html";
                break;
            }
        }
    }
}

bool HttpReq::ParseRequestLine_(const std::string &line) {
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch subMatch;
    if (regex_match(line, subMatch, patten)) {
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpReq::ParseHeader_(const std::string &line) {
    regex patten ("^([^:]) ?(.*)$");
    smatch subMatch;
    if (regex_match(line, subMatch, patten)) {
        header_[subMatch[1]] = subMatch[2];
    }
    else state_ = HEADERS;
}

void HttpReq::ParseBody_(const std::string &line) {
    body_ = line;
    ParsePost_();
    state_ = FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

int HttpReq::ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

void HttpReq::ParseFromUrlencoded_() {
    if (body_.size() == 0) return;

    string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = body_[i];
        switch (ch) {
            case '=':
                key = body_.substr(j, i - j);
                j = i + 1;
                break;
            case '+':
                body_[i] = ' ';
                break;
            case '%':
                // @why % decoding?
                num = ConverHex(body_[i + 1]) * 16 + ConverHex(body_[i + 2]);
                body_[i + 2] = num % 10 + '0';
                body_[i + 1] = num / 10 + '0';
                i += 2;
                break;
            case '&':
                value = body_.substr(j, i - j);
                j = i + 1;
                post_[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;
        }
    }
    assert(j <= i);
    if (post_.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

void HttpReq::ParsePost_() {
    if (method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlencoded_();
        if (DEFAULT_HTML_TAG.count(path_)) {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("Tag:%d", tag);
            if (tag == 0 || tag == 1) {
                bool isLogin = (tag == 1);
                if(UserVerify(post_["username"], post_["password"], isLogin)) {
                    path_ = "/welcome.html";
                }
                else path_ = "error.html";
            }
        }
    }
}

bool HttpReq::UserVerify(const std::string &name, const std::string &pwd, bool isLogin) {
    if (name == "" || pwd == "") return false;
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
    MYSQL *sql;
    SqlConnRAII(sql, SqlConnPool::GetInstence());
    assert(sql);

    bool flag = false;
    unsigned int j = 0;
    char order[256] = { 0 };
    MYSQL_FIELD* field = nullptr;
    MYSQL_RES *res = nullptr;
    if (!isLogin) flag = true;

    snprintf(order, 256, "SELECT username, password FROM "
                         "user WHERE username='%s' LIMIT 1",name.c_str());
    LOG_DEBUG("%s", order);

    if(mysql_query(sql, order)) {
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        string password(row[1]);

        if(isLogin) {
            if(pwd == password) { flag = true; }
            else {
                flag = false;
                LOG_DEBUG("pwd error!");
            }
        }
        else {
            // this username is used
            flag = false;
            LOG_DEBUG("user used!");
        }
    }
    mysql_free_result(res);

    // no one use the username
    if(!isLogin && flag) {
        LOG_DEBUG("regirster!");
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG( "%s", order);
        if(mysql_query(sql, order)) {
            LOG_DEBUG( "Insert error!");
            flag = false;
        }
        flag = true;
    }
    SqlConnPool::Instance()->FreeConn(sql);
    LOG_DEBUG( "UserVerify success!!");
    return flag;
}

std::string HttpReq::path() const{
    return path_;
}

std::string& HttpReq::path(){
    return path_;
}
std::string HttpReq::method() const {
    return method_;
}

std::string HttpReq::version() const {
    return version_;
}

std::string HttpReq::GetPost(const std::string& key) const {
    assert(key != "");
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpReq::GetPost(const char* key) const {
    assert(key != nullptr);
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}