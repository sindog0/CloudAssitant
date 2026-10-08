#pragma once

#include <mutex>
#include <memory>
#include <mysql/mysql.h>

class ORMManager
{
public:
    ~ORMManager();
    static ORMManager *GetInstance();
    void UserRegister(const char *name, const char *account, const char *password, const char *usercode, const char *sig_server);
    MYSQL_ROW UserLogin(const char *usercode);
    void UserDestroy(const char *usercode);
    void insertClient(const char *name, const char *account, const char *password, const char *usercode, int online, long recently_login, const char *sig_server);

protected:
    void deleteClientByUsercode(const char *usercode);
    MYSQL_ROW selectClientByUsercode(const char *usercode);

private:
    ORMManager();
    MYSQL mysql_;

private:
    static std::unique_ptr<ORMManager> instance_;
};