/*
* @Author       : wangshuxiao
* @Date         : 2021-03-14
* @copyleft Apache 2.0
*/

#include "httpresp.h"

using namespace std;

const unordered_map<string, string> HttpResp::SUFFIX_TYPE = {
        { ".html",  "text/html" },
        { ".xml",   "text/xml" },
        { ".xhtml", "application/xhtml+xml" },
        { ".txt",   "text/plain" },
        { ".rtf",   "application/rtf" },
        { ".pdf",   "application/pdf" },
        { ".word",  "application/nsword" },
        { ".png",   "image/png" },
        { ".gif",   "image/gif" },
        { ".jpg",   "image/jpeg" },
        { ".jpeg",  "image/jpeg" },
        { ".au",    "audio/basic" },
        { ".mpeg",  "video/mpeg" },
        { ".mpg",   "video/mpeg" },
        { ".avi",   "video/x-msvideo" },
        { ".gz",    "application/x-gzip" },
        { ".tar",   "application/x-tar" },
        { ".css",   "text/css "},
        { ".js",    "text/javascript "},
};

const unordered_map<int, string> HttpResp::CODE_STATUS = {
        { 200, "OK" },
        { 400, "Bad Request" },
        { 403, "Forbidden" },
        { 404, "Not Found" },
};

const unordered_map<int, string> HttpResp::CODE_PATH = {
        { 400, "/400.html" },
        { 403, "/403.html" },
        { 404, "/404.html" },
};

HttpResp::HttpResp() {
    code_ = -1;
    path_ = srcDir_ = "";
    isKeepAlive_ = false;
    mmFile_ = nullptr;
    mmFileStat_ = {0};
}

HttpResp::~HttpResp() {
    UnmapFile();
}

void HttpResp::Init(const std::string &srcDir, std::string &path, bool isKeepAlive, int code) {
    assert(srcDir != "");
    if (mmFile_) UnmapFile();
    code_ = code;
    isKeepAlive_ = isKeepAlive;
    path_ = path;
    srcDir_ = srcDir;
    mmFile_ = nullptr;
    mmFileStat_ = {0};
}

void HttpResp::MakeResponse(Buffer &buffer) {
    /*
     * st_mode  judge the type of file...
     * S_ISDIR  judge if Dir...
     * S_IROTH  judge if others have the permission...
     */
    if (stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) {
        code_ = 404;
    }
    else if (!(mmFileStat_.st_mode & S_IROTH)) code_ = 403;
    else if (code_ == -1) code_ = 200;

    ErrorHtml_();
    AddStateLine_(buffer);
    AddHeader_(buffer);
    AddContent_(buffer);
}

char* HttpResp::File() {
    return mmFile_;
}

size_t HttpResp::FileLen() const {
    return mmFileStat_.st_size;
}

void HttpResp::ErrorHtml_() {
    if (CODE_PATH.count(code_) == 1) {
        path_ = CODE_PATH.find(code_)->second;
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
}

void HttpResp::AddStateLine_(Buffer &buffer) {
    string status;
    if (CODE_STATUS.count(code_) == 1) status = CODE_STATUS.find(code_)->second;
    else {
        code_ = 400;
        status = CODE_STATUS.find(code_)->second;
    }
    buffer.Append("HTTP/1.1 " + to_string(code_) + " " + status + "\r\n");
}

void HttpResp::AddHeader_(Buffer &buffer) {
    buffer.Append("Connection: ");
    if (isKeepAlive_) {
        buffer.Append("keep-alive\r\n");
        buffer.Append("keep-alive: max=6, timeout=120\r\n");
    }
    else {
        buffer.Append("close\r\n");
    }
    buffer.Append("Content-type: " + GetFileType_() + "\r\n");
}

void HttpResp::AddContent_(Buffer &buffer) {
    /*
     *  O_RDONLY    read-only
     *  mmay        file to memory
     *  PROT_READ   allow to read
     *  MAP_PRIVATE It's unchange and private
     */
    int srcFd = open((srcDir_ + path_).data(), O_RDONLY);
    if (srcFd < 0) {
        ErrorContent(buffer, "File NotFound");
        return;
    }
    LOG_DEBUG("file path %s", (srcDir_ + path_).data());

    // mmap file into memory to imporve the speed...
    int* mmRet = (int *)mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if (*mmRet == -1) {
        ErrorContent(buffer, "File NotFound");
        return;
    }
    mmFile_ = (char*) mmRet;
    close(srcFd);
    buffer.Append("Content-length: " + to_string(mmFileStat_.st_size) + "\r\n\r\n");
}

string HttpResp::GetFileType_() {
    string::size_type idx = path_.find_last_of('.');
    if (idx == string::npos) return "text/plain";
    string suffix = path_.substr(idx);
    if (SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

void HttpResp::ErrorContent(Buffer &buffer, std::string message) {
    string body;
    string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(code_) == 1) status = CODE_STATUS.find(code_)->second;
    else status = "Bad Request";
    body += to_string(code_) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>WSX's WebServer</em></body></html>";

    buffer.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}

void HttpResp::UnmapFile() {
    // remove the mmap from file to memory
    if (mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
}
