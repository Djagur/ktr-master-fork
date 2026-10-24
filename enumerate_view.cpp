#include <map>
#include <set>
#include <regex>
#include <string>
#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <unordered_map>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
using namespace rapidjson;

//文字列が整数かどうか
bool isNumber(const std::string& str){
	for (char const &c : str) {
 		if (std::isdigit(c) == 0) return false;
  }
  return true;
}

//ファイルパスを取得
std::map<std::string, std::string> FilePath(std::string filename, std::string dataset){
	std::string org_dataset, datasize, querydir;
	std::map<std::string, std::string> filepath;
	Document doc;
	std::map<std::string, std::string>::iterator iter, end;

	//JSONファイル読み込み
	std::ifstream ifs(filename);
	IStreamWrapper isw(ifs);
	doc.ParseStream(isw);
	ifs.close();

	if (dataset.find(".")==dataset.npos){
		org_dataset = dataset;
		datasize = "";
	}
	else{
		org_dataset = dataset.substr(0, dataset.find("_"));
		datasize = dataset.substr(dataset.find("_")+1) + "/";
	}
	
	filepath["EnumerateDir"] = doc["EnumerateDir"].GetString() + org_dataset + "/";
	filepath["DatasetDir"] = doc["DatasetDir"].GetString() + org_dataset + "/";
	filepath["EntityEnumerate"] = filepath["DatasetDir"] + datasize + org_dataset + "_entity.json";
	filepath["EntityLabelEnumerate"] = filepath["EnumerateDir"] + datasize + doc["EntityEnumerateFile"].GetString();
	filepath["LinkEnumerate"] = filepath["EnumerateDir"] + datasize + doc["LinkEnumerateFile"].GetString();
	filepath["CombinationEnumerate"] = filepath["EnumerateDir"] + datasize + doc["CombinationEnumerateFile"].GetString();

	//for (iter=filepath.begin(), end=filepath.end(); iter!=end; ++iter) std::cout << iter->first << ": " << iter->second << std::endl;
	return filepath;
}

//エンティティ情報を出力
//PrintDetail=trueでそのエンティティが持つエンティティラベルも表示
void PrintEntityInfo(Document& doc_entity, Document& doc_entitylabel, int idx, bool PrintDetail){
	std::cout << idx+1 << ": " << doc_entity[idx]["Name"].GetString() << std::endl;
	if (PrintDetail){
		std::cout << "Entity Label: " << std::endl;
		for (int j=0, size_j=doc_entity[idx]["Label"].Size(); j<size_j; ++j){
			std::cout << "\t" << doc_entity[idx]["Label"][j].GetInt() << ": " << doc_entitylabel[doc_entity[idx]["Label"][j].GetInt()-1]["Name"].GetString() << std::endl;
		}
	}
}

//エンティティラベル情報を出力
//PrintDetail=trueでマッチするエンティティも表示
void PrintEntityLabelInfo(Document& doc, int idx, bool PrintDetail){
	std::cout << idx+1 << ": " << doc[idx]["Name"].GetString() << " (" << doc[idx]["MatchEntity"].Size() << ")" << std::endl;
	if (PrintDetail){
		std::cout << "Match Entity: " << std::endl;
		for (int j=0, size_j=doc[idx]["MatchEntity"].Size(); j<size_j; ++j){
			std::cout << "\t" << doc[idx]["MatchEntity"][j]["ID"].GetInt() << ": " << doc[idx]["MatchEntity"][j]["Name"].GetString() << std::endl;
		}
	}
}

//リンク情報を出力
void PrintLinkInfo(Document& doc, int idx){
	std::cout << idx+1 << ": " << doc[idx]["Name"].GetString() << std::endl;
}

//隣接情報を出力
void PrintCmbInfo(Document& doc, int idx, std::string label_name){
	rapidjson::Value::ConstMemberIterator iter_doc, end_doc;
	std::cout << idx+1 << ": " << label_name << std::endl;
	std::cout << "Combination: " << std::endl;
	std::cout << doc.Size();
	for (iter_doc=doc[idx]["Combination"].MemberBegin(), end_doc=doc[idx]["Combination"].MemberEnd(); iter_doc!=end_doc; ++iter_doc){
		std::cout << "\t" << iter_doc->name.GetString() << std::endl;
		for (int j=0, size_j=iter_doc->value.Size(); j<size_j; ++j){
			std::cout << "\t\t" << iter_doc->value[j]["ID"].GetInt() << ": " << iter_doc->value[j]["Name"].GetString() << std::endl;
		}
	}
}

//データの内容を表示
void ShowData(std::map<std::string, std::string>& filepath){
	int query_id;
	std::string doc_kind, query, searching_name;
	Document doc_entity, doc_entitylabel, doc_link, doc_cmb;

	std::ifstream ifs_entity(filepath["EntityEnumerate"]);
	IStreamWrapper isw_entity(ifs_entity);
	doc_entity.ParseStream(isw_entity);
	ifs_entity.close();
	std::ifstream ifs_entitylabel(filepath["EntityLabelEnumerate"]);
	IStreamWrapper isw_entitylabel(ifs_entitylabel);
	doc_entitylabel.ParseStream(isw_entitylabel);
	ifs_entitylabel.close();
	std::ifstream ifs_link(filepath["LinkEnumerate"]);
	IStreamWrapper isw_link(ifs_link);
	doc_link.ParseStream(isw_link);
	ifs_link.close();
	std::ifstream ifs_combination(filepath["CombinationEnumerate"]);
	IStreamWrapper isw_combination(ifs_combination);
	doc_cmb.ParseStream(isw_combination);
	ifs_combination.close();
	
	while (true){
		std::cout << "e <ID> で，IDが<ID>のエンティティの名前とそのエンティティラベルを出力" << std::endl;
		std::cout << "e <エンティティ名> で，<エンティティ名>を名前に含むエンティティのIDとそのエンティティラベルを出力" << std::endl;
		std::cout << "el <ID> で，IDが<ID>のエンティティラベルの名前とそれにマッチするエンティティを出力" << std::endl;
		std::cout << "el 0 で，全てのエンティティラベルIDとエンティティラベル名を出力" << std::endl;
		std::cout << "el <ラベル名> で，<ラベル名>を名前に含むエンティティラベルのIDを出力" << std::endl;
		std::cout << "r <ID> で，IDが<ID>のリンクの名前を出力" << std::endl;
		std::cout << "r <リンク名> で，<リンク名>を名前に含むリンクのIDを出力" << std::endl;
		std::cout << "c <ID> で，IDが<ID>のエンティティラベルの名前とそれにマッチするエンティティの隣接情報を出力" << std::endl;
		std::cout << "Input query >> ";
		std::cin >> doc_kind >> query; //読み込むJSONとクエリを入力
		if (doc_kind=="quit") return; //"quit"なら終了
		std::regex re(query, std::regex_constants::icase);
		std::smatch m;

		//e [ID]で，doc_enityから，IDが[ID]のエンティティの名前とそのエンティティラベルを出力
		//e [エンティティ名] で，doc_entityから，[エンティティ名]を名前に含むエンティティのIDとそのエンティティラベルを出力
		if (doc_kind == "e"){ //エンティティ情報を表示
			if (isNumber(query)){ //IDを指定していたら（=整数ならば）
				query_id = atoi(query.c_str());
				if (query_id>doc_entity.Size() || query_id<1) std::cout << "Nonexistent ID" << std::endl; //指定のIDが範囲外であれば
				else PrintEntityInfo(doc_entity, doc_entitylabel, query_id-1, true); //範囲内であれば
			}
			else { //エンティティ名を指定していたら
				for (int i=0, size_i=doc_entity.Size(); i<size_i; ++i){
					searching_name = doc_entity[i]["Name"].GetString();
					if (std::regex_search(searching_name, m, re)) PrintEntityInfo(doc_entity, doc_entitylabel, i, false);
				}
			}
		}

		//el 0で，doc_enityから，全てのエンティティラベルIDとエンティティラベル名を出力
		//el [ID]で，doc_enityから，IDが[ID]のエンティティラベルの名前とそれにマッチするエンティティを出力
		//el [ラベル名]で，doc_enityから，[ラベル名]を名前に含むエンティティラベルのIDを出力
		else if (doc_kind == "el"){ //エンティティラベル情報を表示
			if (isNumber(query)){ //IDを指定していたら（=整数ならば）
				query_id = atoi(query.c_str());
				if (query_id==0){ //全指定
					for (int i=0, size_i=doc_entitylabel.Size(); i<size_i; ++i) PrintEntityLabelInfo(doc_entitylabel, i, false);
				}
				else if (query_id>doc_entitylabel.Size() || query_id<0) std::cout << "Nonexistent ID" << std::endl; //指定のIDが範囲外であれば
				else PrintEntityLabelInfo(doc_entitylabel, query_id-1, true); //範囲内であれば
			}
			else { //エンティティ名を指定していたら
				for (int i=0, size_i=doc_entitylabel.Size(); i<size_i; ++i){
					searching_name = doc_entitylabel[i]["Name"].GetString();
					if (std::regex_search(searching_name, m, re)) PrintEntityLabelInfo(doc_entitylabel, i, false);
				}
			}
		}

		//r [ID]で，doc_linkから，IDが[ID]のリンクの名前を出力
		//r [リンク名]で，doc_linkから，[リンク名]を名前に含むリンクのIDを出力
		else if (doc_kind == "r"){ //リンク情報を表示
			if (isNumber(query)){ //IDを指定していたら（=整数ならば）
				query_id = atoi(query.c_str());
				if (query_id>doc_link.Size() || query_id<1) std::cout << "Nonexistent ID" << std::endl; //指定のIDが範囲外であれば
				else PrintLinkInfo(doc_link, query_id-1); //範囲内であれば
			}
			else { //エンティティ名を指定していたら
				for (int i=0, size_i=doc_link.Size(); i<size_i; ++i){
					searching_name = doc_link[i]["Name"].GetString();
					if (std::regex_search(searching_name, m, re)) PrintLinkInfo(doc_link, i);
				}
			}
		}

		//c [ID]で，doc_cmbから，IDが[ID]のエンティティラベルの名前とそれにマッチするエンティティの隣接情報を出力
		else if (doc_kind == "c"){ //隣接情報を表示
			if (isNumber(query)){ //IDを指定していたら（=整数ならば）
				query_id = atoi(query.c_str());
				if (query_id>doc_entitylabel.Size() || query_id<1) std::cout << "Nonexistent ID" << std::endl; //指定のIDが範囲外であれば
				else PrintCmbInfo(doc_cmb, query_id-1, doc_entitylabel[query_id-1]["Name"].GetString()); //範囲内であれば
			}
			else { //エンティティ名を指定していたら
				std::cout << "Wrong Query" << std::endl;
			}
		}

		else std::cout << "Wrong Query" << std::endl; //どれにも当てはまらなければ再入力を促す
		std::cout << std::endl;
	}
}

int main(int argc, char *argv[]){
	std::string filename = "config/outlier.json";
	std::string dataset = "";
    int opt;
    const char *optstring = "d:";
    while ((opt=getopt(argc, argv, optstring)) != -1){
        switch(opt){
            case 'd':
                dataset = optarg;
                break;
        }
    }
	std::map<std::string, std::string> filepath = FilePath(filename, dataset);
	ShowData(filepath);
	//DatasetToJson(filepath);
}
