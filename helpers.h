#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<cstring>
#include<fstream>
#include<iostream>
#include<unistd.h>
#include <arpa/inet.h>


char* root=nullptr; //i think keeping this as global is reasonable
std::string rootDir; 


bool areEqual(const char* str1,const char* str2);

void checkFilePath(int argc)
{
    if(argc<=1) 
    {
        std::cerr<<"No file Path given\n";
        exit(-1);
    }
}

int findLastIndex(const char* arr, char target='.') {
    
    int size=strlen(arr);

    for (int i = size - 1; i >= 0; --i) {
        if (arr[i] == target) {
            return i; 
        }
    }
    return -1; // Return -1 if the character is not in the array
}

bool returnFileExtension(const char* str,char *&ret)
{
    ret=nullptr;

    if(str==nullptr){std::cerr<<"[returnFileExtension]: STR was null\n"; return false;}
   
    if(areEqual("",str))
    {
        ret=new char[5];
        ret[0]='H';
        ret[1]='T';
        ret[2]='M';
        ret[3]='L';
        ret[4]='\0';
        return true;
    }

    int last;

    if((last=findLastIndex(str))==-1){std::cerr<<"[returnFileExtension]: NO '.' found in PATH given \n Path was: "<<str<<"\n"; return false;}
    else 
    {
        int size=strlen(str)+1;//account for nullterminator    
        ret = new char[size-last];
        int i=0;
        last++;
        while(last!=size)
        {
            ret[i++]=str[last++];
        }
    }

    return true;
}

void getaddrinfoError(int status)
{
    if(status)
    {
        std::cerr<<gai_strerror(status)<<"\n";
        exit(0);
    }
    
}



std::fstream* openFile(char* FilePath,bool openRoot=false)
{


    //====================================
    // If user requests root then skip
    // all chekc for filepath and just 
    // returned a fstream ptr pointing to
    // root passed as argv[1] to main
    //====================================



    if(openRoot)
    {
        if(root==nullptr){std::cerr<<"Root contained nullptr\n"; exit(-2);}
        else 
        {
            std::fstream* file=new std::fstream;
            file->open(root);
            if(!file->is_open()){std::cerr<<"Root Failed to open, check path: "<<root<<std::endl;exit(-2);}
            else return file;
        }

    }

    

    if(FilePath==nullptr) {std::cerr<<"FilePath contained nullptr\n"; exit(-2);}
    
    std::fstream* file=new std::fstream;
    if(areEqual("",FilePath)) file->open(root);
   else file->open(rootDir + "/" + FilePath);
    if(!(file->is_open()))
     {
        std::cerr<<"Error opening File\nPath given to function: "
               <<FilePath<<std::endl
                <<"\nPath after building with rootDIR: "
                <<rootDir+"/"+FilePath<<std::endl;
                
                
                exit(-3);
                
    }
    else return file;
}


bool areEqual(const char* str1,const char* str2)
{
    if(!str1 || !str2) return false;
    if(strlen(str1)!=strlen(str2)) return false;

    for(int i=0;str1[i]!='\0';i++)
    {
        if(str1[i]!=str2[i]) return false;
    
    }

    return true;


}


int FindSubstring(const char* str, const char* sub)
{
    if (!str || !sub)
        return -1;

    if (*sub == '\0')   // Empty substring
        return 0;

    for (int i = 0; str[i] != '\0'; i++)
    {
        int j = 0;

        while (sub[j] != '\0' && str[i + j] == sub[j])
            j++;

        if (sub[j] == '\0')
            return i;   // Found at index i
    }

    return -1;          // Not found
}

bool parseHTTP(const char* Response,char*& ret)
{

    ret=nullptr;
    int ResPos=FindSubstring(Response,"GET /");
    int HTTPpos=FindSubstring(Response," HTTP");
    if(ResPos!=-1)
        ResPos+=5; //skip GET 
    else{
        ResPos= FindSubstring(Response,"HEAD /");
        if(ResPos==-1){return false;}
        else ResPos+=6;
    }
    if(HTTPpos<ResPos){return false;}
    else 
    {
        char *buffer = new char[HTTPpos-ResPos+1];
        int i=0;
        while(ResPos!=HTTPpos)
        {
            buffer[i++]=Response[ResPos++];
            
        }
        buffer[i]='\0';
        ret=buffer;
        return true;
    }

    //std::cout<<"Get Found at index "<<ResPos<<std::endl;
    
    if(ResPos == -1){ std::cout<<"No Get found in substring\n";return false; } 
    int start=ResPos+3;

    bool newlineFound=Response[ResPos+1]=='\n' && Response[ResPos] == '\r';
     
    int size = strlen(Response);
    while(!newlineFound)
    {
        ResPos++;
        if(ResPos>size) {std::cout<<"no \\r \\n found \n"<<ResPos<<std::endl; return false;}
        newlineFound=Response[ResPos+1]=='\n' && Response[ResPos] == '\r';
    } //this is horrible dumb code that wont make sense the next time i look at it
      //after this loop respos points to the first '\r' in the string 
    
    //  std::cout<<ResPos<<std::endl;  
      if(ResPos<start){std::cout<<"ResPos is somehow less than start T-T\n"<<ResPos<<std::endl;return false;}

    int i=0;
    char * buffer=new char[(ResPos-start)+1];
    while (start!=ResPos)
    {
        buffer[i++]=Response[start++];
    }
    buffer[i]='\0';
    ret=buffer;
    return true;

}