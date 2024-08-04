#include <iostream>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <memory>
#include <cppcms/json.h>

#define DB_NAME ""  //set DB name here
#define DB_USERNAME "root" //default user
#define DB_PASSWORD "root" //default password set it accordingly
#define DB_HOST "tcp://127.0.0.1:3306" //default port on localhost

using namespace std;

class User{
private:
    sql::mysql::MySQL_Driver *driver;
    unique_ptr<sql::Connection> con;
    unique_ptr<sql::PreparedStatement> stmt;

public:
    User() {
        try {
            driver = sql::mysql::get_mysql_driver_instance();
            con.reset(driver->connect(DB_HOST, DB_USERNAME, DB_PASSWORD));
            con->setSchema(DB_NAME);
        } catch (sql::SQLException &e) {
            cerr << "SQLException: " << e.what() << endl;
        } catch (...) {
            cerr << "Unexpected error during connection" << endl;
        }
    }


    int create_user(const string &name, const string &password, const string &email) {
        try {
            stmt.reset(con->prepareStatement("INSERT INTO user(name, password, email) VALUES (?, ?, ?)"));
            stmt->setString(1, name);
            stmt->setString(2, password);
            stmt->setString(3, email);

            // Execute the statement
            stmt->executeUpdate();

            // Get the last inserted ID
            stmt.reset(con->prepareStatement("SELECT LAST_INSERT_ID() AS id;"));
            unique_ptr<sql::ResultSet> result(stmt->executeQuery());
            if (result->next()) {
                return result->getInt("id");
            }

            return -1;

        } catch (sql::SQLException &e) {
            cerr << "SQLException: " << e.what() << endl;
            return -1;
        } catch (...) {
            cerr << "Unexpected error during insert" << endl;
            return -1;
        }
    }

    cppcms::json::value get_user(int id){
        try {
            stmt.reset(con->prepareStatement("SELECT name,email FROM user WHERE id = ?"));
            stmt->setInt(1, id);

            unique_ptr<sql::ResultSet> result(stmt->executeQuery());

            cppcms::json::value user;

            if(result->next()) {
                string name = result->getString("name");
                string email = result->getString("email");

                user["name"]=name;
                user["email"]=email;
                user["id"]=id;

                return user;
            }

        } catch(sql::SQLException &e) {
            cerr << e.what();
        }
        return cppcms::json::null();
    } 

    int validate_user(string email,string password){
        try{
            stmt.reset(con->prepareStatement("select id from user where email =? and password =?"));
            stmt->setString(1,email);
            stmt->setString(2,password);
            unique_ptr<sql::ResultSet> result(stmt->executeQuery());
            if(result->next()){
                int id=result->getInt(1);
                return id;
            }
        }
        catch(sql::SQLException &e){
            cerr<<endl<<e.what()<<endl;
        }
        return -1;
    }
};

