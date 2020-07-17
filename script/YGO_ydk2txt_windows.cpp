#include <iostream>

#include <stdio.h>
#include <dirent.h>
#include <algorithm>
#include <fstream>
#include <vector>
#include <string>
#include <sstream> 
#include <regex>
#include <sys/stat.h>
#include <filesystem>
#include <stdlib.h>
#include <windows.h>

using namespace std;;

string getCD(){
    // obtain current path
    char Path[MAX_PATH+1]; 
    if(0!=GetModuleFileName( NULL, Path, MAX_PATH )){
        string str_tmp=regex_replace(Path, regex("\\.+$"), "");
        string str_tmp2=regex_replace(str_tmp, regex("/.+$"), "");
        return str_tmp2;
    }
    else{
        return "";
    }
}

vector<string> mk_ls(string dir_path="", string keyword="") {
    // like ls
    string path_pre = "./";
    vector<string> vec_ls={};
    struct dirent *dent;
    if (dir_path.size() > 0) {
        path_pre = dir_path;
    }
    filesystem::path path_tmp=path_pre;
    string path_tmp2=path_tmp.make_preferred().string();
    DIR *dir;
    try{
        dir = opendir(path_tmp2.c_str());
    }catch(exception){
        perror((path_tmp2.c_str()));
        return {};
        
    }

    string dir_name;
    while ((dent = readdir(dir)) != NULL) {
        dir_name=dent->d_name;
        if(dir_name.find(keyword)!=string::npos or keyword==""){
            vec_ls.push_back(dir_name);
        }
    }
    closedir(dir);
    return vec_ls;
}


string find_path(string path_cand="./", vector<string> judge_dirs={}) {
    // find path by judging some directories
    string ans_path="";
    vector<string> list_ls={};
    int count;
    for(int i=0;i<5;i++){
        list_ls=mk_ls(path_cand, "");

        count=0;
        for(int j=0;j<judge_dirs.size();j++){
            for(auto dir_ls: list_ls){
                if(dir_ls==judge_dirs[j]){
                    count++;
                }
            }
        }
        if(count==judge_dirs.size()) {
            ans_path=path_cand;
            break;
        }
        path_cand+="/../";
    }
    return ans_path;
}


vector<string> split2vector(string str){
    //split string to vector
    vector<string> v;
    string s;
    stringstream ss{str};
    while ( getline(ss, s) ){
        string r=regex_replace(s,regex("\\s+$"), "");
        // also support the form of CR+LF
        string r2=regex_replace(r,regex("\\r"), "");
        v.push_back(r2);
    }
    return v;
}

vector<vector<string>> read_ydk(string deck_dir_path, string ydk_name){
    int i, j, k;
    string ydk_path=deck_dir_path +ydk_name;
    
    // read .ydk
    string deck_content_tmp;
    filesystem::path path_tmp=ydk_path;
    ifstream ifs(path_tmp.make_preferred());
    if(!ifs){
        cout<< "can't open "+ydk_name <<"\n";
        return {{""}};
    }

    deck_content_tmp=string(istreambuf_iterator<char>(ifs),
        istreambuf_iterator<char>());
    ifs.close();

    // string to vector
    vector<string> deck_content=split2vector(deck_content_tmp);

    // obtain card indexes
    vector<int> check_indexes={};
    vector<string> check_keys={"#main", "#extra", "!side"};
    for(auto check_key: check_keys){
        auto found=find(deck_content.begin(), deck_content.end(), check_key);
        int ind=found-deck_content.begin();
        check_indexes.push_back(ind);
    }
    if(check_indexes.size()!=check_keys.size()){
        cout<<"the ydk file is incompleted"<<endl;
    }
    check_indexes.push_back(deck_content.size());

    vector<vector<string>> cards_indexes_all(3);
    for(i=0;i<3;i++){
        auto ind1 = deck_content.begin()+check_indexes[i]+1;
        auto ind2 = deck_content.begin()+check_indexes[i+1];
        cards_indexes_all[i].resize(ind2-ind1+1);
        copy(ind1, ind2 ,cards_indexes_all[i].begin());
    }
    return cards_indexes_all;
}

vector<string> obtain_names_Jap(string ADS_path,vector<string>cards_indexes){
    int i,j,k;
    vector<string> lua_content;
    vector<string> cards_names_Jap;
    vector<string> lua_deltas={"/script/official/",
     "/repositories/delta/script/official/",
     "/repositories/delta/script/delta/pre-release/",
     "/repositories/delta/script/delta/pre-errata/"};
    
    for(auto cards_index: cards_indexes){
        // notice only components starting with numbers
        smatch match_tmp;
        if(regex_search(cards_index, match_tmp, regex("^\\D"))){
            continue;
        }
        int ind;
        try{
            ind=stoi(cards_index);
        } catch(exception){
            continue;
        }
        // make list of lua file candidates
        vector<string> lua_names;
        for(auto lua_delta: lua_deltas){
            lua_names.push_back(ADS_path+"/"+lua_delta+"/c"+to_string(ind)+".lua");
        }
        for(auto lua_delta: lua_deltas){
            lua_names.push_back(ADS_path+"/"+lua_delta+"/c"+to_string(ind-1)+".lua");
        }

        string lua_tmp="----CAN'T FIND c"+cards_index+".lua---";
        
        for(auto lua_name: lua_names){ 
            filesystem::path path_tmp= lua_name;           
            ifstream ifs(path_tmp.make_preferred());
            if(!ifs){
                continue;
            }
            string lua_content_tmp=string(istreambuf_iterator<char>(ifs),
            istreambuf_iterator<char>());
            ifs.close();
            lua_content=split2vector(lua_content_tmp);
            lua_tmp=lua_content[0];
            break;            
        }
        // erase prefixes("--")
        cards_names_Jap.push_back(lua_tmp.substr(2, lua_tmp.length() -2));
    }
    return cards_names_Jap;
}

int main(){
    int i,j,k;
    string orig_path=getCD();

    // obtain ADS path and this directory's path
    vector<vector<string>> judges_dirs_s={{"deck", "expansions", "replay"},
     {"deck", "output"}};
    string ADS_path=find_path(orig_path, judges_dirs_s[0]);
    string dir_path=find_path(orig_path, judges_dirs_s[1]);

    // check if directories exist, and if not make them
    string deck_dir_path= dir_path+"/deck/";
    if(! mkdir(deck_dir_path.c_str())){
        cout<<"directory deck/ doesn't exist."<<endl;
        return 1;
    }
    string output_dir_path=dir_path+"/output/";
    if(! mkdir(output_dir_path.c_str())){
        cout<<"directory output/ was made"<<endl;
    }

    // obtain ydk names
    vector<string> ydk_names=mk_ls(deck_dir_path, ".ydk");
    if (ydk_names.size()==0){
        printf("can't find .ydk in /deck\n");
        return 1;
    }
    vector<string> check_keys = {"#main", "#extra", "#side"};
    for(auto ydk_name: ydk_names){
        cout<<"working on "+ydk_name <<endl;
        // read ydk file and obtain Japanese names
        vector<vector<string>> cards_indexes_all=read_ydk(deck_dir_path, ydk_name);

        // summarize Japanese names into string
        vector<vector<string>> cards_names_Japs;
        string sentences="";
        for(j=0;j<3;j++){
            cards_names_Japs.push_back(obtain_names_Jap(ADS_path, cards_indexes_all[j]));
            sentences += check_keys[j]+"\n";
            for(auto cards_name_Jap:cards_names_Japs[j]){
                sentences+=cards_name_Jap+"\n";
            }
            sentences+="\n";
        }
        // output Japanese names
        string txt_name_tmp=output_dir_path+ydk_name;
        string txt_name=regex_replace(txt_name_tmp, regex("\\.ydk$"), ".txt");
        filesystem::path path_tmp=txt_name;
        ofstream ofs(path_tmp.make_preferred());
        if(!ofs){
            cout<< "can't write to "+path_tmp.make_preferred().string() <<endl;
            continue;
        }
        ofs<<sentences<<endl;
        ofs.close();
    }
    return 0;
}
