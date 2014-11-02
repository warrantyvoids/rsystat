#include <iostream>
#include <curses.h>
#include <cmath>
#include <algorithm>

#include "Connection.h"
#include "Configuration.h"

static std::vector<Connection> connections;

void offending() {
  std::vector<Process> processes;
  for (Connection& conn : connections) {
    if (!conn)
      break;
    
    std::vector<Process> process = conn.getOS()->getProcesses();
    for (Process& p : process)
      processes.push_back(p);
  }
  std::sort(processes.begin(), processes.end(), [](Process a, Process b) -> bool {
    return a.time > b.time;
  });
  
  clear();
  refresh();
  
  int maxx, maxy;
  getmaxyx(stdscr, maxy, maxx);
  char action;
  do {
    clear();
    mvaddstr(0,0, "Host");
    mvaddstr(0,20, "Program");
    mvaddstr(0,40, "User");
    mvaddstr(0,50, "PID");
    mvaddstr(0,60, "CPU");
    mvaddstr(0,66, "MEM");
    mvaddstr(0,72, "Time");
    int row = 1;
    for (Process& p : processes) {
      mvaddnstr(row, 0, p.connection->getHostname().c_str(), 19);
      mvaddnstr(row, 20, p.command.c_str(), 19);
      mvaddnstr(row, 40, p.user.c_str(), 10);
      mvaddnstr(row, 50, std::to_string(p.pid).c_str(), 10);
      mvaddnstr(row, 60, std::to_string(p.cpu).c_str(), 5);
      mvaddnstr(row, 66, std::to_string(p.mem).c_str(), 5);
      mvaddnstr(row, 72, std::to_string(p.time / 60.0 / 60.0).c_str(), 8);
      
      row++;
      if (row == maxy)
        break;
    }
    refresh();
    action = getchar();
    if (action == 'c') {
      std::sort(processes.begin(), processes.end(), [](Process a, Process b) -> bool {
        return a.cpu > b.cpu;
      });
    } else if (action == 'm') {
      std::sort(processes.begin(), processes.end(), [](Process a, Process b) -> bool {
        return a.mem > b.mem;
      });
    }
  } while (action != 'q');
  
}

int main(int argc, char** argv) {
  Configuration conf;
  
  connections.reserve(conf.getHosts().size());

  for (std::string hostname : conf.getHosts()) {
    std::cout << hostname << std::endl;
    connections.emplace_back( hostname );
  }
  

  for (Connection& conn : connections) {
    if (!conn) {
      std::cerr << "Error!" << std::endl;
      continue;
    }
    OperatingSystem * os = conn.getOS();
    auto loads = os->getLoad();
    for (double load : loads) {
      std::cout << load << " ";
    }
    std::cout << std::endl;
  }

  setlocale(LC_ALL, "en_US.UTF-8");

  initscr();
  timeout(1000);
  char action;
  do {
    int row = 1;
    attron(A_DIM);
    mvaddstr(0, 0, "Hostname");
    mvaddstr(0,40, "| Load");
    mvaddstr(0,47, "| #usr");
    attroff(A_DIM);
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    std::string progress = "|/-\\";
    int prog = 0;

    attron(A_STANDOUT);
    mvaddch(0, 79, progress[prog]);
    attroff(A_STANDOUT);
    for (Connection & conn : connections) {
      mvaddstr(row, 1, conn.getHostname().c_str());
    
      mvaddch(row, 40, '|');
      if (!conn) {
        attron(A_BLINK | COLOR_PAIR(1));
        addstr(" OFFLINE ");
        attroff(A_BLINK | COLOR_PAIR(1));
        continue;
      }
    
      std::array<double,3> load = conn.getOS()->getLoad();
      std::array<double,5> interp;
      interp[0] = load[0];
      interp[1] = (load[0] + load[1])/2;
      interp[2] = load[1];
      interp[3] = (load[1] + load[2])/2;
      interp[4] = load[2];
      cchar_t t;
      t.attr = 0;
      t.chars[1] = L'\0';
      for (double load : interp) {
        int loadScaled = (int)( std::log(load)/std::log(10) * 4 + 6 );
        loadScaled = loadScaled > 7 ? 7 : loadScaled;
        loadScaled = loadScaled < 0 ? 0 : loadScaled;
        t.chars[0] = L'\u2581'+ loadScaled ;
        add_wch( &t );
      }
      
      mvaddstr(row, 47, "| ");
      addstr( std::to_string( conn.getOS()->getNUsers() ).c_str() );      
      
      row++;
    }
    refresh();
    row = 1;
    
    prog++;
    if (prog == 4)
      prog = 0;
    
    action = getch();
    if (action == 'o') {
      offending();
      clear();
    }
  } while (action != 'q');
  
  endwin();
  
  return 0;
}