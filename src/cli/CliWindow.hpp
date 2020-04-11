#pragma once
#include <ncurses.h>
#include <pthread.h>
#include "ChatClient.hpp"

class Win {
protected:
    static pthread_mutex_t _lockWin;

public:
    static void init() {
        initscr();
        curs_set(0);
    }

    virtual void draw() = 0;
    virtual void fresh() = 0;

    static void end() {
        endwin();
    }

    virtual ~Win() {}
};

pthread_mutex_t Win::_lockWin = PTHREAD_MUTEX_INITIALIZER;

class Header : public Win {
private:
    WINDOW* _win;
public:
    Header() : _win(nullptr) { }

    virtual void fresh() {
        pthread_mutex_lock(&_lockWin);
        wrefresh(_win);
        pthread_mutex_unlock(&_lockWin);
    }

    void draw() {
        _win =  newwin(LINES / 5, COLS, 0, 0);
        box(_win, 0, 0);
        fresh();
    }

    void putStringToWin() {
        int y, x;
        int pos = 1;
        int flags = 0;
        string str = "Speek OUT Freely";
        while (1) {
            draw();
            //getmaxyx函数返回窗口最大的行数存储在变量y当中，返回最大的列数存储在变量x当中
            getmaxyx(_win, y, x);
            mvwaddstr(_win, y / 2, pos, str.c_str());
            fresh();
            if (pos < 2) {
                flags = 0;
            } else if (pos > x - (int)str.size() - 2) {
                flags = 1;
            }
            if (flags == 0) {
                pos++;
            } else {
                pos--;
            }
            sleep(1);
        }
    }
};

class Output : public Win {
private:
    WINDOW* _win;
public:
    Output() : _win(nullptr) {
    }

    ~Output() {
        if (_win)
            delwin(_win);
    }

    virtual void fresh() {
        pthread_mutex_lock(&_lockWin);
        wrefresh(_win);
        pthread_mutex_unlock(&_lockWin);
    }

    virtual void draw() {
        _win = newwin(LINES * 3 / 5, COLS * 3 / 5, LINES / 5, 0);
        box(_win, 0, 0);
        fresh();
    }

    void putStringToWin(const string& msg, int line) {
        mvwaddstr(_win, line, 3, msg.c_str());
        fresh();
    }

    void getXY(int& y, int& x)  {
        getmaxyx(_win, y, x);
    }
};

class Member : public Win {
private:
    WINDOW* _win;
public:
    Member() : _win(nullptr) {}

    virtual void fresh() {
        pthread_mutex_lock(&_lockWin);
        wrefresh(_win);
        pthread_mutex_unlock(&_lockWin);
    }

    virtual void draw() {
        _win = newwin(LINES * 3 / 5, COLS * 2 / 5, LINES / 5, COLS * 3 / 5);
        //box(_win, '|', '\"');
        box(_win, 0, 0);
        fresh();
    }

    void getXY(int& y, int& x) {
        getmaxyx(_win, y, x);
    }

    void putStringToWin(std::unordered_set<std::string> online) {
        int y, x;
        getXY(y, x);
        int line = 1;
        for (const auto& str : online) {
            mvwaddstr(_win, line, 1, str.c_str());
            ++line;
            fresh();
            if (line > y) {
                break;
            }
        }
    }

    ~Member() {
        if (_win)
            delwin(_win);
    }
};

class Chat : public Win {
private:
    WINDOW* _win;
public:
    Chat() : _win(nullptr) {}

    virtual void fresh() {
        pthread_mutex_lock(&_lockWin);
        wrefresh(_win);
        pthread_mutex_unlock(&_lockWin);
    }

    virtual void draw() {
        _win = newwin(LINES / 5, COLS, LINES * 4 / 5, 0);
        //box(_win, '|', '+');
        box(_win, 0, 0);
        fresh();
    }

    virtual void putStringToWin() {
        std::string tips = "Please Enter# ";
        mvwaddstr(_win, 2, 2, tips.c_str());
        fresh();
    }

    void getStringFromWin(string& userTyping) {
        char buf[1024] = { 0 };
        wgetnstr(_win, buf, sizeof(buf) - 1);
        userTyping.assign(buf, strlen(buf));
    }

    ~Chat() {
        if (_win)
            delwin(_win);
    }
};
