#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include<sstream>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <map>
#include <queue>
#include <unistd.h>
#include <ctime>

using namespace std;

class process{

    public:
    int pid;
    int num_pages;
    map<int,bool> page_distribution;
    map<int,int> page_table; // map from page number to frame number
    string file_name; 

} ;

ofstream outfile;
int flag = 0;
int globalpid = 1;
int size_of_mm;
int size_of_vm;
int page_frames_mm; // number of page frames in main memory
int page_frames_vm; // number of page frames in swap space
int page_size;
int page_frames_mm_used = 0;    // number of pages used in main memory
int page_frames_vm_used = 0; // number of pages used in swap space

map<int,process> process_map; // map to store process id and process object
map<int,int> page_occupied_mm; // map to store which pid is using which page frame in main memory
map<int,int> page_occupied_vm; // map to store which pid is using which page frame in swap space
map<int,map<int,int>> main_memory ; // map from frame number to map from offset to value
map<int,map<int,int>> swap_space ; // map from frame number to map from offset to value

map<int,int> lru_map; // map to store the time of last access of a page frame
int lru_time = 0; // time of last access


void load (vector<string> files)
{
    for(int i = 0 ; i< files.size();i++)
    {
        ifstream infile(files[i]) ;
        if(!infile)
        {
            outfile<<files[i]<<" could not be loaded - file does not exist"<<endl;
            continue;
        }
        string line;
        
        getline(infile,line);
            int ii = 0;
            string file_size_str ;
            while(line[ii] != ' ' && line[ii] != '\0')
            {
                file_size_str += line[ii];
                ii++;
            }
           // cout << "hi" << endl;
            int file_size = stoi(file_size_str)*1024 ;
            ii++;
            int num_pages = file_size/page_size ;
            process p;
            int free_mm = page_frames_mm - page_frames_mm_used ;
            int free_vm = page_frames_vm - page_frames_vm_used ;

            if(num_pages <= free_mm + free_vm)
            {
                p.pid = globalpid ;
                globalpid++;
                p.num_pages = num_pages ;
                p.file_name = files[i];
                process_map[p.pid] = p;
                outfile<<files[i]<<" is loaded and is assigned process id "<<p.pid<<endl;
                for(int j = 0 ; j < num_pages ;j++)
                {
                    if(page_frames_mm_used < page_frames_mm)
                    {
                        for(int k = 0 ; k < page_frames_mm ;k++)
                        {
                            if(page_occupied_mm[k] == 0 || page_occupied_mm.find(k) == page_occupied_mm.end()) // page frame is not occupied
                            {
                                page_occupied_mm[k] = p.pid;
                                page_frames_mm_used++;
                                p.page_distribution[j] = true; // page is in main memory
                                p.page_table[j] = k; // page is in frame k of main memory
                                process_map[p.pid] = p; // updating the process object in the process map
                                lru_map[k] = lru_time; // updating the lru map
                                lru_time++;
                                break;
                            }
                        }
                    } else {
                        for(int k =0 ; k<page_frames_vm ;k++)
                        {
                            if(page_occupied_vm[k] == 0 || page_occupied_vm.find(k) == page_occupied_vm.end()) // page frame is not occupied
                            {
                                page_occupied_vm[k] = p.pid;
                                page_frames_vm_used++;
                                // swap_space[k] = main_memory[p.page_table[j]]; // swapping the data from main memory to swap space
                                p.page_distribution[j] = false; // page is in swap space
                                p.page_table[j] = k; // page is in frame k of swap space
                                process_map[p.pid] = p; // updating the process object in the process map
                                break;
                            }
                    }
                }
            }
            }
            else
            {
                outfile<< files[i] <<" could not be loaded - memory is full"<<endl;
            }      
        
    }
}

void swap(int page_num,int pid)
{
    process& p = process_map[pid];
    
    int frame_num = p.page_table[page_num]; // frame number of the page in swap space (i need to swap this page with a page frame in main memory)

    // first i need to find a page frame in main memory which is not occupied and swap the data stored in that page frame with the data stored in the page frame in swap space
    if(page_frames_mm_used < page_frames_mm)
    {
        for(int i = 0 ; i<page_frames_mm ;i++)
        {
            if(page_occupied_mm[i] == 0 || page_occupied_mm.find(i) == page_occupied_mm.end()) // page frame is not occupied
            {
                page_occupied_mm[i] = pid;
                page_frames_mm_used++;

                main_memory[i] = swap_space[frame_num];

                page_occupied_vm[frame_num] = 0; // freeing the page frame in swap space
                
                // updating the page table of the process
                p.page_distribution[page_num] = true; // page is now in main memory
                p.page_table[page_num] = i; // page is now in frame i of main memory
                process_map[pid] = p; // updating the process object in the process map
                lru_map[i] = lru_time; // updating the lru map
                lru_time++;
                page_frames_vm_used--; // decreasing the number of pages used in swap space
                break;
            }
        }
    }    else {
        // i need to find the page frame in main memory which is least recently used
        int frame_num_to_swap = -1;
        int min_time = 1e9;

        for(auto it = lru_map.begin() ; it != lru_map.end() ; it++)
        {
            if(it->second < min_time)
            {
                min_time = it->second;
                frame_num_to_swap = it->first;
            }
        }


        // process that is currently using the page frame in main memory which is to be swapped
        int pid_to_swap = page_occupied_mm[frame_num_to_swap];
        int page_num_to_swap = -1;
        
        process& p1 = process_map[pid_to_swap];
     //   cout << p1.page_table.size() << endl;
        for(auto it = p1.page_table.begin() ; it != p1.page_table.end() ; it++)
        {
            if(it->second == frame_num_to_swap && p1.page_distribution[it->first] == true)
            {
                page_num_to_swap = it->first;
                break;
            }
        }

       // cout << "page_num_to_swap" << page_num_to_swap << endl;

        map<int,int> data_to_swap = main_memory[frame_num_to_swap]; // data stored in the page frame in main memory which is to be swapped
        main_memory[frame_num_to_swap] = swap_space[frame_num]; // swapping the data
        swap_space[frame_num] = data_to_swap; // swapping the data
        p.page_distribution[page_num] = true ; // page is now in main memory
        p.page_table[page_num] = frame_num_to_swap; // page is now in frame frame_num_to_swap of main memory
        
        lru_map[frame_num_to_swap] = lru_time; // updating the lru map
        lru_time++;

        p1.page_distribution[page_num_to_swap] = false ; // page is now in swap space
        p1.page_table[page_num_to_swap] = frame_num; // page is now in frame frame_num of swap space
        page_occupied_vm[frame_num] = pid_to_swap; // updating the page_occupied_vm map

        //cout << p.page_distribution[page_num] << endl;
    }

}



void run(int pid)
{
    if(process_map.find(pid) == process_map.end())
    {
        outfile<<"Invalid PID "<< pid <<endl;
        return;
    }
    process p = process_map[pid];
    string file_name = p.file_name;

    ifstream infile(file_name);
    if(!infile)
    {
        outfile<<"file does not exist"<<endl;
        return;
    }

    string line;
    while(getline(infile,line))
    {
       process p = process_map[pid];
        int i = 0 ;
        string command;

        while(line[i] != ' ')
        {
            command += line[i];
            i++;
        }

        if(command == "load")
        {
            int num ; // number that has to be loaded
            string num_str;
            i++;
            while(line[i] != ',')
            {
                num_str += line[i];
                i++;
            }
            num = stoi(num_str);
            
            int dest; // destination where the number has to be loaded
            string dest_str;
            i++;
            while(i<line.size())
            {
                dest_str += line[i];
                i++;
            }
            dest = stoi(dest_str);

            int page = dest/page_size;

            if(p.num_pages*page_size <= dest)
            {
                outfile<<"Invalid Memory Address " << dest << " specified for process id " << pid << endl;
                return;
            }



            if(p.page_distribution[page] == false) // page is in swap space
            {   
                // cout << command << endl;
                // cout << dest << endl;
                // cout << "page" << page << endl;
                swap(page,pid);
            }

            p = process_map[pid];

            int frame = p.page_table[page];
            int offset = dest%page_size;

            main_memory[frame][offset] = num;

            lru_map[frame] = lru_time; // updating the lru map
            lru_time++;

            outfile << "Command: " << command << " " << num << "," << dest << "; " << "Result: Value of " << num << " is now stored in addr " << dest << endl;

        }

         else  if(command == "print")
        {
            int mem_loc ;
            string mem_loc_str;

            i++;
            while(i<line.size())
            {
                mem_loc_str += line[i];
                i++;
            }
            mem_loc = stoi(mem_loc_str);

            if(mem_loc > p.num_pages*page_size)
            {
                outfile<<"Invalid Memory Address " << mem_loc << " specified for process id " << pid << endl;
                return;
            }

            int page = mem_loc/page_size;
            if(p.page_distribution[page] == false) // page is in swap space
            {
                // cout << command << endl;
                // cout << mem_loc << endl;
                swap(page,pid);
            }

            p = process_map[pid];

            int frame = p.page_table[page];
            int offset = mem_loc%page_size;

            lru_map[frame] = lru_time; // updating the lru map
            lru_time++;

            outfile<<"Command: " << command << " " << mem_loc << "; " << "Result: Value in addr " << mem_loc << " = " << main_memory[frame][offset] << endl; 

        }

        else if(command == "add")
        {
            int addr1 , addr2 , dest;
            string addr1_str , addr2_str , dest_str;

            i++;
            while(line[i] != ',')
            {
                addr1_str += line[i];
                i++;
            }
            addr1 = stoi(addr1_str);

            if(addr1 >= p.num_pages*page_size)
            {
                outfile<<"Invalid Memory Address " << addr1 << " specified for process id " << pid << endl;
                return;
            }

            i++;
            while(line[i] != ',')
            {
                addr2_str += line[i];
                i++;
            }
            addr2 = stoi(addr2_str);

            if(addr2 >= p.num_pages*page_size)
            {
                outfile<<"Invalid Memory Address " << addr2 << " specified for process id " << pid << endl;
                return;
            }

            i++;

            while(i<line.size())
            {
                dest_str += line[i];
                i++;
            }

            dest = stoi(dest_str);

            if(dest >= p.num_pages*page_size)
            {
                outfile<<"Invalid Memory Address " << dest << " specified for process id " << pid << endl;
                return;
            }

            int page1 = addr1/page_size;
            int page2 = addr2/page_size;
            int page3 = dest/page_size;

            if(p.page_distribution[page1] == false) // page is in swap space
            {
                swap(page1,pid);
            }

            p = process_map[pid];

            int frame1 = p.page_table[page1];
            int offset1 = addr1%page_size;

            int val1 = main_memory[frame1][offset1];

            lru_map[frame1] = lru_time; // updating the lru map
            lru_time++;

            if(p.page_distribution[page2] == false) // page is in swap space
            {
                swap(page2,pid);
            }

            p = process_map[pid];

            int frame2 = p.page_table[page2];
            int offset2 = addr2%page_size;

            int val2 = main_memory[frame2][offset2];

            lru_map[frame2] = lru_time; // updating the lru map
            lru_time++;

            int sum = val1 + val2;

            if(p.page_distribution[page3] == false) // page is in swap space
            {
                swap(page3,pid);
            }

            p = process_map[pid];

            int frame3 = p.page_table[page3];
            int offset3 = dest%page_size;

            main_memory[frame3][offset3] = sum;

            lru_map[frame3] = lru_time; // updating the lru map
            lru_time++;

            outfile << "Command: " << command << " " << addr1 << "," << addr2 << "," << dest << "; " << "Result: Value in " << "addr " << addr1 << " = " << val1 << " , addr " << addr2 << " = " << val2 << " , addr " << dest << " = " << sum << endl;

        }

        else if(command == "sub")
        {
            int addr1 , addr2 , dest;
            string addr1_str , addr2_str , dest_str;

            i++;
            while(line[i] != ',')
            {
                addr1_str += line[i];
                i++;
            }
            addr1 = stoi(addr1_str);

            if(addr1 >= p.num_pages*page_size)
            {
                outfile<<"Invalid Memory Address " << addr1 << " specified for process id " << pid << endl;
                return;
            }

            i++;
            while(line[i] != ',')
            {
                addr2_str += line[i];
                i++;
            }
            addr2 = stoi(addr2_str);

            if(addr2 >= p.num_pages*page_size)
            {
                outfile<<"Invalid Memory Address " << addr2 << " specified for process id " << pid << endl;
                return;
            }

            i++;

            

            while(i<line.size())
            {
                dest_str += line[i];
                i++;
            }

            dest = stoi(dest_str);

            if(dest >= p.num_pages*page_size)
            {
                outfile<<"Invalid Memory Address " << dest << " specified for process id " << pid << endl;
                return;
            }

            int page1 = addr1/page_size;
            int page2 = addr2/page_size;
            int page3 = dest/page_size;

            if(p.page_distribution[page1] == false) // page is in swap space
            {
                swap(page1,pid);
            }


            p = process_map[pid];
            int frame1 = p.page_table[page1];
            int offset1 = addr1%page_size;

            int val1 = main_memory[frame1][offset1];

            lru_map[frame1] = lru_time; // updating the lru map
            lru_time++;

            if(p.page_distribution[page2] == false) // page is in swap space
            {
                swap(page2,pid);
            }

            p = process_map[pid];

            int frame2 = p.page_table[page2];
            int offset2 = addr2%page_size;

            int val2 = main_memory[frame2][offset2];

            lru_map[frame2] = lru_time; // updating the lru map
            lru_time++;

            int diff = val1 - val2;

            if(p.page_distribution[page3] == false) // page is in swap space
            {
                swap(page3,pid);
            }

            p = process_map[pid];

            int frame3 = p.page_table[page3];
            int offset3 = dest%page_size;

            main_memory[frame3][offset3] = diff;

            lru_map[frame3] = lru_time; // updating the lru map
            lru_time++;

            outfile << "Command: " << command << " " << addr1 << "," << addr2 << "," << dest << "; " << "Result: Value in " << "addr " << addr1 << " = " << val1 << " , addr " << addr2 << " = " << val2 << " , addr " << dest << " = " << diff << endl;

        }

    }

}


void kill(int pid)
{
    if(process_map.find(pid) == process_map.end())
    {   
        if(flag == 0)
        {outfile<<"Invalid PID "<< pid <<endl;}
        return;
    }
    process p = process_map[pid];
    for(auto it = p.page_table.begin() ; it != p.page_table.end() ; it++)
    {
        if(p.page_distribution[it->first] == true) // page is in main memory
        {
            int frame = p.page_table[it->first];
            page_occupied_mm[frame] = 0; // freeing the page frame in main memory
            page_frames_mm_used--; // decreasing the number of pages used in main memory
        } else {
            int frame = p.page_table[it->first];
            page_occupied_vm[frame] = 0; // freeing the page frame in swap space
            page_frames_vm_used--; // decreasing the number of pages used in swap space
        }
    }
    process_map.erase(pid);
    p.pid = 0;

    // removing the pageframes occupied by the process from lru map

   vector<int> to_remove;
    for(auto it = lru_map.begin() ; it != lru_map.end() ; it++)
    {
        if(page_occupied_mm[it->first] == pid)
        {
            to_remove.push_back(it->first);
        }
    }

    for(int i = 0 ; i<to_remove.size() ;i++)
    {
        lru_map.erase(to_remove[i]);
    }

    if(flag == 0)
    {
        outfile<<"killed " << pid << endl;
    }
}

void listpr()
{   
    //cout << "hi" << endl ;
    for(auto it = process_map.begin() ; it != process_map.end() ; it++)
    {
        outfile<<it->first<<" " ;
    }
    outfile<<endl;
}

void pte(int pid,string file) // print the page table entry into the file
{   
    // have to include date and time in the file
    if(process_map.find(pid) == process_map.end())
    {
        outfile<<"Invalid PID is given"<<endl;
        return;
    }

    process p = process_map[pid];
    
    ofstream outfile1(file, ios::app);

    if(!outfile1)
    {
        outfile<<"file does not exist"<<endl;
        return;
    }

   // print date and time
    time_t now = time(nullptr);
    tm* local_time = localtime(&now);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%a %b %d %I:%M:%S %p IST %Y", local_time);
    outfile1 << buffer << endl;

    for(auto it = p.page_table.begin() ; it != p.page_table.end() ; it++)
    {   
      //  cout << "namaste" << endl;
        outfile1 << it->first << " " << it->second << " " << p.page_distribution[it->first] << endl;
    }
}

void pteall(string file) // print the page table entry of all the processes into the file
{
    ofstream outfile1(file, ios::app);
    if(!outfile1)
    {
        outfile<<"file does not exist"<<endl;
        return;
    }   

  // print date and time
    time_t now = time(nullptr);
    tm* local_time = localtime(&now);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%a %b %d %I:%M:%S %p IST %Y", local_time);
    outfile1 << buffer << endl;

    for(auto it = process_map.begin() ; it != process_map.end() ; it++)
    {
        process p = it->second;
        for(auto it1 = p.page_table.begin() ; it1 != p.page_table.end() ; it1++)
        {
            outfile1 << p.pid << " " << it1->first << " " << it1->second << " " << p.page_distribution[it1->first] << endl;
        }
    }
}

void print(int memloc , int length)
{
    int temp = 0 ;

    

    while (temp < length)
    {   
        int page = (memloc + temp)/page_size;
        int offset = (memloc + temp)%page_size;

        if(memloc + temp >= size_of_mm)
        {
            outfile<<"Invalid Memory Address "<<memloc + temp<<" specified"<<endl;
            return;
        }

        outfile<<"Value of "<<memloc + temp<<" = "<<main_memory[page][offset]<<endl;
        temp++;
    }
}

void exit()
{   
    flag = 1;
    for(auto it = process_map.begin() ; it != process_map.end() ; it++)
    {
        kill(it->first);
    }
    flag = 0;
}

int main(int argc, char *argv[])
{   
    string input_file;
    string output_file;
    int opt ;

    while((opt = getopt(argc,argv,"M:V:P:i:o:")))
    {   
        if(opt == -1)
        {
            break;
        }
        switch(opt)
        {
            case 'M':
                size_of_mm = stoi(optarg)*1024;
                break;
            case 'V':
                size_of_vm = stoi(optarg)*1024;
                break;
            case 'P':
                page_size = stoi(optarg);
                break;
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case -1:
                break;
            default:
                cout<<"Invalid arguments"<<endl;
                return 0;
        }
    }

    // cout << "Memory size: " << size_of_mm << " bytes" << endl;
    // cout << "Page size: " << page_size << " bytes" << endl;
    // cout << "Swap size: " << size_of_vm << " bytes" << endl;
    // cout << "input file: " << input_file << endl;
    // cout << "output file: " << output_file << endl;


    page_frames_mm = size_of_mm/page_size ;
    page_frames_vm = size_of_vm/page_size ;

    ifstream infile(input_file);
    
    outfile.open(output_file);

    string line;

    if(!infile)
    {
        cout<<"Error in opening file"<<endl;
        return 0;
    }

    while(getline(infile,line))
    {
        string command;

        int i = 0;
        while(line[i] != ' ' && line[i] != '\0')
        {
            command += line[i];
            i++;
        }
      // cout << command << endl;
        if(command == "load")
        {
            vector<string> files ;
            string file;
            i++;
            while(i<line.size())
            {
                if(line[i] == ' ')
                {
                    files.push_back(file);
                    file = "";
                }
                else
                {
                    file += line[i];
                }
                i++;
            }
            files.push_back(file);
            load(files);
        }

        else if(command == "run")
        {
            int pid;
            i++;
            string pid_str;
            while(i<line.size())
            {
                pid_str += line[i];
                i++;
            }
            pid = stoi(pid_str);
            run(pid);
        }
         else if(command == "kill")
         {
            int pid;
            i++;
            string pid_str;
            while(i<line.size())
            {
                pid_str += line[i];
                i++;
            }
            pid = stoi(pid_str);
            kill(pid);
         }

         else if(command == "listpr")
         {  
           // cout << "hi11" << endl ;
            listpr();
         }

         else if(command == "pte")
         {
            int pid ;
            string file ;
            i++;
            string pid_str;
            while(line[i] != ' ')
            {
                pid_str += line[i];
                i++;
            }
            pid = stoi(pid_str);
            i++;
            while(i<line.size())
            {
                file += line[i];
                i++;
            }
            pte(pid,file);
         }

         else if(command == "pteall")
         {
            string file ;
            i++;
            while(i<line.size())
            {
                file += line[i];
                i++;
            }
            pteall(file);
         }

         else if(command  == "print")
         {
            int mem_loc;
            int length;
            i++;
            string mem_loc_str;
            while(line[i] != ' ')
            {
                mem_loc_str += line[i];
                i++;
            }
            mem_loc = stoi(mem_loc_str);
            i++;
            string length_str;
            while(i<line.size())
            {
                length_str += line[i];
                i++;
            }
            length = stoi(length_str);
            print(mem_loc,length);
         }

         else if(command == "exit")
         {
            exit();
         }

    }

    infile.close();
    outfile.close();
    return 0;

}
