#pragma once
#include <json/json.h>
#include <string>
using std::string;

class Message {
public:
    //反序列化客户端发送给服务端的json数据串
    void deserialize(const string& msg) {
        Json::Reader reader;
        Json::Value val;
        reader.parse(msg, val, false);

        _nickName = val["_nickName"].asString();
        _career = val["_career"].asString();
        _msg = val["_msg"].asString();
        _userId  = val["_userId"].asUInt();
    }

    //序列化接口
    void serialize(string& msg) {
        Json::Value val;
        val["_nickName"] = _nickName;
        val["_career"] = _career;
        val["_msg"] = _msg;
        val["_userId"] = _userId;

        Json::FastWriter writer;
        msg = writer.write(val);
    }

    uint32_t& getUserId() {
        return _userId;
    }

    string& getCareer() {
        return _career;
    }

    string& getNickName() {
        return _nickName;
    }

    string& getMsg() {
        return _msg;
    }

    void setNickName(const string& nickname) {
        _nickName = nickname;
    }

    void setCaree(const string& career) {
        _career = career;
    }

    void setUserId(const uint32_t& userid) {
        _userId = userid;
    }

    void setMsg(const string& msg) {
        _msg = msg;
    }

private:
    string _nickName;
    string _career;
    string _msg;
    uint32_t _userId;
};

