
#include "helpers.h"



using namespace std;

int main(int argc,char* argv[])
{


    std::cout << std::unitbuf;

    if(argc<=1) 
    {
        cerr<<"No file Path given\n";
        exit(-1);
    }
 
    root=argv[1];
    rootDir=root;
    rootDir = rootDir.substr(0, rootDir.find_last_of('/')); // "Compiler2.0/web/index.html" -> "Compiler2.0/web"
    cout<<"Root Calculated as: "<<rootDir<<"\n";


    const int connectionLimit=25;
    const int bufferSize=1024;

    char buffer[bufferSize];
    
    int status=0;

    char* FilePath=nullptr; //used later 

    //always open the root file first
    fstream* html=openFile(nullptr,true);
    
    
   

    //server code 

    struct addrinfo hints;
    struct addrinfo* results;
    struct addrinfo* cursor;

    memset(&hints,0,sizeof hints);
    
    hints.ai_family=AF_INET6;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;

    status=getaddrinfo(NULL,"8080",&hints,&results);
    cursor=results;

    while(cursor !=nullptr && cursor->ai_family != AF_INET6  )
    {
        cursor=cursor->ai_next;
    }
    
    if(cursor==nullptr)
    {
        cerr<<"No Valid IPv6 address found \n"; exit(-4);
    }

    int Socketfd=socket(cursor->ai_family,cursor->ai_socktype,cursor->ai_protocol);

    if(Socketfd == -1) {cerr<<"Socket Call failed\n";exit(-5);}


    //================================================
    //      SOCKET CONFIG
    //================================================

    int yes = 1;
    if (setsockopt(Socketfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) == -1)
    {
        perror("setsockopt");
    }

    if (bind(Socketfd, cursor->ai_addr, cursor->ai_addrlen) == -1)
    {
        perror("bind");
        return 1;
    }

    sockaddr_in6 * Myaddress=(sockaddr_in6*)(cursor->ai_addr);
    char str[INET6_ADDRSTRLEN]; 

    if (inet_ntop(AF_INET6, &Myaddress->sin6_addr, str, sizeof(str))) {
        std::cout << "IPv6 Address: " << str << std::endl;
    } else {
        std::cerr << "Error converting IP address" << std::endl;
    }


    cout<<"listening on port "<<ntohs(Myaddress->sin6_port)<<" with IP "<<str<<"\n";

    if (listen(Socketfd, SOMAXCONN) == -1)
    {
        perror("listen");
        return 1;
    }



    //=====================================================
    //      SERVER COMMUNICATION
    //=====================================================
   
 
 
    while (true)
    {   
        
        //==========================================
        //  ACCEPT NEW CLIENT
        //==========================================

        sockaddr_storage clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientfd = accept(Socketfd, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientfd == -1) { perror("accept"); return 1; }
        bool once=true;
        char* ext=nullptr;
        bool headerSent=false;
    
       
     
        //=============================================
        //  RECIEVE DATA
        //============================================

        string Response="\0";
    
        int status=0;
       
        cout<<"WAITING FOR BROWSER RESPONSE\n";
        while(status=recv(clientfd,buffer,bufferSize,0))
        {
            Response.append(buffer,status);
            if(status==-1){cerr<<"Error Encountered while reading request\n";exit(-6);}
            if(FindSubstring(Response.c_str(),"\r\n\r\n")!=-1){break;};
        }
        
        cout<<"Browser Responded with : "<<Response;

        if(!once && !FilePath) delete[] FilePath;
        if(!once && !ext) delete[] ext;

        if(!parseHTTP(Response.c_str(),FilePath)){cerr<<"PARSE HTTP FAILED\n"; exit(-7);}
        else
        {
            if(FilePath != nullptr)
            {
                for(int i = 0; FilePath[i] != '\0'; i++)
                {
                    if(FilePath[i] == '?')
                    {
                        FilePath[i] = '\0'; // truncate string at '?'
                        break;
                    }
                }
            }
        }


        if (html != nullptr)
        {
            html->close();
            delete html;
            html = nullptr;
        }
        

        cout<<"file: "<<FilePath<<endl;
        html=openFile(FilePath);
        cout<<"\nFile opened\n";
        



        //=======================
        //  MISC
        //========================
        
        headerSent=false;
        if(!returnFileExtension(FilePath,ext)){cerr<<"RETURN FILE EXTENSION FAILED\n"; /*exit(-8);*/}
        long fileSize;

        if(html!=nullptr){
            html->clear();
            html->seekg(0, ios::end);
            fileSize = html->tellg();
            html->seekg(0, ios::beg);
        }


        //====================================
        // build and send the header
        //====================================

        if(html == nullptr)
        {
            string body =
                "<!DOCTYPE html>"
                "<html>"
                "<head><title>404 Not Found</title></head>"
                "<body><h1>File Not Found</h1></body>"
                "</html>";

            string header =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html; charset=UTF-8\r\n"
                "Content-Length: " + to_string(body.size()) + "\r\n"
                "Connection: close\r\n"
                "\r\n";

            send(clientfd, header.c_str(), header.size(), 0);
            send(clientfd, body.c_str(), body.size(), 0);

            headerSent = true;
        }


        if(ext!=nullptr && !headerSent)
        {
            string fileExtension=ext;

            string mimeType;
            if(fileExtension=="js") mimeType="text/javascript";
            else if(fileExtension=="css") mimeType="text/css";
            else if(fileExtension=="html" || fileExtension=="HTML") mimeType="text/html";
            else if(fileExtension=="json") mimeType="application/json";
            else if(fileExtension=="png") mimeType="image/png";
            else mimeType="text/"+fileExtension; //fallback for anything else

            string header = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: "+mimeType+"\r\n"
                            "Content-Length: " + to_string(fileSize) + "\r\n"
                            "Connection: close\r\n"
                            "\r\n";
            send(clientfd, header.c_str(), header.size(), 0);

            cout<<"HEADER SENT TO CLIENT: "<<header<<endl;
            headerSent=true;
        } 
         



        //======================================
        //  SEND DATA
        //======================================

        if(headerSent){
            while(true)
            {
                if(html==nullptr) break;
                html->read(buffer, bufferSize);
                std::streamsize bytesRead = html->gcount();
                if(bytesRead <= 0) break;

                int sentBytes = 0;
                while(sentBytes < bytesRead)
                {
                    int n = send(clientfd, buffer + sentBytes, bytesRead - sentBytes, 0);
                    if(n <= 0) { /* handle error / disconnect */ break; }
                    sentBytes += n;
                }

                if(bytesRead < bufferSize) break; // reached EOF
            }
            cout<<"DATA SENT TO BROWSER\n";
        }
        

        //==================
        // CLOES CONNECTION
        //==================
        close(clientfd);
    
       
    
    }
    delete html;
    freeaddrinfo(results);
    close(Socketfd);
    return 0;
}