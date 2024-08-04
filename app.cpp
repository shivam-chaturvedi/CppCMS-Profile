#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<cppcms/application.h>
#include<cppcms/http_response.h>
#include<cppcms/url_dispatcher.h>
#include<cppcms/http_request.h>
#include<cppcms/service.h>
#include<cppcms/applications_pool.h>
#include<cppcms/json.h>
#include<cppcms/serialization.h>
#include<chrono>
#include <boost/date_time/gregorian/gregorian.hpp>
#include<cppcms/http_file.h>

#include "models.cpp"



using namespace std;

class App :public cppcms::application{
    private:
    cppcms::json::value success;
    cppcms::json::value error;

    string error_path="error.html";
    string default_error_str="<h1>404 - Page Not Found<br> <span style=\"color:red;text-wrap:wrap;\">To edit defualt error page make a error.html file inside templates folder parent directory</span></h1>";
    
    string render(string path,bool binary_mode=false){
        ios_base::openmode mode=binary_mode?ios::binary|ios::in:ios::in;
        fstream file(path,mode);
        if(!file.is_open()){
            throw "File Not Found !";
        }
        stringstream ss;
        ss<<file.rdbuf();
        file.close();
        return ss.str();
    }

    void serve_static(string url){
        try{
            if(url.find(".html")!=url.npos){
                response().status(200);
                response().content_type("text/html");
                response().out()<<render(url);
            }else if(url.find(".css")!=url.npos){
                response().status(200);
                response().content_type("text/css");
                response().out()<<render(url);
            }else if(url.find(".js")!=url.npos){
                response().status(200);
                response().content_type("application/javascript");
                response().out()<<render(url);
            }else if(url.find(".png")!=url.npos){
                response().status(200);
                response().content_type("image/png");
                response().out()<<render(url,true);
            }else if(url.find(".jpg")!=url.npos){
                response().status(200);
                response().content_type("image/jpg");
                response().out()<<render(url,true);
            }else if(url.find(".gif")!=url.npos){
                response().status(200);
                response().content_type("image/gif");
                response().out()<<render(url,true);
            }else{
                response().status(200);
                response().content_type("application/octet-stream");
                response().out()<<render(url,true);
            }
        }
        catch(...){
            response().status(404);
            response().out()<<default_error_str;
        }
    }

    void send_file(string path){
        try{
            fstream file(path,ios::in|ios::binary);
            int dot=path.find_last_of(".");
            string extension="";
            if(dot!=path.npos){
                extension=path.substr(++dot);
            }
            string name="file";
            int i=path.find("/");
            if(i!=path.npos){
                name=path.substr(++i);
            }
            else{
                name=path;
            }
            response().content_type("application/"+extension);
            response().status(200);
            response().set_header("Content-Disposition","attachment;filename=\" "+name+"\" ");
            response().out()<<file.rdbuf();
            file.close();
        }
        catch(const exception &e){
            response().status(404);
            response().out()<<e.what();
        }
    }

    public:

    App(cppcms::service &srv):cppcms::application(srv){
        // register your url dispatcher here
        error["msg"]="error";
        success["msg"]="success";

        dispatcher().assign("/",&App::Home,this);
        dispatcher().assign("/get_user",&App::get_user,this);
        dispatcher().assign("/download",&App::download,this);
        dispatcher().assign("/upload-profile",&App::upload_profile,this);
        dispatcher().assign("/profiles/(.+)",&App::serve_profile_pictures,this,1);
        dispatcher().assign("/register",&App::register_user,this);
        dispatcher().assign("/login",&App::login,this);
        dispatcher().assign("/css/(.+)",&App::css_handler,this,1);
        dispatcher().assign(".*",&App::not_found,this);
    }

    void serve_profile_pictures(string image_name){
        serve_static("profiles/"+image_name+".png");
    }

    void upload_profile(){
        string method=request().request_method();
        if(method=="POST"){
            shared_ptr<cppcms::http::file> profile=request().files().front();
            string id=request().post("id");

            string name=id+".png";
            fstream os("profiles/"+name,ios::binary|ios::out);
            os<<profile->data().rdbuf();
            os.close();

            response().status(201);
            response().content_type("application/json");
            success["msg"]="Profile Picture Updated!";
            response().out()<<success;
        }
        else{
            response().status(400);
            response().content_type("application/json");
            response().out()<<error;   
        }
    }

    void get_user(){
        string method=request().request_method();
        if(method=="GET"){
            try{
                string id=request().query_string();
                int idx=stoi(id.substr(3));
                User u;
                cppcms::json::value user=u.get_user(idx);
                user["profileImageUrl"]="/profiles/"+id.substr(3);
                if(!user.is_null()){
                response().content_type("application/json");
                response().status(200);
                response().out()<<user;
                }
                else{
                    response().status(400);
                    response().out()<<error;
                }
            }
            catch(...){
                response().status(400);
                response().out()<<error;
            }
        }
        else{
            error["msg"]="invalid method";
            response().status(400);
            response().out()<<error;
        }
    }

    void register_user(){
        if(request().request_method()=="POST"){
            try{
                pair<void *,int>  data=request().raw_post_data();
                string s(static_cast<char*>(data.first),data.second);
                stringstream ss(s);
                cppcms::json::value json;
                json.load(ss,true);

                string name=json.get<string>("name");
                string email=json.get<string>("email");
                string password=json.get<string>("password");

                User u;
                int id=u.create_user(name,password,email);
                if(id==-1){
                response().status(400);
                response().content_type("application/json");
                error["msg"]="error";
                response().out()<<error;
                }
                success["id"]=id;
                response().status(200);
                response().content_type("application/json");
                response().out()<<success;
            }
            catch(sql::SQLException &e){
                cerr<<endl<<e.what()<<endl;
                response().status(400);
                response().out()<<error;
            }
        }
        else if(request().request_method()=="GET"){
            serve_static("templates/register.html");
        }
        else{
            response().status(400);
            response().out()<<"invalid method";
        }
    }

    
    void login(){
        if(request().request_method()=="POST"){
            pair<void *,int> data=request().raw_post_data();
            string s(static_cast<char*>(data.first),data.second);
            stringstream ss(s);
            cppcms::json::value json;
            json.load(ss,true);
            string email=json.get<string>("email");
            string pass=json.get<string>("password");
            User u;
            int id=u.validate_user(email,pass);
            if(id!=-1){
                success["id"]=id;
                response().out()<<success;
            }
            else{
                response().status(400);
                response().out()<<error;
            }
        }
        else if(request().request_method()=="GET"){
            serve_static("templates/login.html");
        }
        else{
            response().out()<<"Invalid Method";
        }
    }

    void Home(){
        serve_static("templates/index.html");
    }

    void download(){
        send_file("i.exe");
    }

    void css_handler(string css_file_name){
        serve_static("templates/css/"+css_file_name);
    }

    void not_found(){
        serve_static("templates/error.html");
    }

    virtual void main(string url){
        dispatcher().dispatch(url);
    }
    
};

int main(int argc,char *argv[]){
    try{
        cppcms::service service(argc,argv);
        service.applications_pool().mount(cppcms::applications_factory<App>());
        string ip=service.settings().get<string>("service.ip");
        int port=service.settings().get<int>("service.port");
        cout<<endl<<"Server started on "<<ip<<":"<<port<<endl;
        service.run();
    }catch(const exception &e){
        cout<<endl<<e.what()<<endl;
    }
    return 0;
}


