#include <bits/stdc++.h>
using namespace std;

static const int NUM_CITIES = 20;
static const double FEMALE_RANK_RELAXATION = 1.20;

static const vector<string> CATEGORIES = {"GEN","EWS","OBC","SC","ST"};
static const vector<string> BRANCHES   = {
    "CS","ECE","ME","CE","EE","BT","CH","IT","DS","AI",
    "AE","MT","MN","CB","BS","EEE","SS","BIO","BBA"
};
static const vector<string> CITY_NAMES = {
    "Delhi","Mumbai","Chennai","Kolkata","Bangalore",
    "Hyderabad","Pune","Ahmedabad","Jaipur","Lucknow",
    "Kanpur","Kharagpur","Roorkee","Pilani","Vellore",
    "Manipal","Coimbatore","Bhopal","Nagpur","Noida"
};

static string jstr(const string &s){
    string r = "\"";
    for(char c : s){
        if(c=='"')  r += "\\\"";
        else if(c=='\\') r += "\\\\";
        else r += c;
    }
    r += "\"";
    return r;
}
static string jnum(double v){
    if(v == (long long)v) return to_string((long long)v);
    char buf[64]; snprintf(buf,sizeof(buf),"%.4f",v);
    return buf;
}
static string jbool(bool v){ return v ? "true" : "false"; }

struct TrieNode {
    map<char,TrieNode*> ch;
    bool is_end = false;
    vector<int> college_ids;
};

class Trie {
    TrieNode* root;
    void collect(TrieNode* node, const string& prefix, vector<string>& results){
        if(node->is_end) results.push_back(prefix);
        for(auto it = node->ch.begin(); it != node->ch.end(); ++it)
            collect(it->second, prefix + it->first, results);
    }
public:
    Trie(){ root = new TrieNode(); }
    void insert(const string& name, int id){
        TrieNode* node = root;
        for(char c : name){
            c = tolower(c);
            if(!node->ch.count(c)) node->ch[c] = new TrieNode();
            node = node->ch[c];
        }
        node->is_end = true;
        node->college_ids.push_back(id);
    }
    vector<string> autocomplete(const string& prefix){
        TrieNode* node = root;
        string lp = prefix;
        for(char& c : lp) c = tolower(c);
        for(char c : lp){
            if(!node->ch.count(c)) return {};
            node = node->ch[c];
        }
        vector<string> results;
        collect(node, lp, results);
        if((int)results.size() > 10) results.resize(10);
        return results;
    }
};

class SegmentTree {
    int n;
    vector<pair<double,int>> data;   
    vector<double> tree_val;
    vector<int>    tree_id;

    void build(int node, int l, int r){
        if(l==r){
            tree_val[node] = data[l].first;
            tree_id[node]  = data[l].second;
            return;
        }
        int mid = (l+r)/2;
        build(2*node,l,mid); build(2*node+1,mid+1,r);
        if(tree_val[2*node] < tree_val[2*node+1]){
            tree_val[node] = tree_val[2*node];
            tree_id[node]  = tree_id[2*node];
        } else {
            tree_val[node] = tree_val[2*node+1];
            tree_id[node]  = tree_id[2*node+1];
        }
    }
    pair<double,int> query(int node, int l, int r, int ql, int qr){
        if(qr<l || r<ql) return {1e18,-1};
        if(ql<=l && r<=qr) return {tree_val[node],tree_id[node]};
        int mid=(l+r)/2;
        auto lv = query(2*node,l,mid,ql,qr);
        auto rv = query(2*node+1,mid+1,r,ql,qr);
        return lv.first < rv.first ? lv : rv;
    }
public:
    SegmentTree(){}
    SegmentTree(vector<pair<double,int>> fees_with_id){
        data = fees_with_id;
        sort(data.begin(), data.end());
        n = (int)data.size();
        tree_val.assign(4*n, 1e18);
        tree_id.assign(4*n, -1);
        if(n>0) build(1,0,n-1);
    }
    // Returns cheapest college (by fees) within sorted index range [l,r]
    pair<double,int> query_range(int l, int r){
        if(n==0) return {1e18,-1};
        return query(1,0,n-1,l,r);
    }
    int size(){ return n; }
};
class CityGraph {
    int V;
    vector<vector<pair<int,double>>> adj;
public:
    CityGraph(int v): V(v), adj(v){}
    void add_edge(int u, int v, double dist){
        adj[u].push_back({v,dist});
        adj[v].push_back({u,dist});
    }
    vector<double> dijkstra(int src){
        vector<double> dist(V, 1e18);
        dist[src] = 0;
        priority_queue<pair<double,int>, vector<pair<double,int>>, greater<>> pq;
        pq.push({0,src});
        while(!pq.empty()){
            double d = pq.top().first;
            int u    = pq.top().second;
            pq.pop();
            if(d > dist[u]) continue;
            for(int ei = 0; ei < (int)adj[u].size(); ei++){
                int v    = adj[u][ei].first;
                double w = adj[u][ei].second;
                if(dist[u]+w < dist[v]){
                    dist[v] = dist[u]+w;
                    pq.push({dist[v],v});
                }
            }
        }
        return dist;
    }
};

static CityGraph build_city_graph(){
    CityGraph g(NUM_CITIES);
    vector<tuple<int,int,double>> edges = {
        {0,19,25},{0,8,268},{0,12,315},{0,9,558},{0,7,935},{0,17,780},
        {9,10,82},{8,13,195},{1,6,149},{1,7,534},{1,4,981},{1,5,711},
        {2,5,627},{2,4,346},{2,14,135},{2,16,494},{3,11,120},{4,5,570},
        {4,15,387},{4,16,365},{5,18,495},{17,18,356},{17,10,410},{6,18,843},
    };
    for(int i = 0; i < (int)edges.size(); i++)
        g.add_edge(get<0>(edges[i]), get<1>(edges[i]), get<2>(edges[i]));
    return g;
}

template<typename T, typename KeyFn>
vector<T> merge_sorted(vector<T> arr, KeyFn key_fn, bool reverse_order){
    if((int)arr.size() <= 1) return arr;
    int mid = arr.size()/2;
    auto left  = merge_sorted(vector<T>(arr.begin(), arr.begin()+mid), key_fn, reverse_order);
    auto right = merge_sorted(vector<T>(arr.begin()+mid, arr.end()),   key_fn, reverse_order);
    vector<T> result;
    int i=0, j=0;
    while(i<(int)left.size() && j<(int)right.size()){
        auto lv = key_fn(left[i]), rv = key_fn(right[j]);
        bool take_left = reverse_order ? (lv >= rv) : (lv <= rv);
        if(take_left){ result.push_back(left[i++]); }
        else          { result.push_back(right[j++]); }
    }
    while(i<(int)left.size())  result.push_back(left[i++]);
    while(j<(int)right.size()) result.push_back(right[j++]);
    return result;
}

using CutoffMap = map<string, map<string,int>>;   

struct College {
    int    id;
    string name, city, state, type;
    int    nirf_rank;
    double avg_package, fees;
    int    bitsat_cutoff;
    bool   female_supernumerary;
    double placement_pct, research_score, infra_score;
    int    city_id;
    vector<string> branches;
    CutoffMap adv_cutoffs;
    CutoffMap main_hs_cutoffs;
    CutoffMap main_os_cutoffs;
};

static College make_iit(int cid, string name, string city, string state,
    int nirf, double pkg, double fees, double plc, double res, double inf,
    int city_id, vector<string> branches, CutoffMap adv)
{
    College c;
    c.id=cid; c.name=name; c.city=city; c.state=state; c.type="IIT";
    c.nirf_rank=nirf; c.avg_package=pkg; c.fees=fees; c.bitsat_cutoff=0;
    c.female_supernumerary=true; c.placement_pct=plc;
    c.research_score=res; c.infra_score=inf; c.city_id=city_id;
    c.branches=branches; c.adv_cutoffs=adv;
    return c;
}

static College make_nit(int cid, string name, string city, string state,
    int nirf, double pkg, double fees, double plc, double res, double inf,
    int city_id, vector<string> branches, CutoffMap hs, CutoffMap os)
{
    College c;
    c.id=cid; c.name=name; c.city=city; c.state=state; c.type="NIT";
    c.nirf_rank=nirf; c.avg_package=pkg; c.fees=fees; c.bitsat_cutoff=0;
    c.female_supernumerary=false; c.placement_pct=plc;
    c.research_score=res; c.infra_score=inf; c.city_id=city_id;
    c.branches=branches; c.main_hs_cutoffs=hs; c.main_os_cutoffs=os;
    return c;
}

static College make_iiit(int cid, string name, string city, string state,
    int nirf, double pkg, double fees, double plc, double res, double inf,
    int city_id, vector<string> branches, CutoffMap hs, CutoffMap os)
{
    College c = make_nit(cid,name,city,state,nirf,pkg,fees,plc,res,inf,
                         city_id,branches,hs,os);
    c.type = "IIIT";
    return c;
}

static College make_bits(int cid, string name, string city, string state,
    int nirf, double pkg, double fees, double plc, double res, double inf,
    int city_id, vector<string> branches, int bitsat_cut)
{
    College c;
    c.id=cid; c.name=name; c.city=city; c.state=state; c.type="Deemed";
    c.nirf_rank=nirf; c.avg_package=pkg; c.fees=fees; c.bitsat_cutoff=bitsat_cut;
    c.female_supernumerary=false; c.placement_pct=plc;
    c.research_score=res; c.infra_score=inf; c.city_id=city_id;
    c.branches=branches;
    return c;
}

static College make_open(int cid, string name, string city, string state,
    string typ, int nirf, double pkg, double fees, double plc, double res,
    double inf, int city_id, vector<string> branches, int hs_gen=0, int os_gen=0)
{
    College c;
    c.id=cid; c.name=name; c.city=city; c.state=state; c.type=typ;
    c.nirf_rank=nirf; c.avg_package=pkg; c.fees=fees; c.bitsat_cutoff=0;
    c.female_supernumerary=false; c.placement_pct=plc;
    c.research_score=res; c.infra_score=inf; c.city_id=city_id;
    c.branches=branches;
    if(hs_gen > 0) for(auto& b : branches) c.main_hs_cutoffs[b]["GEN"]=hs_gen;
    if(os_gen > 0) for(auto& b : branches) c.main_os_cutoffs[b]["GEN"]=os_gen;
    return c;
}

static vector<College> load_college_data(){
    vector<College> cols;

    cols.push_back(make_iit(0,"IIT Bombay","Mumbai","Maharashtra",1,28.0,9.0,95.0,9.8,9.9,1,
        {"CS","ECE","ME","CE","EE"},{
            {"CS", {{"GEN",67},  {"EWS",200}, {"OBC",350}, {"SC",800},  {"ST",1200}}},
            {"ECE",{{"GEN",850}, {"EWS",1400},{"OBC",1900},{"SC",3500}, {"ST",5000}}},
            {"ME", {{"GEN",2200},{"EWS",3500},{"OBC",5000},{"SC",9000}, {"ST",13000}}},
            {"CE", {{"GEN",3000},{"EWS",4800},{"OBC",7000},{"SC",12000},{"ST",17000}}},
            {"EE", {{"GEN",300}, {"EWS",700}, {"OBC",1100},{"SC",2200}, {"ST",3500}}},
        }));

    cols.push_back(make_iit(1,"IIT Delhi","Delhi","Delhi",2,26.5,9.5,94.0,9.7,9.8,0,
        {"CS","ECE","ME","CE","CH"},{
            {"CS", {{"GEN",100}, {"EWS",280}, {"OBC",480}, {"SC",950},  {"ST",1500}}},
            {"ECE",{{"GEN",900}, {"EWS",1500},{"OBC",2200},{"SC",4000}, {"ST",5800}}},
            {"ME", {{"GEN",2800},{"EWS",4200},{"OBC",6000},{"SC",10500},{"ST",15000}}},
            {"CE", {{"GEN",3500},{"EWS",5500},{"OBC",8000},{"SC",14000},{"ST",20000}}},
            {"CH", {{"GEN",4000},{"EWS",6000},{"OBC",9000},{"SC",16000},{"ST",22000}}},
        }));

    cols.push_back(make_iit(2,"IIT Madras","Chennai","Tamil Nadu",3,25.0,9.0,93.0,9.8,9.7,2,
        {"CS","ECE","ME","CE","BT"},{
            {"CS", {{"GEN",130}, {"EWS",350}, {"OBC",580}, {"SC",1100}, {"ST",1800}}},
            {"ECE",{{"GEN",950}, {"EWS",1600},{"OBC",2500},{"SC",4500}, {"ST",6500}}},
            {"ME", {{"GEN",3000},{"EWS",4600},{"OBC",6500},{"SC",11000},{"ST",16000}}},
            {"CE", {{"GEN",3800},{"EWS",5800},{"OBC",8500},{"SC",15000},{"ST",21000}}},
            {"BT", {{"GEN",5000},{"EWS",7500},{"OBC",11000},{"SC",20000},{"ST",28000}}},
        }));

    cols.push_back(make_iit(3,"IIT Kharagpur","Kharagpur","West Bengal",4,22.0,8.5,92.0,9.6,9.5,11,
        {"CS","ECE","ME","CE","MT"},{
            {"CS", {{"GEN",160}, {"EWS",430}, {"OBC",700}, {"SC",1400}, {"ST",2200}}},
            {"ECE",{{"GEN",1100},{"EWS",1900},{"OBC",2900},{"SC",5500}, {"ST",8000}}},
            {"ME", {{"GEN",3500},{"EWS",5200},{"OBC",7500},{"SC",13000},{"ST",19000}}},
            {"CE", {{"GEN",4200},{"EWS",6500},{"OBC",9500},{"SC",17000},{"ST",24000}}},
            {"MT", {{"GEN",6000},{"EWS",9000},{"OBC",13000},{"SC",24000},{"ST",34000}}},
        }));

    cols.push_back(make_iit(4,"IIT Kanpur","Kanpur","UP",5,24.0,9.0,91.0,9.7,9.6,10,
        {"CS","ECE","ME","AE","CH"},{
            {"CS", {{"GEN",200}, {"EWS",500}, {"OBC",850}, {"SC",1700}, {"ST",2700}}},
            {"ECE",{{"GEN",1300},{"EWS",2200},{"OBC",3300},{"SC",6000}, {"ST",8800}}},
            {"ME", {{"GEN",4000},{"EWS",6000},{"OBC",8500},{"SC",15000},{"ST",22000}}},
            {"AE", {{"GEN",4500},{"EWS",6800},{"OBC",9800},{"SC",18000},{"ST",26000}}},
            {"CH", {{"GEN",5000},{"EWS",7500},{"OBC",11000},{"SC",20000},{"ST",29000}}},
        }));

    cols.push_back(make_iit(5,"IIT Roorkee","Roorkee","Uttarakhand",6,21.0,8.5,90.0,9.5,9.5,12,
        {"CS","ECE","CE","ME","BT"},{
            {"CS", {{"GEN",250}, {"EWS",600}, {"OBC",1000},{"SC",2100}, {"ST",3300}}},
            {"ECE",{{"GEN",1600},{"EWS",2700},{"OBC",4000},{"SC",7500}, {"ST",11000}}},
            {"CE", {{"GEN",5000},{"EWS",7500},{"OBC",11000},{"SC",20000},{"ST",29000}}},
            {"ME", {{"GEN",4500},{"EWS",6800},{"OBC",9800},{"SC",18000},{"ST",26000}}},
            {"BT", {{"GEN",7000},{"EWS",10500},{"OBC",15500},{"SC",28000},{"ST",40000}}},
        }));

    cols.push_back(make_iit(6,"IIT Hyderabad","Hyderabad","Telangana",8,19.0,8.5,88.0,9.3,9.3,5,
        {"CS","ECE","ME","EE","BT"},{
            {"CS", {{"GEN",400}, {"EWS",900}, {"OBC",1500},{"SC",3000}, {"ST",5000}}},
            {"ECE",{{"GEN",2000},{"EWS",3500},{"OBC",5200},{"SC",9500}, {"ST",14000}}},
            {"ME", {{"GEN",6000},{"EWS",9000},{"OBC",13000},{"SC",24000},{"ST",35000}}},
            {"EE", {{"GEN",2500},{"EWS",4200},{"OBC",6500},{"SC",12000},{"ST",18000}}},
            {"BT", {{"GEN",8000},{"EWS",12000},{"OBC",18000},{"SC",33000},{"ST",48000}}},
        }));

    cols.push_back(make_iit(7,"IIT Gandhinagar","Ahmedabad","Gujarat",9,18.5,8.0,87.0,9.2,9.1,7,
        {"CS","ECE","ME","CH","BS"},{
            {"CS", {{"GEN",500}, {"EWS",1100},{"OBC",1800},{"SC",3500}, {"ST",5800}}},
            {"ECE",{{"GEN",2500},{"EWS",4200},{"OBC",6200},{"SC",11500},{"ST",17000}}},
            {"ME", {{"GEN",7000},{"EWS",10500},{"OBC",15500},{"SC",28000},{"ST",40000}}},
            {"CH", {{"GEN",8000},{"EWS",12000},{"OBC",18000},{"SC",33000},{"ST",47000}}},
            {"BS", {{"GEN",9000},{"EWS",13500},{"OBC",20000},{"SC",37000},{"ST",54000}}},
        }));

    cols.push_back(make_iit(8,"IIT Guwahati","Guwahati","Assam",7,20.0,8.5,89.0,9.4,9.2,20,
        {"CS","ECE","ME","CE","BT"},{
            {"CS", {{"GEN",350}, {"EWS",800}, {"OBC",1300},{"SC",2600}, {"ST",4300}}},
            {"ECE",{{"GEN",1800},{"EWS",3200},{"OBC",4800},{"SC",8800}, {"ST",13000}}},
            {"ME", {{"GEN",5500},{"EWS",8300},{"OBC",12000},{"SC",22000},{"ST",32000}}},
            {"CE", {{"GEN",6500},{"EWS",9800},{"OBC",14500},{"SC",26000},{"ST",38000}}},
            {"BT", {{"GEN",9000},{"EWS",13500},{"OBC",20000},{"SC",37000},{"ST",54000}}},
        }));

    cols.push_back(make_iit(9,"IIT BHU","Varanasi","UP",10,17.0,8.5,85.0,9.1,9.0,20,
        {"CS","ECE","ME","CE","MN"},{
            {"CS", {{"GEN",600}, {"EWS",1200},{"OBC",2000},{"SC",4000}, {"ST",6500}}},
            {"ECE",{{"GEN",2200},{"EWS",3800},{"OBC",5700},{"SC",10500},{"ST",15500}}},
            {"ME", {{"GEN",7000},{"EWS",10500},{"OBC",15500},{"SC",28000},{"ST",40000}}},
            {"CE", {{"GEN",8000},{"EWS",12000},{"OBC",18000},{"SC",33000},{"ST",47000}}},
            {"MN", {{"GEN",10000},{"EWS",15000},{"OBC",22000},{"SC",40000},{"ST",57000}}},
        }));

    cols.push_back(make_nit(10,"NIT Trichy","Trichy","Tamil Nadu",10,16.0,3.5,82.0,8.8,8.8,2,
        {"CS","ECE","ME","CE","EE"},
        {{"CS",{{"GEN",1200},{"EWS",2500},{"OBC",4000},{"SC",8500},{"ST",14000}}},
         {"ECE",{{"GEN",4500},{"EWS",8000},{"OBC",12000},{"SC",25000},{"ST",40000}}},
         {"ME",{{"GEN",12000},{"EWS",22000},{"OBC",35000},{"SC",70000},{"ST",110000}}},
         {"CE",{{"GEN",15000},{"EWS",27000},{"OBC",43000},{"SC",90000},{"ST",140000}}},
         {"EE",{{"GEN",6000},{"EWS",11000},{"OBC",17000},{"SC",36000},{"ST",58000}}}},
        {{"CS",{{"GEN",800},{"EWS",1800},{"OBC",2800},{"SC",6000},{"ST",10000}}},
         {"ECE",{{"GEN",3000},{"EWS",5500},{"OBC",8500},{"SC",18000},{"ST",29000}}},
         {"ME",{{"GEN",8000},{"EWS",15000},{"OBC",24000},{"SC",50000},{"ST",80000}}},
         {"CE",{{"GEN",10000},{"EWS",18500},{"OBC",30000},{"SC",62000},{"ST",100000}}},
         {"EE",{{"GEN",4000},{"EWS",7500},{"OBC",12000},{"SC",25000},{"ST",40000}}}}
    ));

    cols.push_back(make_nit(11,"NIT Surathkal","Mangalore","Karnataka",14,14.5,3.2,80.0,8.6,8.6,4,
        {"CS","ECE","ME","CE","EE"},
        {{"CS",{{"GEN",1500},{"EWS",3000},{"OBC",5000},{"SC",10000},{"ST",16000}}},
         {"ECE",{{"GEN",5500},{"EWS",10000},{"OBC",15000},{"SC",30000},{"ST",48000}}},
         {"ME",{{"GEN",14000},{"EWS",25000},{"OBC",40000},{"SC",80000},{"ST",125000}}},
         {"CE",{{"GEN",17000},{"EWS",30000},{"OBC",48000},{"SC",100000},{"ST",155000}}},
         {"EE",{{"GEN",7000},{"EWS",12500},{"OBC",20000},{"SC",42000},{"ST",66000}}}},
        {{"CS",{{"GEN",1000},{"EWS",2000},{"OBC",3300},{"SC",7000},{"ST",11500}}},
         {"ECE",{{"GEN",3500},{"EWS",6500},{"OBC",10000},{"SC",21000},{"ST",34000}}},
         {"ME",{{"GEN",9000},{"EWS",17000},{"OBC",27000},{"SC",56000},{"ST",90000}}},
         {"CE",{{"GEN",11000},{"EWS",20000},{"OBC",32000},{"SC",67000},{"ST",108000}}},
         {"EE",{{"GEN",5000},{"EWS",9000},{"OBC",14000},{"SC",30000},{"ST",47000}}}}
    ));

    cols.push_back(make_nit(12,"NIT Warangal","Warangal","Telangana",12,15.0,3.4,81.0,8.7,8.7,5,
        {"CS","ECE","ME","CE","EE"},
        {{"CS",{{"GEN",1300},{"EWS",2700},{"OBC",4500},{"SC",9000},{"ST",14500}}},
         {"ECE",{{"GEN",5000},{"EWS",9000},{"OBC",13500},{"SC",27000},{"ST",43000}}},
         {"ME",{{"GEN",13000},{"EWS",23000},{"OBC",37000},{"SC",75000},{"ST",118000}}},
         {"CE",{{"GEN",16000},{"EWS",28000},{"OBC",45000},{"SC",95000},{"ST",148000}}},
         {"EE",{{"GEN",6500},{"EWS",12000},{"OBC",18500},{"SC",39000},{"ST",62000}}}},
        {{"CS",{{"GEN",900},{"EWS",1900},{"OBC",3100},{"SC",6500},{"ST",10500}}},
         {"ECE",{{"GEN",3300},{"EWS",6000},{"OBC",9200},{"SC",19500},{"ST",31000}}},
         {"ME",{{"GEN",8500},{"EWS",16000},{"OBC",25500},{"SC",53000},{"ST",85000}}},
         {"CE",{{"GEN",10500},{"EWS",19000},{"OBC",31000},{"SC",64000},{"ST",103000}}},
         {"EE",{{"GEN",4300},{"EWS",8000},{"OBC",13000},{"SC",27000},{"ST",43000}}}}
    ));

    cols.push_back(make_nit(13,"NIT Calicut","Calicut","Kerala",17,13.5,3.0,78.0,8.4,8.4,2,
        {"CS","ECE","ME","CE","EE"},
        {{"CS",{{"GEN",2000},{"EWS",3800},{"OBC",6000},{"SC",12000},{"ST",19000}}},
         {"ECE",{{"GEN",7000},{"EWS",12000},{"OBC",18000},{"SC",36000},{"ST",57000}}},
         {"ME",{{"GEN",17000},{"EWS",30000},{"OBC",48000},{"SC",95000},{"ST",148000}}},
         {"CE",{{"GEN",20000},{"EWS",36000},{"OBC",57000},{"SC",113000},{"ST",175000}}},
         {"EE",{{"GEN",9000},{"EWS",16000},{"OBC",25000},{"SC",51000},{"ST",80000}}}},
        {{"CS",{{"GEN",1300},{"EWS",2600},{"OBC",4100},{"SC",8500},{"ST",13500}}},
         {"ECE",{{"GEN",4500},{"EWS",8000},{"OBC",12500},{"SC",25500},{"ST",41000}}},
         {"ME",{{"GEN",11000},{"EWS",20000},{"OBC",32000},{"SC",66000},{"ST",106000}}},
         {"CE",{{"GEN",13500},{"EWS",24000},{"OBC",38500},{"SC",79000},{"ST",127000}}},
         {"EE",{{"GEN",6000},{"EWS",11000},{"OBC",17500},{"SC",36000},{"ST",57000}}}}
    ));

    auto iiit_hyd_cuts = CutoffMap{
        {"CS", {{"GEN",500},{"EWS",1000},{"OBC",1700},{"SC",4000},{"ST",6500}}},
        {"ECE",{{"GEN",3000},{"EWS",5500},{"OBC",8500},{"SC",18000},{"ST",28000}}},
        {"DS", {{"GEN",1000},{"EWS",2000},{"OBC",3300},{"SC",7500},{"ST",12000}}},
        {"AI", {{"GEN",800},{"EWS",1600},{"OBC",2700},{"SC",6000},{"ST",9500}}},
    };
    cols.push_back(make_iiit(20,"IIIT Hyderabad","Hyderabad","Telangana",19,16.0,10.0,83.0,9.0,8.8,5,
        {"CS","ECE","DS","AI"}, iiit_hyd_cuts, iiit_hyd_cuts));

    auto iiit_all_cuts = CutoffMap{
        {"CS", {{"GEN",1800},{"EWS",3500},{"OBC",5500},{"SC",11000},{"ST",17000}}},
        {"ECE",{{"GEN",4500},{"EWS",7500},{"OBC",11000},{"SC",22000},{"ST",35000}}},
        {"IT", {{"GEN",2500},{"EWS",4500},{"OBC",7000},{"SC",14000},{"ST",22000}}},
        {"BT", {{"GEN",6000},{"EWS",10000},{"OBC",15000},{"SC",30000},{"ST",47000}}},
    };
    cols.push_back(make_iiit(21,"IIIT Allahabad","Prayagraj","UP",16,13.0,6.5,79.0,8.8,8.6,9,
        {"CS","ECE","IT","BT"}, iiit_all_cuts, iiit_all_cuts));

    auto iiit_blr_cuts = CutoffMap{
        {"CS", {{"GEN",2000},{"EWS",4000},{"OBC",6000},{"SC",12000},{"ST",19000}}},
        {"ECE",{{"GEN",6000},{"EWS",10000},{"OBC",16000},{"SC",30000},{"ST",47000}}},
        {"DS", {{"GEN",3000},{"EWS",5500},{"OBC",8500},{"SC",17000},{"ST",27000}}},
        {"AI", {{"GEN",2500},{"EWS",4500},{"OBC",7000},{"SC",14000},{"ST",22000}}},
    };
    cols.push_back(make_iiit(22,"IIIT Bangalore","Bangalore","Karnataka",20,14.0,8.5,80.0,8.5,8.2,4,
        {"CS","ECE","DS","AI"}, iiit_blr_cuts, iiit_blr_cuts));

    auto iiit_del_cuts = CutoffMap{
        {"CS", {{"GEN",1800},{"EWS",3300},{"OBC",5000},{"SC",10000},{"ST",16000}}},
        {"ECE",{{"GEN",5000},{"EWS",8500},{"OBC",13000},{"SC",25000},{"ST",40000}}},
        {"CB", {{"GEN",4000},{"EWS",7000},{"OBC",11000},{"SC",22000},{"ST",34000}}},
        {"SS", {{"GEN",5500},{"EWS",9500},{"OBC",14500},{"SC",28000},{"ST",44000}}},
    };
    cols.push_back(make_iiit(23,"IIIT Delhi","Delhi","Delhi",18,13.5,7.5,82.0,8.7,8.4,0,
        {"CS","ECE","CB","SS"}, iiit_del_cuts, iiit_del_cuts));

    cols.push_back(make_bits(24,"BITS Pilani","Pilani","Rajasthan",27,20.0,15.0,90.0,9.0,9.0,13,
        {"CS","ECE","ME","CH","EEE"},330));
    cols.push_back(make_bits(25,"BITS Hyderabad","Hyderabad","Telangana",30,18.0,14.0,88.0,8.8,8.8,5,
        {"CS","ECE","ME","BT","EEE"},310));
    cols.push_back(make_bits(26,"BITS Goa","Panaji","Goa",33,17.0,13.5,85.0,8.6,8.9,1,
        {"CS","ECE","ME","CH","EEE"},300));

    cols.push_back(make_open(27,"VIT Vellore","Vellore","Tamil Nadu","Private",11,7.5,8.5,75.0,7.5,8.5,14,{"CS","ECE","ME","CE","BT"}));
    cols.push_back(make_open(28,"Manipal Institute","Manipal","Karnataka","Private",13,7.0,12.0,72.0,7.2,8.8,15,{"CS","ECE","ME","BT","CE"}));
    cols.push_back(make_open(29,"SRM Chennai","Chennai","Tamil Nadu","Private",36,6.0,7.5,65.0,6.8,8.2,2,{"CS","ECE","ME","CE","BT"}));
    cols.push_back(make_open(30,"Thapar Patiala","Patiala","Punjab","Deemed",28,9.0,11.0,78.0,8.0,8.3,0,{"CS","ECE","ME","CE","EE"},80000,30000));
    cols.push_back(make_open(31,"PESIT Bangalore","Bangalore","Karnataka","Private",50,8.0,6.0,70.0,7.0,7.8,4,{"CS","ECE","ME","CE","EE"}));
    cols.push_back(make_open(32,"Amity Noida","Noida","UP","Private",60,5.5,9.0,60.0,6.0,7.5,19,{"CS","ECE","ME","CE","BBA"}));
    cols.push_back(make_open(33,"LPU Jalandhar","Jalandhar","Punjab","Private",70,4.5,4.5,55.0,5.5,7.0,0,{"CS","ECE","ME","CE","BBA"}));
    cols.push_back(make_open(44,"DTU Delhi","Delhi","Delhi","Govt",35,11.0,1.5,78.0,7.8,8.0,0,{"CS","ECE","ME","CE","EE"},12000,5000));
    cols.push_back(make_open(45,"NSUT Delhi","Delhi","Delhi","Govt",40,10.0,1.2,75.0,7.5,7.8,0,{"CS","ECE","ME","CE","IT"},14000,6000));
    cols.push_back(make_open(46,"IGDTUW Delhi","Delhi","Delhi","Govt",42,9.5,1.3,73.0,7.3,7.7,0,{"CS","ECE","IT","EE"},15000,7000));
    cols.push_back(make_open(47,"Jadavpur University","Kolkata","West Bengal","Govt",7,10.5,0.8,74.0,8.5,7.5,3,{"CS","ECE","ME","CE","CH"},10000,5000));
    cols.push_back(make_open(48,"Anna University","Chennai","Tamil Nadu","Govt",24,8.0,0.5,68.0,7.0,7.0,2,{"CS","ECE","ME","CE","EE"}));
    cols.push_back(make_open(49,"Pune University (COEP)","Pune","Maharashtra","Govt",32,9.0,0.6,70.0,7.2,7.3,6,{"CS","ECE","ME","CE","IT"}));
    cols.push_back(make_open(50,"Jaypee Inst. of IT","Noida","UP","Private",75,6.5,8.0,65.0,6.5,7.8,19,{"CS","ECE","IT","BT"},120000,150000));

    sort(cols.begin(), cols.end(), [](const College& a, const College& b){ return a.id < b.id; });
    return cols;
}

static string college_to_json(const College& c){
    string branches_json = "[";
    for(int i=0;i<(int)c.branches.size();i++){
        if(i) branches_json += ",";
        branches_json += jstr(c.branches[i]);
    }
    branches_json += "]";

    auto cutoff_map_to_json = [&](const CutoffMap& cm) -> string {
        string r = "{";
        bool first_br = true;
        for(auto it_br = cm.begin(); it_br != cm.end(); ++it_br){
            if(!first_br) r += ",";
            first_br = false;
            r += jstr(it_br->first) + ":{";
            bool first_cat = true;
            for(auto it_cat = it_br->second.begin(); it_cat != it_br->second.end(); ++it_cat){
                if(!first_cat) r += ",";
                first_cat = false;
                r += jstr(it_cat->first) + ":" + to_string(it_cat->second);
            }
            r += "}";
        }
        r += "}";
        return r;
    };

    string s = "{";
    s += "\"id\":"              + to_string(c.id)           + ",";
    s += "\"name\":"            + jstr(c.name)               + ",";
    s += "\"city\":"            + jstr(c.city)               + ",";
    s += "\"state\":"           + jstr(c.state)              + ",";
    s += "\"type\":"            + jstr(c.type)               + ",";
    s += "\"nirf_rank\":"       + to_string(c.nirf_rank)     + ",";
    s += "\"avg_package\":"     + jnum(c.avg_package)        + ",";
    s += "\"fees\":"            + jnum(c.fees)               + ",";
    s += "\"bitsat_cutoff\":"   + to_string(c.bitsat_cutoff) + ",";
    s += "\"female_supernumerary\":" + jbool(c.female_supernumerary) + ",";
    s += "\"placement_pct\":"   + jnum(c.placement_pct)     + ",";
    s += "\"research_score\":"  + jnum(c.research_score)    + ",";
    s += "\"infra_score\":"     + jnum(c.infra_score)       + ",";
    s += "\"city_id\":"         + to_string(c.city_id)      + ",";
    s += "\"branches\":"        + branches_json              + ",";
    s += "\"adv_cutoffs\":"     + cutoff_map_to_json(c.adv_cutoffs)      + ",";
    s += "\"main_hs_cutoffs\":" + cutoff_map_to_json(c.main_hs_cutoffs)  + ",";
    s += "\"main_os_cutoffs\":" + cutoff_map_to_json(c.main_os_cutoffs);
    s += "}";
    return s;
}

static map<string,string> parse_flat_json(const string& json){
    map<string,string> result;
    size_t i = 0;
    auto skip_ws = [&](){ while(i<json.size() && isspace(json[i])) i++; };
    auto read_str = [&]() -> string {
        if(i>=json.size() || json[i]!='"') return "";
        i++; string s;
        while(i<json.size() && json[i]!='"'){
            if(json[i]=='\\') i++;
            if(i<json.size()) s += json[i++];
        }
        if(i<json.size()) i++; 
        return s;
    };
    skip_ws(); if(i<json.size() && json[i]=='{') i++;
    while(i<json.size() && json[i]!='}'){
        skip_ws();
        string key = read_str();
        skip_ws(); if(i<json.size() && json[i]==':') i++;
        skip_ws();
        string val;
        if(i<json.size() && json[i]=='"'){
            val = read_str();
        } else {
            while(i<json.size() && json[i]!=',' && json[i]!='}' && !isspace(json[i]))
                val += json[i++];
        }
        if(!key.empty()) result[key] = val;
        skip_ws(); if(i<json.size() && json[i]==',') i++;
    }
    return result;
}

static pair<bool,string> is_eligible(const College& col,
    int jee_main_rank, int jee_adv_rank, int bitsat_score,
    const string& category, const string& gender,
    const string& home_state, const string& preferred_branch)
{
    auto check_cutoffs = [&](const CutoffMap& cutoffs, int rank) -> pair<bool,string> {
        vector<string> check_branches;
        if(!preferred_branch.empty() && cutoffs.count(preferred_branch))
            check_branches = {preferred_branch};
        else
            for(auto it = cutoffs.begin(); it != cutoffs.end(); ++it) check_branches.push_back(it->first);

        for(auto& br : check_branches){
            if(!cutoffs.count(br)) continue;
            auto it = cutoffs.at(br).find(category);
            if(it == cutoffs.at(br).end()) continue;
            int cutoff = it->second;
            if(col.female_supernumerary && gender=="FEMALE"){
                auto git = cutoffs.at(br).find("GEN");
                if(git != cutoffs.at(br).end())
                    cutoff = max(cutoff, (int)(git->second * FEMALE_RANK_RELAXATION));
            }
            if(cutoff > 0 && rank <= cutoff) return {true, br};
        }
        return {false, ""};
    };

    if(col.type == "IIT"){
        if(jee_adv_rank <= 0) return {false,""};
        return check_cutoffs(col.adv_cutoffs, jee_adv_rank);
    }
    if(col.type == "NIT" || col.type == "IIIT"){
        if(jee_main_rank <= 0) return {false,""};
        string col_state_lower = col.state; for(char& c:col_state_lower) c=tolower(c);
        string hs_lower = home_state; for(char& c:hs_lower) c=tolower(c);
        bool is_hs = (hs_lower == col_state_lower);
        const CutoffMap& cmap = is_hs ? col.main_hs_cutoffs : col.main_os_cutoffs;
        return check_cutoffs(cmap, jee_main_rank);
    }
    if(col.type == "Deemed" && col.bitsat_cutoff > 0){
        if(bitsat_score <= 0) return {false,""};
        if(bitsat_score >= col.bitsat_cutoff){
            string br = (!preferred_branch.empty() &&
                         find(col.branches.begin(),col.branches.end(),preferred_branch)!=col.branches.end())
                        ? preferred_branch
                        : (col.branches.empty() ? "" : col.branches[0]);
            return {true, br};
        }
        return {false,""};
    }

    string br = (!preferred_branch.empty() &&
                 find(col.branches.begin(),col.branches.end(),preferred_branch)!=col.branches.end())
                ? preferred_branch
                : (col.branches.empty() ? "" : col.branches[0]);
    return {true, br};
}

struct Weights {
    double rank=0.20, package=0.25, fees=0.15, placement=0.20, distance=0.10, infra=0.10;
};

static double compute_score(const College& col, double distance_km,
    double max_fees, double max_pkg, double max_dist, const Weights& w)
{
    double rank_s      = 1.0 - col.nirf_rank / 200.0;
    double package_s   = max_pkg   > 0 ? min(col.avg_package / max_pkg, 1.0)     : 0;
    double fees_s      = max_fees  > 0 ? 1.0 - min(col.fees / max_fees, 1.0)     : 0;
    double placement_s = col.placement_pct / 100.0;
    double dist_s      = max_dist  > 0 ? 1.0 - min(distance_km / max_dist, 1.0) : 1.0;
    double infra_s     = col.infra_score / 10.0;
    return w.rank*rank_s + w.package*package_s + w.fees*fees_s
         + w.placement*placement_s + w.distance*dist_s + w.infra*infra_s;
}

static void cmd_recommend(const string& json_arg, const vector<College>& colleges){
    auto p = parse_flat_json(json_arg);
    auto gd = [&](const string& k, double def=0){ auto it=p.find(k); return it!=p.end()?stod(it->second):def; };
    auto gi = [&](const string& k, int def=0)  { auto it=p.find(k); return it!=p.end()?stoi(it->second):def; };
    auto gs = [&](const string& k, const string& def="") -> string { auto it=p.find(k); return it!=p.end()?it->second:def; };

    int jee_main_rank    = gi("jee_main_rank");
    int jee_adv_rank     = gi("jee_adv_rank");
    int bitsat_score     = gi("bitsat_score");
    string category      = gs("category","GEN");
    string gender        = gs("gender","MALE");
    string home_state    = gs("home_state");
    string pref_branch   = gs("preferred_branch");
    double budget        = gd("budget_lakhs");
    double min_pkg       = gd("min_package");
    int city_id          = gi("city_id");
    int top_k            = gi("top_k", 10);
    if(city_id >= NUM_CITIES) city_id = 0;

    Weights w;
    w.rank      = gd("w_rank",      0.20);
    w.package   = gd("w_package",   0.25);
    w.fees      = gd("w_fees",      0.15);
    w.placement = gd("w_placement", 0.20);
    w.distance  = gd("w_distance",  0.10);
    w.infra     = gd("w_infra",     0.10);

    CityGraph g = build_city_graph();
    vector<double> distances = g.dijkstra(city_id);

    struct Result {
        const College* col;
        string matched_branch;
        double score, distance_km;
        string verdict;
    };
    vector<Result> eligible;
    for(auto& col : colleges){
        pair<bool,string> elig1 = is_eligible(col, jee_main_rank, jee_adv_rank, bitsat_score,
                                    category, gender, home_state, pref_branch);
        if(!elig1.first) continue;
        if(budget > 0 && col.fees > budget) continue;
        if(min_pkg > 0 && col.avg_package < min_pkg) continue;
        eligible.push_back({&col, elig1.second, 0, 0, ""});
    }
    if(eligible.empty()){
        for(auto& col : colleges){
            pair<bool,string> elig2 = is_eligible(col, jee_main_rank, jee_adv_rank, bitsat_score,
                                        category, gender, home_state, pref_branch);
            if(elig2.first) eligible.push_back({&col, elig2.second, 0, 0, ""});
        }
    }

    if(eligible.empty()){ cout << "[]"; return; }

    double max_fees = 1, max_pkg_v = 1, max_dist = 1;
    for(auto& r : eligible){
        max_fees  = max(max_fees, r.col->fees);
        max_pkg_v = max(max_pkg_v, r.col->avg_package);
        double d  = r.col->city_id < NUM_CITIES ? distances[r.col->city_id] : 2000;
        max_dist  = max(max_dist, d);
    }
    max_fees += 1; max_pkg_v += 1; max_dist += 1;

    for(auto& r : eligible){
        r.distance_km = r.col->city_id < NUM_CITIES ? distances[r.col->city_id] : 2000;
        r.score = compute_score(*r.col, r.distance_km, max_fees, max_pkg_v, max_dist, w);
        if     (r.score > 0.75) r.verdict = "Excellent Fit";
        else if(r.score > 0.55) r.verdict = "Good Fit";
        else if(r.score > 0.40) r.verdict = "Fair Fit";
        else                    r.verdict = "Stretch";
    }

    auto sorted = merge_sorted(eligible,
        [](const Result& r){ return -r.score; }, false);
    if((int)sorted.size() > top_k) sorted.resize(top_k);

    cout << "[";
    for(int i=0;i<(int)sorted.size();i++){
        if(i) cout << ",";
        auto& r = sorted[i];
        char score_buf[32]; snprintf(score_buf,sizeof(score_buf),"%.4f",r.score);
        char dist_buf[32];  snprintf(dist_buf, sizeof(dist_buf), "%.1f",r.distance_km);
        cout << "{\"college\":" << college_to_json(*r.col)
             << ",\"score\":"   << score_buf
             << ",\"distance_km\":" << dist_buf
             << ",\"verdict\":"     << jstr(r.verdict)
             << ",\"matched_branch\":" << jstr(r.matched_branch)
             << "}";
    }
    cout << "]";
}

static void cmd_compare(int id1, int id2, const vector<College>& colleges){
    const College* c1 = nullptr;
    const College* c2 = nullptr;
    for(auto& c : colleges){
        if(c.id == id1) c1 = &c;
        if(c.id == id2) c2 = &c;
    }
    cout << "{\"c1\":" << (c1 ? college_to_json(*c1) : "null")
         << ",\"c2\":" << (c2 ? college_to_json(*c2) : "null") << "}";
}

static void cmd_sort(const string& criterion, const vector<College>& colleges){
    vector<College> arr = colleges;
    if(criterion == "package"){
        arr = merge_sorted(arr, [](const College& c){ return -c.avg_package; }, false);
    } else if(criterion == "fees"){
        arr = merge_sorted(arr, [](const College& c){ return c.fees; }, false);
    } else if(criterion == "placement"){
        arr = merge_sorted(arr, [](const College& c){ return -c.placement_pct; }, false);
    } else { // default: rank
        arr = merge_sorted(arr, [](const College& c){ return (double)c.nirf_rank; }, false);
    }
    cout << "[";
    for(int i=0;i<(int)arr.size();i++){
        if(i) cout << ",";
        cout << college_to_json(arr[i]);
    }
    cout << "]";
}

static void cmd_autocomplete(const string& prefix, const vector<College>& colleges){
    Trie trie;
    for(auto& c : colleges) trie.insert(c.name, c.id);
    auto results = trie.autocomplete(prefix);
    cout << "[";
    for(int i=0;i<(int)results.size();i++){
        if(i) cout << ",";
        cout << jstr(results[i]);
    }
    cout << "]";
}

static void cmd_cutoffs(int college_id, const string& exam_type,
                         const string& category, const vector<College>& colleges){
    const College* col = nullptr;
    for(auto& c : colleges) if(c.id == college_id){ col = &c; break; }
    if(!col){ cout << "null"; return; }

    const CutoffMap* cmap = nullptr;
    if(exam_type == "adv")      cmap = &col->adv_cutoffs;
    else if(exam_type=="main_hs") cmap = &col->main_hs_cutoffs;
    else                          cmap = &col->main_os_cutoffs;

    cout << "{\"college\":" << jstr(col->name)
         << ",\"type\":"    << jstr(col->type)
         << ",\"branches\":{";
    bool first = true;
    for(auto it = cmap->begin(); it != cmap->end(); ++it){
        if(!first) cout << ",";
        first = false;
        auto it2 = it->second.find(category);
        string val = it2 != it->second.end() ? to_string(it2->second) : "\"N/A\"";
        cout << jstr(it->first) << ":" << val;
    }
    cout << "}}";
}

static void cmd_colleges(const vector<College>& colleges){
    cout << "[";
    for(int i=0;i<(int)colleges.size();i++){
        if(i) cout << ",";
        cout << college_to_json(colleges[i]);
    }
    cout << "]";
}

int main(int argc, char* argv[]){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if(argc < 2){ cerr << "Usage: ./algorithms <command> [args...]\n"; return 1; }

    vector<College> colleges = load_college_data();
    string cmd = argv[1];

    if(cmd == "recommend" && argc >= 3){
        cmd_recommend(argv[2], colleges);
    } else if(cmd == "compare" && argc >= 4){
        cmd_compare(stoi(argv[2]), stoi(argv[3]), colleges);
    } else if(cmd == "sort" && argc >= 3){
        cmd_sort(argv[2], colleges);
    } else if(cmd == "autocomplete" && argc >= 3){
        cmd_autocomplete(argv[2], colleges);
    } else if(cmd == "cutoffs" && argc >= 5){
        cmd_cutoffs(stoi(argv[2]), argv[3], argv[4], colleges);
    } else if(cmd == "colleges"){
        cmd_colleges(colleges);
    } else {
        cerr << "Unknown command: " << cmd << "\n";
        return 1;
    }
    return 0;
}
