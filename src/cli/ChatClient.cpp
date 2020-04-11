#include "ChatClient.hpp"
#include "Message.hpp"
#include <json/json.h>
#include "CliWindow.hpp"

void menu() {
    std::cout << "----------------------------------" << std::endl;
    std::cout << "| 1. register         2. login   |" << std::endl << std::endl;
    std::cout << "| 3. logout           4. exit    |" << std::endl;
    std::cout << "----------------------------------" << std::endl;
}

struct para {
    int _i;
    ChatClient* _cc;

    para(int i, ChatClient* ptr) : _i(i), _cc(ptr) {
    }
};

void* start(void* arg) {
    para* p = (para*)arg;
    int i = p->_i;
    ChatClient* pcc = p->_cc;
    switch(i) {
        case 0: {
            Header* head = new Header();
            head->putStringToWin();
            break;
         }
        case 1: {
            Output* out = new Output();
            Message msg;
            std::string recv_msg;
            out->draw();
            int line = 1;
            int x, y;
            while (1) {
                out->getXY(y, x);
                pcc->Recv(recv_msg);
                //反序列化
                msg.deserialize(recv_msg);
                //展示数据 _nickname-caree# msg
                std::string show_msg;
                //debug
                //std::cout << msg.getNickName();
                //string s = msg.getNickName();
                //std::cout << "---------------" << s << std::endl;
                show_msg.append(msg.getNickName());
                //debug
                //std::cout << "------------------------getNickName" << show_msg << std::endl;
                show_msg += "-";
                show_msg += msg.getCareer();
                pcc->pushUser(show_msg);
                show_msg += "#  ";
                show_msg += msg.getMsg();
                if (line > y - 2) {
                    line = 1;
                    out->draw();
                }

                out->putStringToWin(show_msg, line);
                ++line;
            }
            break;
        }
        case 2: {
            Member* mem = new Member();
            while (1) {
                mem->draw();
                mem->putStringToWin(pcc->getonlineUser());
                sleep(1);
            }
            break;
        }
        case 3: {
            Chat* ch = new Chat();
            Message msg;
            msg.setNickName(pcc->getMyself()._nickname);
            msg.setCaree(pcc->getMyself()._career);
            msg.setUserId(pcc->getMyself()._userId);
            //用户输入的原始消息    
            std::string user_enter_msg;
            //序列化完成之后的消息    
            std::string send_msg;    
            while (1) {
                ch->draw();
                ch->putStringToWin();
                ch->getStringFromWin(user_enter_msg);
                msg.setMsg(user_enter_msg);

                msg.serialize(send_msg);
                pcc->Send(send_msg);
            }
            break;
        }
    }
    delete p;
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "operating parameter format is ./ChatCli [ip]" << std::endl;
        exit(1);
    }
    ChatClient* cc = new ChatClient(argv[1]);
    while (1) {
        menu();
        int cliSelect = -1;
        std::cout << "please input your option:";
        fflush(stdout);
        std::cin >> cliSelect;
        if (cliSelect == 1) {
            cc->Init();
            if (!cc->regiser()) {
                std::cout << "regiser failed! please try again " << std::endl;
            } else {
                std::cout << "register success! please login" << std::endl;
            }
        } else if (cliSelect == 2) {
            cc->Init();
            if (!cc->login()) {
                std::cout << "login failed! please check your userId and password " << std::endl;
            } else {
                std::cout << "login success! Let's chat!" << std::endl;
            }

            Win::init();
            pthread_t tid[4] = { 0 };

            for (int i = 0; i < 4; ++i) {
                para* p = new para(i, cc);
                pthread_create(tid + i, NULL, start, p);
            }

            for (int i = 0; i < 4; ++i) {
                pthread_join(tid[i], NULL);
            }
#if 0
            while (1) {
                Win::init();
                Header* head = new Header();
                head->fresh();
                head->putStringToWin();
                Output* out = new Output();
                out->fresh();
                Member* list = new Member();
                list->fresh();
                Chat* input = new Chat();
                input->fresh();
            }

            Json::Value val;
            val["_nickName"] = "1";
            val["_career"] = "1";

            while (1) {
                std::cout << "input:" << std::endl;
                string msg;
                std::cin >> msg;
                val["_msg"] = msg;
                val["_userId"] = 1;
                Json::FastWriter writer;
                msg = writer.write(val);
                if (!cc->Send(msg)) {
                    std::cout << "send failed, please try again" << std::endl;
                }
                msg.clear();

                cc->Recv(msg);
                std::cout << "recv msg:" << msg << std::endl; 

            }
#endif

        } else if (cliSelect == 3) {

        } else if (cliSelect == 4) {
            break;
        } else {
            std::cout << "input error" << std::endl;
        }
    }

    delete cc;
    return 0;
}
