#include <map>
#include <set>
#include <omp.h>
#include <cmath>
#include <tuple>
#include <queue>
#include <string>
#include <chrono>
#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
using namespace rapidjson;

class entity;
class expscore;
class patternentity;
class pattern;
class triple;
class graph;

std::chrono::system_clock::time_point start_read, end_read, start_detect, end_detect;
std::chrono::system_clock::time_point start, end;
std::map<std::string, std::map<std::string, double> > ProcessTime;
std::vector<std::string> QueryVector;
double time_div = 1000.0;

double weight = 1; //無接続の隣接要素のカウント数の重み
int WildCardNum = 0; //ワイルドカードとなる値
int EntityNum, LabelNum, LinkNum;
std::string NowQuery;

std::map<std::string, std::vector<int> > SearchedPatternNum;
std::map<std::string, std::vector<int> > ExistMatchPatternNum;

//現在時刻を表示
void PrintNow(){
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::cout << "Now = " << std::ctime(&now_c);
}

//前回呼び出し時からの経過時間を計測
//queryには実行中のクエリ，process_nameには直前に行なっていた計測対象の処理名を入力
//process_no=""の場合は前回呼出時刻更新のみ行う（プログラム開始時または入力処理の場合）
void TimeTime(std::string process_name){
	if (process_name != ""){ //経過時間を計測
		end = std::chrono::system_clock::now();
		double msec = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
		if (ProcessTime.find(NowQuery)==ProcessTime.end()) QueryVector.push_back(NowQuery);
		if (ProcessTime[NowQuery].find(process_name)==ProcessTime[NowQuery].end()) ProcessTime[NowQuery][process_name] = 0.0;
		ProcessTime[NowQuery][process_name] += msec / time_div;
	}
	start = std::chrono::system_clock::now();
}

//処理時間を表示
void PrintProcessTime(){
	double SumTime = 0.0;
	std::string SearchQuery;
	std::map<std::string, double>::iterator iter_sd, end_sd;
	
	std::cout << "Process Time:" << std::endl << std::endl;
	for (int i=0, size_i=QueryVector.size(); i<size_i; ++i){
		SearchQuery = QueryVector[i];
		std::cout << "<" << SearchQuery << ">" << std::endl;
		for (iter_sd=ProcessTime[SearchQuery].begin(), end_sd=ProcessTime[SearchQuery].end(); iter_sd!=end_sd; ++iter_sd){
			std::cout << "\t" << iter_sd->first << ": " << iter_sd->second << std::endl;
			SumTime += iter_sd->second;
		}
		std::cout << "\tSumTime: " << SumTime << std::endl << std::endl;
		SumTime = 0.0;
	}
}

//探索したパターン数を表示
void PrintSearchedPatternNum(){
	std::string SearchQuery;
	std::cout << "Searched Pattern Num:" << std::endl << std::endl;
	for (int i=1, size_i=QueryVector.size(); i<size_i; ++i){
		SearchQuery = QueryVector[i];
		std::cout << "<" << SearchQuery << ">" << std::endl;
		for (int i=0, size_i=SearchedPatternNum[SearchQuery].size(); i<size_i; ++i) std::cout << "\t" << i << ": " << SearchedPatternNum[SearchQuery][i] << std::endl;
	}
	std::cout << std::endl;
}

////マッチするエンティティが存在するパターン数を表示
void PrintExistMatchPatternNum(){
	std::string SearchQuery;
	std::map<std::string, std::vector<int> >::iterator iter, end;
	std::cout << "Pattern which has matched entities Num:" << std::endl << std::endl;
	for (int i=1, size_i=QueryVector.size(); i<size_i; ++i){
		SearchQuery = QueryVector[i];
		std::cout << "<" << SearchQuery << ">" << std::endl;
		for (int i=0, size_i=ExistMatchPatternNum[SearchQuery].size(); i<size_i; ++i) std::cout << "\t" << i << ": " << ExistMatchPatternNum[SearchQuery][i] << std::endl;
	}
	std::cout << std::endl;
}

//ラベルIDをリンク名に変換
std::string LabelEncoder(int label_id, std::vector<std::string>& LabelList){
	if (label_id>-1 and label_id<LabelList.size()) return LabelList[label_id];
	else return "*";
}

//リンクIDをリンク名に変換
std::string LinkEncoder(int link_id, std::vector<std::string>& LinkList){
	if (link_id>0 and link_id<LinkList.size()) return LinkList[link_id] + "->";
	else if (link_id<0 and link_id>-LinkList.size()) return LinkList[-link_id] + "<-";
	else if (link_id==0) return "_NoLink_";
	else return "*";
}

//指定なしのエンティティかどうか
bool IsWildCardEntity(int entity_id, bool IsLabel){
	return (entity_id>LabelNum or entity_id< -LabelNum) and IsLabel or (entity_id>EntityNum or entity_id< -EntityNum) and !IsLabel;
}


//指定なしのリンクかどうか
bool IsWildCardLink(int link_id){
	return link_id > LinkNum or link_id< -LinkNum;
}

//taskの処理コストを基にGreedy法で割り当て
std::vector<std::vector<int> > GreedyAlloc(std::vector<int>& task, int ThreadNum){
	int min_thread, min_sumtime;
	std::vector<std::vector<int> > thread_alloc(ThreadNum); //返すvector(行でスレッド，列でタスク番号を指定)
	std::vector<int> thread_sumtime(ThreadNum); //各スレッドの処理合計時間
	std::map<int, std::vector<int> > task_map;  //taskを処理時間でソートしたもの(key:処理時間，value=タスク番号集合)
	std::map<int, std::vector<int> >::reverse_iterator riter_ivi, rend_ivi;

	//初期化
	for (int i=0; i<ThreadNum; ++i) thread_sumtime[i] = 0;
	for (int i=0, size_i=task.size(); i<size_i; ++i) task_map[task[i]].push_back(i);

	//処理時間の多いタスクから，一番処理合計時間が少ないスレッドに割り当て
	for (riter_ivi=task_map.rbegin(), rend_ivi=task_map.rend(); riter_ivi!=rend_ivi; ++riter_ivi){
		for (int i=0, size_i=riter_ivi->second.size(); i<size_i; ++i){
			min_thread = 0; //処理合計時間最小のスレッド(合計時間同じの場合は最小のスレッド番号)
			min_sumtime = thread_sumtime[0]; //最小の処理合計時間
			for (int i=1; i<ThreadNum; ++i){
				if (thread_sumtime[i] < min_sumtime){
					min_thread = i;
					min_sumtime = thread_sumtime[i];
				}
			}
			thread_alloc[min_thread].push_back(riter_ivi->second[i]); //min_threadのスレッドに走査中のタスクを追加
			thread_sumtime[min_thread] += riter_ivi->first; //min_threadのスレッドの処理時間を加算
		}
	}
	return thread_alloc;
}

//ファイル名に関するパラメータを読み込み
std::map<std::string, std::string> FilePath(std::string filename, std::string queries_json_name, std::string dataset){
	std::string EntityFileName, LinkFileName, org_dataset, datasize, querydir, indexdir;
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
	//データセットファイル
	filepath["DatasetEntity"] = doc["DatasetDir"].GetString() + org_dataset + "/" + datasize + org_dataset + "_entity.json";
	filepath["DatasetLabel"] = doc["DatasetDir"].GetString() + org_dataset + "/" + datasize + org_dataset + "_label.json";
	filepath["DatasetLink"] = doc["DatasetDir"].GetString() + org_dataset + "/" + datasize  + org_dataset + "_link.json";

	//クエリファイル
	filepath["QueryDir"] = doc["QueryDir"].GetString() + org_dataset + "/";
	filepath["QueriesFile"] = filepath["QueryDir"] + queries_json_name + ".json";
	filepath["TargetFileName"] = doc["TargetFile"].GetString();
	filepath["ContextFileName"] = doc["ContextFile"].GetString();
	
	//一覧ファイル
	filepath["EnumerateDir"] = doc["EnumerateDir"].GetString() + org_dataset + "/";
	filepath["EntityEnumerate"] = filepath["EnumerateDir"] + datasize + doc["EntityEnumerateFile"].GetString();
	filepath["LinkEnumerate"] = filepath["EnumerateDir"] + datasize + doc["LinkEnumerateFile"].GetString();
	filepath["CombinationEnumerate"] = filepath["EnumerateDir"] + datasize + doc["CombinationEnumerateFile"].GetString();
	
	//インデックスファイル
	filepath["IndexDir"] = doc["IndexDir"].GetString() + org_dataset + "/" + datasize;
	filepath["IndexFileName"] = filepath["IndexDir"] + doc["IndexFile"].GetString();

	//for (iter=filepath.begin(), end=filepath.end(); iter!=end; ++iter) std::cout << iter->first << ": " << iter->second << std::endl;
	return filepath;
}

class entity{
	private:
		int ID; //エンティティID
		std::string Name; //エンティティ名
		std::unordered_set<int> Label; //エンティティラベルのID
		std::unordered_map<int, std::vector<int> > Relation; //リンク情報
	public:
		int GetId(){return ID;}; //IDを返す
		std::string& GetName(){return Name;} //Nameを返す
		std::unordered_set<int>& GetLabel(){return Label;}; //Labelを返す
		std::unordered_map<int, std::vector<int> >& GetRelation(){return Relation;}; //Relationを返す
		bool empty(); //空かどうか
		void ModifyEntity(int, std::string, std::unordered_set<int>); //entityを変更
		void AddLink(int, int); //リンクを追加
		void PrintEntity(std::ostream&, bool, bool, std::vector<entity>&, std::vector<std::string>&, std::vector<std::string>&); //エンティティを表示
		entity(){ID = 0;};
		~entity(){Name.clear(); Label.clear(); Relation.clear();};
};

//空かどうか
bool entity::empty(){
	return ID == 0;
}

//entityを変更
void entity::ModifyEntity(int id, std::string name, std::unordered_set<int> label){
	ID = id;
	Name = name;
	Label = label;
}

//リンクを追加
void entity::AddLink(int link_id, int ee_id){
	Relation[link_id].push_back(ee_id);
}

//エンティティを表示(PrintLink：隣接エンティティを表示するかどうか)
void entity::PrintEntity(std::ostream& os, bool PrintLink, bool PrintName, std::vector<entity>& EntityList, std::vector<std::string>& LabelList, std::vector<std::string>& LinkList){
	std::unordered_set<int>::iterator iter_l, end_l;
	std::unordered_map<int, std::vector<int> >::iterator iter_rlt, end_rlt;

	if (!empty()){
		if (ID>-1 and ID<EntityList.size()) os << ID << ". " << Name << "("; //IDとエンティティ名
		else os << "*" << "("; //IDとエンティティ名
		if (PrintName) for (iter_l=Label.begin(), end_l=Label.end(); iter_l!=end_l; ++iter_l) os << LabelEncoder(*iter_l, LabelList) << " "; //ラベル
		else for (iter_l=Label.begin(), end_l=Label.end(); iter_l!=end_l; ++iter_l) os << *iter_l << " ";
	}
	os << ")" << std::endl;

	if (PrintLink and !Relation.empty()){
		for (iter_rlt=Relation.begin(), end_rlt=Relation.end(); iter_rlt!=end_rlt; ++iter_rlt) { //各リンク
			if (PrintName) os << "\t" << LinkEncoder(iter_rlt->first, LinkList) << ":";
			else os << "\t" << iter_rlt->first << ":";
			for (int j=0, size_j=iter_rlt->second.size(); j<size_j; ++j) os << iter_rlt->second[j] << ", ";
			os << std::endl;
		}
	}
}

//エンティティIDをリンク名に変換
std::string EntityEncoder(int entity_id, std::vector<entity>& EntityList){
	if (entity_id>-1 and entity_id<EntityList.size()) return EntityList[entity_id].GetName();
	else return "*";
}


//トリプルクラス（Exprankingのkeyとして使用）
class triple{
	public:
		int ID_se; //始点エンティティID
		int ID_link; //リンクラベルID
		int ID_ee; //終点エンティティID
		void SetTriple(int se_id, int link_id, int ee_id){ID_se=se_id; ID_link=link_id; ID_ee=ee_id;};
		bool operator<(const triple& t) const{return ID_se<t.ID_se or ID_se==t.ID_se and (ID_link<t.ID_link or ID_link==t.ID_link and ID_ee<t.ID_ee);}
		triple(){};
		triple(int se_id, int link_id, int ee_id){ID_se=se_id; ID_link=link_id; ID_ee=ee_id;};
		~triple(){};
};


//パターン中のエンティティクラス
class patternentity{
	private:
		int ID; //ID(エンティティID，ラベルIDどちらも可)
		bool IsLabel; //そのエンティティがラベルかどうか
		int ID_pe; //PatternList中のインデックス値
		std::unordered_map<int, std::vector<int> > Relation; //リンク情報
		std::unordered_set<int> MatchEntity; //該当エンティティ
		std::unordered_map<int, std::unordered_set<int> > LinkClassifiedME; //隣接リンクごとに分類したマッチエンティティ
		std::unordered_map<int, std::unordered_map<int, int> > CombineLabelEntity; //結合するエンティティラベル（key:リンクラベルID，value:結合しているエンティティラベル集合）
		std::unordered_map<int, std::unordered_map<int, int> > CombineUniqueEntity; //結合するエンティティ（key:リンクラベルID，value:結合しているエンティティ集合）
		std::unordered_map<int, std::unordered_map<int, int> > CombineLink; //結合するリンク（key:終点エンティティラベル，value:結合しているリンク）
		std::vector<bool> Prunable; //枝刈り可能かどうか（順に，エンティティラベル，固有エンティティ，リンク，patternentityの枝刈り可能性）
		double UpperBoundScore; //patternentityの上界スコア(PatternUnitListでのみ使用)
	public:
		//メンバ返し・等号
			int GetId(){return ID;}; //IDを返す
			bool GetIsLabel(){return IsLabel;}; //IsLabelを返す
			int GetId_pe(){return ID_pe;}; //ID_peを返す
			std::unordered_map<int, std::vector<int> >& GetRelation(){return Relation;}; //Relationを返す
			std::unordered_set<int>& GetMatchEntity(){return MatchEntity;}; //MatchEntityを返す
			std::unordered_map<int, std::unordered_set<int> >& GetLinkClassifiedME(){return LinkClassifiedME;}; //LinkClassifiedMEを返す
			std::unordered_map<int, std::unordered_map<int, int> >& GetCombineLabelEntity(){return CombineLabelEntity;}; //CombineLabelEntityを返す
			std::unordered_map<int, std::unordered_map<int, int> >& GetCombineUniqueEntity(){return CombineUniqueEntity;}; //CombineUniqueEntityを返す
			std::unordered_map<int, std::unordered_map<int, int> >& GetCombineLink(){return CombineLink;}; //CombineLinkを返す
			std::vector<bool>& GetPrunable(){return Prunable;}; //Prunableを返す
			bool SamePatternentity(patternentity&); //同じpatternentityかどうか
			void CopyPrunable(patternentity&); //枝刈り状況をコピー
			patternentity& operator=(patternentity&); //等号演算
		//メンバ変更
			void ModifyEntity(int, int, bool); //パターン構造中のエンティティを変更
			void ModifyLink(int, int, int); //リンクラベルを変更
			void AddLink(int, int); //リンクを追加
			void ClearEntity(); //エンティティ情報をクリア（ワイルドカードにする）
			void ClearLink(int, int, bool); //リンク情報をクリア（ワイルドカードにする）
			void ModifyMatchEntity(std::unordered_set<int>&); //MatchEntityを変更
			void AddMatchEntity(int); //エンティティをMatchEntityに追加
			void AddLinkClassifiedME(int, int); //エンティティをLinkClassifiedMEに追加
			void ModifyCombineElement(std::unordered_map<int, std::unordered_map<int, int> >& combine, int Kind); //CombineXXをセット
			void AddCombineEntity(int, int, std::unordered_set<int>&); //結合しているエンティティとして追加
			void AddCombineLink(int, int); //結合しているリンクとして追加
			void ClearCombineElement(int); //CombineXXをクリア
			void SetUpperBoundScore(double); //UpperBoundScoreを変更
		//枝刈り関係
			double GetUpperBoundScore_kind_r_dist(std::map<double, int>&, double); //rでの例外の種類レベルでの上界スコアを計算（分布の偏りベース）
			double GetUpperBoundScore_kind_r_freq(std::map<double, int>&, double); //rでの例外の種類レベルでの上界スコアを計算（頻度ベース）
			double GetUpperBoundScore_kind(int, std::string&); //例外の種類レベルでの上界スコアを計算（kindは0から順に，エンティティラベル，固有エンティティ，リンク）
			double GetUpperBoundScore(std::string&); //UpperBoundScoreを返す
			void Pruning_kind(int, std::string&, double); //例外の種類レベルでの枝刈りが可能なら実行（kindは0から順に，エンティティラベル，固有エンティティ，リンク）
			void Pruning(std::string&, double); //パターンエンティティレベルでの枝刈りが可能なら実行
		//表示
			void PrintPatternentity(std::ostream&, bool, bool, bool, bool, bool, bool, std::vector<entity>&, std::vector<std::string>&, std::vector<std::string>&); //patternentityを表示
			void PrintPrunable(std::ostream&); //枝刈り状況を表示
		//その他
			patternentity(){Prunable.resize(4); for (int i=0; i<4; ++i) Prunable[i] = false; UpperBoundScore=0;};
			~patternentity(){Relation.clear(); MatchEntity.clear(); CombineLabelEntity.clear(); CombineUniqueEntity.clear(); CombineLink.clear(); Prunable.clear(); UpperBoundScore=0;};
};

//同じpatternentityかどうか
bool patternentity::SamePatternentity(patternentity& pe){
	return ID==pe.GetId() && (IsLabel && pe.GetIsLabel() || !IsLabel && !pe.GetIsLabel());
}

//枝刈り状況をコピー
void patternentity::CopyPrunable(patternentity& pe){
	Prunable = pe.GetPrunable();
}

//等号演算
patternentity& patternentity::operator=(patternentity& pe){
	ID_pe = pe.GetId_pe();
	ID = pe.GetId();
	IsLabel = pe.GetIsLabel();
	Relation = pe.GetRelation();
	return *this;
}

//エンティティを変更
void patternentity::ModifyEntity(int id_pe, int id, bool islabel){
	ID_pe = id_pe;
	ID = id;
	IsLabel = islabel;
}

//リンクラベルを変更
void patternentity::ModifyLink(int link_id, int epe_idx, int new_link_id){
	int idx;
	if (Relation[link_id].size()==1) Relation.erase(link_id); //Relation[link_id]が一つしかない場合，タプルを削除
	else{ //複数ある場合
		for (int i=0, size_i=Relation[link_id].size(); i<size_i; ++i){
			if (Relation[link_id][i]==epe_idx){
				idx = i;
				break;
			}
		}
		for (int i=idx, size_i=Relation[link_id].size()-1; i<size_i; ++i) Relation[link_id][i] = Relation[link_id][i+1];
		Relation[link_id].resize(Relation[link_id].size()-1);
	}
	Relation[new_link_id].push_back(epe_idx);
}

//リンクを追加
void patternentity::AddLink(int link_id, int pe_idx){
	Relation[link_id].push_back(pe_idx);
}

//エンティティ情報をクリア（ワイルドカードにする）
void patternentity::ClearEntity(){
	ModifyEntity(ID_pe, WildCardNum, true);
}

//リンク情報をクリア（ワイルドカードにする）
void patternentity::ClearLink(int link_id, int epe_idx, bool WildCardIsPosNum){
	if (WildCardIsPosNum) ModifyLink(link_id, epe_idx, WildCardNum);
	else ModifyLink(link_id, epe_idx, -WildCardNum);
}

//MatchEntityを変更
void patternentity::ModifyMatchEntity(std::unordered_set<int>& ME){
	MatchEntity = ME;
}

//エンティティをMatchEntityに追加
void patternentity::AddMatchEntity(int id){
	MatchEntity.insert(id);
}

//エンティティをLinkClassifiedMEに追加
void patternentity::AddLinkClassifiedME(int link_id, int entity_id){
	LinkClassifiedME[link_id].insert(entity_id);
}

//CombineXXを変更
void patternentity::ModifyCombineElement(std::unordered_map<int, std::unordered_map<int, int> >& combine, int Kind){
	if (Kind==0) CombineLabelEntity = combine;
	else if (Kind==1) CombineUniqueEntity = combine;
	else CombineLink = combine;
}

//結合しているエンティティとして追加
void patternentity::AddCombineEntity(int link_id, int ee_id, std::unordered_set<int>& label){
	std::unordered_set<int>::iterator iter, end;

	CombineUniqueEntity[link_id][ee_id]++;
	for (iter=label.begin(), end=label.end(); iter!=end; ++iter) CombineLabelEntity[link_id][*iter]++;
}

//結合しているリンクとして追加
void patternentity::AddCombineLink(int link_id, int label){
	CombineLink[label][link_id]++;
}

//CombineXXをクリア
void patternentity::ClearCombineElement(int kind){
	if (kind==0 or kind==3) CombineLabelEntity.clear();
	if (kind==1 or kind==3) CombineUniqueEntity.clear();
	if (kind==2 or kind==3) CombineLink.clear();
}

//UpperBoundScoreを変更
void patternentity::SetUpperBoundScore(double score){
	UpperBoundScore = score;
}

//rでの例外の種類レベルでの上界スコアを計算（分布の偏りベース）
double patternentity::GetUpperBoundScore_kind_r_dist(std::map<double, int>& CountAll, double score_old){
	double mincnt, sumsq, num, score_bef, score_aft;
	bool decrease = false;
	std::map<double, int>::reverse_iterator riter_di, rend_di;
	mincnt = (double)CountAll.begin()->first; //最小カウント数
	sumsq = 1.0; //二乗和
	num = 1.0; //二乗和を割る数
	score_bef = 1.0;

	for (riter_di=CountAll.rbegin(), rend_di=CountAll.rend(); riter_di!=rend_di; ++riter_di){ //カウント数の大きなものから順に走査
		for (int i=1, size=riter_di->second; i<=size; ++i){
			if (riter_di->first!=mincnt or i!=riter_di->second){ //最後の要素でない場合
				sumsq += std::pow(riter_di->first, 2);
				num++;
				score_aft = sumsq / num;
				if (score_bef >= score_aft){ //上界スコアが下がった場合，走査を終了
					decrease = true;
					break;
				}
				else score_bef = score_aft; //そうでない場合，score_befを更新
			}
		}
		if (decrease) break;
	}

	return std::max(score_bef, score_old); //古い方と新しい方の大きい方を返す
}

//rでの例外の種類レベルでの上界スコアを計算（頻度ベース）
double patternentity::GetUpperBoundScore_kind_r_freq(std::map<double, int>& CountAll, double score_old){
	double mincnt, sum;
	std::map<double, int>::reverse_iterator riter_di, rend_di;
	mincnt = (double)CountAll.begin()->first; //最小カウント数
	sum = 0.0; //1つの最小カウント数以外の和

	for (riter_di=CountAll.rbegin(), rend_di=CountAll.rend(); riter_di!=rend_di; ++riter_di){ //カウント数の大きなものから順に走査
		for (int i=1, size=riter_di->second; i<=size; ++i){
			if (riter_di->first!=mincnt or i!=riter_di->second) sum += riter_di->first; //最後の要素でない場合
			else sum += 1; //最後の要素の場合
		}
	}

	return 1 - 1/sum; //古い方と新しい方の大きい方を返す
}

//例外の種類レベルでの上界スコアを計算（kindは0から順に，エンティティラベル，固有エンティティ，リンク）
double patternentity::GetUpperBoundScore_kind(int exp_kind, std::string& score_kind){
	double count;
	double score = 0;
	std::map<double, int> CountAll; //隣接要素のカウント（key:カウント数，value:そのカウント数の数）
	std::unordered_map<int, std::unordered_map<int, int> >::iterator iter_i_ii, end_i_ii;
	std::unordered_map<int, int>::iterator iter_ii, end_ii;
	
	//カウント数を格納し，rでの例外の種類レベルでの上界スコアを計算
	if (exp_kind==0){ //エンティティラベルのとき
		for (iter_i_ii=CombineLabelEntity.begin(), end_i_ii=CombineLabelEntity.end(); iter_i_ii!=end_i_ii; ++iter_i_ii){
			for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(); iter_ii!=end_ii; ++iter_ii){
				if (iter_ii->first!=0) count = (double)iter_ii->second;
				else count = weight * (double)iter_ii->second;
				if (CountAll.find(count)==CountAll.end()) CountAll[count] = 1; //初めてのカウント数のとき
				else CountAll[count]++; //既出のとき
			}
			if (score_kind=="dist") score = GetUpperBoundScore_kind_r_dist(CountAll, score); //rでの上界スコアを計算
			else if (score_kind=="freq") score = GetUpperBoundScore_kind_r_freq(CountAll, score); 
			CountAll.clear();
		}
	}
	else if (exp_kind==1){ //固有エンティティのとき
		for (iter_i_ii=CombineUniqueEntity.begin(), end_i_ii=CombineUniqueEntity.end(); iter_i_ii!=end_i_ii; ++iter_i_ii){
			for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(); iter_ii!=end_ii; ++iter_ii){
				count = (double)iter_ii->second;
				if (CountAll.find(count)==CountAll.end()) CountAll[count] = 1; //初めてのカウント数のとき
				else CountAll[count]++; //既出のとき
			}
			if (score_kind=="dist") score = GetUpperBoundScore_kind_r_dist(CountAll, score); //rでの上界スコアを計算
			else if (score_kind=="freq") score = GetUpperBoundScore_kind_r_freq(CountAll, score); 
			CountAll.clear();
		}
	}
	else{ //リンクのとき
		for (iter_i_ii=CombineLink.begin(), end_i_ii=CombineLink.end(); iter_i_ii!=end_i_ii; ++iter_i_ii){
			for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(); iter_ii!=end_ii; ++iter_ii){
				if (iter_ii->first!=0) count = (double)iter_ii->second;
				else count = weight * (double)iter_ii->second;
				if (CountAll.find(count)==CountAll.end()) CountAll[count] = 1; //初めてのカウント数のとき
				else CountAll[count]++; //既出のとき
			}
			if (score_kind=="dist") score = GetUpperBoundScore_kind_r_dist(CountAll, score); //rでの上界スコアを計算
			else if (score_kind=="freq") score = GetUpperBoundScore_kind_r_freq(CountAll, score); //rでの上界スコアを計算
			CountAll.clear();
		}
	}

	return score;
}

//パターンエンティティレベルでの上界スコアを計算
double patternentity::GetUpperBoundScore(std::string& score_kind){
	if (IsLabel==false) return 0; //エンティティのときは0を返す
	else if (UpperBoundScore!=0) return UpperBoundScore; //上界スコア保存済みのときはそのスコアを返す
	else{ //初めての上界スコア計算のときは計算も行う
		double tmpscore, score;
		score = 0.0;
		for (int kind=0; kind<3; ++kind){
			tmpscore = GetUpperBoundScore_kind(kind, score_kind);
			if (tmpscore > score) score = tmpscore;
		}
		return score;
	}
}

//例外の種類レベルでの枝刈りが可能なら実行（kindは0から順に，エンティティラベル，固有エンティティ，リンク）
void patternentity::Pruning_kind(int exp_kind, std::string& score_kind, double minscore){
	if (minscore==-1){ //例外スコアランキングが空の場合はfalseを格納
		Prunable[exp_kind] = false;
		return;
	}
	if (!Prunable[exp_kind]){ //枝刈りが不可能であれば
		 if (GetUpperBoundScore_kind(exp_kind, score_kind)<minscore) Prunable[exp_kind] = true; //上界スコアがk番目より低い場合，枝刈り
	}
}

//パターンエンティティレベルでの枝刈りが可能なら実行
void patternentity::Pruning(std::string& score_kind, double minscore){
	if (Prunable[3]) return; //既に枝刈り可能なら実行せずに終了
		if (minscore==-1){ //例外スコアランキングが空の場合はfalseを格納
		Prunable[3] = false;
		return;
	}
	for (int kind=0; kind<3; ++kind) Pruning_kind(kind, score_kind, minscore); //例外の種類レベルでの枝刈りを実行
	Prunable[3] = Prunable[0] and Prunable[1] and Prunable[2]; //それらの論理積を格納
}

//patternentityを表示
void patternentity::PrintPatternentity(std::ostream& os, bool PrintName, bool PrintRelation, bool PrintMatchEntity, bool PrintLinkClassifiedME, bool PrintCombineElement, bool PrintExpScore, std::vector<entity>& EntityList, std::vector<std::string>& LabelList, std::vector<std::string>& LinkList){
	std::unordered_map<int, std::vector<int> >::iterator iter_r, end_r;
	std::unordered_map<int, std::unordered_map<int, int> >::iterator iter_cmb, end_cmb;
	std::unordered_set<int>::iterator iter_mt, end_mt;
	std::unordered_map<int, std::unordered_set<int> >::iterator iter_lcme, end_lcme;
	std::unordered_map<int, int>::iterator iter_ii, end_ii;
	std::map<triple, expscore>::iterator iter_es, end_es;
	std::unordered_set<int>::iterator iter_sei, end_sei;

	//patternentityの基本情報の表示
	if (PrintName){
		if (IsLabel) os << ID_pe << ":" << LabelEncoder(ID, LabelList) << "(";
		else os << ID_pe << ":" << EntityEncoder(ID, EntityList) << "(";
	}
	else os << ID_pe << ":" << ID << "(";
	if (IsLabel) os << "Label)" << std::endl;
	else os << "Entity)" << std::endl;

	//Relationの表示
	if (PrintRelation and !Relation.empty()){
		os << "[Relation]" << std::endl;
		for (iter_r=Relation.begin(), end_r=Relation.end(); iter_r!=end_r; ++iter_r) {
			if (PrintName) os << "\t" << LinkEncoder(iter_r->first, LinkList) << ":";
			else os << "\t" << iter_r->first << ":";
			for (int i=0, size=iter_r->second.size(); i<size; ++i) os << iter_r->second[i] << ", ";
			os << std::endl;
		}
	}

	//MatchEntityの表示
	if (PrintMatchEntity){
		os << "[MatchEntity]" << std::endl << "\t";
		if (PrintName){
			for (iter_sei=MatchEntity.begin(), end_sei=MatchEntity.end(); iter_sei!=end_sei; ++iter_sei) os << EntityEncoder(*iter_sei, EntityList) << ", ";
		}
		else{
			for (iter_sei=MatchEntity.begin(), end_sei=MatchEntity.end(); iter_sei!=end_sei; ++iter_sei) os << *iter_sei << ", ";
		}
		os << std::endl;
	}

	//LinkClassifiedMEの表示
	if (PrintLinkClassifiedME){
		os << "[LinkClassifiedME]" << std::endl;
		if (PrintName){
			for (iter_lcme=LinkClassifiedME.begin(), end_lcme=LinkClassifiedME.end(); iter_lcme!=end_lcme; ++iter_lcme){
				os << "\t" << LinkEncoder(iter_lcme->first, LinkList) << std::endl << "\t\t";
				for (iter_sei=iter_lcme->second.begin(), end_sei=iter_lcme->second.end(); iter_sei!=end_sei; ++iter_sei) os << EntityEncoder(*iter_sei, EntityList) << ", ";
				std::cout << std::endl;
			}
		}
		else {
			for (iter_lcme=LinkClassifiedME.begin(), end_lcme=LinkClassifiedME.end(); iter_lcme!=end_lcme; ++iter_lcme){
				os << "\t" << iter_lcme->first << std::endl << "\t\t";
				for (iter_sei=iter_lcme->second.begin(), end_sei=iter_lcme->second.end(); iter_sei!=end_sei; ++iter_sei) os << *iter_sei << ", ";
				std::cout << std::endl;
			}
		}
	}

	if (PrintCombineElement){
		//CombineLabelEntityの表示
		os << "[CombineLabelEntity]" << std::endl;
		for (iter_cmb=CombineLabelEntity.begin(), end_cmb=CombineLabelEntity.end(); iter_cmb!=end_cmb; ++iter_cmb) {
			if (PrintName) os << "\t" << LinkEncoder(iter_cmb->first, LinkList) << std::endl;
			else os << "\t" << iter_cmb->first << std::endl;
			for (iter_ii=iter_cmb->second.begin(), end_ii=iter_cmb->second.end(); iter_ii!=end_ii; ++iter_ii){
				if (PrintName){
					//if (iter_ii->first==0) os << "\t\t_NoEntity_:" << iter_ii->second << std::endl; else
					os << "\t\t" << LabelEncoder(iter_ii->first, LabelList) << ":" << iter_ii->second << std::endl;
				}
				else os << "\t\t" << iter_ii->first << ":" << iter_ii->second << std::endl;
			 }
		}

		//CombineUniqueEntityの表示
		os << "[CombineUniqueEntity]" << std::endl;
		for (iter_cmb=CombineUniqueEntity.begin(), end_cmb=CombineUniqueEntity.end(); iter_cmb!=end_cmb; ++iter_cmb) {
			if (PrintName) os << "\t" << LinkEncoder(iter_cmb->first, LinkList) << std::endl;
			else os << "\t" << iter_cmb->first << std::endl;
			for (iter_ii=iter_cmb->second.begin(), end_ii=iter_cmb->second.end(); iter_ii!=end_ii; ++iter_ii){
				if (PrintName){
					//if (iter_ii->first==0) os << "\t\t_NoEntity_:" << iter_ii->second << std::endl; else
					os << "\t\t" << EntityEncoder(iter_ii->first, EntityList) << ":" << iter_ii->second << std::endl;
				}
				else os << "\t\t" << iter_ii->first << ":" << iter_ii->second << std::endl;
			}
		}

		//CombineLabelEntityの表示
		os << "[CombineLink]" << std::endl;
		for (iter_cmb=CombineLink.begin(), end_cmb=CombineLink.end(); iter_cmb!=end_cmb; ++iter_cmb) {
			if (PrintName) os << "\t" << LabelEncoder(iter_cmb->first, LabelList) << std::endl;
			else os << "\t" << iter_cmb->first << std::endl;
			for (iter_ii=iter_cmb->second.begin(), end_ii=iter_cmb->second.end(); iter_ii!=end_ii; ++iter_ii){
				if (PrintName){
					os << "\t\t" << LinkEncoder(iter_ii->first, LinkList) << ":" << iter_ii->second << std::endl;
				}
				else os << "\t\t" << iter_ii->first << ":" << iter_ii->second << std::endl;
			}
		}
	}
}

//枝刈り状況を表示
void patternentity::PrintPrunable(std::ostream& os){
	if (Prunable[3]) os << "T(";
	else os << "F(";
	for (int kind=0; kind<3; ++kind){
		if (Prunable[kind]) os << "T";
		else os << "F";
		if (kind!=2) os << ",";
		else os << ")" << std::endl;
	}
}


//パターンクラス
class pattern{
	private:
		std::vector<patternentity> PatternentityList; //パターン中のエンティティのリスト
		std::unordered_set<int> Children; //子パターンのリスト（子パターンの第2インデックス値の集合）
		bool Prunable; //パターン構造レベルでの枝刈りが可能かどうか
		int WildCardNum; //ワイルドカードの数
		bool MatchingObjective; //マッチング対象のパターンであればtrue
		bool Searched; //すでに探索済みであれば（もう探索する必要がなければ）true
	public:
		//メンバ返し・コピー
			std::vector<patternentity>& GetPatternentityList(){return PatternentityList;}; //PatternentityListを返す
			std::unordered_set<int>& GetChildren(){return Children;}; //Childrenを返す
			bool GetPrunable(){return Prunable;}; //Prunableを返す
			int GetWildCardNum(){return WildCardNum;}; //WildCardNumを返す
			bool GetMatchingObjective(){return MatchingObjective;}; //MatchingObjectiveを返す
			bool GetSearched(){return Searched;}; //Searchedを返す
			int GetDeg(int); //ワイルドカードを除いた次数を返す
			void CopyMatchEntity(pattern&); //マッチするエンティティをコピー
			void CopyPrunable(pattern&); //枝刈り状況をコピー
			pattern& operator=(pattern&); //等号演算
		//各種要素の変更
			void AddChild(int); //子パターンを追加
			void DeleteChild(int); //子パターンを削除
			void SetWildCardNum(int num); //WildCardNumをセット
			void ChangeMatchingObjective(); //MatchingObjectiveをtrueに切り替え
			void ChangeSearched(); //Searchedをtrueに切り替え
		//patternentity構造の変更
			void ModifyEntity(int, int, bool); //patternentityを修正
			void ModifyLink(int, int, int, int); //リンクラベルを修正
			void AddEntity(int, bool); //パターンエンティティを追加
			void AddLink(int, int, int); //リンクを追加
			void ClearEntity(int); //エンティティ情報をクリア（ワイルドカードにする）
			void ClearLink(int, int, int); //リンク情報をクリア（ワイルドカードにする）
			void ModifyWildCardInTriple(int, int, int, patternentity&, int, patternentity&, bool, bool, bool); //トリプル中のワイルドカードを変更
			void AddTriple(int, int, patternentity&); //トリプルを追加
			void ChangeStructure(pattern&); //パターン構造を変更
		//patternentityのメンバの変更
			void ModifyMatchEntity(int pe_idx, std::unordered_set<int>& ME); //MatchEntityリストを変更
			void AddMatchEntity(int, int); //パターン構造に合致するエンティティを追加
			void AddLinkClassifiedME(int, int, int); //エンティティをLinkClassifiedMEに追加
			void ModifyCombineElement(int, std::unordered_map<int, std::unordered_map<int, int> >&, int); //CombineXXを変更
			void AddCombineEntity(int, int, int, std::unordered_set<int>&); //結合しているエンティティとして追加
			void AddCombineLink(int, int, int); //結合しているリンクとして追加
			void ClearCombineElement(int, int); //CombineXXをクリア
		//パターン構造に関する値
			bool ExistSameEntity(); //同じpatternentityが存在するかどうか
			std::unordered_map<int, int> GetCorrespondingIndex(pattern&); //入力パターンに対応するインデックス値を返す（map[i]でthisのi番目が入力パターンで何番目かを示す）
			std::string GetPatternID(); //パターン識別子を返す
		//枝刈り関係
			double GetUpperBoundScore(std::string&); //上界スコアを返す
			int GetAddedPatternUnit(pattern&); //追加されているパターンユニットのインデックスを返す
			void Pruning(std::string&, double); //パターン構造レベルでの枝刈りが可能なら実行
		//表示
			void PrintPattern(std::ostream&, int, int, bool, bool, bool, bool, bool, bool, bool, bool, std::vector<entity>&, std::vector<std::string>&, std::vector<std::string>&); //パターンを表示
			void PrintPrunable(std::ostream&); //枝刈り状態を表示
		//その他
			void ClearPattern(bool); //パターンをクリア
			pattern(){Prunable = false; MatchingObjective = false; Searched = false; WildCardNum=0;};
			~pattern(){ClearPattern(false);};
};

//ワイルドカードを除いた次数を返す
int pattern::GetDeg(int pe_idx){
	std::unordered_map<int, std::vector<int> >& relate = PatternentityList[pe_idx].GetRelation();
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;
	int deg = 0; //次数を0で初期化
	for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){ //各リンク
		if (IsWildCardLink(iter_ivi->first)) continue; //ワイルドカードでないリンクのみ探索
		for (int i=0, size_i=iter_ivi->second.size(); i<size_i; ++i){ //そのリンクによって接続する各エンティティ
			if (!IsWildCardEntity(PatternentityList[iter_ivi->second[i]].GetId(), PatternentityList[iter_ivi->second[i]].GetIsLabel())) deg++; //ワイルドカードでなければ次数を1追加
		}
	}
	return deg;
}

//マッチするエンティティをコピー
void pattern::CopyMatchEntity(pattern& p){
	std::vector<patternentity>& pel = p.GetPatternentityList();
	std::unordered_map<int, int> CorresIndex = GetCorrespondingIndex(p);
	std::unordered_map<int, int>::iterator iter_ii, end_ii;
	for (iter_ii=CorresIndex.begin(), end_ii=CorresIndex.end(); iter_ii!=end_ii; ++iter_ii){
		ModifyMatchEntity(iter_ii->first, pel[CorresIndex[iter_ii->second]].GetMatchEntity());
	}
}

//枝刈り状況をコピー
void pattern::CopyPrunable(pattern& p){
	std::vector<patternentity>& pel = p.GetPatternentityList();
	std::unordered_map<int, int> CorresIndex = GetCorrespondingIndex(p);
	std::unordered_map<int, int>::iterator iter_ii, end_ii;
	Prunable = p.GetPrunable();
	for (iter_ii=CorresIndex.begin(), end_ii=CorresIndex.end(); iter_ii!=end_ii; ++iter_ii){
		PatternentityList[iter_ii->first].CopyPrunable(pel[CorresIndex[iter_ii->second]]);
	}
}

//等号演算
 pattern& pattern::operator=(pattern& p){
 	PatternentityList.resize(p.GetPatternentityList().size());
 	for (int i=0, size=PatternentityList.size(); i<size; ++i) PatternentityList[i] = p.GetPatternentityList()[i];
 	WildCardNum = p.GetWildCardNum();
 	return *this;
 }

 //子パターンを追加
 void pattern::AddChild(int p_idx){
 	Children.insert(p_idx);
 }

void pattern::DeleteChild(int p_idx){
	Children.erase(p_idx);
}

//WildCardNumをセット
void pattern::SetWildCardNum(int num){
	WildCardNum = num;
}

//MatchingObjectiveをtrueに切り替え
void pattern::ChangeMatchingObjective(){
	MatchingObjective = true;
}

//Searchedをtrueに切り替え
void pattern::ChangeSearched(){
	Searched = true;
}

//patternentityを変更
void pattern::ModifyEntity(int id_pe, int id, bool islabel){
	PatternentityList[id_pe].ModifyEntity(id_pe, id, islabel);
}

//リンクラベルを変更
void pattern::ModifyLink(int spe_idx, int link_id, int epe_idx, int new_link_id){
	PatternentityList[spe_idx].ModifyLink(link_id, epe_idx, new_link_id);
	PatternentityList[epe_idx].ModifyLink(-link_id, spe_idx, -new_link_id);
}

//パターンエンティティを追加
void pattern::AddEntity(int id, bool islabel){
	patternentity new_pe;
	new_pe.ModifyEntity(PatternentityList.size(), id, islabel);
	PatternentityList.push_back(new_pe);
}

//リンクを追加
void pattern::AddLink(int spe_idx, int link_id, int epe_idx){
	PatternentityList[spe_idx].AddLink(link_id, epe_idx);
	PatternentityList[epe_idx].AddLink(-link_id, spe_idx);
}

//エンティティ情報をクリア（ワイルドカードにする）
void pattern::ClearEntity(int pe_idx){
	std::unordered_map<int, std::vector<int> > relate = PatternentityList[pe_idx].GetRelation();
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;

	for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){ //各トリプルを走査
		for (int i=0, size_i=iter_ivi->second.size(); i<size_i; ++i){
			ClearLink(pe_idx, iter_ivi->first, iter_ivi->second[i]); //pe_idxのパターンエンティティに隣接するリンクをクリア
		}
	}
	if (!IsWildCardEntity(PatternentityList[pe_idx].GetId(), PatternentityList[pe_idx].GetIsLabel())){ //元々ワイルドカードでなければ
		PatternentityList[pe_idx].ClearEntity(); //エンティティのクリア処理を施し
		WildCardNum++; //WildCardNumをインクリメントする
	}
}

//リンク情報をクリア（ワイルドカードにする）
void pattern::ClearLink(int spe_idx, int link_id, int epe_idx){
	if (!IsWildCardLink(link_id)){ //元々ワイルドカードでなければ
		PatternentityList[spe_idx].ClearLink(link_id, epe_idx, true); //リンクのクリア処理を施し
		PatternentityList[epe_idx].ClearLink(-link_id, spe_idx, false);
	 	WildCardNum++; //WildCardNumをインクリメントする
 	}
}

//patternを修正
void pattern::ModifyWildCardInTriple(int spe_idx, int link_id, int epe_idx, patternentity& new_spe, int new_link, patternentity& new_epe, bool HeadIsWildCard, bool LinkIsWildCard, bool TailIsWildCard){
	if (HeadIsWildCard){
		PatternentityList[spe_idx].ModifyEntity(spe_idx, new_spe.GetId(), new_spe.GetIsLabel());
		WildCardNum--;
	}
	if (TailIsWildCard){
		PatternentityList[epe_idx].ModifyEntity(epe_idx, new_epe.GetId(), new_epe.GetIsLabel());
		WildCardNum--;
	}
	if (LinkIsWildCard){
		ModifyLink(spe_idx, link_id, epe_idx, new_link);
		WildCardNum--;
	}
}

//パターンエンティティとリンクを追加
void pattern::AddTriple(int pe_id, int link_id, patternentity& new_pe){
	AddEntity(new_pe.GetId(), new_pe.GetIsLabel());
	AddLink(pe_id, link_id, PatternentityList.size()-1);
}

//パターン構造を変更
void pattern::ChangeStructure(pattern& p){
	PatternentityList.resize(p.GetPatternentityList().size());
	for (int i=0, size_i=PatternentityList.size(); i<size_i; ++i) PatternentityList[i] = p.GetPatternentityList()[i];
	WildCardNum = p.GetWildCardNum();
}

//MatchEntityリストを変更
void pattern::ModifyMatchEntity(int pe_idx, std::unordered_set<int>& ME){
	PatternentityList[pe_idx].ModifyMatchEntity(ME);
}

//パターン構造に合致するエンティティを追加
void pattern::AddMatchEntity(int pe_idx, int id){
	PatternentityList[pe_idx].AddMatchEntity(id);
}

//エンティティをLinkClassifiedMEに追加
void pattern::AddLinkClassifiedME(int pe_idx, int link_id, int entity_id){
	PatternentityList[pe_idx].AddLinkClassifiedME(link_id, entity_id);
}

//CombinexxEntityを変更
void pattern::ModifyCombineElement(int pe_idx, std::unordered_map<int, std::unordered_map<int, int> >& CombineElement, int kind){
	PatternentityList[pe_idx].ModifyCombineElement(CombineElement, kind);
}

//結合しているエンティティとして追加
void pattern::AddCombineEntity(int pe_idx, int link_id, int ee_id, std::unordered_set<int>& label){
	PatternentityList[pe_idx].AddCombineEntity(link_id, ee_id, label);
}

//パターン構造に合致するリンクを追加
void pattern::AddCombineLink(int pe_idx, int link_id, int label){
	PatternentityList[pe_idx].AddCombineLink(link_id, label);
}

 //CombineXXをクリア
void pattern::ClearCombineElement(int pe_idx, int kind){
	PatternentityList[pe_idx].ClearCombineElement(kind);
}

//同じpatternentityが存在するかどうか
bool pattern::ExistSameEntity(){
	bool return_bool = false;
	for (int i=0, size_i=PatternentityList.size()-1; i<size_i; ++i){
		if (IsWildCardEntity(PatternentityList[i].GetId(), PatternentityList[i].GetIsLabel())) continue;
		for (int j=i+1, size_j=PatternentityList.size(); j<size_j; ++j){
			if (IsWildCardEntity(PatternentityList[j].GetId(), PatternentityList[j].GetIsLabel())) continue;			
			if (PatternentityList[i].SamePatternentity(PatternentityList[j])){
				return_bool = true;
				break;
			}
		}
		if (return_bool) break;
	}
	return return_bool;
} 

//入力パターンに対応するインデックス値を返す（map[i]でthisのi番目が入力パターンで何番目かを示す）
std::unordered_map<int, int> pattern::GetCorrespondingIndex(pattern& p){
	std::unordered_map <int, int> return_map;
	std::vector<patternentity>& pel = p.GetPatternentityList();
	
	for (int i=0, size_i=PatternentityList.size(); i<size_i; ++i){
		if (IsWildCardEntity(PatternentityList[i].GetId(), PatternentityList[i].GetIsLabel())) continue;
		for (int j=0, size_j=pel.size(); j<size_j; ++j){
			if (IsWildCardEntity(pel[j].GetId(), pel[j].GetIsLabel())) continue;
			if (PatternentityList[i].SamePatternentity(pel[j])){
				return_map[i] = j;
				break;
			}
		}
	}
	return return_map;
}

//パターン識別子を返す
std::string pattern::GetPatternID(){
	int tmp;
	std::map<int, std::unordered_set<int> > map_content_orgidx; //パターンエンティティの中身とPatternentityListのインデックスを対応させるmap
	std::vector<int> vec_prcidx_orgidx; //処理用の新しいインデックス値とPatternentityListのインデックス値を対応させるvector
	std::unordered_map<int, int> map_orgidx_prcidx; //vec_prcidx_orgidxを逆対応させたmap
	std::string ReturnStr = "";
	std::map<int, std::unordered_set<int> >::iterator iter_oisei, end_oisei;
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;
	std::map<int, std::vector<int> >::iterator iter_oivi, end_oivi;
	std::unordered_set<int>::iterator iter_sei, end_sei;

	//パターン構造を整理 & パターン識別子のエンティティ部分を完成
	for (int i=0, size_i=PatternentityList.size(); i<size_i; ++i){ //map_content_orgidxを作成
		if (!IsWildCardEntity(PatternentityList[i].GetId(), PatternentityList[i].GetIsLabel())){
			if (PatternentityList[i].GetIsLabel()) map_content_orgidx[PatternentityList[i].GetId()].insert(i);
			else map_content_orgidx[LabelNum+PatternentityList[i].GetId()].insert(i);
		}
	}
	for (iter_oisei=map_content_orgidx.begin(), end_oisei=map_content_orgidx.end(); iter_oisei!=end_oisei; ++iter_oisei){ //vec_*_*を作成
		for (iter_sei=iter_oisei->second.begin(), end_sei=iter_oisei->second.end(); iter_sei!=end_sei; ++iter_sei){
			vec_prcidx_orgidx.push_back(*iter_sei);
			map_orgidx_prcidx[*iter_sei] = vec_prcidx_orgidx.size()-1;
			ReturnStr += std::to_string(iter_oisei->first) + ",";
		}
	}

	//パターン識別子のリンク部分を完成
	for (int i=0, size_i=vec_prcidx_orgidx.size(); i<size_i; ++i){
		std::unordered_map<int, std::vector<int> >& relate = PatternentityList[vec_prcidx_orgidx[i]].GetRelation();
		std::map<int, std::vector<int> > new_relate; //処理用の新しいRelation
		for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){
			if (iter_ivi->first<0 || IsWildCardLink(iter_ivi->first)) continue; //逆方向のリンクかワイルドカードならスルー
			for (int j=0, size_j=iter_ivi->second.size(); j<size_j; ++j){ //順方向のリンクと接続するエンティティを見る
				new_relate[iter_ivi->first].push_back(map_orgidx_prcidx[iter_ivi->second[j]]);
				for (int h=new_relate[iter_ivi->first].size()-1; h>0; --h){
					if (new_relate[iter_ivi->first][h-1] > new_relate[iter_ivi->first][h]){ //処理用のインデックス値の昇順に並べる
						tmp = new_relate[iter_ivi->first][h];
						new_relate[iter_ivi->first][h] = new_relate[iter_ivi->first][h-1];
						new_relate[iter_ivi->first][h-1] = tmp;
					}
					else break;
				}
			}
		}
		for (iter_oivi=new_relate.begin(), end_oivi=new_relate.end(); iter_oivi!=end_oivi; ++iter_oivi){
			for (int j=0, size_j=iter_oivi->second.size(); j<size_j; ++j){
				ReturnStr += "(" + std::to_string(i) + "," + std::to_string(iter_oivi->first) + "," + std::to_string(iter_oivi->second[j]) + "),";
			}
		}
	}

	return ReturnStr;
}

//上界スコアを返す
double pattern::GetUpperBoundScore(std::string& score_kind){
	double tmpscore, score;
	score = 0.0;
	for (int i=0, size_i=PatternentityList.size(); i<size_i; ++i){
		if (IsWildCardEntity(PatternentityList[i].GetId(), PatternentityList[i].GetIsLabel())) continue; //ワイルドカードの時はスキップ
		tmpscore = PatternentityList[i].GetUpperBoundScore(score_kind);
		if (tmpscore > score) score = tmpscore;
	}
	return score;
}

//pと比較したときに追加されているパターンユニットのインデックスを返す
int pattern::GetAddedPatternUnit(pattern& p){
	int id_self, id_cmp;
	bool islabel_self, islabel_cmp;
	std::vector<patternentity>& pel_cmp = p.GetPatternentityList();
	for (int i=0, size_i=PatternentityList.size(); i<size_i; ++i){
		id_self = PatternentityList[i].GetId();
		id_cmp = pel_cmp[i].GetId();
		islabel_self = PatternentityList[i].GetIsLabel();
		islabel_cmp = PatternentityList[i].GetIsLabel();
		if (!IsWildCardEntity(id_self, islabel_self) && IsWildCardEntity(id_cmp, islabel_cmp)){ //自身側がワイルドカードでなくて比較側がワイルドカードの時（差分であった時）
			if (islabel_self) return id_self; //対応するパターンユニットのインデックスを返す
			else return id_self + LabelNum - 1;
		}
	}
	return -1;
}

//パターン構造レベルでの枝刈りが可能なら実行
void pattern::Pruning(std::string& score_kind, double minscore){
	bool TmpPrunable = true;
	if (Prunable) return; //既に枝刈り可能なら実行しない
	if (minscore==-1){ //例外スコアランキングが空の場合はfalseを格納
		Prunable = false;
		return;
	}
	for (int i=0, size=PatternentityList.size(); i<size; ++i){ //各パターンエンティティを走査
		if (IsWildCardEntity(PatternentityList[i].GetId(), PatternentityList[i].GetIsLabel())) continue; //ワイルドカードの時はスキップ
		PatternentityList[i].Pruning(score_kind, minscore); //patternentityの枝刈り
		if (!PatternentityList[i].GetPrunable()[3]){ //patternentityが枝刈り不可能ならTmpPrunableをfalseに
			TmpPrunable = false;
			break;
		}
	}
	Prunable = TmpPrunable; //全てのパターンエンティティの枝刈りが可能なら，trueを格納
}

//パターンを表示
void pattern::PrintPattern(std::ostream& os, int p_idx_1, int p_idx_2, bool PrintName, bool PrintRelation, bool PrintMatchEntity, bool PrintLinkClassifiedME, bool PrintCombineElement, bool PrintChild, bool PrintContextInfo, bool PrintExpScore, std::vector<entity>& EntityList, std::vector<std::string>& LabelList, std::vector<std::string>& LinkList){
	std::unordered_set<int>::iterator iter_cld, end_cld;
	
	os << "<Pattern " << p_idx_1 << "-" << p_idx_2 << ">" << std::endl;
	os << "Matching Objective: ";
	if (MatchingObjective) os << "yes" << std::endl;
	else os << "no" << std::endl;
	os << "Searched:";
	if (Searched) os << "yes" << std::endl;
	else os << "no" << std::endl;
	if (PrintContextInfo){
		os << "The Number of WildCard: " << WildCardNum << std::endl;
	}
	if (PrintChild){
		os << "Child Pattern: ";
		for (iter_cld=Children.begin(), end_cld=Children.end(); iter_cld!=end_cld; ++iter_cld) os << *iter_cld << ",";
		os << std::endl;
	}
	for (int i=0, size=PatternentityList.size(); i<size; ++i){
		os << std::endl;
		PatternentityList[i].PrintPatternentity(os, PrintName, PrintRelation, PrintMatchEntity, PrintLinkClassifiedME, PrintCombineElement, PrintExpScore, EntityList, LabelList, LinkList);
	}
}

//枝刈り状況を表示　
void pattern::PrintPrunable(std::ostream& os){
	if (Prunable) os<< "T" << std::endl;
	else os<< "F" << std::endl;
	for (int i=0, size=PatternentityList.size(); i<size; ++i) {os << "\t" << i << ":"; PatternentityList[i].PrintPrunable(os);}
}

//パターンをクリア
void pattern::ClearPattern(bool searched){
	PatternentityList.clear();
	Children.clear();
	Prunable = false;
	MatchingObjective = false;
	Searched = searched;
	WildCardNum=0;
}


//ターゲットクラス
class target{
	private:
		patternentity Head; //head
		int Link; //link
		patternentity Tail; //tail
	public:
		//メンバ返し
			patternentity& GetHead(){return Head;}; //Headを返す
			int GetLink(){return Link;}; //Linkを返す
			patternentity& GetTail(){return Tail;}; //Tailを返す
		//メンバ変更
			void ModifyHead(patternentity&); //Headを変更
			void ModifyLink(int); //Linkを変更
			void ModifyTail(patternentity&); //Tailを変更
		//patternentity->ターゲットのマッチング
			bool HeadMatch(entity&); //指定のエンティティがheadにマッチするかどうか
			bool LinkMatch(int); //指定のリンクラベルがlinkにマッチするかどうか
			bool TailMatch(entity&); //指定のエンティティがtailにマッチするかどうか
			bool TripleMatch(entity&, int, entity&); //指定のトリプルがマッチするかどうか
		//表示
			void PrintTarget(std::ostream&, bool, std::vector<entity>&, std::vector<std::string>&, std::vector<std::string>&); //ターゲットを表示
		//その他
			target(){};
			~target(){};
};

//Headを変更
void target::ModifyHead(patternentity& h){
	Head = h;
}

//Linkを変更
void target::ModifyLink(int l){
	Link = l;
}

//Tailを変更
void target::ModifyTail(patternentity& t){
	Tail = t;
}

//指定のエンティティがheadにマッチするかどうか
bool target::HeadMatch(entity& e){
	if (IsWildCardEntity(Head.GetId(), Head.GetIsLabel())) return true; //headが指定なしのエンティティの場合はtrue
	else { //指定ありの場合
		if (Head.GetIsLabel()){ //Headがラベルのとき
			if (e.GetLabel().find(Head.GetId())!=e.GetLabel().end()) return true;
			else return false;
		}
		else { //Headがエンティティのとき
			if (e.GetId()==Head.GetId()) return true;
			else return false;
		}
	}
}

//指定のリンクラベルがlinkにマッチするかどうか
bool target::LinkMatch(int link){
	if (IsWildCardLink(Link)) return true; //Linkが指定なしの場合はtrue
	else if (Link==link) return true; //指定ありの場合は，同じリンクの時はtrue}
	else return false; //どれでもない場合はfalse
}

//指定のエンティティがtailにマッチするかどうか
bool target::TailMatch(entity& e){
	if (IsWildCardEntity(Tail.GetId(), Tail.GetIsLabel())) return true; //tailが指定なしのエンティティの場合はtrue
	else { //指定ありの場合
		if (Tail.GetIsLabel()){ //Tailがラベルのとき
			if (e.GetLabel().find(Tail.GetId())!=e.GetLabel().end()) return true;
			else return false;
		}
		else { //Headがエンティティのとき
			if (e.GetId()==Tail.GetId()) return true;
			else return false;
		}
	}
}

//指定のトリプルがマッチするかどうか
bool target::TripleMatch(entity& e_head, int link, entity& e_tail){
	return HeadMatch(e_head) && LinkMatch(link) && TailMatch(e_tail);
}

//ターゲットを表示
void target::PrintTarget(std::ostream& os, bool PrintName, std::vector<entity>& EntityList, std::vector<std::string>& LabelList, std::vector<std::string>& LinkList){
	os << "Head: ";
	Head.PrintPatternentity(os, PrintName, false, false, false, false, false, EntityList, LabelList, LinkList);
	os << std::endl;
	if (PrintName) os << "Link: " << LinkList[Link] << std::endl;
	else os << "Link: " << Link << std::endl;
	os << "Tail: ";
	Tail.PrintPatternentity(os, PrintName, false, false, false, false, false, EntityList, LabelList, LinkList);
	os << std::endl;
}


//例外スコアと，それを算出したトリプルと分布
class expscore{
	private:
		std::vector<int> Idx_p; //PatternList，patternentityListのインデックス値（0:PatternListの行，1:PatternListの列，2:PatternentityListのインデックス値）
		int ID_se; //始点エンティティID
		int ID_link; //リンクラベルID
		int ID_ee; //終点エンティティID
		int Label_ee; //終点エンティティのラベル
		double Score; //例外スコア
		int Kind; //例外の種類(0:エンティティラベルによる例外，1:固有エンティティによる例外，2:リンクラベルによる例外)
	public:
		std::vector<int> GetIdx_p(){return Idx_p;}; //Idx_pを返す
		int GetId_se(){return ID_se;} //ID_seを返す
		int GetID_link(){return ID_link;} //ID_linkを返す
		int GetId_ee(){return ID_ee;} //ID_eeを返す
		int GetLabel_ee(){return Label_ee;}; //Label_eeを返す
		double GetScore(){return Score;} //Scoreを返す
		int GetKind(){return Kind;} //Kindを返す
		void SetExpScore(std::vector<int>, int, int, int, int, double, int); //expscoreをセット
		void PrintExpScore(std::ostream&, bool, bool, patternentity&, std::vector<entity>&, std::vector<std::string>&, std::vector<std::string>&); //expscoreを表示
		long GetDataSize(); //expscoreのデータサイズを返す
		void clear(){Idx_p.clear();};
		expscore(){Idx_p.resize(3);};
		~expscore(){Idx_p.clear();};
};

//expscoreをセット（エンティティによる例外の場合）
void expscore::SetExpScore(std::vector<int> idx_p, int se_id, int link_id, int ee_id, int ee_label, double score, int kind){
	Idx_p = idx_p;
	ID_se = se_id;
	ID_link = link_id;
	ID_ee = ee_id;
	Label_ee = ee_label;
	Score = score;
	Kind = kind;
}

//expscoreを表示
void expscore::PrintExpScore(std::ostream& os, bool PrintName, bool PrintIdx_p, patternentity& pe, std::vector<entity>& EntityList, std::vector<std::string>& LabelList, std::vector<std::string>& LinkList){
	std::unordered_map<int, int> distribution;
	std::unordered_map<int, int>::iterator iter_dist, end_dist;

	if (PrintIdx_p) os << "(" << Idx_p[0] << "," << Idx_p[1] << ") - " << Idx_p[2] << std::endl;
	os << "pe-" << Idx_p[2] << ": ";
	if (PrintName){
		os << EntityEncoder(ID_se, EntityList) << " " << LinkEncoder(ID_link, LinkList);
		os << " " << EntityEncoder(ID_ee, EntityList) << " /  " << Score;
	}
	else os << ID_se << " " << ID_link << " " << ID_ee << " /  " << Score;

	if (Kind==0 or Kind==1){
		if (Kind==0) os << " (LabelException)" << std::endl;
		else os << " (UniqueEntityException)" << std::endl;
		if (PrintName) os << "Destination of pe-" << Idx_p[2] << " by " << LinkEncoder(ID_link, LinkList) << ":" << std::endl;
		else os << "Destination of pe-" << Idx_p[2] << " by " << ID_link << ":" << std::endl;
	}
	else{
		os << " (LinkException)" << std::endl;
		if (PrintName) os << "Link between pe-" << Idx_p[2] << " and label-" << LabelEncoder(Label_ee, LabelList) << ":" << std::endl;
		else os << "Link between pe-" << Idx_p[2] << " and label-" << Label_ee << ":" << std::endl;
	}

	if (Kind==0){
		distribution = pe.GetCombineLabelEntity()[ID_link];
		for (iter_dist=distribution.begin(), end_dist=distribution.end(); iter_dist!=end_dist; ++iter_dist){
			if (PrintName){
				//if (iter_dist->first==0) os << "\t_NoEntity_:" << iter_dist->second; else
				os << "\t" << LabelEncoder(iter_dist->first, LabelList) << ":" << iter_dist->second;
			}
			else os << "\t" << iter_dist->first << ":" << iter_dist->second;
			if (iter_dist->first==Label_ee) os << "<-";
			os << std::endl;
		}
	}
	else if (Kind==1){
		distribution = pe.GetCombineUniqueEntity()[ID_link];
		for (iter_dist=distribution.begin(), end_dist=distribution.end(); iter_dist!=end_dist; ++iter_dist){
			if (PrintName){
				//if (iter_dist->first==0) os << "\t_NoEntity_:" << iter_dist->second; else
				os << "\t" << EntityEncoder(iter_dist->first, EntityList) << ":" << iter_dist->second;
			}
			else os << "\t" << iter_dist->first << ":" << iter_dist->second;
			if (iter_dist->first==Label_ee) os << "<-";
			os << std::endl;
		}
	}
	else{
		distribution = pe.GetCombineLink()[Label_ee];
		for (iter_dist=distribution.begin(), end_dist=distribution.end(); iter_dist!=end_dist; ++iter_dist){
			if (PrintName){
				os << "\t" << LinkEncoder(iter_dist->first, LinkList) << ":" << iter_dist->second;
			}
			else os << "\t" << iter_dist->first << ":" << iter_dist->second;
			if (iter_dist->first==ID_link) os << "<-";
			os << std::endl;
		}
	}
}


class graph{
	private:
		std::vector<entity> EntityList; //エンティティのvector（インデックス値=entityのID）
		std::vector<std::string> LabelList; //ラベルのvector（インデックス値=ラベルのID）
		std::vector<std::string> LinkList; //リンクラベルのvector（インデックス値=リンクラベルのID）
		std::vector<target> TargetList; //ターゲットのvector
		std::vector<pattern> ContextList; //コンテキストのvector
		std::vector<std::vector<pattern> > PatternList; //パターンのvector（行:リンク数，列:ID_peと同じ）
		std::vector<patternentity> PatternUnitList; //補助情報として使うパターンエンティティのリスト
		std::vector<std::unordered_map<std::string, int> > PatternIndex; //パターンの索引（インデックス値：パターンのリンク数，key：パターン識別子，value：該当するパターンの第2インデックス値
		std::map<double, std::vector<expscore> > ExpRanking; //例外スコアのtop-k
		int ExpscoreNum; //ExpRanking中のexpscoreのインスタンスの数
	public:
		//メンバ返し
			std::vector<entity>& GetEntityList(){return EntityList;}; //EntityListを返す
			std::vector<std::string>& GetLabelList(){return LabelList;}; //LabelListを返す
			std::vector<std::string>& GetLinkList(){return LinkList;}; //LinkListを返す
			std::vector<std::vector<pattern> >& GetPatternList(){return PatternList;};//PatternListを返す
			std::vector<patternentity>& GetPatternUnitList(){return PatternUnitList;}; //PatternUnitListを返す
			std::vector<std::unordered_map<std::string, int> > GetPatternIndex(){return PatternIndex;}; //PatternIndexを返す
			std::map<double, std::vector<expscore> >& GetExpRanking(){return ExpRanking;}; //ExpRankingを返す
			int GetExpscoreNum(){return ExpscoreNum;}; //ExpscoreNumを返す
		//メンバの変更
			void AddLink(int, int, int); //リンクを追加
			void ModifyCombineElement(int, int, int, bool); //指定のpatternのpatternentityにおいてCombineXXを修正
		//パターンの操作
			void AddPattern(int, pattern&); //パターンを追加
			bool GetAllDescendantsSearched(int, int); //子孫のマッチング対象のパターンの全てが探索済かどうか
			void FinishPatternSearch(int, int, bool, std::unordered_set<int>&); //パターンの探索を終える
		//コンテキストの操作
			void AddModifiedContext(int, int, int, int, int, patternentity&, int, patternentity&, bool, bool, bool, std::vector<std::unordered_set<int> >&); //patternentityを修正しコンテキストとして追加
			void InstancizeContextTriple(int, int, int, int, int, int, bool, bool, bool, std::vector<std::unordered_set<int> >&); //コンテキスト中のワイルドカードをトリプル単位でインスタンス化
		//マッチング
			void SelectMatchEntity(int, int, int, int, int, std::map<triple, bool>&, int&, std::vector<std::unordered_set<int> >&); //MatchEntityを選別（MergePatternで使用する関数）
			bool PatternMatching(int, int); //パターンマッチング
			bool ExistMatchingTarget(entity&, int, entity&); //指定のトリプルにマッチするターゲットが存在するかどうか
		//例外スコア計算
			bool UpdateRanking(int, expscore&); //ExpRankingを変更
			bool CalcExpScore_dist(int, int, int, int); //あるpattern中のあるpatternentityに結合するエンティティに関して例外スコアを格納（分布の偏りベース）
			bool CalcExpScore_freq(int, int, int, int); //あるpattern中のあるpatternentityに結合するエンティティに関して例外スコアを格納（頻度ベース）
		//MakePatternListで呼び出す関数
			std::vector<int> AddCutOffPattern(int); //ワイルドカードで切り取ったパターンを追加
			void AddPatternToMatch(int, int, int); //AddCutOffPattern()で追加したパターンの子を追加していくことで，マッチング対象のパターンを列挙
			void AddParent(int, int); //親パターンを追加
			void DetermineParent(int, int, std::unordered_map<int, bool>&, int&, std::unordered_map<int, std::unordered_set<int> >&); //指定のパターンを親として決定
			void DeleteParent(int); //必要な親パターン以外を削除
		//FindExceptionで呼び出す関数
			void ReadQuery(std::string, std::string); //クエリ読み込み
			void MakePatternList(int); //パターン探索空間を作成
			void DFSforPatternList(int, int, int, std::string&, std::vector<std::unordered_map<int, std::unordered_set<int> > >&); //パターンを探索しながらtop-kの例外サブグラフを格納（枝刈りをする効率的な探索方法）
			void AllPatternSearch(int, std::string&); //パターンを探索しながらtop-kの例外サブグラフを格納（全探索）
			void CountPattern(); //パターン数を調査
			void ClearGraphElm(); //一部グラフの要素をリセット
		//main関数で呼び出す関数
			void ReadGraph(std::string, std::string, std::string);//グラフを読み込む
			void SetCombineInfo(bool, std::string); //各パターンの隣接要素の情報を格納
			void MakeIndex(std::string); //隣接要素の情報を格納したインデックスを作成
			void MakeSortout(std::string, std::string, std::string); //グラフ情報を整理したファイルを作成
			void FindException(int, std::string, int, std::map<std::string, std::string>&); //例外検出
		//デバッグ・調査
			void GlobalExp(bool, expscore&); //大域例外度合いと比較
			void CountExpKind(); //どのような種類の例外があるかをカウント
			void PrintStat(); //統計情報を出力
			void HaveLinkEntity(int); //特定のリンクと結合するエンティティの情報を表示
			void ConnectedLinkWithEntity(int); //特定のエンティティラベルと結合するリンクの情報を表示
			void AllSearchable(); //PatternListを深さ優先探索して全探索できるか確認
		//表示
			void PrintGraph(std::ostream&, bool, bool, bool); //グラフを表示
			void PrintPatternList(std::ostream&, bool, bool, bool, bool, bool, bool, bool, bool, bool); //PatternListを表示
			void PrintPatternUnitList(std::ostream&, bool, bool, bool, bool, bool); //PatternUnitListを表示
			void PrintContextList(std::ostream&, bool, bool); //ContextListを表示
			//void PrintTargetList(std::ostream&, bool, bool, bool ,bool ,bool, bool, bool); //TargetListを表示
			void PrintExpRanking(std::ostream&, bool, bool); //ExpRankingを表示
		//その他
			graph(){ExpscoreNum=0;};
			~graph(){EntityList.clear(); LabelList.clear(); LinkList.clear(); PatternUnitList.clear(); ClearGraphElm();};
};

//リンクを追加
void graph::AddLink(int id_head, int id_link, int id_tail){
	EntityList[id_head].AddLink(id_link, id_tail);
	EntityList[id_tail].AddLink(-id_link, id_head);
}

//指定のpatternのpatternentityのCombineXXを修正
void graph::ModifyCombineElement(int PatternSize, int p_idx, int pe_idx, bool NonExistElm){
	std::unordered_set<int> me; //着目中のpatternentityに合致するエンティティ集合
	std::unordered_map<int, std::unordered_map<int, int> > cle; //着目中のpatternentityの隣接するラベル
	std::unordered_map<int, std::unordered_map<int, int> > cl; //CombineLinkとして追加されるマップ
	std::unordered_map<int, std::unordered_map<int, int> >::iterator iter_i_ii, end_i_ii;
	std::unordered_map<int, int>::iterator iter_ii, end_ii;
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;
	std::unordered_map<int, std::unordered_set<int> >::iterator iter_i_sei, end_i_sei;
	std::unordered_set<int>::iterator iter_sei, end_sei, iter_sei2, end_sei2;
	std::unordered_set<int> NoLabel;
	NoLabel.insert(0);

	if (PatternSize == -1) me = PatternUnitList[p_idx].GetMatchEntity();
	else me = PatternList[PatternSize][p_idx].GetPatternentityList()[pe_idx].GetMatchEntity();
	//std::cout << "DEBUG me " << me.size() << std::endl;

	//リンクが消失していないエンティティをCombine××Entityに格納
	for (iter_sei=me.begin(), end_sei=me.end(); iter_sei!=end_sei; ++iter_sei){ //meを捜査
		std::unordered_map<int, std::vector<int> >& relate = EntityList[*iter_sei].GetRelation(); //エンティティのリンク情報
		for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){ //各リンクラベルを捜査
			for (int j=0, size_j=iter_ivi->second.size(); j<size_j; ++j){ //そのリンクラベルによって結合するエンティティを捜査
				if (PatternSize == -1) PatternUnitList[p_idx].AddCombineEntity(iter_ivi->first, iter_ivi->second[j], EntityList[iter_ivi->second[j]].GetLabel());
				else PatternList[PatternSize][p_idx].AddCombineEntity(pe_idx, iter_ivi->first, iter_ivi->second[j], EntityList[iter_ivi->second[j]].GetLabel());
			}
		}
	}

	//リンクが消失しているエンティティをCombinexxEntityに格納
	if (NonExistElm){
		if (PatternSize == -1) cle = PatternUnitList[p_idx].GetCombineLabelEntity();
		else cle = PatternList[PatternSize][p_idx].GetPatternentityList()[pe_idx].GetCombineLabelEntity();
		for (iter_i_ii=cle.begin(), end_i_ii=cle.end(); iter_i_ii!=end_i_ii; ++iter_i_ii){ //cleのリンクを走査
			for (iter_sei=me.begin(), end_sei=me.end(); iter_sei!=end_sei; ++iter_sei){ //meを走査
				std::unordered_map<int, std::vector<int> >& relate = EntityList[*iter_sei].GetRelation(); //エンティティのリンク情報
				if (relate.find(iter_i_ii->first)==relate.end()){ //着目中のリンクがrelateにない場合
					if (PatternSize == -1) PatternUnitList[p_idx].AddCombineEntity(iter_i_ii->first, 0, NoLabel);
					else PatternList[PatternSize][p_idx].AddCombineEntity(pe_idx, iter_i_ii->first, 0, NoLabel);
				}
			}
		}

		//単一エンティティのパターン構造について，CombineLinkに格納
		for(iter_i_ii=cle.begin(), end_i_ii=cle.end(); iter_i_ii!=end_i_ii; ++iter_i_ii){ //cleのリンクを走査
			for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(); iter_ii!=end_ii; ++iter_ii){ //リンクに結合する各ラベル
				if (iter_ii->first!=0) cl[iter_ii->first][iter_i_ii->first] += iter_ii->second; //消失リンクのときはスキップ
			}
		}
		if (PatternSize == -1) PatternUnitList[p_idx].ModifyCombineElement(cl, 2);
		else PatternList[PatternSize][p_idx].ModifyCombineElement(pe_idx, cl, 2);

		//ラベルが消失しているエンティティをCombineLinkに格納
		for (iter_i_ii=cl.begin(), end_i_ii=cl.end(); iter_i_ii!=end_i_ii; ++iter_i_ii){ //clのラベルを走査
			for (iter_sei=me.begin(), end_sei=me.end(); iter_sei!=end_sei; ++iter_sei){ //meを走査
				bool ExistLabel = false;
				std::unordered_map<int, std::vector<int> >& relate = EntityList[*iter_sei].GetRelation(); //エンティティのリンク情報
				for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){
					for (int i=0, size_i=iter_ivi->second.size(); i<size_i; ++i){
						std::unordered_set<int> label = EntityList[iter_ivi->second[i]].GetLabel();
						if (label.find(iter_i_ii->first)!=label.end()){
							ExistLabel = true;
							break;
						}
					}
					if (ExistLabel) break;
				}
				if (!ExistLabel){ //着目中のラベルがrelateにない場合
					if (PatternSize == -1) PatternUnitList[p_idx].AddCombineLink(0, iter_i_ii->first);
					else PatternList[PatternSize][p_idx].AddCombineLink(pe_idx, 0, iter_i_ii->first);
				}
			}
		}
	}
}

//パターンを追加
void graph::AddPattern(int PatternSize, pattern& p){
	std::string pid = p.GetPatternID();
	PatternList[PatternSize].push_back(p);
	PatternIndex[PatternSize][pid] = PatternList[PatternSize].size()-1;
}

//子孫のマッチング対象のパターンの全てが探索済かどうか（全て探索済みでtrue）（自身は対象外）
bool graph::GetAllDescendantsSearched(int PatternSize, int p_idx){
	bool return_bool = true;
	std::vector<std::unordered_set<int> > SearchPattern(PatternList.size()); //走査するパターン
	std::unordered_set<int> children;
	std::unordered_set<int>::iterator iter_sei1, iter_sei2, end_sei1, end_sei2;
	
	SearchPattern[PatternSize].insert(p_idx);
	for (int i=PatternSize, size_i=PatternList.size(); i<size_i; ++i){
		for (iter_sei1=SearchPattern[i].begin(), end_sei1=SearchPattern[i].end(); iter_sei1!=end_sei1; ++iter_sei1){
			children = PatternList[i][*iter_sei1].GetChildren(); //探索中のパターンの子
			for (iter_sei2=children.begin(), end_sei2=children.end(); iter_sei2!=end_sei2; ++iter_sei2){ //子を探索
				if (!PatternList[i+1][*iter_sei2].GetSearched()){ //子が未探索であれば
					SearchPattern[i+1].insert(*iter_sei2); //走査するパターンとして追加
					if (PatternList[i+1][*iter_sei2].GetMatchingObjective()){ //マッチング対象のパターンであれば			
						return_bool = false; //返すboolをfalseにし終了
						break;
					}
				}
			}
			if (!return_bool) break;
		}
		if (!return_bool) break;
	}
	return return_bool;
}

//パターンの探索を終える
void graph::FinishPatternSearch(int PatternSize, int p_idx, bool clear_pattern, std::unordered_set<int>& parents){
	std::unordered_set<int>::iterator iter, end;
	for (iter=parents.begin(), end=parents.end(); iter!=end; ++iter) PatternList[PatternSize-1][*iter].DeleteChild(p_idx); //探索を終えたので自身の親との関係を削除
	if (clear_pattern) PatternList[PatternSize][p_idx].ClearPattern(true); //例外スコアランキングに載らないパターンの場合，パターンのメモリもclear
	else PatternList[PatternSize][p_idx].ChangeSearched(); //載る場合は探索済みとだけ更新
	TimeTime("05-" + std::to_string(PatternSize) + ". Search Pattern (extra)");
}

//patternentityを修正しコンテキストとして追加
void graph::AddModifiedContext(int PatternSize, int p_idx, int spe_idx, int link_id, int epe_idx, patternentity& new_spe, int new_link, patternentity& new_epe, bool ChangeHead, bool ChangeLink, bool ChangeTail, std::vector<std::unordered_set<int> >& SearchPattern){	
	pattern p;
	p = PatternList[PatternSize][p_idx];
	p.ModifyWildCardInTriple(spe_idx, link_id, epe_idx, new_spe, new_link, new_epe, ChangeHead, ChangeLink, ChangeTail);
	if (p.ExistSameEntity()) return;
	std::string pid = p.GetPatternID();
	if (PatternIndex[PatternSize+1].find(pid) == PatternIndex[PatternSize+1].end()){ //パターンが存在していなければ
		if (p.GetWildCardNum()==0) p.ChangeMatchingObjective(); //ワイルドカードがなければマッチング対象のパターンとする
		AddPattern(PatternSize+1, p); //パターンを追加
		PatternList[PatternSize][p_idx].AddChild(PatternList[PatternSize+1].size()-1); //子として追加
		if (p.GetWildCardNum()!=0) SearchPattern[PatternSize+1].insert(PatternList[PatternSize+1].size()-1); //まだワイルドカードがあれば要探索パターンとして追加
	}
	else{ //パターンが存在していれば
		int existing_p_idx = PatternIndex[PatternSize+1][pid]; //対応するパターンの第2インデックス値
		if (p.GetWildCardNum()==0) PatternList[PatternSize+1][existing_p_idx].ChangeMatchingObjective(); //ワイルドカードがなければマッチング対象のパターンとする
		PatternList[PatternSize][p_idx].AddChild(existing_p_idx); //子として追加
		PatternList[PatternSize+1][existing_p_idx].ChangeStructure(p); //パターン構造を新たに生成したパターンに変更
		if (p.GetWildCardNum()!=0) SearchPattern[PatternSize+1].insert(existing_p_idx); //まだワイルドカードがあれば要探索パターンとして追加
	}
}

//コンテキスト中のワイルドカードをトリプル単位でインスタンス化
//genre_idxのコンテキストの情報を照らし合わせながら指定のパターン中の指定のトリプルをインスタンス化
//インスタンス化して追加されたパターンはSearchPatternにインデックス値で追加
void graph::InstancizeContextTriple(int genre_idx, int PatternSize, int p_idx, int spe_idx, int link_id, int epe_idx, bool PatternHeadIsWildCard, bool PatternLinkIsWildCard, bool PatternTailIsWildCard, std::vector<std::unordered_set<int> >& SearchPattern){
	int context_link, spe_pe_idx, new_link, new_epe_id;
	bool ContextLinkIsWildCard, ContextTailIsWildCard, spe_IsLabel, epe_IsLabel, EndLoop;
	patternentity dummy_pe;
	std::vector<patternentity>& cel = ContextList[genre_idx].GetPatternentityList();
	std::vector<patternentity>& pel = PatternList[PatternSize][p_idx].GetPatternentityList();
	std::unordered_map<int, std::unordered_map<int, int> > cle;
	std::unordered_map<int, std::unordered_map<int, int> > cue;
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;
	std::unordered_map<int, std::unordered_map<int, int> >::iterator iter_i_ii, end_i_ii;
	std::unordered_map<int, int>::iterator iter_ii, end_ii;
	
	//headがワイルドカードのとき（コンテキストが全てワイルドカードで構成されているとき）（T, T, T）
	if (PatternHeadIsWildCard){
		for (int i=1, size_i=PatternUnitList.size(); i<size_i; ++i){ //各パターンユニット一つだけを継ぎ足したパターンを作成
			AddModifiedContext(PatternSize, p_idx, spe_idx, link_id, epe_idx, PatternUnitList[i], 0, dummy_pe, true, false, false, SearchPattern);
		}
		return;
	}

	ContextTailIsWildCard = IsWildCardEntity(cel[epe_idx].GetId(), cel[epe_idx].GetIsLabel()); //コンテキストの対応トリプルのtailがワイルドカードか
	EndLoop = false;
	std::unordered_map<int, std::vector<int> >& relate = cel[spe_idx].GetRelation();
	for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){
		for (int j=0, size_j=iter_ivi->second.size(); j<size_j; ++j){
			if (iter_ivi->second[j] == epe_idx){
				ContextLinkIsWildCard = IsWildCardLink(iter_ivi->first); //コンテキストの対応トリプルのlinkがワイルドカードかどうか
				context_link = iter_ivi->first; //当てはめるlink
				EndLoop = true;
				break;
			}
		}
		if (EndLoop) break;
	}

	//知識グラフの情報をもとに指定のトリプルを完全にインスタンス化
	//パターン側のトリプルとコンテキスト側のトリプルで，どちらか一方がワイルドカーでない要素をインスタンス化したトリプルを想定し，そのトリプルをさらにインスタンス化
	spe_IsLabel = pel[spe_idx].GetIsLabel();
	if (spe_IsLabel) spe_pe_idx = pel[spe_idx].GetId(); //spe_pe_idx: headがPatternUnitListのどのインデックス値と対応するか
	else spe_pe_idx = pel[spe_idx].GetId() + LabelList.size() - 1;
	cle = PatternUnitList[spe_pe_idx].GetCombineLabelEntity();
	if (spe_IsLabel) cue = PatternUnitList[spe_pe_idx].GetCombineUniqueEntity();
	
	if (PatternTailIsWildCard && ContextTailIsWildCard){ //tailがワイルドカードのとき（F, *, T）
		if (PatternLinkIsWildCard && ContextLinkIsWildCard){ //linkがワイルドカードのとき（F, T, T）
			//std::cout << "DEBUG (F,T,T)" << std::endl;
			for (iter_i_ii=cle.begin(), end_i_ii=cle.end(); iter_i_ii!=end_i_ii; ++iter_i_ii){
				for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(); iter_ii!=end_ii; ++iter_ii){
					if (iter_ii->first!=0) AddModifiedContext(PatternSize, p_idx, spe_idx, link_id, epe_idx, dummy_pe, iter_i_ii->first, PatternUnitList[iter_ii->first], false, true, true, SearchPattern);
				}
			}
			if (spe_IsLabel){
				for (iter_i_ii=cue.begin(), end_i_ii=cue.end(); iter_i_ii!=end_i_ii; ++iter_i_ii){
					for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(); iter_ii!=end_ii; ++iter_ii){
						if (iter_ii->first!=0) AddModifiedContext(PatternSize, p_idx, spe_idx, link_id, epe_idx, dummy_pe, iter_i_ii->first, PatternUnitList[iter_ii->first+LabelList.size()-1], false, true, true, SearchPattern);
					}
				}
			}
		}
		else { //リンクがワイルドカードでないとき（F, F, T）
			//std::cout << "DEBUG (F,F,T)" << std::endl;
			if (!PatternLinkIsWildCard) new_link = link_id;
			else new_link = context_link;
			for (iter_ii=cle[new_link].begin(), end_ii=cle[new_link].end(); iter_ii!=end_ii; ++iter_ii){
				if (iter_ii->first!=0) AddModifiedContext(PatternSize, p_idx, spe_idx, link_id, epe_idx, dummy_pe, new_link, PatternUnitList[iter_ii->first], false, PatternLinkIsWildCard, true, SearchPattern);
			}
			if (spe_IsLabel){
				for (iter_ii=cue[new_link].begin(), end_ii=cue[new_link].end(); iter_ii!=end_ii; ++iter_ii){
					if (iter_ii->first!=0) AddModifiedContext(PatternSize, p_idx, spe_idx, link_id, epe_idx, dummy_pe, new_link, PatternUnitList[iter_ii->first+LabelList.size()-1], false, PatternLinkIsWildCard, true, SearchPattern);
				}
			}
		}
	}

	else{ //tailがワイルドカードでないとき（F, *, F）
		if (!PatternTailIsWildCard){
			new_epe_id = pel[epe_idx].GetId();
			epe_IsLabel = pel[epe_idx].GetIsLabel();
		}
		else{
			new_epe_id = cel[epe_idx].GetId();
			epe_IsLabel = cel[epe_idx].GetIsLabel();
		}
		if (PatternLinkIsWildCard && ContextLinkIsWildCard){ //リンクがワイルドカードのとき（F, T, F）
			//std::cout << "DEBUG (F,T,F)" << std::endl;
			if (epe_IsLabel){ //tailがエンティティラベルのとき
				for (iter_i_ii=cle.begin(), end_i_ii=cle.end(); iter_i_ii!=end_i_ii; ++iter_i_ii){
					for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(); iter_ii!=end_ii; ++iter_ii){
						if (iter_ii->first == new_epe_id) AddModifiedContext(PatternSize, p_idx, spe_idx, link_id, epe_idx, dummy_pe, iter_i_ii->first, PatternUnitList[new_epe_id], false, true, PatternTailIsWildCard, SearchPattern);
					}
				}
			}
			else if (spe_IsLabel){ //tailがエンティティのとき
				for (iter_i_ii=cue.begin(), end_i_ii=cue.end(); iter_i_ii!=end_i_ii; ++iter_i_ii){
					for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(); iter_ii!=end_ii; ++iter_ii){
						if (iter_ii->first == new_epe_id) AddModifiedContext(PatternSize, p_idx, spe_idx, link_id, epe_idx, dummy_pe, iter_i_ii->first, PatternUnitList[new_epe_id+LabelList.size()-1], false, true, PatternTailIsWildCard, SearchPattern);
					}
				}
			}
		}
		else{ //リンクがワイルドカードでないとき（F, F, F）
			if (!PatternLinkIsWildCard) new_link = link_id;
			else new_link = context_link;
			//std::cout << "DEBUG (F,F,F)" << std::endl;
			if (epe_IsLabel){ //tailがエンティティラベルのとき
				for (iter_ii=cle[new_link].begin(), end_ii=cle[new_link].end(); iter_ii!=end_ii; ++iter_ii){
					if (iter_ii->first == new_epe_id) AddModifiedContext(PatternSize, p_idx, spe_idx, link_id, epe_idx, dummy_pe, new_link, PatternUnitList[new_epe_id], false, PatternLinkIsWildCard, PatternTailIsWildCard, SearchPattern);
				}
			}
			else if (spe_IsLabel){ //tailがエンティティのとき
				for (iter_ii=cue[context_link].begin(), end_ii=cue[context_link].end(); iter_ii!=end_ii; ++iter_ii){
					if (iter_ii->first == new_epe_id) AddModifiedContext(PatternSize, p_idx, spe_idx, link_id, epe_idx, dummy_pe, new_link, PatternUnitList[new_epe_id+LabelList.size()-1], false, PatternLinkIsWildCard, PatternTailIsWildCard, SearchPattern);
				}
			}
		}
	}
}

//MatchEntityを選別（MergePatternで使用する関数）
void graph::SelectMatchEntity(int PatternSize, int p_idx, int spe_idx, int link_id, int epe_idx, std::map<triple, bool>& ChangeME, int& TrueNum, std::vector<std::unordered_set<int> >& NewMatchEntity){
	std::vector<std::vector<int> > CombineTriple; //始点patternentityと終点patternentityがrによって接続しているトリプル
	std::unordered_set<int> TmpMatchEntity_s; //更新される始点patternentityのMatchEntity
	std::unordered_set<int> TmpMatchEntity_e; //更新される終点patternentityのMatchEntity
	triple t;
	std::map<triple, bool>::iterator iter_tb, end_tb;
	std::unordered_set<int>::iterator iter_sei, end_sei;

	//NewMatchEntityの格納
	for (iter_sei=NewMatchEntity[spe_idx].begin(), end_sei=NewMatchEntity[spe_idx].end(); iter_sei!=end_sei; ++iter_sei){
		std::vector<int> relate_r = EntityList[*iter_sei].GetRelation()[link_id]; //*iterからラベルlink_idのリンクのよって接続しているエンティティ
		for (int i=0, size_i=relate_r.size(); i<size_i; ++i){
			if (PatternList[PatternSize][p_idx].GetPatternentityList()[epe_idx].GetIsLabel()){ //接続エンティティが終点パターンエンティティの性質を満たさない場合スキップ(エンティティラベルのとき)
				std::unordered_set<int> label = EntityList[relate_r[i]].GetLabel();
				if (label.find(PatternList[PatternSize][p_idx].GetPatternentityList()[epe_idx].GetId())==label.end()) continue;
			}
			else{ //接続エンティティが終点パターンエンティティの性質を満たさない場合スキップ(固有エンティティのとき)
				if (PatternList[PatternSize][p_idx].GetPatternentityList()[epe_idx].GetId()!=relate_r[i]) continue;
			}
			if (NewMatchEntity[epe_idx].find(relate_r[i])==NewMatchEntity[epe_idx].end()) continue; //終点patternentityに対応するNewMatchEntityになければスキップ
			TmpMatchEntity_s.insert(*iter_sei);
			TmpMatchEntity_e.insert(relate_r[i]);
		}
	}

	//どちらかのTmpMatchEntityが空のとき，TrueNumを-1にして終了
	if (TmpMatchEntity_s.empty() or TmpMatchEntity_e.empty()){
		TrueNum = -1;
		return;
	}

	 //ChangeME, TrueNum MatchEntityをセット
	if (TmpMatchEntity_s!=NewMatchEntity[spe_idx] or TmpMatchEntity_e!=NewMatchEntity[epe_idx]){ //どちらかのNewMatchEntityに変更があったとき
		for (iter_tb=ChangeME.begin(), end_tb=ChangeME.end(); iter_tb!=end_tb; ++iter_tb) iter_tb->second = false; //ChangeMEの全てのvalueをfalseに書き換え
		TrueNum = 2;
		NewMatchEntity[spe_idx] = TmpMatchEntity_s;
		NewMatchEntity[epe_idx] = TmpMatchEntity_e;
	}
	else{ //変更がなかったとき
		t.SetTriple(spe_idx, link_id, epe_idx);
		if (!ChangeME[t]) TrueNum += 2; //着目中のトリプルがfalseのままであったとき
		if (TrueNum==ChangeME.size()) return; //全てのvalueがtrueになれば終了
	}
	t.SetTriple(spe_idx, link_id, epe_idx);
	ChangeME[t] = true;
	t.SetTriple(epe_idx, -link_id, spe_idx);
	ChangeME[t] = true;
}

//パターンマッチングを行う
bool graph::PatternMatching(int PatternSize, int p_idx){
	int pe_idx, min_me, minimize_link, pibot_head;
	triple t, SearchTriple;
	std::vector<patternentity>& pel = PatternList[PatternSize][p_idx].GetPatternentityList();
	std::queue<triple> SearchQueue; //捜査するトリプルの順番を表したキュー
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;
	std::map<triple, bool>::iterator iter_tb, end_tb;
	std::unordered_set<int>::iterator iter_sei, end_sei;
	SearchedPatternNum[NowQuery][PatternSize]++; //探索済みのパターンとしてカウント
	
	//各patternentityがワイルドカードかどうか調べる
	std::vector<bool> IsWildCardEntities(pel.size());
	int NonWildCardEntityNum = 0; //ワイルドカードでないエンティティ数
	for (int i=0, size_i=pel.size(); i<size_i; ++i){
		IsWildCardEntities[i] = IsWildCardEntity(pel[i].GetId(), pel[i].GetIsLabel());
		if (!IsWildCardEntities[i]) NonWildCardEntityNum++;
	}
	
	//NewMatchEntityの初期化
	std::vector<std::unordered_set<int> > NewMatchEntity(pel.size()); //patternentityごとの新しいMatchEntity
	for (int i=0, size_i=pel.size(); i<size_i; ++i){
		if (IsWildCardEntities[i]) continue; //ワイルドカードエンティティの場合はスキップ
		if (pel[i].GetIsLabel()) pe_idx = pel[i].GetId(); //pe_idx: pel[i]に対応するPatternUnitListのインデックス値
		else pe_idx = pel[i].GetId() + LabelList.size() - 1;
		if (NonWildCardEntityNum==1){ //リンクがないとき，MatchEntityとCombineXXをセット
			PatternList[PatternSize][p_idx].ModifyMatchEntity(i, PatternUnitList[pe_idx].GetMatchEntity());
			PatternList[PatternSize][p_idx].ModifyCombineElement(i, PatternUnitList[pe_idx].GetCombineLabelEntity(), 0);
			PatternList[PatternSize][p_idx].ModifyCombineElement(i, PatternUnitList[pe_idx].GetCombineUniqueEntity(), 1);
			PatternList[PatternSize][p_idx].ModifyCombineElement(i, PatternUnitList[pe_idx].GetCombineLink(), 2);
		}
		else{ //リンクがあるとき
			std::unordered_map<int, std::vector<int> >& relate = pel[i].GetRelation();
			std::unordered_map<int, std::unordered_set<int> > lcme = PatternUnitList[pe_idx].GetLinkClassifiedME();
			min_me = PatternUnitList[pe_idx].GetMatchEntity().size()+1; //minimize_linkと接続し，pel[i]にマッチするエンティティ数
			minimize_link = 0; //pel[i]にマッチするエンティティと接続するリンクの中で，最もそのリンクと接続するマッチエンティティが少ないもの
			for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){
				if (IsWildCardLink(iter_ivi->first)) continue;
				if (lcme[iter_ivi->first].size()<min_me){
					min_me = lcme[iter_ivi->first].size();
					minimize_link = iter_ivi->first;
				}
			}
			NewMatchEntity[i] = lcme[minimize_link];
		}
	}
	if (NonWildCardEntityNum==1){ //リンクがない時はすでにマッチングできているため以降の処理は行わない
		return true;
	}

	//ChangeMEの初期化
	std::map<triple, bool> ChangeME; //MatchEntityの変更状況（key:トリプル，value:変更があったかどうか）
	int TrueNum = 0; //ChangeMEのtrueの数
	for (int i=0, size_i=pel.size(); i<size_i; ++i){ //headを走査
		if (IsWildCardEntities[i]) continue; //headがワイルドカードの場合はスキップ
		std::unordered_map<int, std::vector<int> >& relate = pel[i].GetRelation(); //リンク情報
		for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){ //linkを走査
			if (IsWildCardLink(iter_ivi->first)) continue; //linkがワイルドカーの場合はスキップ		
			for (int j=0, size_j=iter_ivi->second.size(); j<size_j; ++j){ //tailを走査
				if (!IsWildCardEntities[iter_ivi->second[j]]){ //tailがワイルドカードでなければ
					t.SetTriple(i, iter_ivi->first, iter_ivi->second[j]);
					ChangeME[t] = false; //ChangeMEをfalseにセット
				}
			}
		}
	}

	//最初に探索するトリプル（FirstSearchTriple）を決定
	//次数が大きいpatternentityをgreedy法でピックする
	std::vector<int> degs(pel.size()); //各patternentityの次数
	int maximize_head = 0; //最も次数が大きいpatternentity=head
	int max_deg = 0;
	for (int i=0, size_i=pel.size(); i<size_i; ++i){
		if (IsWildCardEntities[i]) continue; //ワイルドカードの場合はスキップ
		degs[i] = PatternList[PatternSize][p_idx].GetDeg(i);
		if (degs[i] > max_deg){
			max_deg = degs[i];
			maximize_head = i;
		}
	}
	int maximize_tail = 0; //maximize_headと接続するpatternentityで最も次数が大きいpatternentity=tail
	int maximize_link = 0; //maximize_headとmaximize_tailの間のリンク=link
	max_deg = 0;
	std::unordered_map<int, std::vector<int> >& relate = pel[maximize_head].GetRelation();
	for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){
		for (int j=0, size_j=iter_ivi->second.size(); j<size_j; ++j){
			if (IsWildCardEntities[iter_ivi->second[j]]) continue; //ワイルドカードの場合はスキップ
			if (degs[iter_ivi->second[j]] > max_deg){
				max_deg = degs[iter_ivi->second[j]];
				maximize_link = iter_ivi->first;
				maximize_tail = iter_ivi->second[j];
			}
		}
	}
	triple FirstSearchTriple(maximize_head, maximize_link, maximize_tail);
	
	//パターンのMatchEntityを変更
	while (true) {
		if(SearchQueue.empty()) SearchTriple = FirstSearchTriple; //キューが空の場合，ChangeMEの最初のトリプルを探索トリプルに
		else{ //そうでない場合，キューの先頭を探索トリプルに
			SearchTriple = SearchQueue.front();
			SearchQueue.pop();
		}
		for (iter_tb=ChangeME.begin(), end_tb=ChangeME.end(); iter_tb!=end_tb; ++iter_tb){
			if(iter_tb->first.ID_se==SearchTriple.ID_se and !(iter_tb->first.ID_link==SearchTriple.ID_link and iter_tb->first.ID_ee==SearchTriple.ID_ee)) SearchQueue.push(iter_tb->first); //SeachTripleのheadがheadとなるようなトリプルをpush（SearchTripleと同じトリプルは除外）
			else if (iter_tb->first.ID_se==SearchTriple.ID_ee and !(iter_tb->first.ID_link!=-SearchTriple.ID_link and iter_tb->first.ID_ee==SearchTriple.ID_se)) SearchQueue.push(iter_tb->first); //SeachTripleのtailがheadとなるようなトリプルをpush（SearchTripleの逆向きトリプルは除外）
		}
		SelectMatchEntity(PatternSize, p_idx, SearchTriple.ID_se, SearchTriple.ID_link, SearchTriple.ID_ee, ChangeME, TrueNum, NewMatchEntity);
		if (TrueNum==ChangeME.size() or TrueNum==-1) break; //全てのvalueがtrueになるかいずれかのMatchEntityが空になれば終了
	}

	//CombineXXを変更
	if (TrueNum==-1){ //MatchEntityが空の場合，falseを返す
		return false;
	}
	else{ //そうでない場合，変更
		for (int i=0, size_i=pel.size(); i<size_i; ++i){
			if (IsWildCardEntities[i]) continue;
			PatternList[PatternSize][p_idx].ModifyMatchEntity(i, NewMatchEntity[i]);
			ModifyCombineElement(PatternSize, p_idx, i, true); //CombineXXをセット
		}
		return true;
	}
}

//指定のトリプルにマッチするターゲットが存在するかどうか
bool graph::ExistMatchingTarget(entity& head, int link, entity& tail){
	bool return_bool = false;
	for (int i=0, size_i=TargetList.size(); i<size_i; ++i){
		if (TargetList[i].TripleMatch(head, link, tail)){
			return_bool = true;
			break;
		}
	}
	return return_bool;
}

//ExpRankingに挿入
//戻り値はランキングの更新があったかどうか
bool graph::UpdateRanking(int k, expscore& es){
	std::map<triple, expscore>::iterator iter_te, end_te;

	if (ExpscoreNum>=k){ //既にk個以上埋まっている場合
		 if (es.GetScore() < ExpRanking.begin()->first) return false; //現在の最下位より低い場合，挿入操作はスキップ
		 else if (es.GetScore() > ExpRanking.begin()->first and ExpscoreNum-ExpRanking.begin()->second.size()+1==k){ //現在の最下位より高く，最下位スコアのトリプル以外でk個になる場合
			ExpscoreNum -= ExpRanking.begin()->second.size(); //削除する分だけExpscoreNumを減らす
		 	ExpRanking.erase(ExpRanking.begin(), std::next(ExpRanking.begin())); //最下位を削除
		 	}
	}
	ExpRanking[es.GetScore()].push_back(es); //k個埋まっていてかつ現在の最下位より低い場合以外は，ExpRankingに挿入
	ExpscoreNum++; //挿入したのでExpscoreNumをインクリメント
	return true;
}

//あるpattern中のあるpatternentityに結合するエンティティに関して例外スコアを格納（分布の偏りベース）
//返すbool値は，1トリプルでもExpRankingの更新があったかどうか
bool graph::CalcExpScore_dist(int k, int PatternSize, int p_idx, int pe_idx){
	int target_head_id, target_link, target_tail_id, ee_label, kind, link, target;
	bool return_bool, update_er, target_head_islabel, target_tail_islabel;
	double sum, score, tmpscore;
	expscore es;
	patternentity pe = PatternList[PatternSize][p_idx].GetPatternentityList()[pe_idx];
	std::unordered_set<int> me = pe.GetMatchEntity();
	std::unordered_map<int, std::unordered_map<int, int> >& cle = pe.GetCombineLabelEntity();
	std::unordered_map<int, std::unordered_map<int, int> >& cue= pe.GetCombineUniqueEntity();
	std::unordered_map<int, std::unordered_map<int, int> >& cl = pe.GetCombineLink();
	std::vector<bool> prunable = pe.GetPrunable();
	std::unordered_set<int> CalcEntityLabel, CalcLinkLabel;
	std::unordered_map<int, double> SumsqAve_label; //各ラベルのカウント数の二乗平均（リンクラベル毎の計算）
	std::unordered_map<int, double>SumsqAve_unique; //各固有エンティティのカウント数の二乗平均（リンクラベル毎の計算）
	std::unordered_map<int, double>SumsqAve_link; //各リンクラベルのカウント数の二乗平均（ラベル毎の計算）
	std::map<triple, expscore> ExpScoreMap; //expscoreのマップ（key:トリプル，value:expscore）
	std::unordered_set<int> NeighborLabel; //隣接しているラベル
	std::unordered_map<int, std::unordered_set<int> >::iterator iter_i_sei, end_i_sei;
	std::unordered_map<int, std::unordered_map<int, int> >::iterator iter_i_ii, end_i_ii;
	std::unordered_map<int, int>::iterator iter_ii, end_ii;
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;
	std::unordered_map<int, double>::iterator iter_id, end_id;
	std::unordered_set<int>::iterator  iter_sei1, end_sei1, iter_sei2, end_sei2;
	return_bool = false;

	//ターゲットにマッチするトリプルに対応するリンクラベル，エンティティラベルに絞る
	if (!prunable[0] or !prunable[1]){ //エンティティラベル，固有エンティティのいずれかが枝刈り不可能
		for (int i=0, size_i=TargetList.size(); i<size_i; ++i){
			target_link = TargetList[i].GetLink();
			if (IsWildCardLink(target_link)){ //指定なしのリンクのとき
				for (iter_i_ii=cle.begin(), end_i_ii=cle.end(); iter_i_ii!=end_i_ii; ++iter_i_ii) CalcLinkLabel.insert(iter_i_ii->first);
				break;
			}
			else{ //指定ありのリンクのとき
				if (cle.find(target_link)!=cle.end()) CalcLinkLabel.insert(target_link);
			}
		}
	}
	if (!prunable[2]){
		for (int i=0, size_i=TargetList.size(); i<size_i; ++i){
			target_head_id = TargetList[i].GetHead().GetId();
			target_head_islabel = TargetList[i].GetHead().GetIsLabel();
			target_tail_id = TargetList[i].GetTail().GetId();
			target_tail_islabel = TargetList[i].GetTail().GetIsLabel();
			if (IsWildCardEntity(target_tail_id, target_tail_islabel)){ //tailが指定なしのとき
				for (iter_i_ii=cl.begin(), end_i_ii=cl.end(); iter_i_ii!=end_i_ii; ++iter_i_ii) CalcEntityLabel.insert(iter_i_ii->first);
				break;
			}
			else { //tailが指定ありのとき
				if (target_tail_islabel){ //tailがラベルのとき
					if (cl.find(target_tail_id)!=cl.end()) CalcEntityLabel.insert(target_tail_id);
				}
				else { //tailがエンティティのとき
					std::unordered_set<int> labellist = EntityList[target_tail_id].GetLabel();
					for (iter_sei1=labellist.begin(), end_sei1=labellist.end(); iter_sei1!=end_sei1; ++iter_sei1){
						if (cl.find(*iter_sei1)!=cl.end()) CalcEntityLabel.insert(*iter_sei1);
					}
				}
			}
		}
	}

	//SumsqAveの計算
	if (!prunable[0] or !prunable[1]){ //エンティティラベル，固有エンティティのいずれかが枝刈り不可能
		for(iter_sei1=CalcLinkLabel.begin(), end_sei1=CalcLinkLabel.end(); iter_sei1!=end_sei1; ++iter_sei1){ //着目中のpatternentityと結合している各リンクラベルでターゲットにマッチするもの
			if (!prunable[0]){ //エンティティラベルに関する例外が枝刈り不可能
				std::unordered_map<int, int> cle_r = cle[*iter_sei1]; //rによって結合するラベルの集計
				sum = 0.0; //SumsqAve_label[r]の格納
				for (iter_ii=cle_r.begin(), end_ii=cle_r.end(); iter_ii!=end_ii; ++iter_ii){
					if (iter_ii->first!=0) sum += (double)std::pow(iter_ii->second, 2);
					else sum += std::pow(weight * (double)iter_ii->second, 2);
				}
				SumsqAve_label[*iter_sei1] = sum / (double)(cle_r.size());
			}

			if (!prunable[1]){ //固有エンティティに関する例外が枝刈り不可能
				std::unordered_map<int, int> cue_r = cue[*iter_sei1]; //rによって結合するエンティティの集計
				sum = 0.0; //SumsqAve_unique[r]の格納
				for (iter_ii=cue_r.begin(), end_ii=cue_r.end(); iter_ii!=end_ii; ++iter_ii) sum += (double)std::pow(iter_ii->second, 2);
				SumsqAve_unique[*iter_sei1] = sum / (double)cue_r.size();
			}
		}
	}
	if (!prunable[2]){ //リンクに関する例外が枝刈り不可能
		for(iter_sei1=CalcEntityLabel.begin(), end_sei1=CalcEntityLabel.end(); iter_sei1!=end_sei1; ++iter_sei1){ //着目中のpatternentityの結合している各エンティティラベル
			std::unordered_map<int, int> cl_r = cl[*iter_sei1]; //lとの間のリンクの集計
			sum = 0.0; //SumsqAve_linkの格納
			for (iter_ii=cl_r.begin(), end_ii=cl_r.end(); iter_ii!=end_ii; ++iter_ii){ //SumsqAve_linkの格納
				if (iter_ii->first!=0) sum += (double)std::pow(iter_ii->second, 2);
				else sum += std::pow(weight * (double)iter_ii->second, 2);
			}
			SumsqAve_link[*iter_sei1] = sum / (double)cl_r.size();
		}
	}
	
	//例外スコアの計算
	std::vector<int> idx_p(3);
	idx_p[0] = PatternSize;
	idx_p[1] = p_idx;
	idx_p[2] = pe_idx;
	for (iter_sei1=me.begin(), end_sei1=me.end(); iter_sei1!=end_sei1; ++iter_sei1){ //各当てはまるエンティティを走査
		//トリプルの例外スコア
		std::unordered_map<int, std::vector<int> >& relate = EntityList[*iter_sei1].GetRelation();
		for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){ //各隣接要素を走査
			for (int i=0, size_i=iter_ivi->second.size(); i<size_i; ++i){
				if (!ExistMatchingTarget(EntityList[*iter_sei1], iter_ivi->first, EntityList[iter_ivi->second[i]])) continue; //走査中のトリプルがマッチするターゲットがない場合スキップ
				score = 0;
				if (!prunable[0]){ //エンティティラベルに関する例外の枝刈が不可能なとき
					for (iter_sei2=EntityList[iter_ivi->second[i]].GetLabel().begin(), end_sei2=EntityList[iter_ivi->second[i]].GetLabel().end(); iter_sei2!=end_sei2; ++iter_sei2){
						tmpscore = SumsqAve_label[iter_ivi->first] / (double)std::pow(cle[iter_ivi->first][*iter_sei2], 2);
						if (tmpscore > score){
							ee_label = *iter_sei2;
							kind = 0;
							score = tmpscore;
						}
					}
				}
				if (!prunable[1]){ //固有エンティティに関する例外の枝刈が不可能なとき
					tmpscore = SumsqAve_unique[iter_ivi->first] / (double)std::pow(cue[iter_ivi->first][iter_ivi->second[i]], 2);
					if (tmpscore > score){
						ee_label = iter_ivi->second[i];
						kind = 1;
						score = tmpscore;
					}
				}
				if (!prunable[2]){ //リンクに関する例外の枝刈が不可能な鳥
					for (iter_sei2=EntityList[iter_ivi->second[i]].GetLabel().begin(), end_sei2=EntityList[iter_ivi->second[i]].GetLabel().end(); iter_sei2!=end_sei2; ++iter_sei2){
						tmpscore = SumsqAve_link[*iter_sei2] / (double)std::pow(cl[*iter_sei2][iter_ivi->first], 2);
						if (tmpscore > score){
							ee_label = *iter_sei2;
							kind = 2;
							score = tmpscore;
						}
					}
				}
				if (score!=0){
					if (kind==0) es.SetExpScore(idx_p, *iter_sei1, iter_ivi->first, iter_ivi->second[i], ee_label, score, kind);
					else if (kind==1) es.SetExpScore(idx_p, *iter_sei1, iter_ivi->first, iter_ivi->second[i], ee_label, score, kind);
					else es.SetExpScore(idx_p, *iter_sei1, iter_ivi->first, iter_ivi->second[i], ee_label, score, kind);
					update_er = UpdateRanking(k, es);
					return_bool = return_bool || update_er;
				}
			}
		}
	}
	return return_bool;
}

//あるpattern中のあるpatternentityに結合するエンティティに関して例外スコアを格納（頻度ベース）
//返すbool値は，1トリプルでもExpRankingの更新があったかどうか
bool graph::CalcExpScore_freq(int k, int PatternSize, int p_idx, int pe_idx){
	int target_head_id, target_link, target_tail_id, ee_label, kind, link, target;
	bool return_bool, update_er, target_head_islabel, target_tail_islabel;
	double sum, score, tmpscore;
	expscore es;
	patternentity pe = PatternList[PatternSize][p_idx].GetPatternentityList()[pe_idx];
	std::unordered_set<int> me = pe.GetMatchEntity();
	std::unordered_map<int, std::unordered_map<int, int> >& cle = pe.GetCombineLabelEntity();
	std::unordered_map<int, std::unordered_map<int, int> >& cue= pe.GetCombineUniqueEntity();
	std::unordered_map<int, std::unordered_map<int, int> >& cl = pe.GetCombineLink();
	std::vector<bool> prunable = pe.GetPrunable();
	std::unordered_set<int> CalcEntityLabel, CalcLinkLabel;
	std::unordered_map<int, double> CountSum_label; //各ラベルのカウント数の和（リンクラベル毎の計算）
	std::unordered_map<int, double> CountSum_unique; //各固有エンティティのカウント数の和（リンクラベル毎の計算）
	std::unordered_map<int, double> CountSum_link; //各リンクラベルのカウント数の和（ラベル毎の計算）
	std::map<triple, expscore> ExpScoreMap; //expscoreのマップ（key:トリプル，value:expscore）
	std::unordered_set<int> NeighborLabel; //隣接しているラベル
	std::unordered_map<int, std::unordered_set<int> >::iterator iter_i_sei, end_i_sei;
	std::unordered_map<int, std::unordered_map<int, int> >::iterator iter_i_ii, end_i_ii;
	std::unordered_map<int, int>::iterator iter_ii, end_ii;
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;
	std::unordered_map<int, double>::iterator iter_id, end_id;
	std::unordered_set<int>::iterator  iter_sei1, end_sei1, iter_sei2, end_sei2;
	return_bool = false;

	//ターゲットにマッチするトリプルに対応するリンクラベル，エンティティラベルに絞る
	if (!prunable[0] or !prunable[1]){ //エンティティラベル，固有エンティティのいずれかが枝刈り不可能
		for (int i=0, size_i=TargetList.size(); i<size_i; ++i){
			target_link = TargetList[i].GetLink();
			if (IsWildCardLink(target_link)){ //指定なしのリンクのとき
				for (iter_i_ii=cle.begin(), end_i_ii=cle.end(); iter_i_ii!=end_i_ii; ++iter_i_ii) CalcLinkLabel.insert(iter_i_ii->first);
				break;
			}
			else{ //指定ありのリンクのとき
				if (cle.find(target_link)!=cle.end()) CalcLinkLabel.insert(target_link);
			}
		}
	}
	if (!prunable[2]){
		for (int i=0, size_i=TargetList.size(); i<size_i; ++i){
			target_head_id = TargetList[i].GetHead().GetId();
			target_head_islabel = TargetList[i].GetHead().GetIsLabel();
			target_tail_id = TargetList[i].GetTail().GetId();
			target_tail_islabel = TargetList[i].GetTail().GetIsLabel();
			if (IsWildCardEntity(target_tail_id, target_tail_islabel)){ //tailが指定なしのとき
				for (iter_i_ii=cl.begin(), end_i_ii=cl.end(); iter_i_ii!=end_i_ii; ++iter_i_ii) CalcEntityLabel.insert(iter_i_ii->first);
				break;
			}
			else { //tailが指定ありのとき
				if (target_tail_islabel){ //tailがラベルのとき
					if (cl.find(target_tail_id)!=cl.end()) CalcEntityLabel.insert(target_tail_id);
				}
				else { //tailがエンティティのとき
					std::unordered_set<int> labellist = EntityList[target_tail_id].GetLabel();
					for (iter_sei1=labellist.begin(), end_sei1=labellist.end(); iter_sei1!=end_sei1; ++iter_sei1){
						if (cl.find(*iter_sei1)!=cl.end()) CalcEntityLabel.insert(*iter_sei1);
					}
				}
			}
		}
	}

	//SumsqAveの計算
	if (!prunable[0] or !prunable[1]){ //エンティティラベル，固有エンティティのいずれかが枝刈り不可能
		for(iter_sei1=CalcLinkLabel.begin(), end_sei1=CalcLinkLabel.end(); iter_sei1!=end_sei1; ++iter_sei1){ //着目中のpatternentityと結合している各リンクラベルでターゲットにマッチするもの
			if (!prunable[0]){ //エンティティラベルに関する例外が枝刈り不可能
				std::unordered_map<int, int> cle_r = cle[*iter_sei1]; //rによって結合するラベルの集計
				sum = 0.0; //SumsqAve_label[r]の格納
				for (iter_ii=cle_r.begin(), end_ii=cle_r.end(); iter_ii!=end_ii; ++iter_ii){
					if (iter_ii->first!=0) sum += (double)iter_ii->second;
					else sum += weight * (double)iter_ii->second;
				}
				CountSum_label[*iter_sei1] = sum;
			}

			if (!prunable[1]){ //固有エンティティに関する例外が枝刈り不可能
				std::unordered_map<int, int> cue_r = cue[*iter_sei1]; //rによって結合するエンティティの集計
				sum = 0.0; //SumsqAve_unique[r]の格納
				for (iter_ii=cue_r.begin(), end_ii=cue_r.end(); iter_ii!=end_ii; ++iter_ii) sum += (double)iter_ii->second;
				CountSum_unique[*iter_sei1] = sum;
			}
		}
	}
	if (!prunable[2]){ //リンクに関する例外が枝刈り不可能
		for(iter_sei1=CalcEntityLabel.begin(), end_sei1=CalcEntityLabel.end(); iter_sei1!=end_sei1; ++iter_sei1){ //着目中のpatternentityの結合している各エンティティラベル
			std::unordered_map<int, int> cl_r = cl[*iter_sei1]; //lとの間のリンクの集計
			sum = 0.0; //SumsqAve_linkの格納
			for (iter_ii=cl_r.begin(), end_ii=cl_r.end(); iter_ii!=end_ii; ++iter_ii){ //SumsqAve_linkの格納
				if (iter_ii->first!=0) sum += (double)iter_ii->second;
				else sum += weight * (double)iter_ii->second;
			}
			CountSum_link[*iter_sei1] = sum;
		}
	}
	
	//例外スコアの計算
	std::vector<int> idx_p(3);
	idx_p[0] = PatternSize;
	idx_p[1] = p_idx;
	idx_p[2] = pe_idx;
	for (iter_sei1=me.begin(), end_sei1=me.end(); iter_sei1!=end_sei1; ++iter_sei1){ //各当てはまるエンティティを走査
		//トリプルの例外スコア
		std::unordered_map<int, std::vector<int> >& relate = EntityList[*iter_sei1].GetRelation();
		for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){ //各隣接要素を走査
			for (int i=0, size_i=iter_ivi->second.size(); i<size_i; ++i){
				if (!ExistMatchingTarget(EntityList[*iter_sei1], iter_ivi->first, EntityList[iter_ivi->second[i]])) continue; //走査中のトリプルがマッチするターゲットがない場合スキップ
				score = 0;
				if (!prunable[0]){ //エンティティラベルに関する例外の枝刈が不可能なとき
					for (iter_sei2=EntityList[iter_ivi->second[i]].GetLabel().begin(), end_sei2=EntityList[iter_ivi->second[i]].GetLabel().end(); iter_sei2!=end_sei2; ++iter_sei2){
						//ScoreCalcNum[NowQuery][PatternSize][8]++;
						tmpscore = 1 - (double)cle[iter_ivi->first][*iter_sei2] / CountSum_label[iter_ivi->first];
						if (tmpscore > score){
							ee_label = *iter_sei2;
							kind = 0;
							score = tmpscore;
						}
					}
				}
				if (!prunable[1]){ //固有エンティティに関する例外の枝刈が不可能なとき
					tmpscore = 1 - (double)cue[iter_ivi->first][iter_ivi->second[i]] / CountSum_unique[iter_ivi->first];
					if (tmpscore > score){
						ee_label = iter_ivi->second[i];
						kind = 1;
						score = tmpscore;
					}
				}
				if (!prunable[2]){ //リンクに関する例外の枝刈が不可能な鳥
					for (iter_sei2=EntityList[iter_ivi->second[i]].GetLabel().begin(), end_sei2=EntityList[iter_ivi->second[i]].GetLabel().end(); iter_sei2!=end_sei2; ++iter_sei2){
						tmpscore = 1 - cl[*iter_sei2][iter_ivi->first] / CountSum_link[*iter_sei2];
						if (tmpscore > score){
							ee_label = *iter_sei2;
							kind = 2;
							score = tmpscore;
						}
					}
				}
				if (score!=0){
					if (kind==0) es.SetExpScore(idx_p, *iter_sei1, iter_ivi->first, iter_ivi->second[i], ee_label, score, kind);
					else if (kind==1) es.SetExpScore(idx_p, *iter_sei1, iter_ivi->first, iter_ivi->second[i], ee_label, score, kind);
					else es.SetExpScore(idx_p, *iter_sei1, iter_ivi->first, iter_ivi->second[i], ee_label, score, kind);
					update_er = UpdateRanking(k, es);
					return_bool = return_bool || update_er;
				}
			}
		}
	}
	return return_bool;
}

//ワイルドカードで切り取ったパターンを返す
std::vector<int> graph::AddCutOffPattern(int genre_idx){
	int PopElm, MaxGroupSize, MaximizeGroupIdx;
	pattern add_pattern;
	std::vector<int> return_idx(2);
	std::queue<int> SearchQueue; //走査のためのエンティティIDキュー
	std::vector<std::unordered_set<int> > LinkingGroupList; //連結状況を表したセット（各セットごとでエンティティが連結している）
	std::unordered_set<int> UnsearchedSet; //まだ走査していないエンティティIDのリスト
	std::vector<patternentity>& pel = ContextList[genre_idx].GetPatternentityList();
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;

	//UnsearchedSetをセット
	for (int i=0, size_i=pel.size(); i<size_i; ++i){
		if (!IsWildCardEntity(pel[i].GetId(), pel[i].GetIsLabel())){ //ワイルドカードでなければ追加
			UnsearchedSet.insert(i);
		}
	}

	//パターンエンティティを連結しているもの同士でグルーピング（LinkingGroupListに追加）
	while(!UnsearchedSet.empty()){ //全て走査するまで繰り返す
		LinkingGroupList.resize(LinkingGroupList.size()+1);
		PopElm = *UnsearchedSet.begin(); //UnsearchedSetの先頭をポップして
		UnsearchedSet.erase(UnsearchedSet.begin(), std::next(UnsearchedSet.begin()));
		SearchQueue.push(PopElm); //SearchQueueにプッシュ
		while(!SearchQueue.empty()){ //走査キューが空になるまで繰り返す（SearchingElmと連結しているものは全部追加されていく）
			PopElm = SearchQueue.front(); //SearchQueueの先頭をポップして
			SearchQueue.pop();
			LinkingGroupList[LinkingGroupList.size()-1].insert(PopElm); //LikingListの最後尾にプッシュ
			std::unordered_map<int, std::vector<int> >& relate = pel[PopElm].GetRelation();
			for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){ //PopElmの隣接エンティティを見る
				if (IsWildCardLink(iter_ivi->first)) continue; //リンクがワイルドカードならスルー
				for (int i=0, size_i=iter_ivi->second.size(); i<size_i; ++i){
					if (!IsWildCardEntity(pel[iter_ivi->second[i]].GetId(), pel[iter_ivi->second[i]].GetIsLabel()) && UnsearchedSet.find(iter_ivi->second[i])!=UnsearchedSet.end()){ //隣接エンティティがワイルドカードでなくかつ未走査なら
						UnsearchedSet.erase(iter_ivi->second[i]); //UnsearchedSetから隣接エンティティをポップして
						SearchQueue.push(iter_ivi->second[i]); //SearchQueueにプッシュ
						LinkingGroupList[LinkingGroupList.size()-1].insert(iter_ivi->second[i]); //LinkingGroupListの最後尾にプッシュ
					}
				}
			}
		}
	}

	//各グループの中で一番パターンエンティティが多いものを残す（それ以外は全てワイルドカードにする）
	add_pattern = ContextList[genre_idx];
	MaxGroupSize = 0;
	MaximizeGroupIdx = 0;
	if (LinkingGroupList.size()!=0){ //ワイルドカードでないエンティティがあるとき，一番パターンエンティティが多いLinkingGroupListの要素を探す
		for (int i=0, size_i=LinkingGroupList.size(); i<size_i; ++i){
			if (MaxGroupSize < LinkingGroupList[i].size()){
				MaxGroupSize = LinkingGroupList[i].size();
				MaximizeGroupIdx = i;
			}
		}
	}
	else LinkingGroupList.resize(1); //全部ワイルドカードのとき，とりあえずLinkingGroupListに空の要素を1つ追加
	for (int i=0, size_i=pel.size(); i<size_i; ++i){ //各パターンエンティティを走査
		if (LinkingGroupList[MaximizeGroupIdx].find(i)==LinkingGroupList[MaximizeGroupIdx].end()){ //headが最大の連結グループに存在しなければ
			add_pattern.ClearEntity(i); //headをワイルドカードにする
		}
	}

	//パターンの追加
	return_idx[0] = MaxGroupSize;
	std::string pid = add_pattern.GetPatternID();
	if (PatternIndex[MaxGroupSize].find(pid)==PatternIndex[MaxGroupSize].end()){ //パターンが存在していなければ
		if (add_pattern.GetWildCardNum()==0) add_pattern.ChangeMatchingObjective(); //ワイルドカードがなければマッチング対象のパターンとする
		AddPattern(MaxGroupSize, add_pattern); //パターンを追加
		return_idx[1] = PatternList[MaxGroupSize].size()-1;
	}
	else{ //パターンが存在していれば
		return_idx[1] = PatternIndex[MaxGroupSize][pid];
		if (add_pattern.GetWildCardNum()==0) PatternList[MaxGroupSize][return_idx[1]].ChangeMatchingObjective(); //ワイルドカードがなければマッチング対象のパターンとする
		PatternList[MaxGroupSize][return_idx[1]].ChangeStructure(add_pattern); //パターン構造を新たに生成したパターンに変更
	}
	return return_idx;
}

//AddCutOffPattern()で追加したパターンの子を追加していくことで，マッチング対象のパターンを列挙
//genre_idxのコンテキストの情報を基に，指定の切り取りパターンから子を追加していく
void graph::AddPatternToMatch(int genre_idx, int PatternSize, int p_idx){
	bool InstancizeInLoop, HeadIsWildCard, LinkIsWildCard, TailIsWildCard;
	std::vector<std::unordered_set<int> > SearchPattern(PatternList.size()); //探索するパターン
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;
	std::unordered_set<int>::iterator iter_sei, end_sei;
	SearchPattern[PatternSize].insert(p_idx); //入力されたパターンを探索するパターンとして追加

	//入力されたパターンにトリプルを継ぎ足していく
	if (PatternSize == 0){ //全ての要素がワイルドカードであれば，一つのパターンエンティティのみをインスタンス化
		InstancizeContextTriple(genre_idx, PatternSize, p_idx, 0, 0, 0, true, false, false, SearchPattern);
		SearchPattern[PatternSize].clear();
		PatternSize++;
	}
	while (PatternSize < PatternList.size()){ //PatternListの最下層中のパターンを除いて探索
		for (iter_sei=SearchPattern[PatternSize].begin(), end_sei=SearchPattern[PatternSize].end(); iter_sei!=end_sei; ++iter_sei){ //各探索パターン
			InstancizeInLoop = false;
			std::vector<patternentity>& pel = PatternList[PatternSize][*iter_sei].GetPatternentityList();
			for (int j=0, size_j=pel.size(); j<size_j; ++j){ //各patternentity（トリプルのhead）
				HeadIsWildCard = IsWildCardEntity(pel[j].GetId(), pel[j].GetIsLabel()); //headのpatternentityがワイルドカードかどうか
				if (HeadIsWildCard) continue; //headがワイルドカードならスキップ
				std::unordered_map<int, std::vector<int> >& relate = pel[j].GetRelation();
				for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){ //patternentityに接続する各リンクラベル（トリプルのlink）
					LinkIsWildCard = IsWildCardLink(iter_ivi->first); //そのリンクがワイルドカードかどうか
					for (int h=0, size_h=iter_ivi->second.size(); h<size_h; ++h){ //そのリンクラベルによって接続する各patternentity（トリプルのtail）
						TailIsWildCard = IsWildCardEntity(pel[iter_ivi->second[h]].GetId(), pel[iter_ivi->second[h]].GetIsLabel()); //tailのpatternentityがワイルドカードかどうか
						if(LinkIsWildCard || TailIsWildCard){ //走査中のトリプルのどれかがワイルドカードのとき
							InstancizeContextTriple(genre_idx, PatternSize, *iter_sei, j, iter_ivi->first, iter_ivi->second[h], false, LinkIsWildCard, TailIsWildCard, SearchPattern); //知識グラフの情報を持ちいてトリプル単位でワイルドカードをインスタンス化
							InstancizeInLoop = true;
							break;
						}
					}
					if (InstancizeInLoop) break;
				}
				if (InstancizeInLoop) break;
			}
		}
		SearchPattern[PatternSize].clear(); //探索し終えたので削除
		PatternSize++; //探索先のPatternList階層を一つ下に
	}
}

//親パターンを追加
void graph::AddParent(int PatternSize, int p_idx){
	bool IsLeaf;
	std::string pid;
	std::vector<patternentity>& pel = PatternList[PatternSize][p_idx].GetPatternentityList();
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;
	
	//パターン中の葉となるエンティティを削除して得られる全てのパターンを格納
	for (int i=0, size_i=pel.size(); i<size_i; ++i){ //各patternentity
		if (IsWildCardEntity(pel[i].GetId(), pel[i].GetIsLabel())) continue; //ワイルドカードであればスキップ
		IsLeaf = PatternList[PatternSize][p_idx].GetDeg(i)<=1; //探索中のpatternentityが葉かどうか判定
		if (IsLeaf){ //探索中のpatternentityが葉であれば，それを削除したパターンを追加
			pattern add_pattern; //追加するパターン
			add_pattern = PatternList[PatternSize][p_idx];
			add_pattern.ClearEntity(i); //エンティティを削除
			pid = add_pattern.GetPatternID(); //パターンID
			if (PatternIndex[PatternSize-1].find(pid)==PatternIndex[PatternSize-1].end()){ //パターンが存在しなければ
				AddPattern(PatternSize-1, add_pattern); //パターンを追加
				PatternList[PatternSize-1][PatternList[PatternSize-1].size()-1].AddChild(p_idx); //元のパターンを子として追加
			}
			else{ //パターンが存在していれば
				PatternList[PatternSize-1][PatternIndex[PatternSize-1][pid]].AddChild(p_idx); //元のパターンを子として追加
			}
		}
	}
}

//指定のパターンを親として決定
void graph::DetermineParent(int PatternSize, int p_idx, std::unordered_map<int, bool>& RemainParent, int& SearchedChildNum, std::unordered_map<int, std::unordered_set<int> >& Parents){
	std::unordered_set<int> determine_children, search_children;
	std::unordered_set<int>::iterator iter_sei, end_sei, iter_sei_2, end_sei_2;
	
	RemainParent[p_idx] = true; //指定のパターンを残すパターンとする
	determine_children = PatternList[PatternSize][p_idx].GetChildren();
	for (iter_sei=determine_children.begin(), end_sei=determine_children.end(); iter_sei!=end_sei; ++iter_sei){ //指定のパターンの各子
		SearchedChildNum++;
		for (iter_sei_2=Parents[*iter_sei].begin(), end_sei_2=Parents[*iter_sei].end(); iter_sei_2!=end_sei_2; ++iter_sei_2){ //走査中の子パターンの親
			if (*iter_sei_2!=p_idx){ //指定の親パターン以外を走査
				PatternList[PatternSize][*iter_sei_2].DeleteChild(*iter_sei); //指定のパターンと同じ子を持てば，親子関係を解消（複数のパターンが同じ子を持たない）
			}			
		}
	}
}

//必要な親パターン以外を削除
void graph::DeleteParent(int PatternSize){
	int tmp_childnum, max_childnum, maximize_parent;
	std::unordered_map<int, bool> RemainParent; //残す親パターンかどうか
	std::unordered_map<int, std::unordered_set<int> > Parents; //各子の親パターン集合
	std::unordered_set<int> tmp_children;
	int SearchedChildNum = 0; //探索した子パターンの数
	int NeadSearchChildNum = 0; //探索する必要のある子パターンの数
	std::unordered_map<int, bool>::iterator iter_ib, end_ib;
	std::unordered_set<int>::iterator iter_sei, end_sei;
	
	//RemainParent,SearchedChildの初期化（削除されていないパターンに対してfalseを設定）
	for (int i=0, size_i=PatternList[PatternSize].size(); i<size_i; ++i){
		if (!PatternList[PatternSize][i].GetPatternentityList().empty()) RemainParent[i] = false;
	}
	for (int i=0, size_i=PatternList[PatternSize+1].size(); i<size_i; ++i){
		if (!PatternList[PatternSize+1][i].GetPatternentityList().empty()) NeadSearchChildNum++;
	}
		
	//各子の親パターンを集計
	for (iter_ib=RemainParent.begin(), end_ib=RemainParent.end(); iter_ib!=end_ib; ++iter_ib){
		tmp_children = PatternList[PatternSize][iter_ib->first].GetChildren();
		for (iter_sei=tmp_children.begin(), end_sei=tmp_children.end(); iter_sei!=end_sei; ++iter_sei){
			Parents[*iter_sei].insert(iter_ib->first);
		}
	}

	//マッチング対象の全てのパターンをRemainParentに追加
	for (iter_ib=RemainParent.begin(), end_ib=RemainParent.end(); iter_ib!=end_ib; ++iter_ib){
		if (PatternList[PatternSize][iter_ib->first].GetMatchingObjective()){
			DetermineParent(PatternSize, iter_ib->first, RemainParent, SearchedChildNum, Parents);
		}
	}
	
	//子パターンを全て探索するまで，子パターンの数によるgreedy法で親パターンを残す
	while (SearchedChildNum!=NeadSearchChildNum){
		max_childnum = 0; //最も多い子の数
		maximize_parent = 0; //子の数が最も多くなるパターン
		for (iter_ib=RemainParent.begin(), end_ib=RemainParent.end(); iter_ib!=end_ib; ++iter_ib){
			if (iter_ib->second) continue; //すでに残留が決定したパターンはスキップ
			tmp_childnum = PatternList[PatternSize][iter_ib->first].GetChildren().size();
			if (tmp_childnum > max_childnum){ //max_childnumの更新
				max_childnum = tmp_childnum;
				maximize_parent = iter_ib->first;
			}
		}
		DetermineParent(PatternSize, maximize_parent, RemainParent, SearchedChildNum, Parents); //最も子が多いパターンを残す
	}
	
	//RemainParentにないパターンを削除
	for (iter_ib=RemainParent.begin(), end_ib=RemainParent.end(); iter_ib!=end_ib; ++iter_ib){
		if (!iter_ib->second){ //残さないと決定していれば
			PatternList[PatternSize][iter_ib->first].ClearPattern(true); //パターンを削除
			if (PatternSize!=0){ //値パターンでなければ
				for (int j=0, size_j=PatternList[PatternSize-1].size(); j<size_j; ++j){ //先ほど削除したパターンを子から削除
					if (PatternList[PatternSize-1][j].GetPatternentityList().empty()) continue;
					PatternList[PatternSize-1][j].DeleteChild(iter_ib->first);
				}
			}
		}
	}
}

//クエリ読み込み
void graph::ReadQuery(std::string TargetFileName, std::string ContextFileName){
	int WildCardNum, MaxEntityNum, genre_id;
	bool genre_kind;
	Document doc_target, doc_genre;
	patternentity pe;

	//ターゲット情報を読み込む
	std::ifstream ifs_target(TargetFileName);
	IStreamWrapper isw_target(ifs_target);
	doc_target.ParseStream(isw_target);
	ifs_target.close();
	TargetList.resize(doc_target.Size());
	for (int i=0, size_i=doc_target.Size(); i<size_i; ++i){
		pe.ModifyEntity(0, doc_target[i]["HeadID"].GetInt(), doc_target[i]["HeadKind"].GetBool());
		TargetList[i].ModifyHead(pe);
		TargetList[i].ModifyLink(doc_target[i]["LinkID"].GetInt());
		pe.ModifyEntity(0, doc_target[i]["TailID"].GetInt(), doc_target[i]["TailKind"].GetBool());
		TargetList[i].ModifyTail(pe);
	}

	//コンテキスト情報を読み込む
	std::ifstream ifs_genre(ContextFileName);
	IStreamWrapper isw_genre(ifs_genre);
	doc_genre.ParseStream(isw_genre);
	ifs_genre.close();
	ContextList.resize(doc_genre.Size());
	MaxEntityNum = 0;
	for (int i=0, size_i=doc_genre.Size(); i<size_i; ++i){ //コンテキストを格納
		WildCardNum = 0;
		for (int j=0, size_j=doc_genre[i]["Entity"].Size(); j<size_j; ++j){ //各コンテキストのエンティティの処理
			genre_id = doc_genre[i]["Entity"][j]["EntityID"].GetInt();
			genre_kind = doc_genre[i]["Entity"][j]["EntityKind"].GetBool();
			ContextList[i].AddEntity(genre_id, genre_kind);
			if (IsWildCardEntity(genre_id, genre_kind)) WildCardNum++;
		}
		for (int j=0, size_j=doc_genre[i]["Link"].Size(); j<size_j; ++j){ //各コンテキストのリンクの処理
			ContextList[i].AddLink(doc_genre[i]["Link"][j]["HeadID"].GetInt(), doc_genre[i]["Link"][j]["LinkID"].GetInt(), doc_genre[i]["Link"][j]["TailID"].GetInt());
			if (IsWildCardLink(doc_genre[i]["Link"][j]["LinkID"].GetInt())) WildCardNum++;
		}
		ContextList[i].SetWildCardNum(WildCardNum); //WildCardNumをセット
		if (doc_genre[i]["Entity"].Size() > MaxEntityNum) MaxEntityNum = doc_genre[i]["Entity"].Size(); //最大リンク数を更新
	}
	PatternList.resize(MaxEntityNum+1); //MaxEntityNumがPatternList,PatternIndexの最大インデックス値になるようにリサイズ
	PatternIndex.resize(MaxEntityNum+1);
}

//パターン探索空間を作成
void graph::MakePatternList(int search_mode){
	for (int i=0, size_i=ContextList.size(); i<size_i; ++i){
		std::vector<int> CutOffPatternIdx = AddCutOffPattern(i);
		if (PatternList[CutOffPatternIdx[0]][CutOffPatternIdx[1]].GetWildCardNum()!=0) AddPatternToMatch(i, CutOffPatternIdx[0], CutOffPatternIdx[1]);
	}
	if (search_mode!=0) return; //全探索の場合親パターンを格納する必要がないため終了
	for (int i=PatternList.size()-1; i>0; --i){
		for (int j=0, size_j=PatternList[i].size(); j<size_j; ++j){
			AddParent(i, j);
		}
		//DeleteParent(i-1); //（親削除）
	}
}

//パターンを探索しながらtop-kの例外サブグラフを格納（枝刈りをする効率的な探索方法）
void graph::DFSforPatternList(int PatternSize, int p_idx, int k, std::string& score_kind, std::vector<std::unordered_map<int, std::unordered_set<int> > >& Parents){
	int tmp_grandchild_num, max_grandchild_num, maximize_child;
	double minscore;
	bool exist_matching, clear_pattern, update_er, all_descendants_searched, matching_objective;
	std::vector<std::pair<int, int> > GrandchildNum;
	std::unordered_set<int> children;
	std::unordered_set<int>::iterator iter_sei, end_sei;
	clear_pattern = true; //探索中のパターンをclearするか
	
	if (PatternSize!=0){ //探索中のパターンが根でなければ，マッチング・枝刈りの試行・例外スコア計算を行う
		TimeTime("05-" + std::to_string(PatternSize) + ". Search Pattern (extra)");
		exist_matching = PatternMatching(PatternSize, p_idx); //パターンマッチング
		TimeTime("05-" + std::to_string(PatternSize) + ". Search Pattern (matching)");
		if (!exist_matching){ //マッチするエンティティがなければ終了
			FinishPatternSearch(PatternSize, p_idx, true, Parents[PatternSize][p_idx]);
			return;
		}
		//else ExistMatchPatternNum[NowQuery][PatternSize]++;
		if (ExpscoreNum<k) minscore = -1;
		else minscore = ExpRanking.begin()->first;
		PatternList[PatternSize][p_idx].Pruning(score_kind, minscore); //パターンの枝刈りを試行
		if (PatternList[PatternSize][p_idx].GetPrunable()){ //枝刈り可能であれば，探索済みにし終了
			FinishPatternSearch(PatternSize, p_idx, true, Parents[PatternSize][p_idx]);
			return;
		}
		if (PatternList[PatternSize][p_idx].GetMatchingObjective()){ //マッチング対象のパターであれば
			std::vector<patternentity>& pel = PatternList[PatternSize][p_idx].GetPatternentityList();
			TimeTime("05-" + std::to_string(PatternSize) + ". Search Pattern (extra)");
			for (int i=0, size_i=pel.size(); i<size_i; ++i){
				if (!IsWildCardEntity(pel[i].GetId(), pel[i].GetIsLabel()) && pel[i].GetIsLabel() && !pel[i].GetPrunable()[3]){ //各patternentityに対し，ワイルドカードでなくかつラベルであり，かつ枝刈り不可能であれば，それにマッチするエンティティの例外スコアを計算
					if (score_kind=="dist") update_er = CalcExpScore_dist(k, PatternSize, p_idx, i);
					else if (score_kind=="freq") update_er = CalcExpScore_freq(k, PatternSize, p_idx, i);
					clear_pattern = clear_pattern && !update_er;
				}
			}
			TimeTime("05-" + std::to_string(PatternSize) + ". Search Pattern (calculate)");
		}
	}
	
	else{ //探索中のパターンが根ならば，Parentsを集計
		for (int i=0, size_i=PatternList.size(); i<size_i; ++i){
			for (int j=0, size_j=PatternList[i].size(); j<size_j; ++j){
				children = PatternList[i][j].GetChildren();
				for (iter_sei=children.begin(), end_sei=children.end(); iter_sei!=end_sei; ++iter_sei) Parents[i+1][*iter_sei].insert(j);
			}
		}
	}
	
	children = PatternList[PatternSize][p_idx].GetChildren();
	while(!children.empty()){ //子があればその子を探索
		max_grandchild_num = 0;
		maximize_child = *children.begin();
		for (iter_sei=children.begin(), end_sei=children.end(); iter_sei!=end_sei; ++iter_sei){ //孫の数を集計
			tmp_grandchild_num = PatternList[PatternSize+1][*iter_sei].GetChildren().size();
			if (tmp_grandchild_num > max_grandchild_num){
				max_grandchild_num = tmp_grandchild_num;
				maximize_child = *iter_sei;
			}
		}
		children.erase(maximize_child);
		all_descendants_searched = GetAllDescendantsSearched(PatternSize+1, maximize_child); //子パターンの全ての子孫が探索済みかどうか
		matching_objective = PatternList[PatternSize+1][maximize_child].GetMatchingObjective(); //子パターンがマッチング対象のパターンかどうか	
		if (!all_descendants_searched || matching_objective){ //子パターンの子孫のいずれかが探索済みでないかそのパターンがマッチング対象のパターンであれば，子パターンを同様に探索
			TimeTime("05-" + std::to_string(PatternSize) + ". Search Pattern (extra)");
			DFSforPatternList(PatternSize+1, maximize_child, k, score_kind, Parents);
		}
	}
	FinishPatternSearch(PatternSize, p_idx, clear_pattern, Parents[PatternSize][p_idx]);
}

//パターンを探索しながらtop-kの例外サブグラフを格納（全探索）
void graph::AllPatternSearch(int k, std::string& score_kind){
	bool exist_matching, clear_pattern, update_er;
	
	for (int i=0, size_i=PatternList.size(); i<size_i; ++i){
		for (int j=0, size_j=PatternList[i].size(); j<size_j; ++j){
			if (!PatternList[i][j].GetMatchingObjective()){ //マッチング対象のパターンでなければスキップ
				PatternList[i][j].ClearPattern(true);
				continue;
			}
			clear_pattern = true; //探索中のパターンをクリアするか
			TimeTime("05-" + std::to_string(i) + ". Search Pattern (extra)");
			exist_matching = PatternMatching(i, j);
			TimeTime("05-" + std::to_string(i) + ". Search Pattern (matching)");
			if (!exist_matching) continue; //マッチするエンティティがなければスキップ
			//else ExistMatchPatternNum[NowQuery][i]++;
			std::vector<patternentity>& pel = PatternList[i][j].GetPatternentityList();
			TimeTime("05-" + std::to_string(i) + ". Search Pattern (extra)");
			
			for (int h=0, size_h=pel.size(); h<size_h; ++h){ //各patternentityに対し，ワイルドカードでなくかつラベルであれば，それにマッチするエンティティの例外スコアを計算
				if (!IsWildCardEntity(pel[h].GetId(), pel[h].GetIsLabel()) && pel[h].GetIsLabel()){
					if (score_kind=="dist") update_er = CalcExpScore_dist(k, i, j, h);
					else if (score_kind=="freq") update_er = CalcExpScore_freq(k, i, j, h);
					clear_pattern = clear_pattern && !update_er;
				}
			}
			TimeTime("05-" + std::to_string(i) + ". Search Pattern (calculate)");
			if (clear_pattern) PatternList[i][j].ClearPattern(true);
		}
	TimeTime("05-" + std::to_string(i) + ". Search Pattern (extra)");
	}
}

//パターン数調査
void graph::CountPattern(){
	for (int i=0, size_i=PatternList.size(); i<size_i; ++i){
		for (int j=0, size_j=PatternList[i].size(); j<size_j; ++j){
			if (PatternList[i][j].GetMatchingObjective()) SearchedPatternNum[NowQuery][i]++; //探索済みのパターンとしてカウント
		}
		TimeTime("05-" + std::to_string(i) + ". Search Pattern");
	}
}
	
//一部グラフの要素をリセット
void graph::ClearGraphElm(){
	TargetList.clear();
	ContextList.clear();
	PatternList.clear();
	PatternIndex.clear();
	ExpRanking.clear();
	ExpscoreNum = 0;
}

//グラフを読み込む
void graph::ReadGraph(std::string entityfile, std::string labelfile, std::string linkfile){
	int id, RelationNum;
	std::string name;
	Document doc_entity, doc_label, doc_link;
	std::unordered_set<int> label; //引数として渡すラベルの配列

	//エンティティファイルの処理
	std::ifstream ifs_entity(entityfile);
	IStreamWrapper isw_entity(ifs_entity);
	doc_entity.ParseStream(isw_entity);
	ifs_entity.close();
	EntityList.resize(doc_entity.Size()+1);
	RelationNum = 0;
	std::cout << "   RG entity num: " << doc_entity.Size() << std::endl;
	for (int i=0, size_i=doc_entity.Size(); i<size_i; ++i){ //エンティティの追加
		label.clear();
		for (int j=0, size_j=doc_entity[i]["Label"].Size(); j<size_j; ++j) label.insert(doc_entity[i]["Label"][j].GetInt());
		EntityList[doc_entity[i]["ID"].GetInt()].ModifyEntity(doc_entity[i]["ID"].GetInt(), doc_entity[i]["Name"].GetString(), label);
	}
	for (int i=0, size_i=doc_entity.Size(); i<size_i; ++i){ //リンクの追加
		for (int j=0, size_j=doc_entity[i]["Relation"].Size(); j<size_j; ++j){
			AddLink(doc_entity[i]["ID"].GetInt(), doc_entity[i]["Relation"][j]["Link"].GetInt(), doc_entity[i]["Relation"][j]["Tail"].GetInt());
			RelationNum++;
		}
	}
	std::cout << "   RG relation num: " << RelationNum << std::endl;

	//ラベルファイルの処理
	std::ifstream ifs_label(labelfile);
	IStreamWrapper isw_label(ifs_label);
	doc_label.ParseStream(isw_label);
	ifs_label.close();
	LabelList.resize(doc_label.Size()+1);
	std::cout << "   RG label num: " << doc_label.Size() << std::endl;
	for (int i=0, size_i=doc_label.Size(); i<size_i; ++i) LabelList[doc_label[i]["ID"].GetInt()] = doc_label[i]["Name"].GetString();

	//リンクファイルの処理
	std::ifstream ifs_link(linkfile);
	IStreamWrapper isw_link(ifs_link);
	doc_link.ParseStream(isw_link);
	ifs_link.close();
	LinkList.resize(doc_link.Size()+1);
	std::cout << "   RG link num: " << doc_link.Size() << std::endl;
	for (int i=0, size_i=doc_link.Size(); i<size_i; ++i) LinkList[doc_link[i]["ID"].GetInt()] = doc_link[i]["Name"].GetString();

	//空のエンティティ，ラベル，リンク情報を追加
	label.clear();
	label.insert(0);
	EntityList[0].ModifyEntity(0, "_NoEntity_", label);
	LabelList[0] = "_NoLabel_";
	LinkList[0] = "_NoLink_";
	
	//各データサイズを格納
	EntityNum = EntityList.size()-1;
	LabelNum = LabelList.size()-1;
	LinkNum = LinkList.size()-1;
	WildCardNum = std::max(EntityNum, LabelNum);
	WildCardNum = std::max(WildCardNum, LinkNum) + 1;
	std::cout << "   RG wildcard ≧ " << WildCardNum << std::endl;
}

//各パターンの隣接要素情報を格納
void graph::SetCombineInfo(bool FirstRead, std::string indexfile){
	int ThreadNum = 2;
	//int ThreadNum = omp_get_max_threads()/4;
	//std::cout << "Using ThreadNum: " << ThreadNum << std::endl;
	std::vector<std::vector<int> > ThreadAlloc(ThreadNum);
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;
	std::unordered_map<int, std::unordered_map<int, int> >::iterator iter_i_ii, end_i_ii;
	std::unordered_map<int, int>::iterator iter_ii, end_ii;
	std::unordered_set<int>::iterator iter_sei, end_sei;

	//パターンそのものとマッチするエンティティの追加
	PatternUnitList.resize(LabelList.size() + EntityList.size() - 1);
	for (int i=1, size_i=LabelList.size(); i<size_i; ++i) PatternUnitList[i].ModifyEntity(0, i, true); //パターンユニットそのものの追加(ラベル)
	for (int i=1, size_i=EntityList.size(); i<size_i; ++i){
		PatternUnitList[LabelList.size()+i-1].ModifyEntity(0, i, false); //パターンユニットそのものの追加(固有エンティティ)
		PatternUnitList[LabelList.size()+i-1].AddMatchEntity(i); //マッチするエンティティの追加（固有エンティティ）
		for (iter_sei=EntityList[i].GetLabel().begin(), end_sei=EntityList[i].GetLabel().end(); iter_sei!=end_sei; ++iter_sei){
			PatternUnitList[*iter_sei].AddMatchEntity(i); //マッチするエンティティの追加（ラベル）
		}
	}

	//隣接するリンクごとに分類したマッチエンティティを追加
	for (int i=1, size_i=PatternUnitList.size(); i<size_i; ++i){
		std::unordered_set<int> me = PatternUnitList[i].GetMatchEntity();
		for (iter_sei=me.begin(), end_sei=me.end(); iter_sei!=end_sei; ++iter_sei){
			std::unordered_map<int, std::vector<int> >& relate = EntityList[*iter_sei].GetRelation();
			for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){
				PatternUnitList[i].AddLinkClassifiedME(iter_ivi->first, *iter_sei);
			}
		}
	}

	//隣接情報と上界スコアの追加
	if (FirstRead){ //初めての読み込みの場合
		for (int i=1, size_i=PatternUnitList.size(); i<size_i; ++i) ModifyCombineElement(-1, i, 0, true);
	}	
	//隣接情報の追加
	else{ //初めての読み込みでない場合
		//ファイル読み込み
		int key_id;
		Document doc_index;
		Value doc_dist(kArrayType);
		std::ifstream ifs_index(indexfile);
		IStreamWrapper isw_index(ifs_index);
		doc_index.ParseStream(isw_index);
		ifs_index.close();
		
		for (int i=0, size_i=doc_index.Size(); i<size_i; ++i){
			std::unordered_map<int, std::unordered_map<int, int> > cle, cue, cl;
			for (int j=0, size_j=doc_index[i]["CombineLabelEntity"].Size(); j<size_j; ++j){
				key_id = doc_index[i]["CombineLabelEntity"][j]["Link"].GetInt();
				doc_dist = doc_index[i]["CombineLabelEntity"][j]["Distribution"];
				for (int h=0, size_h=doc_dist.Size(); h<size_h; ++h){
					cle[key_id][doc_dist[h]["EntityLabel"].GetInt()] = doc_dist[h]["Count"].GetInt();
				}
			}
			PatternUnitList[i+1].ModifyCombineElement(cle, 0);
			for (int j=0, size_j=doc_index[i]["CombineEntity"].Size(); j<size_j; ++j){
				key_id = doc_index[i]["CombineEntity"][j]["Link"].GetInt();
				doc_dist = doc_index[i]["CombineEntity"][j]["Distribution"];
				for (int h=0, size_h=doc_dist.Size(); h<size_h; ++h){
					cue[key_id][doc_dist[h]["Entity"].GetInt()] = doc_dist[h]["Count"].GetInt();
				}
			}
			PatternUnitList[i+1].ModifyCombineElement(cue, 1);
			for (int j=0, size_j=doc_index[i]["CombineLink"].Size(); j<size_j; ++j){
				key_id = doc_index[i]["CombineLink"][j]["EntityLabel"].GetInt();
				doc_dist = doc_index[i]["CombineLink"][j]["Distribution"];
				for (int h=0, size_h=doc_dist.Size(); h<size_h; ++h){
					cl[key_id][doc_dist[h]["Link"].GetInt()] = doc_dist[h]["Count"].GetInt();
				}
			}
			PatternUnitList[i+1].ModifyCombineElement(cl, 2);
		}
	}
}

//隣接要素の情報を格納したインデックスを作成
void graph::MakeIndex(std::string indexfile){
	int j, h, size_j, size_h;
	std::string FileContent = "[\n";
	std::ofstream IndexFile;
	std::unordered_map<int, std::unordered_map<int, int> > cle, cue, cl;
	std::unordered_map<int, std::unordered_map<int, int> >::iterator iter_i_ii, end_i_ii;
	std::unordered_map<int, int>::iterator iter_ii, end_ii;
	
	for (int i=1, size_i=PatternUnitList.size(); i<size_i; ++i){
		FileContent += "\t{\n\t\t\"CombineLabelEntity\":[\n";
		cle = PatternUnitList[i].GetCombineLabelEntity();
		for (iter_i_ii=cle.begin(), end_i_ii=cle.end(), j=0, size_j=cle.size(); iter_i_ii!=end_i_ii; ++iter_i_ii, ++j){
			FileContent += "\t\t\t{\"Link\":" + std::to_string(iter_i_ii->first) + ", \"Distribution\":[\n";
			for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(), h=0, size_h=iter_i_ii->second.size(); iter_ii!=end_ii; ++iter_ii, ++h){
				FileContent += "\t\t\t\t{\"EntityLabel\":" + std::to_string(iter_ii->first) + ", \"Count\":" + std::to_string(iter_ii->second) + "}";
				if (h!=size_h-1) FileContent += ",";
				FileContent += "\n";
			}
			FileContent += "\t\t\t]}";
			if (j!=size_j-1) FileContent += ",";
			FileContent += "\n";
		}
		
		FileContent += "\t\t],\n\t\t\"CombineEntity\":[\n";
		cue = PatternUnitList[i].GetCombineUniqueEntity();
		for (iter_i_ii=cue.begin(), end_i_ii=cue.end(), j=0, size_j=cue.size(); iter_i_ii!=end_i_ii; ++iter_i_ii, ++j){
			FileContent += "\t\t\t{\"Link\":" + std::to_string(iter_i_ii->first) + ", \"Distribution\":[\n";
			for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(), h=0, size_h=iter_i_ii->second.size(); iter_ii!=end_ii; ++iter_ii, ++h){
				FileContent += "\t\t\t\t{\"Entity\":" + std::to_string(iter_ii->first) + ", \"Count\":" + std::to_string(iter_ii->second) + "}";
				if (h!=size_h-1) FileContent += ",";
				FileContent += "\n";
			}
			FileContent += "\t\t\t]}";
			if (j!=size_j-1) FileContent += ",";
			FileContent += "\n";
		}
		
		FileContent += "\t\t],\n\t\t\"CombineLink\":[\n";
		cl = PatternUnitList[i].GetCombineLink();
		for (iter_i_ii=cl.begin(), end_i_ii=cl.end(), j=0, size_j=cl.size(); iter_i_ii!=end_i_ii; ++iter_i_ii, ++j){
			FileContent += "\t\t\t{\"EntityLabel\":" + std::to_string(iter_i_ii->first) + ", \"Distribution\":[\n";
			for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(), h=0, size_h=iter_i_ii->second.size(); iter_ii!=end_ii; ++iter_ii, ++h){
				FileContent += "\t\t\t\t{\"Link\":" + std::to_string(iter_ii->first) + ", \"Count\":" + std::to_string(iter_ii->second) + "}";
				if (h!=size_h-1) FileContent += ",";
				FileContent += "\n";
			}
			FileContent += "\t\t\t]}";
			if (j!=size_j-1) FileContent += ",";
			FileContent += "\n";
		}
		FileContent += "\t\t]";
		
		FileContent += "\n\t}";
		if (i!=size_i-1) FileContent += ",";
		FileContent += "\n";		
	}
	FileContent += "]";
	
	IndexFile.open(indexfile, std::ios::out);
	IndexFile << FileContent;
	IndexFile.close();
}

//グラフ情報を整理したファイルを作成
void graph::MakeSortout(std::string EntityFileName, std::string LinkFileName, std::string CombinationFileName){
	int j, h, size_j, size_h;
	std::string EntityFileContent, LinkFileContent, CombinationFileContent, inputsig;
	std::ofstream EntityFile, LinkFile, CombinationFile;
	std::unordered_map<int, std::unordered_map<int, int> > cle;
	EntityFileContent = "[\n";
	LinkFileContent = "[\n";
	CombinationFileContent = "[\n";
	std::unordered_map<int, std::unordered_map<int, int> >::iterator iter_i_ii, end_i_ii;
	std::unordered_map<int, int>::iterator iter_ii, end_ii;
	std::unordered_set<int>::iterator iter_sei, end_sei;

	//エンティティの名前とIDを，それが持つラベルごとにまとめたファイルを出力
	for (int i=1, size_i=LabelList.size(); i<size_i; ++i){
		EntityFileContent +=
			"\t{\n\t\t\"ID\":" + std::to_string(i) + ",\n" +
			"\t\t\"Name\":\"" + LabelList[i] + "\",\n" +
			"\t\t\"MatchEntity\":[\n";
		std::unordered_set<int> me = PatternUnitList[i].GetMatchEntity();
		for (iter_sei=me.begin(), end_sei=me.end(), j=0, size_j=me.size(); iter_sei!=end_sei; ++iter_sei, ++j){
			EntityFileContent += "\t\t\t{\"ID\":" + std::to_string(*iter_sei) + ", \"Name\":\"" + EntityList[*iter_sei].GetName() + "\"}";
			if (j!=size_j-1) EntityFileContent += ",\n";
			else EntityFileContent += "\n";
		}
		if (i!=size_i-1) EntityFileContent += "\t\t]\n\t},\n";
		else EntityFileContent += "\t\t]\n\t}\n";
	}
	EntityFileContent += "]";
	EntityFile.open(EntityFileName, std::ios::out);
	EntityFile << EntityFileContent;
	EntityFile.close();

	//リンクの名前とIDをファイルに出力
	for (int i=1, size_i=LinkList.size(); i<size_i; ++i){
		LinkFileContent += "\t{\"ID\":" + std::to_string(i) + ", \"Name\":\"" + LinkList[i] + "\"}";
		if (i!=size_i-1) LinkFileContent += ",\n";
		else LinkFileContent += "\n";
	}
	LinkFileContent += "]";
	LinkFile.open(LinkFileName, std::ios::out);
	LinkFile << LinkFileContent;
	LinkFile.close();

	//各エンティティラベルの隣接情報をファイルに出力
	for (int i=1, size_i=LabelList.size(); i<size_i; ++i){
		cle = PatternUnitList[i].GetCombineLabelEntity();
		CombinationFileContent +=
			"\t{\n\t\t\"ID\":" + std::to_string(i) + ",\n" +
			"\t\t\"Combination\":{\n";
		if (cle.empty()){
			CombinationFileContent += "\t\t\t\"NoLink\":[]\n\t\t}\n\t}";
		}
		else {
		for (iter_i_ii=cle.begin(), end_i_ii=cle.end(), j=0, size_j=cle.size(); iter_i_ii!=end_i_ii; ++iter_i_ii, ++j){
			CombinationFileContent += "\t\t\t\"" + LinkEncoder(iter_i_ii->first, LinkList) + "\":[\n";
			for (iter_ii=iter_i_ii->second.begin(), end_ii=iter_i_ii->second.end(), h=0, size_h=iter_i_ii->second.size(); iter_ii!=end_ii; ++iter_ii, ++h){
				if (iter_ii->first==0){
					h--;
					continue;
				}
				if (h!=0) CombinationFileContent += ",\n";
				CombinationFileContent += "\t\t\t\t{\"ID\":" + std::to_string(iter_ii->first) + ", \"Name\":\"" + LabelList[iter_ii->first] + "\"}";
			}
			CombinationFileContent += "\n\t\t\t]";
			if (j<size_j-1) CombinationFileContent += ",\n";
			else CombinationFileContent += "\n\t\t}\n\t}";
		}
		}
		if (i!=size_i-1) CombinationFileContent += ",\n";
		else CombinationFileContent += "\n]";
	}
	CombinationFile.open(CombinationFileName, std::ios::out);
	CombinationFile << CombinationFileContent;
	CombinationFile.close();
	start_detect = std::chrono::system_clock::now();
}

//例外検出
void graph::FindException(int k, std::string score_kind, int search_mode, std::map<std::string, std::string>& filepath){
	std::string query_path;
	std::vector<std::string> query_pathes;
	Document doc;
	
	std::ifstream ifs(filepath["QueriesFile"]);
	IStreamWrapper isw(ifs);
	doc.ParseStream(isw);
	ifs.close();
	for (int i=0, size_i=doc.Size(); i<size_i; ++i) query_pathes.push_back(filepath["QueryDir"] + doc[i].GetString() + "/");

	for (int i=0, size_i=query_pathes.size(); i<size_i; ++i){
		std::cout << "> Reading Query" << std::endl;
	 	NowQuery = doc[i].GetString();
	 	query_path = query_pathes[i];		
		ReadQuery(query_path+filepath["TargetFileName"], query_path+filepath["ContextFileName"]); //クエリ読み込み
		TimeTime("03. Read Query");
		
		std::cout << "> Making Pattern List" << std::endl;
		MakePatternList(search_mode); //コンテキストの指定なしの要素を展開
		TimeTime("04. Make Pattern List");

		std::cout << "> Finding Exception" << std::endl;
		SearchedPatternNum[NowQuery].resize(PatternList.size()); //探索済みのパターンを集計する配列を初期化
		for (int j=0, size_j=PatternList.size(); j<size_j; ++j) SearchedPatternNum[NowQuery][j] = 0;
		//ExistMatchPatternNum[NowQuery].resize(PatternList.size()); //マッチするエンティティが存在するパターンを集計する配列を初期化
		//for (int i=0, size_i=PatternList.size(); i<size_i; ++i) ExistMatchPatternNum[NowQuery][i] = 0;		
		if (search_mode==0){ //枝刈り探索
			std::vector<std::unordered_map<int, std::unordered_set<int> > > Parents(PatternList.size());
			DFSforPatternList(0, 0, k, score_kind, Parents); //パターンを探索しながらtop-kの例外サブグラフを格納
		}
		else if (search_mode==1) AllPatternSearch(k, score_kind); //全探索
		else CountPattern(); //パターン数の集計
		
		if (search_mode<2){
			std::cout << std::endl << "---" << NowQuery << "---" << std::endl << std::endl;
			PrintExpRanking(std::cout, true, false); //例外的事実出力
			std::cout << "\n\n\n\n\n" << std::endl;
		}
		TimeTime("");
		ClearGraphElm(); //グラフの要素をリセットする
	}
}

//グラフを表示
void graph::PrintGraph(std::ostream& os, bool PrintName, bool PrintLabelList, bool PrintLinkList){
	os << "[Entity and Link]" << std::endl;
	for (int i=1, size_i=EntityList.size(); i<size_i; ++i) EntityList[i].PrintEntity(os, true, PrintName, EntityList, LabelList, LinkList);

	if (PrintLabelList){
		os << std::endl << "[Label Name]" << std::endl;
		for (int i=1, size_i=LabelList.size(); i<size_i; ++i) os << i << ". " << LabelList[i] << std::endl;
	}

	if (PrintLinkList){
		os << std::endl << "[Link Name]" << std::endl;
		for (int i=1, size_i=LinkList.size(); i<size_i; ++i) os << i << ". " << LabelList[i] << std::endl;
	}
}

//PatternListを表示
void graph::PrintPatternList(std::ostream& os, bool PrintName, bool PrintRelation, bool PrintMatchEntity, bool PrintLinkClassifiedME, bool PrintCombineElement, bool PrintChild, bool PrintContextInfo, bool PrintExpScore, bool PrintPatternIndex){
	std::unordered_map<std::string, int>::iterator iter_pi, end_pi;

	os << "[Pattern]" << std::endl;
	for (int i=0, size_i=PatternList.size(); i<size_i; ++i){
		for (int j=0, size_j=PatternList[i].size(); j<size_j; ++j) {
			if (PatternList[i][j].GetPatternentityList().empty()) continue;
			PatternList[i][j].PrintPattern(std::cout, i, j, PrintName, PrintRelation, PrintMatchEntity, PrintLinkClassifiedME, PrintCombineElement, PrintChild, PrintContextInfo, PrintExpScore, EntityList, LabelList, LinkList);
			std::cout << std::endl;
		}
	}

	if (PrintPatternIndex){
		os << std::endl << "[Pattern Index]" << std::endl;
		for (int i=0, size_i=PatternIndex.size(); i<size_i; ++i){
			os << "size: " << i << std::endl;
			for (iter_pi=PatternIndex[i].begin(), end_pi=PatternIndex[i].end(); iter_pi!=end_pi; ++iter_pi){
				if (PatternList[i][iter_pi->second].GetPatternentityList().empty()) continue;
				os << "\t" << iter_pi->first << ": " << i << " " << iter_pi->second << std::endl;
			}
		}
	}

}

//PatternUnitListを表示
void graph::PrintPatternUnitList(std::ostream& os, bool PrintName, bool PrintRelation, bool PrintMatchEntity, bool PrintLinkClassifiedME, bool PrintCombineElement){
	for (int i=1, size_i=PatternUnitList.size(); i<size_i; ++i) {
		PatternUnitList[i].PrintPatternentity(std::cout, PrintName, PrintRelation, PrintMatchEntity, PrintLinkClassifiedME, PrintCombineElement, false, EntityList, LabelList, LinkList);
		std::cout << std::endl;
	}
}

//ContextListを表示
void graph::PrintContextList(std::ostream& os, bool PrintName, bool PrintRelation){
	for (int i=0, size_i=ContextList.size(); i<size_i; ++i) {
		ContextList[i].PrintPattern(std::cout, 0, i, PrintName, PrintRelation, false, false, false, false, true, false, EntityList, LabelList, LinkList);
		std::cout << std::endl;
	}
}

//ExpRankingを表示
void graph::PrintExpRanking(std::ostream& os, bool PrintName, bool CompareGlobalExp){
	int rank = 1;
	std::vector<int> idx_p;
	std::map<double, std::vector<expscore> >::reverse_iterator riter_er;

	for (riter_er=ExpRanking.rbegin(); riter_er!=ExpRanking.rend(); ++riter_er){
		for (int i=0; i<riter_er->second.size(); ++i){
			idx_p = riter_er->second[i].GetIdx_p();
			if (rank==1) os << "1st";
			else if(rank==2) os << "2nd";
			else if(rank==3) os << "3rd";
			else os << rank << "th";
			os << " Exception." << std::endl;
			os << "Exception Score is " << riter_er->first << std::endl << std::endl;
			PatternList[riter_er->second[i].GetIdx_p()[0]][riter_er->second[i].GetIdx_p()[1]].PrintPattern(os, riter_er->second[i].GetIdx_p()[0], riter_er->second[i].GetIdx_p()[1], PrintName, true, false, false, false, false, false, false, EntityList, LabelList, LinkList);
			os << std::endl;
			riter_er->second[i].PrintExpScore(os, PrintName, false, PatternList[idx_p[0]][idx_p[1]].GetPatternentityList()[idx_p[2]], EntityList, LabelList, LinkList);
			os << std::endl;
			if (CompareGlobalExp){ //大域例外度と比較
				GlobalExp(PrintName, riter_er->second[i]);
				os << std::endl;
			}
			os << std::endl;
		}
		rank += riter_er->second.size();
	}
}

//大域例外度合いと比較
void graph::GlobalExp(bool PrintName, expscore& es){
	int context_peidx;
	std::unordered_map<int, int> distribution;
	std::unordered_map<int, int>::iterator iter_dist, end_dist;
	std::vector<int> Idx_p = es.GetIdx_p();
	int kind = es.GetKind();
	patternentity context_pe = PatternList[Idx_p[0]][Idx_p[1]].GetPatternentityList()[Idx_p[2]]; //コンテキストに対応するパターンエンティティ

	//context_peidx: コンテキストに対応する単位パターンエンティティのインデックス値
	if (context_pe.GetIsLabel()) context_peidx = context_pe.GetId();
	else context_peidx = context_pe.GetId()+LabelList.size()-1;

	if (kind == 0){
		std::cout << "(Global) Destribution of EntityLabel to ";
		if (PrintName){
			if (context_pe.GetIsLabel()) std::cout << LabelEncoder(context_pe.GetId(), LabelList);
			else  std::cout << EntityEncoder(context_pe.GetId(), EntityList);
			std::cout << " by " << LinkEncoder(es.GetID_link(), LinkList) << ":" << std::endl;
		}
		else std::cout << context_pe.GetId() << " by " << es.GetID_link() << ":" << std::endl;
		distribution = PatternUnitList[context_peidx].GetCombineLabelEntity()[es.GetID_link()];
		for (iter_dist=distribution.begin(), end_dist=distribution.end(); iter_dist!=end_dist; ++iter_dist){
			if (PrintName) std::cout << "\t" << LabelEncoder(iter_dist->first, LabelList) << ":" << iter_dist->second;
			else std::cout << "\t" << iter_dist->first << ":" << iter_dist->second;
			if (iter_dist->first==es.GetLabel_ee()) std::cout << "<-";
			std::cout << std::endl;
		}
	}
	else if (kind == 1){
		std::cout << "(Global) Destribution of Entity to ";
		if (PrintName){
			if (context_pe.GetIsLabel()) std::cout << LabelEncoder(context_pe.GetId(), LabelList);
			else  std::cout << EntityEncoder(context_pe.GetId(), EntityList);
			std::cout << " by " << LinkEncoder(es.GetID_link(), LinkList) << ":" << std::endl;
		}
		else std::cout << context_pe.GetId() << " by " << es.GetID_link() << ":" << std::endl;
		distribution = PatternUnitList[context_peidx].GetCombineUniqueEntity()[es.GetID_link()];
		for (iter_dist=distribution.begin(), end_dist=distribution.end(); iter_dist!=end_dist; ++iter_dist){
			if (PrintName) std::cout << "\t" << EntityEncoder(iter_dist->first, EntityList) << ":" << iter_dist->second;
			else std::cout << "\t" << iter_dist->first << ":" << iter_dist->second;
			if (iter_dist->first==es.GetLabel_ee()) std::cout << "<-";
			std::cout << std::endl;
		}
	}
	else{
		std::cout << "(Global) Destribution of Link between ";
		if (PrintName){
			if (context_pe.GetIsLabel()) std::cout << LabelEncoder(context_pe.GetId(), LabelList);
			else  std::cout << EntityEncoder(context_pe.GetId(), EntityList);
			std::cout << " and " << LabelEncoder(es.GetLabel_ee(), LabelList) << ":" << std::endl;
		}
		else std::cout << context_pe.GetId() << " and " << es.GetLabel_ee() << ":" << std::endl;
		distribution = PatternUnitList[context_peidx].GetCombineLink()[es.GetLabel_ee()];
		for (iter_dist=distribution.begin(), end_dist=distribution.end(); iter_dist!=end_dist; ++iter_dist){
			if (PrintName) std::cout << "\t" << LinkEncoder(iter_dist->first, LinkList) << ":" << iter_dist->second;
			else std::cout << "\t" << iter_dist->first << ":" << iter_dist->second;
			if (iter_dist->first==es.GetID_link()) std::cout << "<-";
			std::cout << std::endl;
		}
	}
}

//どのような種類の例外があるかをカウント
void graph::CountExpKind(){
	int PatternNum = 0;
	std::vector<std::map<std::string, int> > cnt(2);
	std::map<double, std::vector<expscore> >::iterator iter_esm, end_esm;
	std::map<std::string, int>::iterator iter_si, end_si;

	for (int i=1; i<2; ++i){
		cnt[i]["1-label"] = 0;
		cnt[i]["2-unique"] = 0;
		cnt[i]["3-link"] = 0;
		cnt[i]["4-missing entity"] = 0;
		cnt[i]["5-missing link"] = 0;
	}
	for (iter_esm=ExpRanking.begin(), end_esm=ExpRanking.end(); iter_esm!=end_esm; ++iter_esm){
		for (int i=0, size_i=iter_esm->second.size(); i<size_i; ++i){
			int LinkNum = iter_esm->second[i].GetIdx_p()[0];
			int kind = iter_esm->second[i].GetKind();
			int id_ee = iter_esm->second[i].GetId_ee();
			if (kind==0 and id_ee!=0) cnt[LinkNum]["1-label"]++;
			else if (kind==1 and id_ee!=0) cnt[LinkNum]["2-unique"]++;
			else if (kind==2 and id_ee!=0) cnt[LinkNum]["3-link"]++;
			else if ((kind==0 or kind==1) and id_ee==0) cnt[LinkNum]["4-missing entity"]++;
			else if (kind==2 and id_ee==0) cnt[LinkNum]["5-missing link"]++;
		}
	}

	for (int i=0; i<2; ++i){
		for (iter_si=cnt[i].begin(), end_si=cnt[i].end(); iter_si!=end_si; ++iter_si){
			std::cout << i << "-" << iter_si->first.substr(2) << ": " << iter_si->second << std::endl;
		}
	}

	for (int i=0; i<2; ++i) PatternNum += PatternList[i].size();
	std::cout << "The number of patterns: " << PatternNum << std::endl;
}

//統計情報を出力
 void graph::PrintStat(){
 	int max, min;
	double mean;
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;

	//リンクラベル数
	int MaxLinkLabel = 0;
	for (int i=0, size_i=EntityList.size(); i<size_i; ++i){
		std::unordered_map<int, std::vector<int> >& relate = EntityList[i].GetRelation();
		for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){
			if (iter_ivi->first>MaxLinkLabel) MaxLinkLabel = iter_ivi->first;
		}
	}
	std::cout << "LinkLabel Num: " << MaxLinkLabel << std::endl;

	//次数
	max = 0;
	min = 100000;
	mean = 0;
	std::vector<int> deg(EntityList.size());
	for (int i=1, size_i=EntityList.size(); i<size_i; ++i){
		deg[i] = 0;
		std::unordered_map<int, std::vector<int> >& relate = EntityList[i].GetRelation();
		for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi) deg[i] += iter_ivi->second.size();
		if (deg[i] > max) max = deg[i];
		if (deg[i] < min) min = deg[i];
	}
	for (int i=1, size_i=deg.size(); i<size_i; ++i) mean += deg[i];
	mean /= (deg.size()-1);
	std::cout << "Max Deg: " << max << std::endl;
	std::cout << "Min Deg: " << min << std::endl;
	std::cout << "Mean Deg: " << mean << std::endl;
	std::cout << std::endl;

	//エンティティあたりのラベル数
	max = 0;
	min = 100000;
	mean = 0;
	std::vector<int> LabNum(EntityList.size());
	for (int i=1, size_i=EntityList.size(); i<size_i; ++i){
		LabNum[i] = EntityList[i].GetLabel().size();
		if (LabNum[i] > max) max = LabNum[i];
		if (LabNum[i] < min) min = LabNum[i];
	}
	for (int i=1, size_i=LabNum.size(); i<size_i; ++i) mean += LabNum[i];
	mean /= (LabNum.size()-1);
	std::cout << "Max Label Num: " << max << std::endl;
	std::cout << "Min Label Num: " << min << std::endl;
	std::cout << "Mean Label Num: " << mean << std::endl;
	std::cout << std::endl;

	//ラベルあたりのそのラベルを持つエンティティ数
	max = 0;
	min = 100000000;
	mean = 0;
	std::vector<int> EntNum(LabelList.size());
	for (int i=1, size_i=LabelList.size(); i<size_i; ++i){
		EntNum[i] = PatternUnitList[i].GetMatchEntity().size();
		if (EntNum[i] > max) max = EntNum[i];
		if (EntNum[i] < min) min = EntNum[i];
	}
	for (int i=1, size_i=EntNum.size(); i<size_i; ++i) mean += EntNum[i];
	//for (int i=1, size_i=EntNum.size(); i<size_i; ++i) std::cout << i << ":" << EntNum[i] << std::endl;
	mean /= (EntNum.size()-1);
	std::cout << "Max Entity Num: " << max << std::endl;
	std::cout << "Min Entity Num: " << min << std::endl;
	std::cout << "Mean Entity Num: " << mean << std::endl;
	std::cout << std::endl;

	//上位10件
	std::map<int, std::unordered_set<int> > MostLabel;
	std::map<int, std::unordered_set<int> >::reverse_iterator riter, rend;
	std::unordered_set<int>::iterator iter_set, end_set;

	int cnt = 0;
	int size_ml = 10;
	for (int i=1, size_i=LabelList.size(); i<size_i; ++i) MostLabel[PatternUnitList[i].GetMatchEntity().size()].insert(i);
	for (riter=MostLabel.rbegin(), rend=MostLabel.rend(); riter!=rend; ++riter){
		for (iter_set=riter->second.begin(), end_set=riter->second.end(); iter_set!=end_set; ++iter_set) std::cout << *iter_set << "(" << LabelList[*iter_set] << "): " << riter->first << std::endl;
		cnt+=riter->second.size();
		if (cnt >= size_ml) break;
	}
}

//特定のリンクと結合するエンティティの情報を表示
void graph::HaveLinkEntity(int link_id){
	std::unordered_map<int, std::vector<int> >::iterator iter_ivi, end_ivi;
	for (int i=1, size_i=EntityList.size(); i<size_i; ++i){
		std::unordered_map<int, std::vector<int> >& relate = EntityList[i].GetRelation();
		for (iter_ivi=relate.begin(), end_ivi=relate.end(); iter_ivi!=end_ivi; ++iter_ivi){
			if (iter_ivi->first!=link_id) continue;
			for (int j=0, size_j=iter_ivi->second.size(); j<size_j; ++j){
				EntityList[i].PrintEntity(std::cout, false, true, EntityList, LabelList, LinkList);
				std::cout << LinkEncoder(link_id, LinkList) << std::endl;
				EntityList[iter_ivi->second[j]].PrintEntity(std::cout, false, true, EntityList, LabelList, LinkList);
				std::cout << std::endl;
			}
		}
	}
}

//特定のエンティティラベルと結合するリンクの情報を表示
void graph::ConnectedLinkWithEntity(int label_id){
	int labelnum, entitynum, bothnum, sum;
	std::unordered_map<int, std::unordered_map<int, int> >& cle = PatternUnitList[label_id].GetCombineLabelEntity();
	std::unordered_map<int, std::unordered_map<int, int> >& cue = PatternUnitList[label_id].GetCombineUniqueEntity();
	std::unordered_map<int, std::unordered_map<int, int> >::iterator iter_i_ii, end_i_ii;
	sum = 0;
	
	for (iter_i_ii=cle.begin(), end_i_ii=cle.end(); iter_i_ii!=end_i_ii; ++iter_i_ii){
		labelnum = iter_i_ii->second.size();
		entitynum = cue[iter_i_ii->first].size();
		if (iter_i_ii->second.find(0)!=iter_i_ii->second.end()) labelnum--;
		if (cue[iter_i_ii->first].find(0)!=cue[iter_i_ii->first].end()) entitynum--;		
		bothnum= labelnum + entitynum;
		sum += bothnum;
		std::cout << LinkEncoder(iter_i_ii->first, LinkList) << ": " << bothnum << "(" << labelnum << "/" << entitynum << ")" << std::endl;
	}
	std::cout << "All: " << sum << std::endl;
}

//PatternListを深さ優先探索して全探索できるか確認
void graph::AllSearchable(){
	int size, idx;
	std::vector<std::unordered_map<int, bool> > IsSearchedPattern(PatternList.size()); //探索済みのパターンかどうか 
	std::queue<int> SearchQueue_size; //探索キュー（パターンサイズ）
	std::queue<int> SearchQueue_idx; //探索キュー（特定サイズ内でのインデックス値）
	std::unordered_set<int> children;
	std::unordered_map<int, bool>::iterator iter_ib, end_ib;
	std::unordered_set<int>::iterator iter_sei, end_sei;
	
	//IsSearchedPaternの初期化
	for (int i=0, size_i=PatternList.size(); i<size_i; ++i){
		for (int j=0, size_j=PatternList[i].size(); j<size_j; ++j){
			if (PatternList[i][j].GetMatchingObjective()) IsSearchedPattern[i][j] = false; 
		}
	}
	
	//深さ優先探索
	SearchQueue_size.push(0);
	SearchQueue_idx.push(0);
	while (!SearchQueue_size.empty()){
		size = SearchQueue_size.front(); SearchQueue_size.pop(); //先頭を取り出し
		idx = SearchQueue_idx.front(); SearchQueue_idx.pop();
		if (IsSearchedPattern[size].find(idx)!=IsSearchedPattern[size].end()){ //マッチング対象のパターンの場合
			if (IsSearchedPattern[size][idx]){ //二重探索の場合
				std::cout << "Double Search (" << size << "," << idx << ")" << std::endl;
				return;
			}
			IsSearchedPattern[size][idx] = true; //探索済みに
		}
		children = PatternList[size][idx].GetChildren(); //子をキューに追加
		for (iter_sei=children.begin(), end_sei=children.end(); iter_sei!=end_sei; ++iter_sei){
			SearchQueue_size.push(size+1);
			SearchQueue_idx.push(*iter_sei);
		}
	}
	
	//すべて探索できているか確認
	int cnt = 0;
	for (int i=0, size_i=PatternList.size(); i<size_i; ++i){
		for (iter_ib=IsSearchedPattern[i].begin(), end_ib=IsSearchedPattern[i].end(); iter_ib!=end_ib; ++iter_ib){
			cnt++;
			if (!iter_ib->second){
				std::cout << "UnSearched (" << i << "," << iter_ib->first << ")" << std::endl;
				return;
			}
		}
	}
	std::cout << "All Searched (" << cnt << "/" << cnt << ")" << std::endl;
}

int main(int argc, char *argv[]){
	NowQuery = "Preparation";
	TimeTime("");
	graph G;
	std::string dataset = "";
	std::string queries_json_name = "";
	int k = 100;
	std::string score_kind = "dist";
	int search_mode = 0;
	bool first_read = false;
	//./main -d dbpedia -q queries -k 100 -s dist -o 0 -f 0
	//./main -d testpedia -q queries -k 100 -s dist -o 0 -f 0
	
    int opt;
    const char *optstring = "d:q:k:s:o:f:";
    while ((opt=getopt(argc, argv, optstring)) != -1){
        switch(opt){
            case 'd':
                dataset = optarg;
                break;
            case 'q':
                queries_json_name = optarg;
                break;
            case 'k':
                k = std::stoi(optarg);
                break;
            case 's':
                score_kind = optarg;
                if (score_kind!="dist" && score_kind!="freq") score_kind = "dist";
                break;
            case 'o':
                search_mode = std::stoi(optarg);
                if (search_mode<0 || search_mode>2) search_mode = 0;
                break;
            case 'f':
                first_read = (bool) std::stoi(optarg);
                break;
        }
    }

	std::map<std::string, std::string> filepath = FilePath("config/outlier.json", queries_json_name, dataset); //ファイル名パラメータ読み込み
	std::cout << "> Reading Graph" << std::endl;
	G.ReadGraph(filepath["DatasetEntity"], filepath["DatasetLabel"], filepath["DatasetLink"]); //グラフ読み込み
	TimeTime("01. Read Graph");

	std::cout << "> Pretreatment" << std::endl;
	std::cout << filepath["IndexFileName"] << std::endl;
	G.SetCombineInfo(first_read, filepath["IndexFileName"]); // 追加しました
	if (first_read){
		G.MakeIndex(filepath["IndexFileName"]); //インデックスに書き込む
		std::cout << "> Making Sortout File" << std::endl;
		G.MakeSortout(filepath["EntityEnumerate"], filepath["LinkEnumerate"], filepath["CombinationEnumerate"]); //グラフ情報を整理したファイルを作成
	}
	TimeTime("02. Pretreatment");
	if (!first_read) G.FindException(k, score_kind, search_mode, filepath); //例外検出

	PrintProcessTime(); //処理時間を表示
	PrintSearchedPatternNum(); //探索したパターン数を表示
	return 0;
}
