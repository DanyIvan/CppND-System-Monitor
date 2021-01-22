  #include <dirent.h>
  #include <unistd.h>
  #include <string>
  #include <vector>
  #include <iostream>
  #include <experimental/filesystem>
  #include <sys/time.h>

  #include "linux_parser.h"
  #include <unistd.h>

  using std::stof;
  using std::string;
  using std::to_string;
  using std::vector;


  // DONE: An example of how to read data from the filesystem
  string LinuxParser::OperatingSystem() {
    string line;
    string key;
    string value;
    std::ifstream filestream(kOSPath);
    if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
        std::replace(line.begin(), line.end(), ' ', '_');
        std::replace(line.begin(), line.end(), '=', ' ');
        std::replace(line.begin(), line.end(), '"', ' ');
        std::istringstream linestream(line);
        while (linestream >> key >> value) {
          if (key == "PRETTY_NAME") {
            std::replace(value.begin(), value.end(), '_', ' ');
            return value;
          }
        }
      }
    }
    return value;
  }

  // DONE: An example of how to read data from the filesystem
  string LinuxParser::Kernel() {
    string os, kernel;
    string line;
    std::ifstream stream(kProcDirectory + kVersionFilename);
    if (stream.is_open()) {
      std::getline(stream, line);
      std::istringstream linestream(line);
      linestream >> os >> kernel;
    }
    return kernel;
  }

//  BONUS: Update this to use std::filesystem
  vector<int> LinuxParser::Pids() {
    vector<int> pids;
    DIR* directory = opendir(kProcDirectory.c_str());
    struct dirent* file;
    while ((file = readdir(directory)) != nullptr) {
      // Is this a directory?
      if (file->d_type == DT_DIR) {
        // Is every character of the name a digit?
        string filename(file->d_name);
        if (std::all_of(filename.begin(), filename.end(), isdigit)) {
          int pid = stoi(filename);
          pids.push_back(pid);
        }
      }
    }
    closedir(directory);
    return pids;
  }

  // TODO: Read and return the system memory utilization
  float LinuxParser::MemoryUtilization() {
    std::ifstream filestream(kProcDirectory + kMeminfoFilename);
    string line1, line2;
    string mem_name;
    float mem_total, mem_free;
    if (filestream.is_open()) {     
          std::getline(filestream, line1);
          std::istringstream linestream1(line1);
          linestream1 >> mem_name >> mem_total;

          std::getline(filestream, line2);
          std::istringstream linestream2(line2);
          linestream2 >> mem_name >> mem_free;

      }
    return (mem_total - mem_free) / mem_total; 
    }

  // TODO: Read and return the system uptime
  long LinuxParser::UpTime() {
    std::ifstream filestream(kProcDirectory + kUptimeFilename);
    string line;
    long uptime, idletime;

    if (filestream.is_open()){
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> uptime >> idletime;
    }
    return uptime; 
    
    }

  // TODO: Read and return the number of jiffies for the system
  long LinuxParser::Jiffies() { 
    string line;
    string cpu;
    long jiffies = 0, i =0;
    std::ifstream filestream(kProcDirectory + kStatFilename);

    if(filestream.is_open()){
      std::getline(filestream, line); 
      std::istringstream linestream(line);
      linestream >> cpu;
      while(linestream >> i){
        jiffies = jiffies + i;
      }   
    }
    return jiffies;
    }

  // TODO: Read and return the number of active jiffies for a PID
  // REMOVE: [[maybe_unused]] once you define the function
  long LinuxParser::ActiveJiffies(int pid) { 
    string dir = kProcDirectory + std::to_string(pid) + kStatFilename;
    string line, word;
    long jiffies;
    std::ifstream filestream(dir);

    if (filestream.is_open()){
      std::getline(filestream, line);
      std::istringstream linestream(line);
      for (int i=1; i<=21; i++) {
        linestream >> word;
      }
      linestream >> jiffies;
    }

    return jiffies; 

  }

  // TODO: Read and return the number of active jiffies for the system
  long LinuxParser::ActiveJiffies() { 
    string line;
    string cpu;
    long user, nice, system, idle, iowait, irq, softirq;
    std::ifstream filestream(kProcDirectory + kStatFilename);

    if(filestream.is_open()){
      std::getline(filestream, line); 
      std::istringstream linestream(line);
      linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
    }
    return user + nice + system;
  }

  // TODO: Read and return the number of idle jiffies for the system
  long LinuxParser::IdleJiffies() { 
    string line;
    string cpu;
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    std::ifstream filestream(kProcDirectory + kStatFilename);

    if(filestream.is_open()){
      std::getline(filestream, line); 
      std::istringstream linestream(line);
      linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >>
        softirq >> steal >> guest >> guest_nice;
      
    }
    return idle + iowait;
  }

  // TODO: Read and return CPU utilization
  vector<string> LinuxParser::CpuUtilization() { 
    string dir = kProcDirectory + kStatFilename;
    string line, cpu;
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    vector<string> cpu_utilization;
    std::ifstream filestream(dir);
    if (filestream.is_open()){
      while(std::getline(filestream, line)){ 
        std::istringstream linestream(line);
        linestream >> cpu;
        if (cpu.substr(0,3) != "cpu") return cpu_utilization;
        linestream >> user >> nice >> system >> idle >> iowait >> irq >>
          softirq >> steal >> guest >> guest_nice;
        float _active = user + nice + system;
        float _idle = idle + iowait;
        float usage = _active / (_active + _idle);
        cpu_utilization.push_back(std::to_string(usage));
        
      }
    }
    return cpu_utilization; 
  }

  // TODO: Read and return a pid CPU utilization
  float LinuxParser::PidCpuUtilization(int pid) {
    string dir = kProcDirectory + to_string(pid) + kStatFilename;
    string line, value;
    int utime, stime, starttime, cutime, cstime, seconds;
    float cpu;
    std::ifstream filestream(dir);
    if(filestream.is_open()){
      std::string line;
        std::getline(filestream, line);
        std::istringstream linestream(line);
        for(int i = 1; i <= 13; i++) linestream >> value;
        linestream >> utime >> stime >> cutime >> cstime ;
        for(int i = 1; i <= 4; i++) linestream >> value;
        linestream >> starttime;
        int hertzs = sysconf(_SC_CLK_TCK);
        seconds = UpTime() - (starttime/hertzs);
        long totalTime = (utime + stime + cutime + cstime)/hertzs;
        cpu = 1.0*  totalTime/ seconds;
        return cpu;
    }
    return 0.;
  }

  // TODO: Read and return the total number of processes
  int LinuxParser::TotalProcesses() { 
    string line;
    string name;
    int total;
    std::ifstream filestream(kProcDirectory + kStatFilename);

    if(filestream.is_open()){
      while(std::getline(filestream, line)){
        std::istringstream linestream(line);
        linestream >> name;
        if(name == "processes"){
          linestream >> total;
        }
      }
    }
    return total;
  }

  // TODO: Read and return the number of running processes
  int LinuxParser::RunningProcesses() { 
    string line;
    string name;
    int running;
    std::ifstream filestream(kProcDirectory + kStatFilename);

    if(filestream.is_open()){
      while(std::getline(filestream, line)){
        std::istringstream linestream(line);
        linestream >> name;
        if(name == "procs_running"){
          linestream >> running;
        }
      }
    }
    return running;

  }

  // TODO: Read and return the command associated with a process
  // REMOVE: [[maybe_unused]] once you define the function
  string LinuxParser::Command(int pid) { 
    string dir = kProcDirectory + std::to_string(pid) + kCmdlineFilename;
    std::ifstream filestream(dir);
    string command;

    if(filestream.is_open()){
      std::getline(filestream, command);
    }
    return command;
  }
  

  // TODO: Read and return the memory used by a process
  // REMOVE: [[maybe_unused]] once you define the function
  string LinuxParser::Ram(int pid) { 
    string dir = kProcDirectory + std::to_string(pid) + kStatusFilename;
    string line, key, unit;
    long size = 0;
    std::ifstream filestream(dir);
    if(filestream.is_open()){
      while(std::getline(filestream, line)){
        std::istringstream linestream(line);
        linestream >> key;
        if(key == "VmSize:"){
          linestream >> size >> unit;
        }         
      }
    }
    int ram = size / 1024;
    return to_string(ram);
  }


  // TODO: Read and return the user ID associated with a process
  // REMOVE: [[maybe_unused]] once you define the function
  string LinuxParser::Uid(int pid) { 
    string dir = kProcDirectory + std::to_string(pid) + kStatusFilename;
    string line, key, uid;
    std::ifstream filestream(dir);
    if(filestream.is_open()){
      while(std::getline(filestream, line)){
        std::istringstream linestream(line);
        linestream >> key;
        if(key == "Uid:"){
          linestream >> uid;
        }
      }
    }
    return uid;
  }

  // TODO: Read and return the user associated with a process
  // REMOVE: [[maybe_unused]] once you define the function
  string LinuxParser::User(int pid) { 
    std:: ifstream filestream(kPasswordPath);
    string line;
    string uid = Uid(pid);
    string uname, name, x, id;
    if(filestream.is_open()){
      while(std::getline(filestream, line)){
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream linestream(line);
        linestream >> name >> x >> id;
        if(id == uid){
          uname = name;
        }
      }
    }
    return uname;
  }

  // TODO: Read and return the uptime of a process
  // REMOVE: [[maybe_unused]] once you define the function
  long LinuxParser::UpTime(int pid) { 
    string dir = kProcDirectory + to_string(pid) + kStatFilename;
    std::ifstream filestream(dir);
    string line, value;
    long uptime;
    if(filestream.is_open()){
      getline(filestream, line);
      std::istringstream linestream(line);
      for(int i = 1; i <= 21; i++){
        linestream >> value;
      }
      linestream >> uptime;
    }
    return UpTime() - uptime/sysconf(_SC_CLK_TCK);
  }
