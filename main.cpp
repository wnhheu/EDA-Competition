#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <sstream>
typedef unsigned uint;

enum Instructions
{
    NOP, SS, SDSN, CRPD, CRSP,
    CRSN, CRPS, APOS, APSS, COSN
};


class my_data_struct
{
private:
    struct my_port;

    template<typename T>
    struct _name_num
    {
        T num;
        std::string name;
        explicit _name_num(T my_num = -1, std::string my_name = "NOP")
        {
            num = my_num;
            name = std::move(my_name);
        }
    };

    struct supply_net
    {
        std::string net_name;
        std::vector<my_port *> ends;
        supply_net(std::string n = "NOP")
        {
            net_name = n;
        }
        bool operator ==(const std::string &str)
        {
            return this->net_name == str;
        }
    };

    struct my_port
    {
        double state;
        std::string name;
        std::vector<_name_num<double>> table;
        my_port(double s = 0, std::string n = "NOP")
        {
            state = s;
            name = n;
        }

        bool operator ==(const std::string &str)
        {
            return this->name == str;
        }

    };

    struct power_domain
    {
        uint num;
        std::string name;
        supply_net pn;
        supply_net gn;
    };

    struct compare {
        template<typename T>
        bool operator()(T &a, T &b) const { return a.name < b.name; }
    };

    struct pst_state
    {
        std::string name;
        std::vector<std::string> state;
    };

    struct my_pst
    {
        uint size;
        std::string name;
        std::vector<std::string> port;
        std::vector<pst_state> pst_table;
        bool operator ==(const std::string &str)
        {
            return this->name == str;
        }
    };

    struct my_scope
    {
        power_domain PD;
        _name_num<int> name_num;
        uint port_size;
        my_scope *far;
        std::vector<supply_net> nets;
        std::vector<my_scope *> child;
        std::vector<my_port> port;
        std::vector<my_pst> pst;
        my_scope()
        {
            far = nullptr;
            port_size = 0;
        }
    };

    void add_scope(std::string name)
    {

        my_scope *tmp = new my_scope;
        tmp = new my_scope;
        tmp->name_num.num = _size++;
        tmp->name_num.name = name;
        root->child.push_back(tmp);
        tmp->far = cur_scope;
        cur_scope = tmp;
        scope_list.push_back(*cur_scope);
        return;

    }

    void add_port(std::string name)
    {
        my_port port(0, name);
        cur_scope->port.push_back(port);
        std::sort(cur_scope->port.begin(), cur_scope->port.end(), compare());
    }

    void add_port_state(std::string str)
    {
        std::stringstream sstream(str);
        std::string name;
        sstream >> name;
        std::vector<my_port>::iterator it;
        it = find(cur_scope->port.begin(), cur_scope->port.end(), name);
        double val;
        std::string state_name;
        std::string str_val;
        _name_num<double> tmp;
        while(sstream)
        {
            sstream >> state_name;
            if(state_name[0] == '-')
                continue;
            sstream >> str_val;
            state_name = state_name.substr(1);
            std::stringstream convert(str_val.substr(0, str_val.length() - 1));
            convert >> val;
            tmp.name = state_name;
            tmp.num = val;
            it->table.push_back(tmp);
        }
    }

    void update_power_domain(std::string str)
    {
        std::stringstream sstream(str);
        std::string name;
        std::string domain;
        sstream >> name >> domain;
        cur_scope->PD.name = name;
        cur_scope->PD.num = cur_scope->name_num.num;
    }

    void add_supply_net(std::string name)
    {
        cur_scope->nets.push_back(supply_net(name));
    }

    void get_far_scope(){    // 如果当前scope是root？
        cur_scope = cur_scope->far;
        return;
    }

    void set_pd_net(std::string str)
    {
        std::stringstream sstream(str);
        std::string pd_name;
        std::string inst;
        std::string net_name;
        sstream >> pd_name;
        while(sstream)
        {
            sstream >> inst;
            sstream >> net_name;
            if(inst[9] == 'p')
                cur_scope->PD.pn.net_name = net_name;
            else
                cur_scope->PD.gn.net_name = net_name;
        }
    }

    void create_pst(std::string str)
    {
        std::stringstream sstream(str);
        std::string name;
        sstream >> name;
        std::string buffer;
        sstream >> buffer;
        std::string port_name;
        std::vector<std::string> port;
        my_pst tmp;
        while(sstream)
        {
            sstream >> port_name;
            if(port_name[0] == '{')
                port_name = port_name.substr(1);
            if(port_name[port_name.length() - 1] == '}')
                port_name = port_name.substr(0, port_name.length() - 1);
//            std::vector<my_port>::iterator it;
//            it = find(cur_scope->port.begin(), cur_scope->port.begin(), port_name);
            tmp.port.push_back(port_name);
            tmp.name = name;
            ++tmp.size;
        }
        cur_scope->pst.push_back(tmp);
    }

    void add_pst_state(std::string str)
    {
        std::stringstream sstream(str);
        std::string state_name;
        sstream >> state_name;
        std::string buffer;
        std::string pst_name;
        sstream >> buffer;
        sstream >> pst_name;
        sstream >> buffer;
        std::string state;
        std::vector<my_pst>::iterator it;
        it = find(cur_scope->pst.begin(), cur_scope->pst.end(), pst_name);
        pst_state tmp;
        tmp.name = state_name;
        while(sstream)
        {
            sstream >> state;
            if(state[0] == '{')
                state = state.substr(1);
            if(state[state.length() - 1] == '}')
                state = state.substr(0, state.length() - 1);
            tmp.state.push_back(state);
        }
        it->pst_table.push_back(tmp);
    };

    void connect_supply_net(std::string str) {
        std::stringstream sstream(str);
        std::string net_name;
        sstream >> net_name;
        std::string buffer;
        sstream >> buffer;
        std::vector<supply_net>::iterator nit;
        std::vector<my_port>::iterator pit;
        nit = find(cur_scope->nets.begin(), cur_scope->nets.end(), net_name);
        std::string port_name;
        while (sstream)
        {
            sstream >> port_name;
            if(port_name[0] == '{')
                port_name = port_name.substr(1);
            if(port_name[port_name.length() - 1] == '}')
                port_name = port_name.substr(0, port_name.length() - 1);
            pit = find(cur_scope->port.begin(), cur_scope->port.end(), port_name);
            nit->ends.push_back(&(*pit));
        }
    }

    uint _size;
    my_scope *root;
    my_scope *cur_scope;
    std::vector<my_scope> scope_list;
    std::vector<my_port> port_list;
public:
    void read()
    {
        freopen("D:\\EDA\\test.txt", "r",stdin);
        std::string buffer;
        while(std::getline(std::cin, buffer))
        {
            if(buffer[0] == 's')
            {
                if(buffer[4] == 's')        // set scope
                {
                    if(buffer[10] == '.')
                    {
                        get_far_scope();
                        continue;
                    }
                    add_scope(buffer.substr(10));
                }
                else                        // set domain supply net
                {
                    set_pd_net(buffer.substr(22));
                }
            }
            else if(buffer[0] == 'c' && buffer[1] == 'r')
            {
                if(buffer[8] == 's')        // create pst
                {
                    create_pst(buffer.substr(11));
                }
                else if(buffer[14] == 'o')  // create power domain
                {
                    update_power_domain(buffer.substr(20));
                }
                else if(buffer[14] == 'p')  // create supply port
                {
                    add_port(buffer.substr(19));
                }
                else                        // create supply net
                {
                    add_supply_net(buffer.substr(18));
                }
            }
            else if(buffer[1] == 'o')       // connect supply net
            {
//                connect_supply_net(buffer.substr(19));
            }
            else if(buffer[5] == 'o')       // add port state
            {
                add_port_state(buffer.substr(15));
            }
            else                            // add port state
            {
                add_pst_state(buffer.substr(14));
            }
        }
    }

    my_data_struct()
    {
        _size = 0;
        root = new my_scope;
        cur_scope = root;
    }
};



int main() {
    my_data_struct my_data_struct;
    my_data_struct.read();
    int x = 0;
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
